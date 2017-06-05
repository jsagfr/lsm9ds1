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

#ifndef _LSM9DS1_AG_H
#define _LSM9DS1_AG_H

#include <linux/iio/iio.h>

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

/* CTRL_REG9 */
#define LSM9DS1_AG_FIFO_EN 0b00000010

/* FIFO_CTRL */
#define LSM9DS1_AG_FMOD_BYPASS         0
#define LSM9DS1_AG_FMOD_FIFO           1 << 5
#define LSM9DS1_AG_FMOD_RESERVED       2 << 5
#define LSM9DS1_AG_FMOD_CONT_TG_FIFO   3 << 5
#define LSM9DS1_AG_FMOD_BYPASS_TG_CONT 4 << 5
#define LSM9DS1_AG_FMOD_CONTINOUS      6 << 5
#define LSM9DS1_AG_FMOD_MASK           0b11100000

#define LSM9DS1_AG_FTH 0b00011111

/* FIFO_SRC */
#define LSM9DS1_AG_FSS 0b00111111


#define LSM9DS1_AG_DATA_SIZE 6*2


#define LSM9DS1_AG_DATA_SIZE 6*2

struct lsm9ds1_ag {
        u8 iio_buffer[ALIGN(LSM9DS1_AG_DATA_SIZE, sizeof(s64)) + sizeof(s64)];
        u16 delta_ts;
};
  
int lsm9ds1_ag_probe(struct iio_dev *indio_dev, struct device *dev);
int lsm9ds1_ag_remove(struct iio_dev *indio_dev);

#endif /* _LSM9DS1_AG_H */
