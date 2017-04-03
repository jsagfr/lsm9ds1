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
#define LSM9DS1_AG_I_AM         0b01101000

#define LSM9DS1_AG_ODR_G_MASK 7 << 5
#define LSM9DS1_AG_ODR_G_PD   0
#define LSM9DS1_AG_ODR_G_14_9 1 << 5
#define LSM9DS1_AG_ODR_G_59_9 2 << 5
#define LSM9DS1_AG_ODR_G_119  3 << 5
#define LSM9DS1_AG_ODR_G_238  4 << 5
#define LSM9DS1_AG_ODR_G_476  5 << 5
#define LSM9DS1_AG_ODR_G_952  6 << 5

#define LSM9DS1_AG_ODR_XL_MASK 7 << 5
#define LSM9DS1_AG_ODR_XL_PD   0
#define LSM9DS1_AG_ODR_XL_10   1 << 5
#define LSM9DS1_AG_ODR_XL_50   2 << 5
#define LSM9DS1_AG_ODR_XL_119  3 << 5
#define LSM9DS1_AG_ODR_XL_238  4 << 5
#define LSM9DS1_AG_ODR_XL_476  5 << 5
#define LSM9DS1_AG_ODR_XL_952  6 << 5

#define LSM9DS1_AG_FS_XL_2G  0
#define LSM9DS1_AG_FS_XL_16G 1 << 3
#define LSM9DS1_AG_FS_XL_4G  2 << 3
#define LSM9DS1_AG_FS_XL_8G  3 << 3
#define LSM9DS1_AG_FS_XL_MASK 0b00011000

#define LSM9DS1_AG_FS_XL_SCALE_2G  IIO_G_TO_M_S_2( 2000000000ULL / S16_MAX)
#define LSM9DS1_AG_FS_XL_SCALE_4G  IIO_G_TO_M_S_2( 4000000000ULL / S16_MAX)
#define LSM9DS1_AG_FS_XL_SCALE_8G  IIO_G_TO_M_S_2( 8000000000ULL / S16_MAX)
#define LSM9DS1_AG_FS_XL_SCALE_16G IIO_G_TO_M_S_2(16000000000ULL / S16_MAX)

#define LSM9DS1_AG_FS_G_245DPS  0
#define LSM9DS1_AG_FS_G_500DPS  1 << 3
#define LSM9DS1_AG_FS_G_2000DPS 3 << 3
#define LSM9DS1_AG_FS_G_MASK    0b00011000

#define LSM9DS1_AG_FS_G_SCALE_245DPS  IIO_DEGREE_TO_RAD( 245000000 / S16_MAX)
#define LSM9DS1_AG_FS_G_SCALE_500DPS  IIO_DEGREE_TO_RAD( 500000000 / S16_MAX)
#define LSM9DS1_AG_FS_G_SCALE_2000DPS IIO_DEGREE_TO_RAD(2000000000 / S16_MAX)

#define LSM9DS1_AG_SW_RESET 1



/* enum { */
/*         LSM9DS0, */
/*         LSM9DS1 */
/* }; */

struct lsm9ds1_ag_data {
        struct i2c_client *client;
};

#define LSM9DS1_AG_CHANNEL_TEMP(reg) {                                  \
                .type = IIO_TEMP,                                       \
                        .address = reg,                                 \
                        .channel = 0,                                   \
                        .scan_type = {                                  \
                        .sign = 's',                                    \
                        .realbits = 12,                                 \
                        .storagebits = 16,                              \
                        .endianness = IIO_LE,                           \
                },                                                      \
                        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) |  \
                        BIT(IIO_CHAN_INFO_PROCESSED),                   \
                        .info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE) | \
                        BIT(IIO_CHAN_INFO_OFFSET) |                     \
                        BIT(IIO_CHAN_INFO_SAMP_FREQ),                   \
                        }

#define LSM9DS1_AG_CHANNEL_XL(reg, axis) {                              \
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
                        .info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE), \
                        }

