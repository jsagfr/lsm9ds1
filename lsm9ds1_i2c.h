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


#ifndef _LSM9DS1_I2C_H
#define _LSM9DS1_I2C_H

#include <linux/iio/iio.h>

int lsm9ds1_i2c_read_reg(struct iio_dev *indio_dev, u8 addr, u8 len, u8 *data);
int lsm9ds1_i2c_read_reg_8(struct iio_dev *indio_dev, u8 addr, u8 *data);
int lsm9ds1_i2c_read_reg_16(struct iio_dev *indio_dev, u8 addr, s16 *data);
int lsm9ds1_i2c_write_reg_8(struct iio_dev *indio_dev, u8 addr, u8 data);
int lsm9ds1_i2c_write_reg_mask_8(struct iio_dev *indio_dev, u8 addr, u8 data, u8 mask);

#endif /* _LSM9DS1_I2C_H */
