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
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>
#include <linux/iio/buffer.h>
#include <linux/iio/triggered_buffer.h>
#include <linux/iio/trigger_consumer.h>
#include "lsm9ds1.h"
#include "lsm9ds1_m.h"


/* enum { */
/*         LSM9DS0, */
/*         LSM9DS1 */
/* }; */

#define LSM9DS1_M_CHANNEL_M(reg, axis, index) {               \
        .type = IIO_MAGN,                                     \
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

static const struct iio_chan_spec lsm9ds1_m_channels[] = {
	LSM9DS1_M_CHANNEL_M(LSM9DS1_REG_OUT_X_M, X, 0),
	LSM9DS1_M_CHANNEL_M(LSM9DS1_REG_OUT_Y_M, Y, 1),
	LSM9DS1_M_CHANNEL_M(LSM9DS1_REG_OUT_Z_M, Z, 2),
        IIO_CHAN_SOFT_TIMESTAMP(4),
};

int lsm9ds1_m_reset(struct iio_dev *indio_dev)
{
        return lsm9ds1_mset_bit_reg(indio_dev, LSM9DS1_REG_CTRL_REG2_M,
                LSM9DS1_M_SW_RESET);
}

int lsm9ds1_m_enable(struct iio_dev *indio_dev, bool enable)
{
        int ret;
        
        if (enable) {
                ret = mwrite_reg_mask_8(
                        indio_dev, LSM9DS1_REG_CTRL_REG3_M,
                        LSM9DS1_M_MD_POWER_DOWN, LSM9DS1_M_MD_MASK);
        } else {
                ret = mwrite_reg_mask_8(
                        indio_dev, LSM9DS1_REG_CTRL_REG3_M,
                        LSM9DS1_M_MD_CONT_CONV, LSM9DS1_M_MD_MASK);
        }

        return ret;
}


static ssize_t
lsm9ds1_m_show_magn_max_gauss(struct device *dev,
                              struct device_attribute *attr,
                              char *buf)
{
        struct iio_dev *indio_dev = dev_to_iio_dev(dev);
        int len = 0, ret;
        u8 data;

        ret = mread_reg_8(indio_dev, LSM9DS1_REG_CTRL_REG6_XL, &data);
        if (ret < 0)
                return ret;

        switch ((u8)ret & LSM9DS1_M_FS_MASK) {
        case LSM9DS1_M_FS_4GAUSS:
                len += sprintf(buf + len, "4\n");
                break;
        case LSM9DS1_M_FS_8GAUSS:
                len += sprintf(buf + len, "8\n");
                break;
        case LSM9DS1_M_FS_12GAUSS:
                len += sprintf(buf + len, "12\n");
                break;
        case LSM9DS1_M_FS_16GAUSS:
                len += sprintf(buf + len, "16\n");
                break;
        default:
                return -ENODEV;
        }
        return len;
}

static ssize_t
lsm9ds1_m_store_magn_max_gauss(struct device *dev,
                               struct device_attribute *attr,
                               const char *buf,
                               size_t len)
{
        struct iio_dev *indio_dev = dev_to_iio_dev(dev);
        int ret;
        u8 val;

        ret = kstrtou8(buf, 10, &val);
        if (ret < 0)
                return ret;

        mutex_lock(&indio_dev->mlock);
        switch (val) {
        case 4:
                ret = mwrite_reg_mask_8(
                        indio_dev, LSM9DS1_REG_CTRL_REG2_M,
                        LSM9DS1_M_FS_4GAUSS, LSM9DS1_M_FS_MASK);
                break;
        case 8:
                ret = mwrite_reg_mask_8(
                        indio_dev, LSM9DS1_REG_CTRL_REG2_M,
                        LSM9DS1_M_FS_8GAUSS, LSM9DS1_M_FS_MASK);
                break;
        case 12:
                ret = mwrite_reg_mask_8(
                        indio_dev, LSM9DS1_REG_CTRL_REG2_M,
                        LSM9DS1_M_FS_12GAUSS, LSM9DS1_M_FS_MASK);
                break;
        case 16:
                ret = mwrite_reg_mask_8(
                        indio_dev, LSM9DS1_REG_CTRL_REG2_M,
                        LSM9DS1_M_FS_16GAUSS, LSM9DS1_M_FS_MASK);
                break;
        default:
                return -EINVAL;
        }
        mutex_unlock(&indio_dev->mlock);
        return (ret ? ret : len);
}

static IIO_CONST_ATTR(in_magn_max_gauss_available, "4 8 12 16");

static IIO_DEVICE_ATTR(magn_max_gauss, S_IRUGO | S_IWUSR,
                       lsm9ds1_m_show_magn_max_gauss,
                       lsm9ds1_m_store_magn_max_gauss,
                       0);


