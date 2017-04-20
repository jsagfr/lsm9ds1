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

#include <linux/i2c.h>
#include <linux/slab.h>
#include "lsm9ds1.h"

/* static int lsm9ds1_i2c_read_hw_buffer( */
/*         struct iio_dev *indio_dev, */
/*         u8 addr_nbsamples, u8 mask_nbsamples, u8 nb_channel, */
/*         u8 addr_buffer, */
/*         s16 *data) */
/* { */
/*         struct lsm9ds1_data *ldata = iio_priv(indio_dev); */
/*         s32 ret; */
/*         int len = 0; */

/*         mutex_lock(&indio_dev->mlock); */
        
/*         ret = i2c_smbus_read_byte_data(ldata->i2c, addr_nbsamples); */
/*         if (ret < 0) */
/*                 goto done; */

/*         len = (((u8)ret) & addr_buffer) * nb_channel * 2; */
/*         data = kmalloc(len, GFP_KERNEL); */

/*         if (!data) */
/*                 goto done; */

/*         ret = i2c_smbus_read_i2c_block_data( */
/*                 ldata->i2c, nb_channel, addr_buffer, (u8 *)data); */

/* done: */
/*         mutex_unlock(&indio_dev->mlock); */

/*         if (ret < 0) */
/*                 return ret; */
/*         if (ret != len) */
/*                 return -EIO; */
/*         return len / 2; */
/* } */
/* EXPORT_SYMBOL(lsm9ds1_i2c_read_hw_buffer); */

static int lsm9ds1_i2c_read_reg(struct iio_dev *indio_dev, u8 len, u8 addr, s16 *data)
{
        struct lsm9ds1_data *ldata = iio_priv(indio_dev);
        s32 ret;

        ret = i2c_smbus_read_i2c_block_data(
                ldata->i2c, 2 * len, addr, (u8 *)data);

        return (ret != 2 * len) ? -EIO : 0;
}
EXPORT_SYMBOL(lsm9ds1_i2c_read_reg);


static int lsm9ds1_i2c_read_reg_8(struct iio_dev *indio_dev, u8 addr, u8 *data)
{
        struct lsm9ds1_data *ldata = iio_priv(indio_dev);
        s32 ret;

        ret = i2c_smbus_read_byte_data(ldata->i2c, addr);

        if (ret < 0)
                return ret;

        *data = (u8)ret;
        return 0;
}
EXPORT_SYMBOL(lsm9ds1_i2c_read_reg_8);

static int lsm9ds1_i2c_read_reg_16(struct iio_dev *indio_dev, u8 addr, s16 *data)
{
        struct lsm9ds1_data *ldata = iio_priv(indio_dev);
        s32 ret;

        ret = i2c_smbus_read_word_data(ldata->i2c, addr);

        if (ret < 0)
                return ret;

        *data = (s16)ret;
        return 0;
}
EXPORT_SYMBOL(lsm9ds1_i2c_read_reg_16);


static int lsm9ds1_i2c_write_reg_8(struct iio_dev *indio_dev, u8 addr, u8 data)
{
        struct lsm9ds1_data *ldata = iio_priv(indio_dev);
        s32 ret;

        ret = i2c_smbus_write_byte_data(ldata->i2c, addr, data);

        return (ret < 0) ? ret : 0;
}
EXPORT_SYMBOL(lsm9ds1_i2c_write_reg_8);

static int lsm9ds1_i2c_write_reg_mask_8(struct iio_dev *indio_dev, u8 addr, u8 data, u8 mask)
{
        struct lsm9ds1_data *ldata = iio_priv(indio_dev);
        s32 ret;

        ret = i2c_smbus_read_byte_data(ldata->i2c, addr);
        if (ret < 0)
                return ret;

        ret = i2c_smbus_write_byte_data(ldata->i2c, addr, (((u8)ret) & ~mask) | (data & mask));

        return (ret < 0) ? ret : 0;
}
EXPORT_SYMBOL(lsm9ds1_i2c_write_reg_mask_8);