#define LSM9DS1_AG_CHANNEL_G(reg, axis) {                               \
                .type = IIO_ANGL_VEL,                                   \
                        .address = reg,                                 \
                        .modified = 1,                                  \
                        .channel2 = IIO_MOD_##axis,                     \
                        .scan_type = {                                  \
                        .sign = 's',                                    \
                        .realbits = 16,                                 \
                        .storagebits = 16,                              \
                        .endianness = IIO_LE,                           \
                },                                                      \
                        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),	\
                        .info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE), \
                        }

static const struct iio_chan_spec lsm9ds1_ag_channels[] = {
        LSM9DS1_AG_CHANNEL_TEMP(LSM9DS1_REG_OUT_TEMP),
	LSM9DS1_AG_CHANNEL_XL(LSM9DS1_REG_OUT_X_XL, X),
	LSM9DS1_AG_CHANNEL_XL(LSM9DS1_REG_OUT_Y_XL, Y),
	LSM9DS1_AG_CHANNEL_XL(LSM9DS1_REG_OUT_Z_XL, Z),
	LSM9DS1_AG_CHANNEL_G(LSM9DS1_REG_OUT_X_G, X),
	LSM9DS1_AG_CHANNEL_G(LSM9DS1_REG_OUT_Y_G, Y),
	LSM9DS1_AG_CHANNEL_G(LSM9DS1_REG_OUT_Z_G, Z),
};


int lsm9ds1_ag_reset(struct i2c_client *client)
{
        return lsm9ds1_register_set_bit(client, LSM9DS1_REG_CTRL_REG8,
                                        LSM9DS1_AG_SW_RESET);
}

int lsm9ds1_ag_enable(struct i2c_client *client, bool enable)
{
        int ret;
        
        if (enable)
                return lsm9ds1_register_mask_write(
                        client, LSM9DS1_REG_CTRL_REG1_G,
                        LSM9DS1_AG_ODR_G_MASK, LSM9DS1_AG_ODR_G_59_9);
        
        ret = lsm9ds1_register_mask_write(
                client, LSM9DS1_REG_CTRL_REG1_G,
                LSM9DS1_AG_ODR_G_MASK, LSM9DS1_AG_ODR_G_PD);
        if (ret < 0)
                return ret;

	return lsm9ds1_register_mask_write(
                client, LSM9DS1_REG_CTRL_REG6_XL,
                LSM9DS1_AG_ODR_XL_MASK, LSM9DS1_AG_ODR_XL_PD);
}


static ssize_t
lsm9ds1_ag_show_accel_max_g(struct device *dev,
                            struct device_attribute *attr,
                            char *buf)
{
        struct iio_dev *indio_dev = dev_to_iio_dev(dev);
        struct lsm9ds1_ag_data *data = iio_priv(indio_dev);
        /* struct lsm9ds1_ag_state *st = iio_priv(indio_dev); */
        // TODO: spinlock
        int len = 0, ret;

        ret = i2c_smbus_read_word_data(data->client, LSM9DS1_REG_CTRL_REG6_XL);
        if (ret < 0)
                return ret;

        switch ((u8)ret & LSM9DS1_AG_FS_XL_MASK) {
        case LSM9DS1_AG_FS_XL_2G:
                len += sprintf(buf + len, "2\n");
                break;
        case LSM9DS1_AG_FS_XL_4G:
                len += sprintf(buf + len, "4\n");
                break;
        case LSM9DS1_AG_FS_XL_8G:
                len += sprintf(buf + len, "8\n");
                break;
        case LSM9DS1_AG_FS_XL_16G:
                len += sprintf(buf + len, "16\n");
                break;
        default:
                return -ENODEV;
        }
        return len;
}

