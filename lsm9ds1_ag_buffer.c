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

#include <linux/iio/iio.h>
#include <linux/iio/buffer.h>
#include <linux/iio/triggered_buffer.h>
#include <linux/iio/trigger_consumer.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include "lsm9ds1.h"
#include "lsm9ds1_ag_buffer.h"

/* CTRL_REG9 */
#define LSM9DS1_AG_FIFO_EN 0b00000010

#define LSM9DS1_AG_FMOD_BYPASS         0
#define LSM9DS1_AG_FMOD_FIFO           1 << 5
#define LSM9DS1_AG_FMOD_RESERVED       2 << 5
#define LSM9DS1_AG_FMOD_CONT_TG_FIFO   3 << 5
#define LSM9DS1_AG_FMOD_BYPASS_TG_CONT 4 << 5
#define LSM9DS1_AG_FMOD_CONTINOUS      5 << 5
#define LSM9DS1_AG_FMOD_MASK           0b11100000

/* FIFO_SRC */
#define LSM9DS1_AG_FSS 0b00111111

irqreturn_t lsm9ds1_ag_trigger_handler(int irq, void *p)
{
        struct iio_poll_func *pf = p;
        struct iio_dev *indio_dev = pf->indio_dev;
        struct lsm9ds1_data *ldata = iio_priv(indio_dev);
        int scan_index, i = 0, j = 0;
        int ret;
        u8 len, samples;
        s16 *buffer_data;
        s16 *i2c_data;

        printk(KERN_ALERT "%s:%d\n",__FUNCTION__,__LINE__);
        // TODO: skip all if there is no active scan index

        mutex_lock(&indio_dev->mlock);
        ret = ldata->read_reg_8(indio_dev, LSM9DS1_REG_FIFO_SRC, &samples);
        samples = samples & LSM9DS1_AG_FSS;
        if (ret < 0 || samples <= 0)
                goto done;

        len = samples * 6 * 2;

        i2c_data = kmalloc(len, GFP_KERNEL);
        buffer_data = kmalloc(2 * samples * indio_dev->masklength, GFP_KERNEL);

        if (!i2c_data || !buffer_data)
                goto freebuf;

        ret = ldata->read_reg(indio_dev, LSM9DS1_REG_OUT_X_XL, len, i2c_data);
        if (ret != len)
                goto freebuf;


        for(i = 0; i < samples; i++) {
                for_each_set_bit(scan_index, indio_dev->active_scan_mask,
                                 indio_dev->masklength) {
                        buffer_data[i * samples + j] = i2c_data[scan_index * samples + j];
                        j++;
                }
        }

        iio_push_to_buffers_with_timestamp(indio_dev, buffer_data,
                                           iio_get_time_ns(indio_dev));


freebuf:
        kfree(buffer_data);
        kfree(i2c_data);

done:
        iio_trigger_notify_done(indio_dev->trig);
        mutex_unlock(&indio_dev->mlock);
 
        printk(KERN_ALERT "%s:%d\n",__FUNCTION__,__LINE__);
        return IRQ_HANDLED;
}
EXPORT_SYMBOL(lsm9ds1_ag_trigger_handler);

static int lsm9ds1_ag_buffer_preenable(struct iio_dev *indio_dev)
{
        struct lsm9ds1_data *ldata = iio_priv(indio_dev);
        int ret;

        printk(KERN_ALERT "%s:%d\n",__FUNCTION__,__LINE__);
        ret = lsm9ds1_set_bit_reg(
                ldata, indio_dev, LSM9DS1_REG_CTRL_REG9,
                LSM9DS1_AG_FIFO_EN);

        printk(KERN_ALERT "%s:%d: ret = %i\n",__FUNCTION__,__LINE__, ret);
        return ret;
}

static int lsm9ds1_ag_buffer_postdisable(struct iio_dev *indio_dev)
{
        struct lsm9ds1_data *ldata = iio_priv(indio_dev);
        int ret;

        printk(KERN_ALERT "%s:%d\n",__FUNCTION__,__LINE__);
        ret = lsm9ds1_reset_bit_reg(
                ldata, indio_dev, LSM9DS1_REG_CTRL_REG9,
                LSM9DS1_AG_FIFO_EN);
        printk(KERN_ALERT "%s:%d: ret = %i\n",__FUNCTION__,__LINE__, ret);
        return ret;
}

const struct iio_buffer_setup_ops lsm9ds1_ag_buffer_setup_ops = {
        .preenable = &lsm9ds1_ag_buffer_preenable,
        .postenable = &iio_triggered_buffer_postenable,
        .predisable = &iio_triggered_buffer_predisable,
        .postdisable = &lsm9ds1_ag_buffer_postdisable,
};


/* static int lsm9ds1_ag_probe(struct i2c_client *client, */
/*                             const struct i2c_device_id *id) */
/* { */
/* 	int ret; */
/* 	struct iio_dev *indio_dev; */
/* 	struct lsm9ds1_ag_data *data; */

/* 	ret = i2c_smbus_read_byte_data(client, LSM9DS1_REG_WHO_AM_I); */
/* 	if (ret != LSM9DS1_AG_I_AM) */
/* 		return (ret < 0) ? ret : -ENODEV; */
        
/* 	indio_dev = devm_iio_device_alloc(&client->dev, sizeof(*data)); */
/* 	if (!indio_dev) */
/* 		return -ENOMEM; */

/* 	data = iio_priv(indio_dev); */
/* 	data->client = client; */
/* 	i2c_set_clientdata(client, indio_dev); */

/* 	indio_dev->dev.parent = &client->dev; */
/* 	indio_dev->info = &lsm9ds1_ag_info; */
/*         indio_dev->name = "lsm9ds1_ag"; */
/* 	indio_dev->modes = INDIO_DIRECT_MODE; */
/* 	indio_dev->channels = lsm9ds1_ag_channels; */
/*         indio_dev->num_channels = ARRAY_SIZE(lsm9ds1_ag_channels); */

/*         ret = lsm9ds1_ag_reset(indio_dev); */
/* 	if (ret < 0) */
/*                 return ret; */

/* 	ret = lsm9ds1_ag_enable(indio_dev, true); */
/* 	if (ret < 0) */
/* 		return ret; */

/*         printk(KERN_ALERT "%s:%d: call lsm9ds1_ag_configure_buffer\n",__FUNCTION__,__LINE__); */
/*         ret = iio_triggered_buffer_setup(indio_dev, NULL, */
/*                                          lsm9ds1_ag_trigger_handler, */
/*                                          lsm9ds1_ag_buffer_setup_ops); */
/*         /\* ret = lsm9ds1_ag_configure_buffer(indio_dev); *\/ */
/* 	if (ret < 0) { */
/*                 printk(KERN_ALERT "%s:%d: lsm9ds1_ag_configure_buffer = %i\n",__FUNCTION__,__LINE__, ret); */
/*                 goto error_buffer_cleanup; */
/*         } */
/*         printk(KERN_ALERT "%s:%d: %i\n",__FUNCTION__,__LINE__, ret); */

/* 	ret = iio_device_register(indio_dev); */
/* 	if (ret < 0) */
/*                 goto error_buffer_cleanup; */

/*         return 0; */
        
/* error_buffer_cleanup:                      */
/*         printk(KERN_ALERT "%s:%d: error_unconfigure_buffer\n",__FUNCTION__,__LINE__); */
/*         iio_triggered_buffer_cleanup(indio_dev); */
/*         lsm9ds1_ag_enable(indio_dev, false); */
/*         dev_err(&client->dev, "device_register failed\n"); */

/* 	return ret; */
/* } */
