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
#include <linux/iio/triggered_buffer.h>
#include "lsm9ds1.h"
#include "lsm9ds1_ag.h"
#include "lsm9ds1_ag_buffer.h"




/* enum { */
/*         LSM9DS0, */
/*         LSM9DS1 */
/* }; */


#define LSM9DS1_AG_CHANNEL_TEMP(reg) {                          \
        .type = IIO_TEMP,                                       \
        .address = reg,                                         \
        .indexed = 0,                                           \
        .channel = 0,                                           \
        .scan_index = -1,                                       \
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) |          \
                BIT(IIO_CHAN_INFO_PROCESSED),                   \
        .info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE) |  \
                BIT(IIO_CHAN_INFO_OFFSET) |                     \
                BIT(IIO_CHAN_INFO_SAMP_FREQ),                   \
}

#define LSM9DS1_AG_CHANNEL_XL(reg, axis, index) {             \
        .type = IIO_ACCEL,                                    \
        .address = reg,                                       \
        .modified = 1,                                        \
        .indexed = 1,                                         \
        .channel2 = IIO_MOD_##axis,                           \
        .scan_index = index,                                  \
        .scan_type = {                                        \
                .sign = 's',                                  \
                .realbits = 16,                               \
                .storagebits = 16,                            \
                .endianness = IIO_LE,                         \
        },                                                    \
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),         \
        .info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE), \
}

#define LSM9DS1_AG_CHANNEL_G(reg, axis, index) {              \
        .type = IIO_ANGL_VEL,                                 \
        .address = reg,                                       \
        .modified = 1,                                        \
        .indexed = 1,                                         \
        .channel2 = IIO_MOD_##axis,                           \
        .scan_index = index,                                  \
        .scan_type = {                                        \
                .sign = 's',                                  \
                .realbits = 16,                               \
                .storagebits = 16,                            \
                .endianness = IIO_LE,                         \
        },                                                    \
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),	      \
        .info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE), \
}

static const struct iio_chan_spec lsm9ds1_ag_channels[] = {
        LSM9DS1_AG_CHANNEL_TEMP(LSM9DS1_REG_OUT_TEMP),
	LSM9DS1_AG_CHANNEL_XL(LSM9DS1_REG_OUT_X_XL, X, 0),
	LSM9DS1_AG_CHANNEL_XL(LSM9DS1_REG_OUT_Y_XL, Y, 1),
	LSM9DS1_AG_CHANNEL_XL(LSM9DS1_REG_OUT_Z_XL, Z, 2),
	LSM9DS1_AG_CHANNEL_G(LSM9DS1_REG_OUT_X_G, X, 3),
	LSM9DS1_AG_CHANNEL_G(LSM9DS1_REG_OUT_Y_G, Y, 4),
	LSM9DS1_AG_CHANNEL_G(LSM9DS1_REG_OUT_Z_G, Z, 5),
};


int lsm9ds1_ag_reset(struct iio_dev *indio_dev)
{
	struct lsm9ds1_data *ldata = iio_priv(indio_dev);
        
        return lsm9ds1_set_bit_reg(ldata,
                indio_dev, LSM9DS1_REG_CTRL_REG8,
                LSM9DS1_AG_SW_RESET);
}

int lsm9ds1_ag_enable(struct iio_dev *indio_dev, bool enable)
{
        int ret;

        if (enable) {
                ret = mwrite_reg_mask_8(
                        indio_dev, LSM9DS1_REG_CTRL_REG1_G,
                        LSM9DS1_AG_ODR_G_59_9, LSM9DS1_AG_ODR_G_MASK);
        } else {
                ret = mwrite_reg_mask_8(
                        indio_dev, LSM9DS1_REG_CTRL_REG1_G,
                        LSM9DS1_AG_ODR_G_PD, LSM9DS1_AG_ODR_G_MASK);
                if (ret < 0)
                        return ret;
                ret = mwrite_reg_mask_8(
                        indio_dev, LSM9DS1_REG_CTRL_REG6_XL,
                        LSM9DS1_AG_ODR_XL_PD, LSM9DS1_AG_ODR_XL_MASK);
        }
        
        return ret;
}