static ssize_t
lsm9ds1_ag_store_accel_max_g(struct device *dev,
                             struct device_attribute *attr,
                             const char *buf,
                             size_t len)
{
        struct iio_dev *indio_dev = dev_to_iio_dev(dev);
        struct lsm9ds1_ag_data *data = iio_priv(indio_dev);
        int ret;
        u8 val;

        ret = kstrtou8(buf, 10, &val);
        if (ret < 0)
                return ret;

        switch (val) {
        case 2:
                ret = lsm9ds1_register_mask_write(
                        data->client, LSM9DS1_REG_CTRL_REG6_XL,
                        LSM9DS1_AG_FS_XL_MASK, LSM9DS1_AG_FS_XL_2G);
                break;
        case 4:
                ret = lsm9ds1_register_mask_write(
                        data->client, LSM9DS1_REG_CTRL_REG6_XL,
                        LSM9DS1_AG_FS_XL_MASK, LSM9DS1_AG_FS_XL_4G);
                break;
        case 8:
                ret = lsm9ds1_register_mask_write(
                        data->client, LSM9DS1_REG_CTRL_REG6_XL,
                        LSM9DS1_AG_FS_XL_MASK, LSM9DS1_AG_FS_XL_8G);
                break;
        case 16:
                ret = lsm9ds1_register_mask_write(
                        data->client, LSM9DS1_REG_CTRL_REG6_XL,
                        LSM9DS1_AG_FS_XL_MASK, LSM9DS1_AG_FS_XL_16G);
                break;
        default:
                return -EINVAL;
        }
        return (ret ? ret : len);
}

static IIO_CONST_ATTR(in_accel_max_g_available, "2 4 8 16");

static IIO_DEVICE_ATTR(accel_max_g, S_IRUGO | S_IWUSR,
                       lsm9ds1_ag_show_accel_max_g,
                       lsm9ds1_ag_store_accel_max_g,
                       0);


static struct attribute *lsm9ds1_ag_attributes[] = {
        &iio_const_attr_in_accel_max_g_available.dev_attr.attr,
        &iio_dev_attr_accel_max_g.dev_attr.attr,
        NULL,
};

static const struct attribute_group lsm9ds1_ag_attribute_group = {
        .attrs = lsm9ds1_ag_attributes,
};

static int lsm9ds1_ag_read_raw(struct iio_dev *indio_dev,
                               struct iio_chan_spec const *chan,
                               int *val, int *val2, long mask)
{
	struct lsm9ds1_ag_data *data = iio_priv(indio_dev);
	int ret;

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		ret = i2c_smbus_read_word_data(data->client, chan->address);
		if (ret < 0)
			return ret;
		*val = (s16) ret;
                *val2 = 0;
		return IIO_VAL_INT;
        case IIO_CHAN_INFO_SCALE:
                switch (chan->type) {
                case IIO_TEMP:
                        *val = 0;
                        *val2 = 1000000 / 16;
                        return IIO_VAL_INT_PLUS_MICRO;
                case IIO_ACCEL:
                        ret = i2c_smbus_read_byte_data(data->client,
                                                       LSM9DS1_REG_CTRL_REG6_XL);
                        if (ret < 0)
                                return ret;

                        *val = 0;
                        switch (ret & LSM9DS1_AG_FS_XL_MASK) {
                        case LSM9DS1_AG_FS_XL_4G:
                                *val2 = LSM9DS1_AG_FS_XL_SCALE_4G;
                                return IIO_VAL_INT_PLUS_NANO;
                        case LSM9DS1_AG_FS_XL_8G:
                                *val2 = LSM9DS1_AG_FS_XL_SCALE_8G;
                                return IIO_VAL_INT_PLUS_NANO;
                        case LSM9DS1_AG_FS_XL_16G:
                                *val2 = LSM9DS1_AG_FS_XL_SCALE_16G;
                                return IIO_VAL_INT_PLUS_NANO;
                        default:
                                *val2 = LSM9DS1_AG_FS_XL_SCALE_2G;
                                return IIO_VAL_INT_PLUS_NANO;
                        }
                case IIO_ANGL_VEL:
                        ret = i2c_smbus_read_byte_data(data->client,
                                                       LSM9DS1_REG_CTRL_REG1_G);
                        if (ret < 0)
                                return ret;

                        *val = 0;
                        switch (ret & LSM9DS1_AG_FS_G_MASK) {
                        case LSM9DS1_AG_FS_G_500DPS:
                                *val2 = LSM9DS1_AG_FS_G_SCALE_500DPS;
                                return IIO_VAL_INT_PLUS_MICRO;
                        case LSM9DS1_AG_FS_G_2000DPS:
                                *val2 = LSM9DS1_AG_FS_G_SCALE_2000DPS;
                                return IIO_VAL_INT_PLUS_MICRO;
                        default:
                                *val2 = LSM9DS1_AG_FS_G_SCALE_245DPS;
                                return IIO_VAL_INT_PLUS_MICRO;
                        }
                default:
                        return -EINVAL;
                }
        case IIO_CHAN_INFO_OFFSET:
                switch (chan->type) {
                case IIO_TEMP:
                        *val = 25 * 16;
                        *val2 = 0;
                        return IIO_VAL_INT;
                default:
                        return -EINVAL;
                }
	case IIO_CHAN_INFO_PROCESSED:
                ret = i2c_smbus_read_word_data(data->client, chan->address);

                if (ret < 0)
                        return ret;
                switch (chan->type) {
                case IIO_TEMP:
                        *val = (s16)ret / 16 + 25;
                        *val2 = abs(1000000 * (s16)ret / 16 % 1000000);
                        return IIO_VAL_INT_PLUS_MICRO;
                default:
                        return -EINVAL;
                }
        case IIO_CHAN_INFO_SAMP_FREQ:
                switch (chan->type) {
                case IIO_TEMP:
                        *val = 59;
                        *val2 = 500000;
                        return IIO_VAL_INT_PLUS_MICRO;
                default:
                        return -EINVAL;
                }
	default:
		return -EINVAL;
	}
}


