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

#include "lsm9ds1.h"

int lsm9ds1_register_mask_write(struct i2c_client *client, u16 addr,
                                u8 mask, u8 data)
{
        int ret;
        u8 tmp_data = 0;
        
        ret = i2c_smbus_read_byte_data(client, addr);
        if (ret < 0)
                return ret;

        tmp_data = ret & ~mask;
        tmp_data |= data & mask;
        return i2c_smbus_write_byte_data(client, addr, tmp_data);
}

EXPORT_SYMBOL(lsm9ds1_register_mask_write);

int lsm9ds1_register_set_bit(struct i2c_client *client, u16 addr,
                             u8 bit)
{
        return lsm9ds1_register_mask_write(client, addr, bit, bit);
}

EXPORT_SYMBOL(lsm9ds1_register_set_bit);

int lsm9ds1_register_reset_bit(struct i2c_client *client, u16 addr,
                               u8 bit)
{
        return lsm9ds1_register_mask_write(client, addr, bit, 0);
}

EXPORT_SYMBOL(lsm9ds1_register_reset_bit);
