/* drivers/regulator/ncp6951-regulator.c
 *
 * Copyright (C) 2014 HTC Corporation.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/of_gpio.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/of_regulator.h>
#include <linux/i2c.h>
#include <linux/regulator/ncp6951-regulator.h>

enum {
	VREG_TYPE_DCDC,
	VREG_TYPE_LDO123,
	VREG_TYPE_LDO45,
	VREG_TYPE_MAX
};

struct vreg_type_lookup_table {
	uint32_t type;
	const char *type_name;
};

struct ncp6951_vreg {
	struct device *dev;
	struct list_head reg_list;
	struct regulator_desc rdesc;
	struct regulator_dev *rdev;
	struct regulator_init_data *init_data;
	const char *regulator_name;
	u32 resource_id;
	int regulator_type;
	u32 enable_addr;
	u32 enable_bit;
	u32 base_addr;
	u32 use_count;
	struct mutex mlock;
	u32 inited;
	bool always_on;
};

struct ncp6951_regulator {
	struct device *dev;
	struct ncp6951_vreg *ncp6951_vregs;
	int en_gpio;
	int total_vregs;
	bool is_enable;
	struct mutex i2clock;
};

static struct ncp6951_regulator *regulator = NULL;

#define LDO123_UV_VMIN            1700000
#define LDO123_UV_STEP              50000
#define LDO123_BIT_START            0x01010
static int ldo123_bit_start[] = {0xa, 0xe, 0x15, 0x1d};
#define LDO45_UV_VMIN             1200000
#define LDO45_UV_STEP               50000
#define LDO45_BIT_START             0x00101
static int ldo45_bit_start[] = {0x5, 0x13, 0x1a};
#define DCDC_UV_VMIN            800000
#define DCDC_UV_STEP            50000
#define DCDC_BIT_START          0x00001

static int use_ioexpander = 0;

static int ncp6951_enable(struct ncp6951_regulator *reg, bool enable)
{
	int ret = 0;
	if(use_ioexpander == 0){
		gpio_direction_output(reg->en_gpio, 1);
		if (enable) {
			gpio_set_value(reg->en_gpio, 1);
			reg->is_enable = true;
			pr_debug("[NCP6951] %s: gpio: 1\n", __func__);
		} else {
			gpio_set_value(reg->en_gpio, 0);
			reg->is_enable = false;
			pr_debug("[NCP6951] %s: gpio: 0\n", __func__);
		}
	}
	return ret;
}

static bool ncp6951_is_enable(struct ncp6951_regulator *reg)
{
	return reg->is_enable;
}

static int ncp6951_i2c_write(struct device *dev, u8 reg_addr, u8 data)
{
	int res;
	int orig_state;
	struct i2c_client *client = to_i2c_client(dev);
	struct ncp6951_regulator *reg = i2c_get_clientdata(client);
	u8 values[] = {reg_addr, data};

	struct i2c_msg msg[] = {
		{
			.addr	= client->addr,
			.flags	= 0,
			.len	= 2,
			.buf	= values,
		},
	};

	mutex_lock(&reg->i2clock);
	orig_state = ncp6951_is_enable(reg);
	if (orig_state == false)
		ncp6951_enable(reg, true);
	res = i2c_transfer(client->adapter, msg, 1);

	pr_info("[NCP6951] %s, i2c: addr: %x, val: %x, %d\n", __func__, reg_addr, data, res);
	
	
	
	if ((orig_state == false && !(reg_addr == NCP6951_ENABLE && data != 0x00)) ||
	    (reg_addr == NCP6951_ENABLE && data == 0x00)) {
		ncp6951_enable(reg, false);
	}
	mutex_unlock(&reg->i2clock);
	if (res > 0)
		res = 0;

	return res;
}

int ncp6951_i2c_ex_write(u8 reg_addr, u8 data)
{
	int res = -EINVAL;
	if (regulator) {
		ncp6951_i2c_write(regulator->dev, reg_addr, data);
		res = 0;
	} else {
		pr_err("%s: regulator not init\n", __func__);
	}

	return res;
}
EXPORT_SYMBOL(ncp6951_i2c_ex_write);

static int ncp6951_i2c_read(struct device *dev, u8 reg_addr, u8 *data)
{
	int res;
	int curr_state;
	struct i2c_client *client = to_i2c_client(dev);
	struct ncp6951_regulator *reg = i2c_get_clientdata(client);

	struct i2c_msg msg[] = {
		{
			.addr	= client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= &reg_addr,
		},
		{
			.addr	= client->addr,
			.flags	= I2C_M_RD,
			.len	= 1,
			.buf	= data,
		},
	};

	mutex_lock(&reg->i2clock);
	curr_state = ncp6951_is_enable(reg);
	if (curr_state == 0)
		ncp6951_enable(reg, true);
	res = i2c_transfer(client->adapter, msg, 2);
	pr_info("[NCP6951] %s, i2c: base: %x, data: %x, %d\n", __func__, reg_addr, *data, res);
	
	if (curr_state == 0)
		ncp6951_enable(reg, false);
	mutex_unlock(&reg->i2clock);
	if (res >= 0)
		res = 0;

	return res;
}
int ncp6951_i2c_ex_read(u8 reg_addr, u8 *data)
{
	int res = -EINVAL;
	if (regulator) {
		ncp6951_i2c_read(regulator->dev, reg_addr, data);
		res = 0;
	} else {
		pr_err("%s: regulator not init\n", __func__);
	}

	return res;
}
EXPORT_SYMBOL(ncp6951_i2c_ex_read);

static int ncp6951_vreg_is_enabled(struct regulator_dev *rdev)
{
	struct ncp6951_vreg *vreg = rdev_get_drvdata(rdev);
	struct device *dev = vreg->dev;
	uint8_t val = 0;
	int rc = 0;

	if (vreg->inited != 1) {
		pr_info("%s: vreg not inited ready\n", __func__);
		return -1;
	}
	mutex_lock(&vreg->mlock);
	rc = ncp6951_i2c_read(dev, vreg->enable_addr, &val);
	mutex_unlock(&vreg->mlock);

	return ((val & (1 << vreg->enable_bit))? 1: 0);
}

static int ncp6951_vreg_enable(struct regulator_dev *rdev)
{
	struct ncp6951_vreg *vreg = rdev_get_drvdata(rdev);
	struct device *dev = vreg->dev;
	uint8_t val = 0;
	int rc = 0;

	mutex_lock(&vreg->mlock);
	rc = ncp6951_i2c_read(dev, vreg->enable_addr, &val);
	val |= (1 << vreg->enable_bit);
	rc = ncp6951_i2c_write(dev, vreg->enable_addr, val);
	mutex_unlock(&vreg->mlock);

	return rc;
}

static int ncp6951_vreg_disable(struct regulator_dev *rdev)
{
	struct ncp6951_vreg *vreg = rdev_get_drvdata(rdev);
	struct device *dev = vreg->dev;
	uint8_t val = 0;
	int rc = 0;

	mutex_lock(&vreg->mlock);
	rc = ncp6951_i2c_read(dev, vreg->enable_addr, &val);
	val &= ~(1 << vreg->enable_bit);
	rc = ncp6951_i2c_write(dev, vreg->enable_addr, val);
	mutex_unlock(&vreg->mlock);

	return rc;
}

static int ncp6951_vreg_set_voltage(struct regulator_dev *rdev, int min_uV, int max_uV,
			    unsigned *selector)
{
	struct ncp6951_vreg *vreg = rdev_get_drvdata(rdev);
	struct device *dev = vreg->dev;
	uint8_t uv_step = 0;
	int rc = 0;

	mutex_lock(&vreg->mlock);
	if (vreg->regulator_type == VREG_TYPE_LDO123) {
		if (max_uV <= 1900000)
			uv_step = (max_uV - LDO123_UV_VMIN) / LDO123_UV_STEP + ldo123_bit_start[0];
		else if (max_uV > 1900000 && max_uV <= 2600000)
			uv_step = (max_uV - 1900000) / 100000 + ldo123_bit_start[1];
		else if (max_uV > 2600000 && max_uV <= 3000000)
			uv_step = (max_uV - 2600000) / 50000 + ldo123_bit_start[2];
		else if (max_uV > 3000000 && max_uV <= 3100000)
			uv_step = (max_uV - 3000000) / 100000 + ldo123_bit_start[3];
		else if (max_uV == 3300000)
			uv_step = 0x1f;
		pr_info("%s: type: 1, uv_step: %d (max:%d)\n", __func__, uv_step, max_uV);
		ncp6951_i2c_write(dev, vreg->base_addr, uv_step);
	} else if (vreg->regulator_type == VREG_TYPE_LDO45) {
		if (max_uV <= 1900000)
			uv_step = (max_uV - LDO45_UV_VMIN) / LDO45_UV_STEP + ldo45_bit_start[0];
		else if (max_uV > 1900000 && max_uV <= 2600000)
			uv_step = (max_uV - 1900000) / 100000 + ldo45_bit_start[1];
		else if (max_uV > 2600000)
			uv_step = (max_uV - 2600000) / 50000 + ldo45_bit_start[2];
		pr_info("%s: type: 2, uv_step: %d (max:%d)\n", __func__, uv_step, max_uV);
		ncp6951_i2c_write(dev, vreg->base_addr, uv_step);
	} else if (vreg->regulator_type == VREG_TYPE_DCDC) {
	    uv_step = (max_uV - DCDC_UV_VMIN) / DCDC_UV_STEP + DCDC_BIT_START;
		pr_info("%s: type: 0, uv_step: %d (max:%d)\n", __func__, uv_step, max_uV);
	    ncp6951_i2c_write(dev, vreg->base_addr, uv_step);
	    ncp6951_i2c_write(dev, vreg->base_addr+1, uv_step);
	} else
		pr_err("%s: non support vreg type %d\n", __func__, vreg->regulator_type);
	mutex_unlock(&vreg->mlock);

	return rc;
}

static int ncp6951_vreg_get_voltage(struct regulator_dev *rdev)
{
	struct ncp6951_vreg *vreg = rdev_get_drvdata(rdev);
	struct device *dev = vreg->dev;
	uint8_t val = 0, dcdc = 1;
	uint32_t vol = 0;

	if (vreg->inited != 1) {
		pr_info("%s: vreg not inited ready: %s\n", __func__, vreg->regulator_name);
		return -1;
	}

	mutex_lock(&vreg->mlock);
	ncp6951_i2c_read(dev, vreg->base_addr, &val);
	if (vreg->regulator_type == VREG_TYPE_LDO123) {
		if (val == 0x1f)
			vol = 3300000;
		else if (val >= ldo123_bit_start[3] && val < 0x1f)
			vol = 100000 * (val - ldo123_bit_start[3]) + 3000000;
		else if (val >= ldo123_bit_start[2] && val < ldo123_bit_start[3])
			vol = 50000 * (val - ldo123_bit_start[2]) + 2600000;
		else if (val >= ldo123_bit_start[1] && val < ldo123_bit_start[2])
			vol = 50000 * (val - ldo123_bit_start[1]) + 1900000;
		else if (val >= ldo123_bit_start[0] && val < ldo123_bit_start[1])
			vol = LDO123_UV_STEP * (val - ldo123_bit_start[0]) + LDO123_UV_VMIN;
		else
			vol = LDO123_UV_VMIN;
		
	} else if (vreg->regulator_type == VREG_TYPE_LDO45) {
		if (val >= ldo45_bit_start[2])
			vol = 50000 * (val - ldo45_bit_start[2]) + 2600000;
		else if (val >= ldo45_bit_start[1] && val < ldo45_bit_start[2])
			vol = 100000 * (val - ldo45_bit_start[1]) + 1900000;
		else if (val >= ldo45_bit_start[0] && val < ldo45_bit_start[1])
			vol = LDO45_UV_STEP * (val - ldo45_bit_start[0]) + LDO45_UV_VMIN;
		else
			vol = LDO45_UV_VMIN;
		
	} else if (vreg->regulator_type == VREG_TYPE_DCDC) {
		ncp6951_i2c_read(dev, NCP6951_ENABLE, &dcdc);
		pr_info("[NCP6951]: %s, ENABLE: %x\n", __func__, dcdc);
		if ((dcdc&0x80) == 0) {
			ncp6951_i2c_read(dev, vreg->base_addr+1, &val);
			if (val > DCDC_BIT_START)
				vol = (val - DCDC_BIT_START) * DCDC_UV_STEP + DCDC_UV_VMIN;
			else
				vol = DCDC_UV_VMIN;
		} else {
			if (val > DCDC_BIT_START)
				vol = (val - DCDC_BIT_START) * DCDC_UV_STEP + DCDC_UV_VMIN;
			else
				vol = DCDC_UV_VMIN;
		}
	} else
		pr_err("%s: non support vreg type %d\n", __func__, vreg->regulator_type);
	mutex_unlock(&vreg->mlock);

	return vol;
}

static struct regulator_ops ncp6951_vreg_ops = {
	.enable		= ncp6951_vreg_enable,
	.disable	= ncp6951_vreg_disable,
	.is_enabled	= ncp6951_vreg_is_enabled,
	.set_voltage	= ncp6951_vreg_set_voltage,
	.get_voltage	= ncp6951_vreg_get_voltage,
};

static struct regulator_ops *vreg_ops[] = {
	[VREG_TYPE_DCDC]	= &ncp6951_vreg_ops,
	[VREG_TYPE_LDO123]		= &ncp6951_vreg_ops,
	[VREG_TYPE_LDO45]		= &ncp6951_vreg_ops,
};

static int ncp6951_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct device_node *node = dev->of_node;
	struct device_node *child = NULL;
	struct ncp6951_vreg *ncp6951_vreg = NULL;
	struct ncp6951_regulator *reg;
	struct regulator_init_data *init_data;
	struct regulator_config reg_config = {};
	int en_gpio = 0;
	int num_vregs = 0;
	int vreg_idx = 0;
	int ret = 0;
	u32 init_volt;

	pr_info("%s007.\n", __func__);
	if (!dev->of_node) {
		dev_err(dev, "%s: device tree information missing\n", __func__);
		return -ENODEV;
	}

	reg = kzalloc(sizeof(struct ncp6951_regulator), GFP_KERNEL);
	if (reg == NULL) {
		dev_err(dev, "%s: could not allocate memory for reg.\n", __func__);
		return -ENOMEM;
	}

	reg->dev = dev;

	ret = of_property_read_u32(node, "ncp,use-ioexpander", &use_ioexpander);
	pr_info("use-ioexpander = %d \n", use_ioexpander);
	if(use_ioexpander == 1) {
		ret = of_property_read_u32(node, "ncp,enable-ioexp",
		&en_gpio);
		pr_info("enable-ioexp = %d \n", en_gpio);
		reg->en_gpio = en_gpio;
	}
	else {
		en_gpio = of_get_named_gpio(node, "ncp,enable-gpio", 0);
		if (gpio_is_valid(en_gpio)) {
			pr_info("%s: Read NCP6951_enable gpio: %d\n", __func__, en_gpio);
			reg->en_gpio = en_gpio;
			gpio_request(reg->en_gpio, "NCP6951_EN_GPIO");
		} else {
			pr_err("%s: Fail to read NCP6951_enable gpio: %d\n", __func__, en_gpio);
			goto fail_free_regulator;
		}
	}
	
	for_each_child_of_node(node, child)
		num_vregs++;
	reg->total_vregs = num_vregs;
	mutex_init(&reg->i2clock);

	reg->ncp6951_vregs = kzalloc(sizeof(struct ncp6951_vreg) * num_vregs, GFP_KERNEL);
	if (reg->ncp6951_vregs == NULL) {
		dev_err(dev, "%s: could not allocate memory for ncp6951_vreg\n", __func__);
		return -ENOMEM;
	}

	
	for_each_child_of_node(node, child) {
		ncp6951_vreg = &reg->ncp6951_vregs[vreg_idx++];
		ret = of_property_read_string(child, "regulator-name",
				&ncp6951_vreg->regulator_name);
		if (ret) {
			dev_err(dev, "%s: regulator-name missing in DT node\n", __func__);
			goto fail_free_vreg;
		}

		ret = of_property_read_u32(child, "ncp,resource-id",
				&ncp6951_vreg->resource_id);
		if (ret) {
			dev_err(dev, "%s: ncp,resource-id missing in DT node\n", __func__);
			goto fail_free_vreg;
		}

		ret = of_property_read_u32(child, "ncp,regulator-type",
				&ncp6951_vreg->regulator_type);
		if (ret) {
			dev_err(dev, "%s: ncp,regulator-type missing in DT node\n", __func__);
			goto fail_free_vreg;
		}

		if ((ncp6951_vreg->regulator_type < 0)
		    || (ncp6951_vreg->regulator_type >= VREG_TYPE_MAX)) {
			dev_err(dev, "%s: invalid regulator type: %d\n", __func__, ncp6951_vreg->regulator_type);
			ret = -EINVAL;
			goto fail_free_vreg;
		}
		ncp6951_vreg->rdesc.ops = vreg_ops[ncp6951_vreg->regulator_type];

		ret = of_property_read_u32(child, "ncp,enable-addr",
				&ncp6951_vreg->enable_addr);
		if (ret) {
			dev_err(dev, "%s: Fail to get vreg enable address.\n", __func__);
			goto fail_free_vreg;
		}

		ret = of_property_read_u32(child, "ncp,enable-bit",
				&ncp6951_vreg->enable_bit);
		if (ret) {
			dev_err(dev, "%s: Fail to get vreg enable bit.\n", __func__);
			goto fail_free_vreg;
		}

		ret = of_property_read_u32(child, "ncp,base-addr",
				&ncp6951_vreg->base_addr);
		if (ret) {
			dev_err(dev, "%s: Fail to get vreg base address.\n", __func__);
			goto fail_free_vreg;
		}

		if (of_property_read_bool(child, "ldo-always-on"))
			ncp6951_vreg->always_on = true;
		else
			ncp6951_vreg->always_on = false;

		init_data = of_get_regulator_init_data(dev, child);
		if (init_data == NULL) {
			dev_err(dev, "%s: unable to allocate memory\n", __func__);
			ret = -ENOMEM;
			goto fail_free_vreg;
		}

		if (init_data->constraints.name == NULL) {
			dev_err(dev, "%s: regulator name not specified\n", __func__);
			ret = -EINVAL;
			goto fail_free_vreg;
		}

		if (of_property_read_bool(child, "parent-supply"))
			init_data->supply_regulator = "ncp6951_ldo45";

		if(!of_property_read_u32(child, "ncp,init-microvolt", &init_volt)) {
			pr_info("%s: init vreg voltage: %d.\n", __func__, init_volt);
			ncp6951_vreg_set_voltage(ncp6951_vreg->rdev, init_volt, init_volt, NULL);
		}

		if (ncp6951_vreg->regulator_type == VREG_TYPE_LDO123)
			ncp6951_vreg->rdesc.n_voltages 	= 22;
		else if (ncp6951_vreg->regulator_type == VREG_TYPE_LDO45)
			ncp6951_vreg->rdesc.n_voltages 	= 27;
		else if (ncp6951_vreg->regulator_type == VREG_TYPE_DCDC)
			ncp6951_vreg->rdesc.n_voltages 	= 31;

		ncp6951_vreg->rdesc.name 	= init_data->constraints.name;
		ncp6951_vreg->dev		= dev;
		init_data->constraints.valid_ops_mask |= REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE;

		INIT_LIST_HEAD(&ncp6951_vreg->reg_list);
		mutex_init(&ncp6951_vreg->mlock);
		reg_config.dev = dev;
		reg_config.init_data = init_data;
		reg_config.driver_data = ncp6951_vreg;
		reg_config.of_node = child;
		ncp6951_vreg->rdev = regulator_register(&ncp6951_vreg->rdesc, &reg_config);
		if (IS_ERR(ncp6951_vreg->rdev)) {
			ret = PTR_ERR(ncp6951_vreg->rdev);
			ncp6951_vreg->rdev = NULL;
			pr_err("%s: regulator register failed: %s, ret = %d\n", __func__, ncp6951_vreg->rdesc.name, ret);
			goto fail_free_vreg;
		}
		ncp6951_vreg->inited = 1;
	}
	i2c_set_clientdata(client, reg);
	regulator = reg;
	ret = ncp6951_enable(reg, true);
	ncp6951_vreg_set_voltage(reg->ncp6951_vregs[0].rdev, 1900000, 1900000, NULL);

	return ret;

fail_free_vreg:
	kfree(reg->ncp6951_vregs);

fail_free_regulator:
	kfree(reg);
	return ret;
}

static int ncp6951_suspend(struct i2c_client *client, pm_message_t state)
{
	struct ncp6951_regulator *reg;
	int total_vreg_num = 0, on_vreg_num = 0;
	int idx = 0;

	reg = i2c_get_clientdata(client);
	total_vreg_num = reg->total_vregs;

	pr_info("%s\n", __func__);
	for (idx = 0; idx < total_vreg_num; idx++) {
		if (reg->ncp6951_vregs[idx].always_on)
			on_vreg_num++;
		else
			ncp6951_vreg_disable(reg->ncp6951_vregs[idx].rdev);
	}

	if (on_vreg_num == 0)
		ncp6951_enable(reg, false);

	return 0;
}

static int ncp6951_resume(struct i2c_client *client)
{
	struct ncp6951_regulator *reg;

	pr_info("%s\n", __func__);
	reg = i2c_get_clientdata(client);
	if (ncp6951_is_enable(reg) == false)
		ncp6951_enable(reg, true);

	return 0;
}

static int ncp6951_remove(struct i2c_client *client)
{
	struct ncp6951_regulator *reg;

	reg = i2c_get_clientdata(client);
	kfree(reg->ncp6951_vregs);
	kfree(reg);

	return 0;
}

static struct of_device_id ncp6951_match_table[] = {
	{.compatible = "htc,ncp6951-regulator"},
	{},
};

static const struct i2c_device_id ncp6951_id[] = {
	{"ncp6951-regulator", 0},
	{},
};

static struct i2c_driver ncp6951_driver = {
	.driver = {
		.name		= "ncp6951-regulator",
		.owner		= THIS_MODULE,
		.of_match_table	= ncp6951_match_table,
	},
	.probe		= ncp6951_probe,
	.remove		= ncp6951_remove,
	.suspend	= ncp6951_suspend,
	.resume		= ncp6951_resume,
	.id_table	= ncp6951_id,
};

int __init ncp6951_regulator_init(void)
{
	int ret = 0;

	ret = i2c_add_driver(&ncp6951_driver);
	if (ret)
		pr_err("%s: Driver registration failed\n", __func__);

	return ret;
}
EXPORT_SYMBOL(ncp6951_regulator_init);

static void __exit ncp6951_regulator_exit(void)
{
	i2c_del_driver(&ncp6951_driver);
}

MODULE_AUTHOR("Jim Hsia <jim_hsia@htc.com>");
MODULE_DESCRIPTION("NCP6951 regulator driver");
MODULE_LICENSE("GPL v2");

module_init(ncp6951_regulator_init);
module_exit(ncp6951_regulator_exit);