static const struct iio_info lsm9ds1_ag_info = {
	.driver_module	= THIS_MODULE,
        .attrs          = &lsm9ds1_ag_attribute_group,
	.read_raw	= lsm9ds1_ag_read_raw,
};

static int lsm9ds1_ag_probe(struct i2c_client *client,
                            const struct i2c_device_id *id)
{
	int ret;
	struct iio_dev *indio_dev;
	struct lsm9ds1_ag_data *data;

	ret = i2c_smbus_read_byte_data(client, LSM9DS1_REG_WHO_AM_I);
	if (ret != LSM9DS1_AG_I_AM)
		return (ret < 0) ? ret : -ENODEV;
        
	indio_dev = devm_iio_device_alloc(&client->dev, sizeof(*data));
	if (!indio_dev)
		return -ENOMEM;

	data = iio_priv(indio_dev);
	data->client = client;
	i2c_set_clientdata(client, indio_dev);

	indio_dev->dev.parent = &client->dev;
	indio_dev->info = &lsm9ds1_ag_info;
        indio_dev->name = "lsm9ds1_ag";
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->channels = lsm9ds1_ag_channels;
        indio_dev->num_channels = ARRAY_SIZE(lsm9ds1_ag_channels);

        ret = lsm9ds1_ag_reset(client);
	if (ret < 0)
                return ret;

	ret = lsm9ds1_ag_enable(client, true);
	if (ret < 0)
		return ret;

	ret = iio_device_register(indio_dev);
	if (ret < 0) {
		dev_err(&client->dev, "device_register failed\n");
		lsm9ds1_ag_enable(client, false);
	}

	return ret;
}

static int lsm9ds1_ag_remove(struct i2c_client *client)
{
	struct iio_dev *indio_dev = i2c_get_clientdata(client);

	iio_device_unregister(indio_dev);

	return lsm9ds1_ag_enable(client, false);
}

static struct i2c_device_id lsm9ds1_ag_i2c_ids[] = {
        { "lsm9ds1_ag", 0 },
        { }
};

MODULE_DEVICE_TABLE(i2c, lsm9ds1_ag_i2c_ids);

static struct i2c_driver lsm9ds1_ag_driver = {
	.driver = {
		.name = "lsm9ds1_ag",
		.pm = NULL,
	},
	.probe		= lsm9ds1_ag_probe,
	.remove		= lsm9ds1_ag_remove,
	.id_table	= lsm9ds1_ag_i2c_ids,
};

module_i2c_driver(lsm9ds1_ag_driver);

MODULE_AUTHOR("Jérôme Guéry <jerome.guery@gmail.com>");
MODULE_DESCRIPTION("accelerometer and gyroscope part of the IMU LSM9DS1");
MODULE_LICENSE("GPL");
