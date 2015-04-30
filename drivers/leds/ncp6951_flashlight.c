/*
 * drivers/leds/ncp6951_flt_flashlight.c
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
#define DEBUG

#include <linux/slab.h>
#include <linux/reboot.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <htc/devices_cmdline.h>

#include <linux/leds.h>
#include <linux/htc_flashlight.h>
#include <linux/ncp6951_flashlight.h>

#define FLT_DBG_LOG(fmt, ...) \
		printk(KERN_DEBUG "[FLT]NCP6951 " fmt, ##__VA_ARGS__)
#define FLT_INFO_LOG(fmt, ...) \
		printk(KERN_INFO "[FLT]NCP6951 " fmt, ##__VA_ARGS__)
#define FLT_ERR_LOG(fmt, ...) \
		printk(KERN_ERR "[FLT][ERR]NCP6951 " fmt, ##__VA_ARGS__)



enum NCP6951_FLT_REG
{
	REG_FLASH_SETTING = 0x0C,	
	REG_REDUCED_CURRENT,		
	REG_TORCH_CURRENT,			
	REG_PROTECTION,				
	REG_FLASH_TIMER,			
	REG_RED_EYE,				
	REG_FLASH_CONF,				
	REG_FLASH_ENABLE,			
	REG_FLASH_STATUS,			
};

enum NCP6951_FLT_SWITCH
{
	OFF = 0,
	ON,
	DISABLE = 0,
	ENABLE,
	LOW = 0,
	HIGH,
};

enum NCP6951_FLT_FLASH_CURRENT
{
	FLASH_CURRENT_100_mA,
	FLASH_CURRENT_133_mA,
	FLASH_CURRENT_166_mA,
	FLASH_CURRENT_200_mA,
	FLASH_CURRENT_233_mA,
	FLASH_CURRENT_266_mA,
	FLASH_CURRENT_300_mA,
	FLASH_CURRENT_333_mA,
	FLASH_CURRENT_366_mA,
	FLASH_CURRENT_400_mA,
	FLASH_CURRENT_433_mA,
	FLASH_CURRENT_466_mA,
	FLASH_CURRENT_500_mA,
	FLASH_CURRENT_533_mA,
	FLASH_CURRENT_566_mA,
	FLASH_CURRENT_600_mA,
	FLASH_CURRENT_700_mA,
	FLASH_CURRENT_800_mA,
	FLASH_CURRENT_900_mA,
	FLASH_CURRENT_1000_mA,
	FLASH_CURRENT_1100_mA,
	FLASH_CURRENT_1200_mA,
	FLASH_CURRENT_1300_mA,
	FLASH_CURRENT_1400_mA,
	FLASH_CURRENT_1500_mA,
	FLASH_CURRENT_1600_mA,
};

enum NCP6951_FLT_REDUCED_REDEYE_CURRENT
{
	FLASH_RD_RE_CURRENT_100_mA,
	FLASH_RD_RE_CURRENT_200_mA,
	FLASH_RD_RE_CURRENT_300_mA,
	FLASH_RD_RE_CURRENT_400_mA,
	FLASH_RD_RE_CURRENT_500_mA,
	FLASH_RD_RE_CURRENT_600_mA,
	FLASH_RD_RE_CURRENT_700_mA,
	FLASH_RD_RE_CURRENT_800_mA,
	FLASH_RD_RE_CURRENT_900_mA,
	FLASH_RD_RE_CURRENT_1000_mA,
	FLASH_RD_RE_CURRENT_1100_mA,
	FLASH_RD_RE_CURRENT_1200_mA,
	FLASH_RD_RE_CURRENT_1300_mA,
	FLASH_RD_RE_CURRENT_1400_mA,
	FLASH_RD_RE_CURRENT_1500_mA,
	FLASH_RD_RE_CURRENT_1600_mA,
};

enum NCP6951_FLT_TORCH_CURRENT
{
	TORCH_CURRENT_33_mA,
	TORCH_CURRENT_66_mA,
	TORCH_CURRENT_100_mA,
	TORCH_CURRENT_133_mA,
	TORCH_CURRENT_166_mA,
	TORCH_CURRENT_200_mA,
	TORCH_CURRENT_233_mA,
	TORCH_CURRENT_266_mA,
	TORCH_CURRENT_300_mA,
	TORCH_CURRENT_333_mA,
	TORCH_CURRENT_366_mA,
	TORCH_CURRENT_400_mA,
	TORCH_CURRENT_433_mA,
	TORCH_CURRENT_466_mA,
	TORCH_CURRENT_500_mA,
	TORCH_CURRENT_533_mA,
} NCP6951_FLT_TORCH_CURRENT;

enum NCP6951_FLT_SAFETY_TIMER
{
	SAFETY_TIMER_32_ms,
	SAFETY_TIMER_64_ms,
	SAFETY_TIMER_128_ms,
	SAFETY_TIMER_256_ms,
	SAFETY_TIMER_512_ms,
	SAFETY_TIMER_1024_ms,
};

enum NCP6951_FLT_INHIBIT_TIMER
{
	INHIBIT_TIMER_512_ms,
	INHIBIT_TIMER_1024_ms,
	INHIBIT_TIMER_1536_ms,
	INHIBIT_TIMER_2048_ms,
	INHIBIT_TIMER_2560_ms,
	INHIBIT_TIMER_3072_ms,
	INHIBIT_TIMER_3584_ms,
	INHIBIT_TIMER_4096_ms,
	INHIBIT_TIMER_4608_ms,
	INHIBIT_TIMER_5120_ms,
	INHIBIT_TIMER_5632_ms,
	INHIBIT_TIMER_6144_ms,
	INHIBIT_TIMER_6656_ms,
	INHIBIT_TIMER_7168_ms,
	INHIBIT_TIMER_7680_ms,
	INHIBIT_TIMER_8192_ms,
};

struct ncp6951_flt_data {
	struct led_classdev	cdev;
	enum flashlight_mode_flags	mode_status;
	struct delayed_work ncp6951_flt_delayed_work;
	struct mutex ncp6951_flt_mutex;
	struct notifier_block reboot_notifier;

	uint32_t flash_timeout_sw;	
	uint32_t flash_timeout_hw;	
	uint32_t flash_timeout_inhibit; 
	uint32_t flash_timeout_inhibit_en;	
	uint32_t flash_timeout_red_eye_en;	

	uint32_t flash_current_max;	
	uint32_t flash_current_reduced; 
	uint32_t flash_current_red_eye; 
	uint32_t torch_current_max;	

	uint32_t flash_count_red_eye;	

	uint32_t ncp6951_pin_flen;	
	uint32_t ncp6951_pin_flsel;	
};

static struct ncp6951_flt_data *this_flt = NULL;
struct workqueue_struct *ncp6951_flt_workqueue = NULL;
static bool switch_state = 1;
static u8 regaddr	= 0x00;
static u8 regdata	= 0x00;



extern int ncp6951_i2c_ex_write(uint8_t reg_addr, uint8_t data);
extern int ncp6951_i2c_ex_read(uint8_t reg_addr, uint8_t *data);

int ncp6951_flt_torch(struct ncp6951_flt_data *, uint32_t);
int ncp6951_flt_flash(struct ncp6951_flt_data *, uint32_t);
int ncp6951_flt_flashlight_control(struct ncp6951_flt_data *, uint8_t);
static int ncp6951_flt_turn_off(struct ncp6951_flt_data *);



static int reboot_notify_sys(struct notifier_block *this,
			      unsigned long event,
			      void *unused)
{
	struct ncp6951_flt_data *flt = container_of(
		this, struct ncp6951_flt_data, reboot_notifier);

	FLT_INFO_LOG("%s: %ld", __func__, event);
	switch (event) {
		case SYS_RESTART:
		case SYS_HALT:
		case SYS_POWER_OFF:
			ncp6951_flt_turn_off(flt);
			return NOTIFY_OK;
	}
	return NOTIFY_DONE;
}



static ssize_t poweroff_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	int input;
	struct ncp6951_flt_data *flt = dev_get_drvdata(dev);

	input = simple_strtoul(buf, NULL, 10);
	FLT_INFO_LOG("%s\n", __func__);

	if(input == 1){
		ncp6951_flt_turn_off(flt);
	}else
		FLT_INFO_LOG("%s: Input out of range\n", __func__);

	return size;
}

static ssize_t sw_timeout_show(
		struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct ncp6951_flt_data *flt = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%4u\n",
			flt->flash_timeout_sw);
}

static ssize_t sw_timeout_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	uint32_t input;
	struct ncp6951_flt_data *flt = dev_get_drvdata(dev);
	input = simple_strtoul(buf, NULL, 10);

	if(input >= 0 && input <= 2000){
		flt->flash_timeout_sw = input;
		FLT_DBG_LOG("%s: %4u\n", __func__, flt->flash_timeout_sw);
	}else
		FLT_DBG_LOG("%s: Input out of range\n", __func__);
	return size;
}

static ssize_t register_show(
		struct device *dev, struct device_attribute *attr,
		char *buf)
{
	regdata = 0x00;
	FLT_DBG_LOG("%s\n", __func__);

	if(regaddr >= REG_FLASH_SETTING && regaddr <= REG_FLASH_STATUS) {
		if (ncp6951_i2c_ex_read(regaddr, &regdata) == 0)
			return scnprintf(buf, PAGE_SIZE, "read (0x%02X): 0x%02X\n",
					regaddr, regdata);
		else
			return scnprintf(buf, PAGE_SIZE, "I2C read fail\n");
	} else
		return scnprintf(buf, PAGE_SIZE, "register addr out of range\n");
}

static ssize_t register_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	char op = 0x00;
	uint8_t count;
	regaddr = regdata = 0x00;

	count = sscanf(buf, "%c:x%2hhx:x%2hhx\n", &op, &regaddr, &regdata);

	FLT_DBG_LOG("%s: %d, %c (0x%02X): 0x%02X\n", __func__,
				count, op, regaddr, regdata);

	if ((count == 3) && (op == 'w' || op == 'W') &&
		(regaddr >= REG_FLASH_SETTING && regaddr <= REG_FLASH_STATUS)) {

		if (ncp6951_i2c_ex_write(regaddr, regdata) == 0)
			FLT_INFO_LOG("%s: write (0x%02X): 0x%02X\n", __func__,
						regaddr, regdata);
		else
			FLT_DBG_LOG("%s: I2C write fail\n", __func__);

	} else if ((count == 2) && (op == 'r' || op == 'R') &&
		(regaddr >= REG_FLASH_SETTING && regaddr <= REG_FLASH_STATUS)) {

		regdata = 0x00;
		FLT_DBG_LOG("%s: ready to read (0x%02X)\n", __func__, regaddr);

	} else {
		regaddr = regdata = 0x00;
		FLT_DBG_LOG("%s: Input out of range\n", __func__);
	}

	return size;
}

static ssize_t switch_show(
		struct device *dev, struct device_attribute *attr,
		char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", switch_state);
}

static ssize_t switch_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	int switch_status;
	switch_status = -1;
	switch_status = simple_strtoul(buf, NULL, 10);

	if(switch_status == 0 || switch_status == 1){
		switch_state = switch_status;
		FLT_DBG_LOG("%s: %d\n", __func__, switch_state);
	}else
		FLT_DBG_LOG("%s: Input out of range\n", __func__);
	return size;
}

static ssize_t max_current_show(
		struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct ncp6951_flt_data *flt = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%4u\n", flt->flash_current_max);
}

static ssize_t flash_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	uint32_t val;
	struct ncp6951_flt_data *flt = dev_get_drvdata(dev);


	val = simple_strtoul(buf, NULL, 10);
	ncp6951_flt_flash(flt, val);

	return size;
}

static ssize_t torch_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	uint32_t val;
	struct ncp6951_flt_data *flt = dev_get_drvdata(dev);

	val = simple_strtoul(buf, NULL, 10);
	ncp6951_flt_torch(flt, val);

	return size;
}

static DEVICE_ATTR(poweroff, S_IWUSR, NULL, poweroff_store);
static DEVICE_ATTR(sw_timeout, S_IRUGO | S_IWUSR, sw_timeout_show, sw_timeout_store);
static DEVICE_ATTR(register, S_IRUGO | S_IWUSR, register_show, register_store);
static DEVICE_ATTR(function_switch, S_IRUGO | S_IWUSR, switch_show, switch_store);
static DEVICE_ATTR(max_current, S_IRUGO, max_current_show, NULL);
static DEVICE_ATTR(flash, S_IWUSR, NULL, flash_store);
static DEVICE_ATTR(torch, S_IWUSR, NULL, torch_store);

static struct attribute *ncp6951_flt_attrs[] = {
	&dev_attr_poweroff.attr,
	&dev_attr_sw_timeout.attr,
	&dev_attr_register.attr,
	&dev_attr_function_switch.attr,
	&dev_attr_max_current.attr,
	&dev_attr_flash.attr,
	&dev_attr_torch.attr,
	NULL,
};

static struct attribute_group ncp6951_flt_attr_group = {
	.attrs = ncp6951_flt_attrs,
};



static int ncp6951_flt_turn_off(struct ncp6951_flt_data *flt)
{
	FLT_INFO_LOG("%s\n", __func__);

	gpio_set_value_cansleep(flt->ncp6951_pin_flen, 0);
	gpio_set_value_cansleep(flt->ncp6951_pin_flsel, 0);
	ncp6951_i2c_ex_write(REG_FLASH_ENABLE, 0x00);

	FLT_INFO_LOG("%s %d\n", __func__, flt->mode_status);
	flt->mode_status = FL_MODE_OFF;
	return 0;
}

int ncp6951_flt_flash(struct ncp6951_flt_data *flt, uint32_t mA)
{
	uint8_t lv = 0, en;
	int ret;
	FLT_INFO_LOG("%s: %d\n", __func__, mA);
	mutex_lock(&flt->ncp6951_flt_mutex);
	gpio_set_value_cansleep(flt->ncp6951_pin_flen, 0);
	gpio_set_value_cansleep(flt->ncp6951_pin_flsel, 0);

	if (mA == 0) {
		en = 0;
	} else if(mA > 0 && mA < 100) {
		lv = 0;
	} else if (mA >= 100 && mA <= 550) {
		lv = mA - 100;
		lv = lv * 15 / 433;
		en = 0x05;
	}  else if (mA > 550 && mA <= 1600) {
		lv = mA - 550;
		lv = lv * 10 / 1000;
		lv += 15;
		en = 0x05;
	} else {
		lv = FLASH_CURRENT_1600_mA;
		en = 0x05;
	}

	ret = ncp6951_i2c_ex_write(REG_FLASH_SETTING, lv);
	ret |= ncp6951_i2c_ex_write(REG_FLASH_ENABLE, en);
	if (mA) {
		gpio_set_value_cansleep(flt->ncp6951_pin_flen, 1);
		queue_delayed_work(ncp6951_flt_workqueue, &flt->ncp6951_flt_delayed_work,
				msecs_to_jiffies(flt->flash_timeout_sw));
	}
	FLT_INFO_LOG("%s: %d, level: %d\n", __func__, ret, lv);

	mutex_unlock(&flt->ncp6951_flt_mutex);
	return ret;
}

int ncp6951_flt_torch(struct ncp6951_flt_data *flt, uint32_t mA)
{
	uint8_t lv = 0, en;
	int ret;
	FLT_INFO_LOG("%s: %d\n", __func__, mA);

	mutex_lock(&flt->ncp6951_flt_mutex);
	gpio_set_value_cansleep(flt->ncp6951_pin_flen, 0);
	gpio_set_value_cansleep(flt->ncp6951_pin_flsel, 0);

	if (mA == 0) {
		en = 0;
	} else if (mA > 0 && mA <= 533) {
		lv = mA * 15 / 533;
		en = 0x02;
	} else {
		lv = TORCH_CURRENT_533_mA;
		en = 0x02;
	}

	ret = ncp6951_i2c_ex_write(REG_TORCH_CURRENT, lv);
	ret |= ncp6951_i2c_ex_write(REG_FLASH_ENABLE, en);
	FLT_INFO_LOG("%s: %d, level: %d\n", __func__, ret, lv);

	mutex_unlock(&flt->ncp6951_flt_mutex);
	return ret;
}

#if defined(CONFIG_HTC_FLASHLIGHT_COMMON)
static int ncp6951_flt_flash_adapter(int mA1, int mA2){
	return ncp6951_flt_flash(this_flt, mA1);
}

static int ncp6951_flt_torch_adapter(int mA1, int mA2){
	return ncp6951_flt_torch(this_flt, mA1);
}
#endif

int ncp6951_flt_flashlight_control(struct ncp6951_flt_data *flt, uint8_t mode)
{
	int ret = 0;
	uint32_t val;

	if (cancel_delayed_work_sync(&flt->ncp6951_flt_delayed_work))
		FLT_INFO_LOG("delayed work is cancelled\n");

	switch (mode) {
	case FL_MODE_OFF:
		val = 0;
		ncp6951_flt_turn_off(flt);
	break;
	case FL_MODE_FLASH:
		FLT_INFO_LOG("flash 1000A\n");
		val = 1000;
		ncp6951_flt_flash(flt, val);
	break;
	case FL_MODE_PRE_FLASH:
		val = 100;
		ncp6951_flt_torch(flt, val);
	break;
	case FL_MODE_TORCH:
		val = 133;
		ncp6951_flt_torch(flt, val);
	break;
	case FL_MODE_TORCH_LEVEL_1:
		val = 33;
		ncp6951_flt_torch(flt, val);
	break;
	case FL_MODE_TORCH_LEVEL_2:
		val = 66;
		ncp6951_flt_torch(flt, val);
	break;
	default:
		FLT_ERR_LOG("%s: unknown flash_light flags: %d\n",
							__func__, mode);
		ret = -EINVAL;
	break;
	}

	FLT_INFO_LOG("%s: mode: %d\n", __func__, mode);
	flt->mode_status = mode;

	return ret;
}

static void ncp6951_flt_brightness_set(struct led_classdev *led_cdev,
						enum led_brightness brightness)
{
	int ret = -1;
	enum flashlight_mode_flags mode;
	struct ncp6951_flt_data *flt = container_of(
			led_cdev, struct ncp6951_flt_data, cdev);

	if (brightness > 0 && brightness <= LED_HALF) {
		if (brightness == (LED_HALF - 2))
			mode = FL_MODE_TORCH_LEVEL_1;	
		else if (brightness == (LED_HALF - 1))
			mode = FL_MODE_TORCH_LEVEL_2;	
		else
			mode = FL_MODE_TORCH;		
	} else if (brightness > LED_HALF && brightness <= LED_FULL) {
		if (brightness == (LED_HALF + 1))
			mode = FL_MODE_PRE_FLASH;	
		else if (brightness == (LED_HALF + 3))
			mode = FL_MODE_FLASH_LEVEL1;	
		else if (brightness == (LED_HALF + 4))
			mode = FL_MODE_FLASH_LEVEL2;	
		else if (brightness == (LED_HALF + 5))
			mode = FL_MODE_FLASH_LEVEL3;	
		else if (brightness == (LED_HALF + 6))
			mode = FL_MODE_FLASH_LEVEL4;	
		else if (brightness == (LED_HALF + 7))
			mode = FL_MODE_FLASH_LEVEL5;	
		else if (brightness == (LED_HALF + 8))
			mode = FL_MODE_FLASH_LEVEL6;	
		else if (brightness == (LED_HALF + 9))
			mode = FL_MODE_FLASH_LEVEL7;	
		else
			mode = FL_MODE_FLASH;		
	} else
		mode = FL_MODE_OFF;			

	if ((mode != FL_MODE_OFF) && switch_state == 0){
		FLT_INFO_LOG("%s flashlight is disabled by switch, mode = %d\n",__func__, mode);
		return;
	}

	ret = ncp6951_flt_flashlight_control(flt, mode);
	if (ret < 0) {
		FLT_ERR_LOG("%s: control failure ret:%d\n", __func__, ret);
		return;
	}
}

static void ncp6951_flt_turn_off_work(struct work_struct *work)
{
	struct ncp6951_flt_data *flt = container_of(
					work,
					struct ncp6951_flt_data,
					ncp6951_flt_delayed_work.work);

	FLT_INFO_LOG("%s\n", __func__);
	ncp6951_flt_turn_off(flt);
}

static int ncp6951_flt_parse_dt(
	struct device_node *dt,
	struct ncp6951_flt_platform_data *pdata)
{
	const char *parser_st[] = {
		"flash_timeout_sw",		
		"flash_timeout_hw",		
		"flash_timeout_inhibit",	
		"flash_timeout_inhibit_en",	
		"flash_timeout_red_eye_en",	
		"flash_current_max",		
		"flash_current_reduced",	
		"flash_current_red_eye",	
		"torch_current_max",		
		"flash_count_red_eye",		
	};

	const char *parser_pin_st[] = {
		"ncp6951_pin_flen",	
		"ncp6951_pin_flsel",	
	};

	int ret;
	uint8_t i;
	int32_t gpio;
	uint32_t parse_result[ARRAY_SIZE(parser_st)] = {0};

	FLT_INFO_LOG("%s\n", __func__);

	for(i = 0; i < ARRAY_SIZE(parser_st); i++) {
		ret = of_property_read_u32(dt, parser_st[i], &parse_result[i]);
		if (ret < 0) {
			FLT_ERR_LOG("DT:%s parser err, ret=%d", parser_st[i], ret);
			goto err_parser;
		} else
			FLT_INFO_LOG("DT:%s=%d", parser_st[i], parse_result[i]);
	}

	pdata->flash_timeout_sw		= parse_result[0];
	pdata->flash_timeout_hw		= parse_result[1];
	pdata->flash_timeout_inhibit	= parse_result[2];
	pdata->flash_timeout_inhibit_en = parse_result[3];
	pdata->flash_timeout_red_eye_en = parse_result[4];
	pdata->flash_current_max	= parse_result[5];
	pdata->flash_current_reduced	= parse_result[6];
	pdata->flash_current_red_eye	= parse_result[7];
	pdata->torch_current_max	= parse_result[8];
	pdata->flash_count_red_eye	= parse_result[9];
	pdata->ncp6951_pin_flen		= -1;
	pdata->ncp6951_pin_flsel	= -1;

	gpio = of_get_named_gpio(dt, parser_pin_st[0], 0);
	if (!gpio_is_valid(gpio)) {
		FLT_INFO_LOG("DT: %s parser failure, ret=%d", parser_pin_st[0], gpio);
	} else {
		pdata->ncp6951_pin_flen = gpio;
		FLT_INFO_LOG("DT:%s[%d] read", parser_pin_st[0], pdata->ncp6951_pin_flen);
	}

	gpio = of_get_named_gpio(dt, parser_pin_st[1], 0);
	if (!gpio_is_valid(gpio)) {
		FLT_INFO_LOG("DT: %s parser failure, ret=%d", parser_pin_st[1], gpio);
	} else {
		pdata->ncp6951_pin_flsel= gpio;
		FLT_INFO_LOG("DT:%s[%d] read", parser_pin_st[1], pdata->ncp6951_pin_flsel);
	}

	return 0;

err_parser:
	return ret;
}

static int ncp6951_flt_probe(struct platform_device *pdev)
{
	struct ncp6951_flt_data *flt;
	struct ncp6951_flt_platform_data *pdata;

	int err = 0;

	FLT_INFO_LOG("%s: +++\n", __func__);

	if (board_mfg_mode() == MFG_MODE_OFFMODE_CHARGING) {
		FLT_INFO_LOG("%s: offmode_charging, "
				"do not probe NCP6951_FLASHLIGHT\n", __func__);
		return -EACCES;
	}

	
	flt = kzalloc(sizeof(struct ncp6951_flt_data), GFP_KERNEL);
	if (flt == NULL) {
		FLT_ERR_LOG("%s fail to allocate flt\n", __func__);
		return -ENOMEM;
	}

	if (pdev->dev.of_node) {
		pdata =  kzalloc(sizeof(*pdata), GFP_KERNEL);
	} else {
		FLT_INFO_LOG("old style\n");
		pdata = pdev->dev.platform_data;
	}

	if (pdata == NULL) {
		FLT_ERR_LOG("%s fail to allocate pdata\n", __func__);
		err = -ENOMEM;
		goto platform_data_null;
	}

	
	if (pdev->dev.of_node) {
		err = ncp6951_flt_parse_dt(pdev->dev.of_node, pdata);
		if (err) {
			FLT_ERR_LOG("%s: parse fail", __func__);
			goto parse_fail;
		}
	}

	flt->flash_timeout_sw		= pdata->flash_timeout_sw;
	flt->flash_timeout_hw		= pdata->flash_timeout_hw;
	flt->flash_timeout_inhibit	= pdata->flash_timeout_inhibit;
	flt->flash_timeout_inhibit_en	= pdata->flash_timeout_inhibit_en;
	flt->flash_timeout_red_eye_en	= pdata->flash_timeout_red_eye_en;
	flt->flash_current_max		= pdata->flash_current_max;
	flt->flash_current_reduced	= pdata->flash_current_reduced;
	flt->flash_current_red_eye	= pdata->flash_current_red_eye;
	flt->torch_current_max		= pdata->torch_current_max;
	flt->flash_count_red_eye	= pdata->flash_count_red_eye;
	flt->ncp6951_pin_flen		= pdata->ncp6951_pin_flen;
	flt->ncp6951_pin_flsel		= pdata->ncp6951_pin_flsel;
#if defined(CONFIG_HTC_FLASHLIGHT_COMMON)
	htc_flash_main			= &ncp6951_flt_flash_adapter;
	htc_torch_main			= &ncp6951_flt_torch_adapter;
#endif
	if (gpio_is_valid(flt->ncp6951_pin_flen)) {
		err = gpio_request(flt->ncp6951_pin_flen, "flt_flen");
		if (err) {
			FLT_ERR_LOG("%s: unable to request gpio %d (%d)\n",
				__func__, flt->ncp6951_pin_flen, err);
			goto gpio_setting_fail;
		}

		err = gpio_direction_output(flt->ncp6951_pin_flen, 0);
		if (err) {
			FLT_ERR_LOG("%s: Unable to set direction\n", __func__);
			goto gpio_setting_fail;
		}
	} else
		FLT_INFO_LOG("%s: ncp6951_pin_flen is not defined\n", __func__);

	if (gpio_is_valid(flt->ncp6951_pin_flsel)) {
		err = gpio_request(flt->ncp6951_pin_flsel, "flt_flsel");
		if (err) {
			FLT_ERR_LOG("%s: unable to request gpio %d (%d)\n",
				__func__, flt->ncp6951_pin_flsel, err);
			goto gpio_setting_fail;
		}

		err = gpio_direction_output(flt->ncp6951_pin_flsel, 0);
		if (err) {
			FLT_ERR_LOG("%s: Unable to set direction\n", __func__);
			goto gpio_setting_fail;
		}
	} else
		FLT_INFO_LOG("%s: ncp6951_pin_flsel is not defined\n", __func__);

	if (flt->flash_timeout_hw > 1024)
		flt->flash_timeout_hw = 1024;

	if (flt->flash_timeout_hw > 2000)
		flt->flash_timeout_sw = 2000;

	if (flt->flash_timeout_hw > flt->flash_timeout_sw)
		flt->flash_timeout_sw = flt->flash_timeout_hw;

	if (flt->flash_timeout_inhibit > 8192)
		flt->flash_timeout_inhibit = 8192;

	if (flt->flash_timeout_inhibit_en > 0)
		FLT_ERR_LOG("%s: enable inhibit timer\n", __func__);

	if (flt->flash_timeout_red_eye_en > 0)
		FLT_ERR_LOG("%s: enable pre-flash timeout protection\n", __func__);

	if (flt->flash_current_max > 1600)
		flt->flash_current_max = 1600;

	if (flt->flash_current_reduced > 1600)
		flt->flash_current_reduced = 1600;

	if (flt->flash_current_red_eye > 200)
		flt->flash_current_red_eye = 200;

	if (flt->torch_current_max < 200)
		flt->torch_current_max = 200;
	else if (flt->torch_current_max > 533)
		flt->torch_current_max = 533;

	if (flt->flash_count_red_eye > 3)
		flt->flash_count_red_eye = 3;

	
	INIT_DELAYED_WORK(&flt->ncp6951_flt_delayed_work, ncp6951_flt_turn_off_work);
	ncp6951_flt_workqueue = create_singlethread_workqueue("ncp6951_flt_wq");
	if (!ncp6951_flt_workqueue)
		goto workqueue_fail;

	mutex_init(&flt->ncp6951_flt_mutex);
	dev_set_drvdata(&pdev->dev, flt);

	
	flt->cdev.name			= FLASHLIGHT_NAME;
	flt->cdev.brightness_set	= ncp6951_flt_brightness_set;
	err = led_classdev_register(&pdev->dev, &flt->cdev);
	if (err < 0) {
		FLT_ERR_LOG("%s: failed on led_classdev_register\n", __func__);
		goto led_class_regist_fail;
	}

	flt->reboot_notifier.notifier_call = reboot_notify_sys;
	err = register_reboot_notifier(&flt->reboot_notifier);
	if (err < 0) {
		FLT_ERR_LOG("%s: Register reboot notifier failed, err: %d\n",
				__func__, err);
		goto notifier_regist_fail;
	}

	err = sysfs_create_group(&flt->cdev.dev->kobj, &ncp6951_flt_attr_group);
	if (err < 0) {
		FLT_ERR_LOG("%s, Unable to create attribute file(s), err: %d\n",
				__func__, err);
		goto attributes_create_fail;
	}

	

	flt->flash_timeout_sw = 600;
	ncp6951_i2c_ex_write(REG_FLASH_SETTING, 0x1F);
	ncp6951_i2c_ex_write(REG_REDUCED_CURRENT, 0x00);
	ncp6951_i2c_ex_write(REG_TORCH_CURRENT, 0x05);
	ncp6951_i2c_ex_write(REG_PROTECTION, 0x20);
	ncp6951_i2c_ex_write(REG_FLASH_TIMER, 0x05);	
	ncp6951_i2c_ex_write(REG_RED_EYE, 0x05);	
	ncp6951_i2c_ex_write(REG_FLASH_CONF, 0x08);
	ncp6951_i2c_ex_write(REG_FLASH_ENABLE, 0x00);

	if(0) {
		FLT_ERR_LOG("%s, Unable to initialize the chip, err: %d\n",
				__func__, err);
		goto chip_initial_fail;
	}


	this_flt = flt;

	kfree(pdata);
	FLT_INFO_LOG("%s: ---\n", __func__);
	return 0;

chip_initial_fail:
	sysfs_remove_group(&flt->cdev.dev->kobj, &ncp6951_flt_attr_group);
attributes_create_fail:
	unregister_reboot_notifier(&flt->reboot_notifier);
notifier_regist_fail:
	led_classdev_unregister(&flt->cdev);
led_class_regist_fail:
	if (ncp6951_flt_workqueue)
		destroy_workqueue(ncp6951_flt_workqueue);
	mutex_destroy(&flt->ncp6951_flt_mutex);
workqueue_fail:
gpio_setting_fail:
parse_fail:
	kfree(pdata);
platform_data_null:
	kfree(flt);
	return err;
}

static int ncp6951_flt_remove(struct platform_device *pdev)
{
	struct ncp6951_flt_data *flt = dev_get_drvdata(&pdev->dev);

	FLT_INFO_LOG("%s: +++\n", __func__);

	cancel_delayed_work_sync(&flt->ncp6951_flt_delayed_work);
	ncp6951_flt_turn_off(flt);

	sysfs_remove_group(&flt->cdev.dev->kobj, &ncp6951_flt_attr_group);
	unregister_reboot_notifier(&flt->reboot_notifier);
	led_classdev_unregister(&flt->cdev);
	if (ncp6951_flt_workqueue)
		destroy_workqueue(ncp6951_flt_workqueue);
	mutex_destroy(&flt->ncp6951_flt_mutex);
	kfree(flt);

	FLT_INFO_LOG("%s: ---\n", __func__);
	return 0;
}

static int ncp6951_flt_resume(struct device *dev)
{
	FLT_INFO_LOG("%s:\n", __func__);
	return 0;
}
static int ncp6951_flt_suspend(struct device *dev)
{
	struct ncp6951_flt_data *flt = dev_get_drvdata(dev);

	FLT_INFO_LOG("%s:\n", __func__);
	ncp6951_flt_turn_off(flt);
	return 0;
}

static SIMPLE_DEV_PM_OPS(ncp6951_flt_pm_ops, ncp6951_flt_suspend, ncp6951_flt_resume);

static const struct of_device_id ncp6951_flt_mttable[] = {
	{ .compatible = "NCP6951_FLASHLIGHT"},
	{ },
};

static struct platform_driver ncp6951_flt_driver = {
	.probe		= ncp6951_flt_probe,
	.remove		= ncp6951_flt_remove,
	.driver		= {
		.name	= "NCP6951_FLASHLIGHT",
		.owner	= THIS_MODULE,
		.pm		= &ncp6951_flt_pm_ops,
		.of_match_table = ncp6951_flt_mttable,
	},
};
module_platform_driver(ncp6951_flt_driver);

MODULE_AUTHOR("Tai Kuo <tai_kuo@htc.com>");
MODULE_DESCRIPTION("NCP6951 Flash driver");
MODULE_LICENSE("GPL v2");
