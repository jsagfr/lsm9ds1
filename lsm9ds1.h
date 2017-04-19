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

#ifndef LSM9DS1_H_
#define LSM9DS1_H_

#include <linux/i2c.h>
#include <linux/iio/iio.h>

/* registers */
#define LSM9DS1_REG_ACT_THS          0x04
#define LSM9DS1_REG_ACT_DUR          0x05
#define LSM9DS1_REG_INT_GEN_CFG_XL   0x06
#define LSM9DS1_REG_INT_GEN_THS_X_XL 0x07
#define LSM9DS1_REG_INT_GEN_THS_Y_XL 0x08
#define LSM9DS1_REG_INT_GEN_THS_Z_XL 0x09
#define LSM9DS1_REG_INT_GEN_DUR_XL   0x0A
#define LSM9DS1_REG_REFERENCE_G      0x0B
#define LSM9DS1_REG_INT1_CTRL        0x0C
#define LSM9DS1_REG_INT2_CTRL        0x0D
#define LSM9DS1_REG_WHO_AM_I         0x0F
#define LSM9DS1_REG_CTRL_REG1_G      0x10
#define LSM9DS1_REG_CTRL_REG2_G      0x11
#define LSM9DS1_REG_CTRL_REG3_G      0x12
#define LSM9DS1_REG_ORIENT_CFG_G     0x13
#define LSM9DS1_REG_INT_GEN_SRC_G    0x14
#define LSM9DS1_REG_OUT_TEMP         0x15
#define LSM9DS1_REG_STATUS_REG       0x17
#define LSM9DS1_REG_OUT_X_G          0x18
#define LSM9DS1_REG_OUT_Y_G          0x1A
#define LSM9DS1_REG_OUT_Z_G          0x1C
#define LSM9DS1_REG_CTRL_REG4        0x1E
#define LSM9DS1_REG_CTRL_REG5_XL     0x1F
#define LSM9DS1_REG_CTRL_REG6_XL     0x20
#define LSM9DS1_REG_CTRL_REG7_XL     0x21
#define LSM9DS1_REG_CTRL_REG8        0x22
#define LSM9DS1_REG_CTRL_REG9        0x23
#define LSM9DS1_REG_CTRL_REG10       0x24
#define LSM9DS1_REG_INT_GEN_SRC_XL   0x26
#define LSM9DS1_REG_STATUS_REG_BIS   0x27
#define LSM9DS1_REG_OUT_X_XL         0x28
#define LSM9DS1_REG_OUT_Y_XL         0x2A
#define LSM9DS1_REG_OUT_Z_XL         0x2C
#define LSM9DS1_REG_FIFO_CTRL        0x2E
#define LSM9DS1_REG_FIFO_SRC         0x2F
#define LSM9DS1_REG_INT_GEN_CFG_G    0x30
#define LSM9DS1_REG_INT_GEN_THS_X_G  0x31
#define LSM9DS1_REG_INT_GEN_THS_Y_G  0x33
#define LSM9DS1_REG_INT_GEN_THS_Z_G  0x35
#define LSM9DS1_REG_INT_GEN_DUR_G    0x37
#define LSM9DS1_REG_OFFSET_X_REG_M   0x05
#define LSM9DS1_REG_OFFSET_Y_REG_M   0x07
#define LSM9DS1_REG_OFFSET_Z_REG_M   0x09
#define LSM9DS1_REG_WHO_AM_I_M       0x0F
#define LSM9DS1_REG_CTRL_REG1_M      0x20
#define LSM9DS1_REG_CTRL_REG2_M      0x21
#define LSM9DS1_REG_CTRL_REG3_M      0x22
#define LSM9DS1_REG_CTRL_REG4_M      0x23
#define LSM9DS1_REG_CTRL_REG5_M      0x24
#define LSM9DS1_REG_STATUS_REG_M     0x27
#define LSM9DS1_REG_OUT_X_M          0x28
#define LSM9DS1_REG_OUT_Y_M          0x2A
#define LSM9DS1_REG_OUT_Z_M          0x2C
#define LSM9DS1_REG_INT_CFG_M        0x30
#define LSM9DS1_REG_INT_SRC_M        0x31
#define LSM9DS1_REG_INT_THS_M        0x32

struct lsm9ds1_data {
        struct spi_device *spi;
        struct i2c_client *i2c;
        int               (*read_reg)(struct iio_dev *indio_dev, u8 addr, u8 len, u8 *data);
        int               (*read_reg_8)(struct iio_dev *indio_dev, u8 addr, u8 *data);
        int               (*read_reg_16)(struct iio_dev *indio_dev, u8 addr, s16 *data);
        int               (*write_reg_8)(struct iio_dev *indio_dev, u8 addr, u8 data);
        int               (*write_reg_mask_8)(struct iio_dev *indio_dev, u8 addr, u8 data, u8 mask);
};

#define lsm9ds1_set_bit_reg(ldata, indio_dev, addr, bit)        \
        ldata->write_reg_mask_8(indio_dev, addr, bit, bit)

#define lsm9ds1_reset_bit_reg(ldata, indio_dev, addr, bit)      \
        ldata->write_reg_mask_8(indio_dev, addr, 0, bit)

#endif /* LSM9DS1_H_ */
