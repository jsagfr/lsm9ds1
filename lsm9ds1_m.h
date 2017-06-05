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

#ifndef _LSM9DS1_M_H
#define _LSM9DS1_M_H

#include <linux/iio/iio.h>

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
#define LSM9DS1_M_FS_8GAUSS   1 << 5
#define LSM9DS1_M_FS_12GAUSS  2 << 5
#define LSM9DS1_M_FS_16GAUSS  3 << 5
#define LSM9DS1_M_FS_MASK     0b01100000

#define LSM9DS1_M_SW_RESET 1 << 2

#define LSM9DS1_M_DATA_SIZE 3*2 /* 3 channels of 16bits */

int lsm9ds1_m_probe(struct iio_dev *indio_dev, struct device *dev);
int lsm9ds1_m_remove(struct iio_dev *indio_dev);

#endif /* _LSM9DS1_M_H */