static struct attribute *lsm9ds1_m_attributes[] = {
        &iio_const_attr_in_magn_max_gauss_available.dev_attr.attr,
        &iio_dev_attr_magn_max_gauss.dev_attr.attr,
        NULL,
};

static const struct attribute_group lsm9ds1_m_attribute_group = {
        .attrs = lsm9ds1_m_attributes,
};

static int lsm9ds1_m_read_raw(struct iio_dev *indio_dev,
                              struct iio_chan_spec const *chan,
                              int *val, int *val2, long mask)
{
	int ret;
        s16 data16;

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
                ret = mread_reg_16(indio_dev, chan->address, &data16);
		if (ret < 0)
			return ret;
		*val = data16;
                *val2 = 0;
		return IIO_VAL_INT;
	default:
		return -EINVAL;
	}
}

irqreturn_t lsm9ds1_m_trigger_handler(int irq, void *p)
{
        struct iio_poll_func *pf = p;
        struct iio_dev *indio_dev = pf->indio_dev;
        struct lsm9ds1_data *ldata = iio_priv(indio_dev);
        int ret;
        u8 iio_buffer[ALIGN(LSM9DS1_M_DATA_SIZE, sizeof(s64)) + sizeof(s64)]; // 3 channels 16bits + ts(64bits) + padding

        printk(KERN_WARNING "%s:%d\n",__FUNCTION__,__LINE__);

        mutex_lock(&ldata->lock);

        ret = ldata->read_reg(indio_dev, LSM9DS1_REG_OUT_X_M, 6, (s16 *) &iio_buffer);
        if (ret < 0)
                goto done;

        iio_push_to_buffers_with_timestamp(indio_dev, &iio_buffer, iio_get_time_ns(indio_dev));

done:
        iio_trigger_notify_done(indio_dev->trig);
        mutex_unlock(&ldata->lock);
 
        printk(KERN_WARNING "%s:%d\n",__FUNCTION__,__LINE__);
        return IRQ_HANDLED;
}

const struct iio_buffer_setup_ops lsm9ds1_m_buffer_setup_ops = {
        .postenable = &iio_triggered_buffer_postenable,
        .predisable = &iio_triggered_buffer_predisable,
};


static const struct iio_info lsm9ds1_m_info = {
	.driver_module	= THIS_MODULE,
        .attrs          = &lsm9ds1_m_attribute_group,
	.read_raw	= lsm9ds1_m_read_raw,
};

int lsm9ds1_m_probe(struct iio_dev *indio_dev, struct device *dev)
{
        int ret;

        indio_dev->dev.parent = dev;
	indio_dev->info = &lsm9ds1_m_info;
        indio_dev->name = "lsm9ds1_m";
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->channels = lsm9ds1_m_channels;
        indio_dev->num_channels = ARRAY_SIZE(lsm9ds1_m_channels);

        ret = lsm9ds1_m_reset(indio_dev);
	if (ret < 0)
                return ret;

	ret = lsm9ds1_m_enable(indio_dev, true);
	if (ret < 0)
		return ret;

        ret = iio_triggered_buffer_setup(indio_dev, NULL,
                                         &lsm9ds1_m_trigger_handler,
                                         &lsm9ds1_m_buffer_setup_ops);
	if (ret < 0) {
                printk(KERN_WARNING "%s:%d: lsm9ds1_m_configure_buffer = %i\n",__FUNCTION__,__LINE__, ret);
                goto error_buffer_cleanup;
        }
	indio_dev->modes |= INDIO_BUFFER_TRIGGERED;
        printk(KERN_WARNING "%s:%d: %i\n",__FUNCTION__,__LINE__, ret);

	ret = iio_device_register(indio_dev);
	if (ret >= 0)
                return ret;

error_buffer_cleanup:
        printk(KERN_WARNING "%s:%d: error_unconfigure_buffer\n",__FUNCTION__,__LINE__);
        /* iio_triggered_buffer_cleanup(indio_dev); */
        lsm9ds1_m_reset(indio_dev);
        dev_err(dev, "device_register failed\n");

	return ret;
}
EXPORT_SYMBOL(lsm9ds1_m_probe);

int lsm9ds1_m_remove(struct iio_dev *indio_dev)
{
	iio_device_unregister(indio_dev);

	return lsm9ds1_m_enable(indio_dev, false);
}
EXPORT_SYMBOL(lsm9ds1_m_remove);

MODULE_AUTHOR("Jérôme Guéry <jerome.guery@gmail.com>");
MODULE_DESCRIPTION("magnetometer part of the IMU LSM9DS1");
MODULE_LICENSE("GPL");
