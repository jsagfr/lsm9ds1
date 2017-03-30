/******************************************************************************
 *
 *   Copyright Jérôme Guéry. All rights reserved.
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *****************************************************************************/


#include <linux/device.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>
#include "lsm9ds1.h"

/* bits */
#define LSM9DS1_M_I_AM       0b00111101

#define LSM9DS1_M_MD_MASK        3
#define LSM9DS1_M_MD_CONT_CONV   0
#define LSM9DS1_M_MD_SINGLE_CONV 1
#define LSM9DS1_M_MD_POWER_DOWN2 2
#define LSM9DS1_M_MD_POWER_DOWN  3

#define LSM9DS1_M_ODR_MASK  7 << 2
#define LSM9DS1_M_ODR_0_625 0
#define LSM9DS1_M_ODR_1_25  1 << 2
#define LSM9DS1_M_ODR_2_5   2 << 2
#define LSM9DS1_M_ODR_5     3 << 2
#define LSM9DS1_M_ODR_10    4 << 2
#define LSM9DS1_M_ODR_20    5 << 2
#define LSM9DS1_M_ODR_40    6 << 2
#define LSM9DS1_M_ODR_80    7 << 2

#define LSM9DS1_M_FS_4GAUSS   0
#define LSM9DS1_M_FS_8GAUSS   1 << 3
#define LSM9DS1_M_FS_12GAUSS  2 << 3
#define LSM9DS1_M_FS_16GAUSS  3 << 3
#define LSM9DS1_M_FS_XL_MASK  0b01100000

/* #define LSM9DS1_M_FS_XL_SCALE_2G  IIO_G_TO_M_S_2( 2000000000ULL / S16_MAX) */
/* #define LSM9DS1_M_FS_XL_SCALE_4G  IIO_G_TO_M_S_2( 4000000000ULL / S16_MAX) */
/* #define LSM9DS1_M_FS_XL_SCALE_8G  IIO_G_TO_M_S_2( 8000000000ULL / S16_MAX) */
/* #define LSM9DS1_M_FS_XL_SCALE_16G IIO_G_TO_M_S_2(16000000000ULL / S16_MAX) */

#define LSM9DS1_M_SW_RESET 1 << 2



/* enum { */
/*         LSM9DS0, */
/*         LSM9DS1 */
/* }; */

struct lsm9ds1_m_data {
        struct i2c_client *client;
};

#define LSM9DS1_M_CHANNEL_M(reg, axis) {                                \
                .type = IIO_ACCEL,                                      \
                        .address = reg,                                 \
                        .modified = 1,                                  \
                        .channel2 = IIO_MOD_##axis,                     \
                        .scan_type = {                                  \
                        .sign = 's',                                    \
                        .realbits = 16,                                 \
                        .storagebits = 16,                              \
                        .endianness = IIO_LE,                           \
                },                                                      \
                        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),   \
                        }

static const struct iio_chan_spec lsm9ds1_m_channels[] = {
	LSM9DS1_M_CHANNEL_M(LSM9DS1_REG_OUT_X_M, X),
	LSM9DS1_M_CHANNEL_M(LSM9DS1_REG_OUT_Y_M, Y),
	LSM9DS1_M_CHANNEL_M(LSM9DS1_REG_OUT_Z_M, Z),
};

int lsm9ds1_m_reset(struct i2c_client *client)
{
        return lsm9ds1_register_set_bit(client, LSM9DS1_REG_CTRL_REG2_M,
                                        LSM9DS1_M_SW_RESET);
}

int lsm9ds1_m_enable(struct i2c_client *client, bool enable)
{
        if (enable)
                return lsm9ds1_register_mask_write(
                        client, LSM9DS1_REG_CTRL_REG3_M,
                        LSM9DS1_M_MD_MASK, LSM9DS1_M_MD_POWER_DOWN);
        
	return lsm9ds1_register_mask_write(
                client, LSM9DS1_REG_CTRL_REG3_M,
                LSM9DS1_M_MD_MASK, LSM9DS1_M_MD_CONT_CONV);
}


static int lsm9ds1_m_read_raw(struct iio_dev *indio_dev,
                              struct iio_chan_spec const *chan,
                              int *val, int *val2, long mask)
{
	struct lsm9ds1_m_data *data = iio_priv(indio_dev);
	int ret;

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		ret = i2c_smbus_read_word_data(data->client, chan->address);
		if (ret < 0)
			return ret;
		*val = (s16) ret;
                *val2 = 0;
		return IIO_VAL_INT;
	default:
		return -EINVAL;
	}
}


static const struct iio_info lsm9ds1_m_info = {
	.driver_module	= THIS_MODULE,
	.read_raw	= lsm9ds1_m_read_raw,
};

static int lsm9ds1_m_probe(struct i2c_client *client,
                            const struct i2c_device_id *id)
{
	int ret;
	struct iio_dev *indio_dev;
	struct lsm9ds1_m_data *data;

	ret = i2c_smbus_read_byte_data(client, LSM9DS1_REG_WHO_AM_I_M);
	if (ret != LSM9DS1_M_I_AM)
		return (ret < 0) ? ret : -ENODEV;
        
	indio_dev = devm_iio_device_alloc(&client->dev, sizeof(*data));
	if (!indio_dev)
		return -ENOMEM;

	data = iio_priv(indio_dev);
	data->client = client;
	i2c_set_clientdata(client, indio_dev);

	indio_dev->dev.parent = &client->dev;
	indio_dev->info = &lsm9ds1_m_info;
        indio_dev->name = "lsm9ds1_m";
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->channels = lsm9ds1_m_channels;
        indio_dev->num_channels = ARRAY_SIZE(lsm9ds1_m_channels);

        ret = lsm9ds1_m_reset(client);
	if (ret < 0)
                return ret;

	ret = lsm9ds1_m_enable(client, true);
	if (ret < 0)
		return ret;

	ret = iio_device_register(indio_dev);
	if (ret < 0) {
		dev_err(&client->dev, "device_register failed\n");
		lsm9ds1_m_enable(client, false);
	}

	return ret;
}

static int lsm9ds1_m_remove(struct i2c_client *client)
{
	struct iio_dev *indio_dev = i2c_get_clientdata(client);

	iio_device_unregister(indio_dev);

	return lsm9ds1_m_enable(client, false);
}

static struct i2c_device_id lsm9ds1_m_i2c_ids[] = {
        { "lsm9ds1_m", 0 },
        { }
};

MODULE_DEVICE_TABLE(i2c, lsm9ds1_m_i2c_ids);

static struct i2c_driver lsm9ds1_m_driver = {
	.driver = {
		.name = "lsm9ds1_m",
		.pm = NULL,
	},
	.probe		= lsm9ds1_m_probe,
	.remove		= lsm9ds1_m_remove,
	.id_table	= lsm9ds1_m_i2c_ids,
};

module_i2c_driver(lsm9ds1_m_driver);

MODULE_AUTHOR("Jérôme Guéry <jerome.guery@gmail.com>");
MODULE_DESCRIPTION("magnetometer part of the IMU LSM9DS1");
MODULE_LICENSE("GPL");