static ssize_t
lsm9ds1_ag_show_accel_max_g(struct device *dev,
                            struct device_attribute *attr,
                            char *buf)
{
        struct iio_dev *indio_dev = dev_to_iio_dev(dev);
        u8 data;
        int len = 0, ret;

        ret = mread_reg_8(indio_dev, LSM9DS1_REG_CTRL_REG6_XL, &data);

        if (ret < 0)
                return ret;

        switch (data & LSM9DS1_AG_FS_XL_MASK) {
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
        int ret;
        u8 data;

        ret = kstrtou8(buf, 10, &data);
        if (ret < 0)
                return ret;

        switch (data) {
        case 2:
                ret = mwrite_reg_mask_8(
                        indio_dev, LSM9DS1_REG_CTRL_REG6_XL,
                        LSM9DS1_AG_FS_XL_2G, LSM9DS1_AG_FS_XL_MASK);
                break;
        case 4:
                ret = mwrite_reg_mask_8(
                        indio_dev, LSM9DS1_REG_CTRL_REG6_XL,
                        LSM9DS1_AG_FS_XL_4G, LSM9DS1_AG_FS_XL_MASK);
                break;
        case 8:
                ret = mwrite_reg_mask_8(
                        indio_dev, LSM9DS1_REG_CTRL_REG6_XL,
                        LSM9DS1_AG_FS_XL_8G, LSM9DS1_AG_FS_XL_MASK);
                break;
        case 16:
                ret = mwrite_reg_mask_8(
                        indio_dev, LSM9DS1_REG_CTRL_REG6_XL,
                        LSM9DS1_AG_FS_XL_16G, LSM9DS1_AG_FS_XL_MASK);
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
	int ret;
        s16 data16;
        u8 data8;

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
                if (chan->type != IIO_TEMP && iio_buffer_enabled(indio_dev))
                        return -EBUSY;
                ret = mread_reg_16(indio_dev, chan->address, &data16);
		if (ret < 0)
			return ret;
		*val = data16;
                *val2 = 0;
		return IIO_VAL_INT;
        case IIO_CHAN_INFO_SCALE:
                switch (chan->type) {
                case IIO_TEMP:
                        *val = 0;
                        *val2 = 1000000 / 16;
                        return IIO_VAL_INT_PLUS_MICRO;
                case IIO_ACCEL:
                        ret = mread_reg_8(indio_dev, LSM9DS1_REG_CTRL_REG6_XL, &data8);
                        if (ret < 0)
                                return ret;

                        *val = 0;
                        switch (data8 & LSM9DS1_AG_FS_XL_MASK) {
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
                        ret = mread_reg_8(indio_dev, LSM9DS1_REG_CTRL_REG1_G, &data8);
                        if (ret < 0)
                                return ret;

                        *val = 0;
                        switch (data8 & LSM9DS1_AG_FS_G_MASK) {
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
                ret = mread_reg_16(indio_dev, chan->address, &data16);
                if (ret < 0)
                        return ret;
                switch (chan->type) {
                case IIO_TEMP:
                        *val = data16 / 16 + 25;
                        *val2 = abs(1000000 * data16 / 16 % 1000000);
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

int lsm9ds1_ag_probe(struct iio_dev *indio_dev, struct device *dev)
{
        int ret;

        indio_dev->dev.parent = dev;
	indio_dev->info = &lsm9ds1_ag_info;
        indio_dev->name = "lsm9ds1_ag";
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->channels = lsm9ds1_ag_channels;
        indio_dev->num_channels = ARRAY_SIZE(lsm9ds1_ag_channels);

        ret = lsm9ds1_ag_reset(indio_dev);
	if (ret < 0)
                return ret;

	ret = lsm9ds1_ag_enable(indio_dev, true);
	if (ret < 0)
		return ret;

        /*  ret = iio_triggered_buffer_setup(indio_dev, NULL, */
        /*                                  lsm9ds1_ag_trigger_handler, */
        /*                                  &lsm9ds1_ag_buffer_setup_ops); */
	/* if (ret < 0) { */
        /*         printk(KERN_ALERT "%s:%d: lsm9ds1_ag_configure_buffer = %i\n",__FUNCTION__,__LINE__, ret); */
        /*         goto error_buffer_cleanup; */
        /* } */
        /* printk(KERN_ALERT "%s:%d: %i\n",__FUNCTION__,__LINE__, ret); */

	ret = iio_device_register(indio_dev);
	if (ret < 0)
                return ret;

error_buffer_cleanup:
        printk(KERN_ALERT "%s:%d: error_unconfigure_buffer\n",__FUNCTION__,__LINE__);
        /* iio_triggered_buffer_cleanup(indio_dev); */
        lsm9ds1_ag_reset(indio_dev);
        dev_err(dev, "device_register failed\n");

	return ret;
}
EXPORT_SYMBOL(lsm9ds1_ag_probe);

int lsm9ds1_ag_remove(struct iio_dev *indio_dev)
{
	iio_device_unregister(indio_dev);

	return lsm9ds1_ag_reset(indio_dev);
}
EXPORT_SYMBOL(lsm9ds1_ag_remove);


MODULE_AUTHOR("Jérôme Guéry <jerome.guery@gmail.com>");
MODULE_DESCRIPTION("accelerometer and gyroscope part of the IMU LSM9DS1");
MODULE_LICENSE("GPL");
