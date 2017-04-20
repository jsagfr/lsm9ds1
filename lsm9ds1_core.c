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
#include "lsm9ds1.h"

int mread_reg_8(struct iio_dev *indio_dev, u8 addr, u8 *data)
{
        struct lsm9ds1_data *ldata = iio_priv(indio_dev);
        s32 ret;

        mutex_lock(&indio_dev->mlock);
        ret = ldata->read_reg_8(indio_dev, addr, data);
        mutex_unlock(&indio_dev->mlock);

        return ret;
}
EXPORT_SYMBOL(mread_reg_8);


int mread_reg_16(struct iio_dev *indio_dev, u8 addr, s16 *data)
{
        struct lsm9ds1_data *ldata = iio_priv(indio_dev);
        s32 ret;

        mutex_lock(&indio_dev->mlock);
        ret = ldata->read_reg_16(indio_dev, addr, data);
        mutex_unlock(&indio_dev->mlock);

        return ret;
}
EXPORT_SYMBOL(mread_reg_16);


int mwrite_reg_8(struct iio_dev *indio_dev, u8 addr, u8 data)
{
        struct lsm9ds1_data *ldata = iio_priv(indio_dev);
        s32 ret;

        mutex_lock(&indio_dev->mlock);
        ret = ldata->write_reg_8(indio_dev, addr, data);
        mutex_unlock(&indio_dev->mlock);

        return ret;
}
EXPORT_SYMBOL(mwrite_reg_8);


int mwrite_reg_mask_8(struct iio_dev *indio_dev, u8 addr, u8 data, u8 mask)
{
        struct lsm9ds1_data *ldata = iio_priv(indio_dev);
        s32 ret;

        mutex_lock(&indio_dev->mlock);
        ret = ldata->write_reg_mask_8(indio_dev, addr, data, mask);
        mutex_unlock(&indio_dev->mlock);

        return ret;
}
EXPORT_SYMBOL(mwrite_reg_mask_8);
