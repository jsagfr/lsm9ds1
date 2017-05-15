#include <linux/device.h>
#include <linux/module.h>
#include "lsm9ds1.h"
#include "lsm9ds1_i2c.h"
#include "lsm9ds1_ag.h"


static int lsm9ds1_ag_i2c_probe(struct i2c_client *client,
                                const struct i2c_device_id *id)
{
	int ret;
	struct iio_dev *indio_dev;
	struct lsm9ds1_data *ldata;

	indio_dev = devm_iio_device_alloc(&client->dev, sizeof(*ldata));
	if (!indio_dev)
		return -ENOMEM;

	ret = i2c_smbus_read_byte_data(client, LSM9DS1_REG_WHO_AM_I);
	if (ret != LSM9DS1_AG_I_AM)
		return (ret < 0) ? ret : -ENODEV;

	ldata = iio_priv(indio_dev);
	ldata->i2c = client;
        ldata->read_reg = lsm9ds1_i2c_read_reg;
        ldata->read_reg_8 = lsm9ds1_i2c_read_reg_8;
        ldata->read_reg_16 = lsm9ds1_i2c_read_reg_16;
        ldata->write_reg_8 = lsm9ds1_i2c_write_reg_8;
        ldata->write_reg_mask_8 = lsm9ds1_i2c_write_reg_mask_8;
        mutex_init(&ldata->lock);
	i2c_set_clientdata(client, indio_dev);

        return lsm9ds1_ag_probe(indio_dev, &client->dev);
}

static int lsm9ds1_ag_i2c_remove(struct i2c_client *client)
{
	struct iio_dev *indio_dev = i2c_get_clientdata(client);

	return lsm9ds1_ag_remove(indio_dev);
}

static struct i2c_device_id lsm9ds1_ag_i2c_ids[] = {
        { "lsm9ds1_ag", 0 },
        { }
};

MODULE_DEVICE_TABLE(i2c, lsm9ds1_ag_i2c_ids);

static struct i2c_driver lsm9ds1_ag_driver = {
	.driver = {
		.name = "lsm9ds1_ag_i2c",
		.pm = NULL,
	},
	.probe		= lsm9ds1_ag_i2c_probe,
	.remove		= lsm9ds1_ag_i2c_remove,
	.id_table	= lsm9ds1_ag_i2c_ids,
};

module_i2c_driver(lsm9ds1_ag_driver);

MODULE_AUTHOR("Jérôme Guéry <jerome.guery@gmail.com>");
MODULE_DESCRIPTION("accelerometer and gyroscope part of the IMU LSM9DS1 I2C driver");
MODULE_LICENSE("GPL");
