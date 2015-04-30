/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include "msm_sensor.h"
#include <linux/gpio.h>

#define s5k5e_htc_SENSOR_NAME "s5k5e_htc"
DEFINE_MSM_MUTEX(s5k5e_mut);

static struct msm_sensor_ctrl_t s5k5e_s_ctrl;

#ifdef CONFIG_CAMERA_AIT
extern int match_front_sensor_ID;
extern uint16_t front_fusedid[4];
extern uint16_t front_fuse_id[4];
#endif

struct msm_camera_GPIO_info{
    int CAM_SEL;
    int CAM_SEL2;
    int ISP9882_EN;
    int ISP_MCLK_SEL;
    int V_SR_3V;
};

static struct msm_camera_GPIO_info *GPIO_ctrl = NULL;

struct msm_sensor_power_setting s5k5e_power_setting[] = {
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VDIG,
		.config_val = 1,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VIO,
		.config_val = 1,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_VANA,
		.config_val = 1,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_CLK,
		.seq_val = SENSOR_CAM_MCLK,
		.config_val = 0,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VANA,
		.config_val = 1,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_HIGH,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_I2C_MUX,
		.seq_val = 0,
		.config_val = 0,
		.delay = 0,
	},
};

struct msm_sensor_power_setting s5k5e_power_down_setting[] = {
	{
		.seq_type = SENSOR_I2C_MUX,
		.seq_val = 0,
		.config_val = 0,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_LOW,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VANA,
		.config_val = 0,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_CLK,
		.seq_val = SENSOR_CAM_MCLK,
		.config_val = 0,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_VANA,
		.config_val = 0,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VIO,
		.config_val = 0,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VDIG,
		.config_val = 0,
		.delay = 1,
	},
};

static struct v4l2_subdev_info s5k5e_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order    = 0,
	},
};

static const struct i2c_device_id s5k5e_i2c_id[] = {
	{s5k5e_htc_SENSOR_NAME, (kernel_ulong_t)&s5k5e_s_ctrl},
	{ }
};

static int32_t msm_s5k5e_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	return msm_sensor_i2c_probe(client, id, &s5k5e_s_ctrl);
}

static struct i2c_driver s5k5e_i2c_driver = {
	.id_table = s5k5e_i2c_id,
	.probe  = msm_s5k5e_i2c_probe,
	.driver = {
		.name = s5k5e_htc_SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client s5k5e_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static const struct of_device_id s5k5e_dt_match[] = {
	{.compatible = "htc,s5k5e_htc", .data = &s5k5e_s_ctrl},
	{}
};

MODULE_DEVICE_TABLE(of, s5k5e_dt_match);

static struct platform_driver s5k5e_platform_driver = {
	.driver = {
		.name = "htc,s5k5e_htc",
		.owner = THIS_MODULE,
		.of_match_table = s5k5e_dt_match,
	},
};

static const char *s5k5eVendor = "Samsung";
static const char *s5k5eNAME = "s5k5e_htc";
static const char *s5k5eSize = "5.0M";

static ssize_t sensor_vendor_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;

	sprintf(buf, "%s %s %s\n", s5k5eVendor, s5k5eNAME, s5k5eSize);
	ret = strlen(buf) + 1;

	return ret;
}

static DEVICE_ATTR(sensor, 0444, sensor_vendor_show, NULL);

static struct kobject *android_s5k5e;

static int s5k5e_sysfs_init(void)
{
	int ret ;
	pr_info("s5k5e_htc:kobject creat and add\n");
	android_s5k5e = kobject_create_and_add("android_camera2", NULL);
	if (android_s5k5e == NULL) {
		pr_info("s5k5e_htc_sysfs_init: subsystem_register " \
		"failed\n");
		ret = -ENOMEM;
		return ret ;
	}
	pr_info("s5k5e_htc:sysfs_create_file\n");
	ret = sysfs_create_file(android_s5k5e, &dev_attr_sensor.attr);
	if (ret) {
		pr_info("s5k5e_htc_sysfs_init: sysfs_create_file " \
		"failed\n");
		kobject_del(android_s5k5e);
	}

	return 0 ;
}

static int32_t s5k5e_platform_probe(struct platform_device *pdev)
{
	int32_t rc = 0;
	const struct of_device_id *match;

	match = of_match_device(s5k5e_dt_match, &pdev->dev);

    
    GPIO_ctrl = kzalloc(sizeof(struct msm_camera_GPIO_info), GFP_ATOMIC);
    if (!GPIO_ctrl) {
		pr_err("%s: could not allocate mem for GPIO_ctrl\n", __func__);
		return -ENOMEM;
	}

    GPIO_ctrl->CAM_SEL= of_get_named_gpio((&pdev->dev)->of_node, "CAM_SEL",0);
	pr_info("CAM_SEL %d\n", GPIO_ctrl->CAM_SEL);
	if ( GPIO_ctrl->CAM_SEL < 0) {
		pr_err("%s:%d CAM_SEL rc %d\n", __func__, __LINE__, GPIO_ctrl->CAM_SEL);
	}
    GPIO_ctrl->CAM_SEL2= of_get_named_gpio((&pdev->dev)->of_node, "CAM_SEL2",0);
	pr_info("CAM_SEL2 %d\n", GPIO_ctrl->CAM_SEL2);
	if ( GPIO_ctrl->CAM_SEL2 < 0) {
		pr_err("%s:%d CAM_SEL2 rc %d\n", __func__, __LINE__, GPIO_ctrl->CAM_SEL2);
	}
    GPIO_ctrl->ISP9882_EN= of_get_named_gpio((&pdev->dev)->of_node, "ISP9882_EN",0);
	pr_info("ISP9882_EN %d\n", GPIO_ctrl->ISP9882_EN);
	if ( GPIO_ctrl->ISP9882_EN < 0) {
		pr_err("%s:%d ISP9882_EN rc %d\n", __func__, __LINE__, GPIO_ctrl->ISP9882_EN);
	}
    GPIO_ctrl->ISP_MCLK_SEL= of_get_named_gpio((&pdev->dev)->of_node, "ISP_MCLK_SEL",0);
	pr_info("ISP_MCLK_SEL %d\n", GPIO_ctrl->ISP_MCLK_SEL);
	if ( GPIO_ctrl->ISP_MCLK_SEL < 0) {
		pr_err("%s:%d ISP_MCLK_SEL rc %d\n", __func__, __LINE__, GPIO_ctrl->ISP_MCLK_SEL);
	}
    GPIO_ctrl->V_SR_3V= of_get_named_gpio((&pdev->dev)->of_node, "V_SR_3V",0);
	pr_info("V_SR_3V %d\n", GPIO_ctrl->V_SR_3V);
	if ( GPIO_ctrl->V_SR_3V < 0) {
		pr_err("%s:%d V_SR_3V rc %d\n", __func__, __LINE__, GPIO_ctrl->V_SR_3V);
	}
    

	
	if (match == NULL) {
		pr_err("%s: match is NULL\n", __func__);
		return -ENODEV;
	}
	

	rc = msm_sensor_platform_probe(pdev, match->data);
	return rc;
}

static int __init s5k5e_init_module(void)
{
	int32_t rc = 0;
	pr_info("%s:%d\n", __func__, __LINE__);

	rc = platform_driver_probe(&s5k5e_platform_driver,
		s5k5e_platform_probe);
	if (!rc) {
		s5k5e_sysfs_init();
		return rc;
	}
	pr_err("%s:%d rc %d\n", __func__, __LINE__, rc);
	return i2c_add_driver(&s5k5e_i2c_driver);
}

static void __exit s5k5e_exit_module(void)
{
	pr_info("%s:%d\n", __func__, __LINE__);
	if (s5k5e_s_ctrl.pdev) {
		msm_sensor_free_sensor_data(&s5k5e_s_ctrl);
		platform_driver_unregister(&s5k5e_platform_driver);
	} else
		i2c_del_driver(&s5k5e_i2c_driver);
	return;
}

int32_t s5k5e_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
    int32_t status;
    int rc = 0;

    pr_info("%s: +\n", __func__);
    
    
    rc = gpio_request_one(GPIO_ctrl->CAM_SEL, 0, "CAM_SEL");
    if (rc < 0) {
		pr_err("GPIO(%d) request failed", GPIO_ctrl->CAM_SEL);
	}
    if (GPIO_ctrl->CAM_SEL != 0){
        gpio_direction_output(GPIO_ctrl->CAM_SEL, 1);
        mdelay(1);
        gpio_free(GPIO_ctrl->CAM_SEL);
    }
    else
        pr_err("GPIO(%d) GPIO_ctrl->CAM_SEL failed\n", GPIO_ctrl->CAM_SEL);

    
    rc = gpio_request_one(GPIO_ctrl->CAM_SEL2, 0, "CAM_SEL2");
    if (rc < 0) {
		pr_err("GPIO(%d) request failed", GPIO_ctrl->CAM_SEL2);
	}
    if (GPIO_ctrl->CAM_SEL2 != 0){
        gpio_direction_output(GPIO_ctrl->CAM_SEL2, 1);
        mdelay(1);
        gpio_free(GPIO_ctrl->CAM_SEL2);
    }
    else
        pr_err("GPIO(%d) GPIO_ctrl->CAM_SEL2 failed\n", GPIO_ctrl->CAM_SEL2);

    
    rc = gpio_request_one(GPIO_ctrl->ISP9882_EN, 0, "ISP9882_EN");
    if (rc < 0) {
		pr_err("GPIO(%d) request failed", GPIO_ctrl->ISP9882_EN);
	}
    if (GPIO_ctrl->ISP9882_EN != 0){
        gpio_direction_output(GPIO_ctrl->ISP9882_EN, 0);
        mdelay(1);
        gpio_free(GPIO_ctrl->ISP9882_EN);
    }
    else
        pr_err("GPIO(%d) GPIO_ctrl->ISP9882_EN failed\n", GPIO_ctrl->ISP9882_EN);

    
    rc = gpio_request_one(GPIO_ctrl->ISP_MCLK_SEL, 0, "ISP_MCLK_SEL");
    if (rc < 0) {
		pr_err("GPIO(%d) request failed", GPIO_ctrl->ISP_MCLK_SEL);
	}
    if (GPIO_ctrl->ISP_MCLK_SEL != 0){
        gpio_direction_output(GPIO_ctrl->ISP_MCLK_SEL, 1);
        mdelay(1);
        gpio_free(GPIO_ctrl->ISP_MCLK_SEL);
    }
    else
        pr_err("GPIO(%d) GPIO_ctrl->ISP_MCLK_SEL failed\n", GPIO_ctrl->ISP_MCLK_SEL);

    
    rc = gpio_request_one(GPIO_ctrl->V_SR_3V, 0, "V_SR_3V");
    if (rc < 0) {
		pr_err("GPIO(%d) request failed", GPIO_ctrl->V_SR_3V);
	}
    if (GPIO_ctrl->V_SR_3V != 0){
        gpio_direction_output(GPIO_ctrl->V_SR_3V, 1);
        mdelay(1);
        gpio_free(GPIO_ctrl->V_SR_3V);
    }
    else
        pr_err("GPIO(%d) GPIO_ctrl->V_SR_3V failed\n", GPIO_ctrl->V_SR_3V);
    

    s_ctrl->power_setting_array.power_setting = s5k5e_power_setting;
    s_ctrl->power_setting_array.size = ARRAY_SIZE(s5k5e_power_setting);
    status = msm_sensor_power_up(s_ctrl);
    pr_info("%s: -\n", __func__);
    return status;
}

int32_t s5k5e_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
    int32_t status;
    int i = 0;
    int j = 0;
    int data_size;
    pr_info("%s: +\n", __func__);
    s_ctrl->power_setting_array.power_setting = s5k5e_power_down_setting;
    s_ctrl->power_setting_array.size = ARRAY_SIZE(s5k5e_power_down_setting);

    
    for(i = 0; i < s_ctrl->power_setting_array.size;  i++)
    {
        data_size = sizeof(s5k5e_power_setting[i].data)/sizeof(void *);
        for (j =0; j < data_size; j++ )
        s5k5e_power_down_setting[i].data[j] = s5k5e_power_setting[i].data[j];
    }
    status = msm_sensor_power_down(s_ctrl);
    pr_info("%s: -\n", __func__);
    return status;
}
#if 1

static int s5k5e_read_fuseid(struct sensorb_cfg_data *cdata,
	struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	int i,j;
	uint16_t read_data = 0;
	uint16_t id_addr[6] = {0x0A04,0x0A05,0x0A06,0x0A07,0x0A08,0x0A09};
	static uint16_t id_data[6] = {0,0,0,0,0,0};
	static int first = true;
	static int32_t valid_page=-1;

	pr_info("%s called\n", __func__);
	if(first){
		first = false;
		for(j=4;j>=2;j--){
				rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client, 0x0A00, 0x04, MSM_CAMERA_I2C_BYTE_DATA);
				if (rc < 0)
					pr_err("%s: i2c_write failed\n", __func__);
				rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client, 0x0A02, j, MSM_CAMERA_I2C_BYTE_DATA);
				if (rc < 0)
					pr_info("%s: i2c_write failed\n", __func__);
				rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client, 0x0A00, 0x01, MSM_CAMERA_I2C_BYTE_DATA);
				if (rc < 0)
					pr_info("%s: i2c_write failed\n", __func__);
				msleep(10);
				for (i=0; i<6; i++) {
					rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client, id_addr[i], &read_data, MSM_CAMERA_I2C_BYTE_DATA);

					if (rc < 0)
						pr_err("%s: i2c_read 0x%x failed\n", __func__, i);

					id_data[i] = read_data & 0xff;
				}
				if (read_data)
					valid_page = j;
				if (valid_page!=-1)
					break;
		}
	}
	rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client, 0x0A00, 0x04, MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0)
		pr_err("%s: i2c_write failed\n", __func__);
	rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client, 0x0A00, 0x00, MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0)
		pr_err("%s: i2c_write failed\n", __func__);
	if (cdata != NULL) {
	cdata->cfg.fuse.fuse_id_word1 = 0;
	cdata->cfg.fuse.fuse_id_word2 = id_data[3];
	cdata->cfg.fuse.fuse_id_word3 = id_data[4];
	cdata->cfg.fuse.fuse_id_word4 = id_data[5];

	cdata->lens_id = id_data[1];

	pr_info("s5k5e: fuse->fuse_id : 0x%x 0x%x 0x%x 0x%x\n",
		cdata->cfg.fuse.fuse_id_word1,
		cdata->cfg.fuse.fuse_id_word2,
		cdata->cfg.fuse.fuse_id_word3,
		cdata->cfg.fuse.fuse_id_word4);
	}
	else
	{
		pr_info("s5k5e: first fuse->fuse_id : 0x0 0x%x 0x%x 0x%x\n",
		id_data[3],
		id_data[4],
		id_data[5]);
#ifdef CONFIG_CAMERA_AIT
		front_fusedid[0] = 0;
		front_fusedid[1] = id_data[3];
		front_fusedid[2] = id_data[4];
		front_fusedid[3] = id_data[5];
		front_fuse_id[0] = 0;
		front_fuse_id[1] = id_data[3];
		front_fuse_id[2] = id_data[4];
		front_fuse_id[3] = id_data[5];
#endif
	}
	pr_info("%s: OTP Module vendor = 0x%x\n", __func__,id_data[0]);
	pr_info("%s: OTP LENS = 0x%x\n",          __func__,id_data[1]);
	pr_info("%s: OTP Sensor Version = 0x%x\n",__func__,id_data[2]);

	return rc;
}


static int s5k5e_read_fuseid32(struct sensorb_cfg_data32 *cdata,
	struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	int i,j;
	uint16_t read_data = 0;
	uint16_t id_addr[6] = {0x0A04,0x0A05,0x0A06,0x0A07,0x0A08,0x0A09};
	static uint16_t id_data[6] = {0,0,0,0,0,0};
	static int first = true;
	static int32_t valid_page=-1;

	pr_info("%s called\n", __func__);
	if(first){
		first = false;
		for(j=4;j>=2;j--){
				rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client, 0x0A00, 0x04, MSM_CAMERA_I2C_BYTE_DATA);
				if (rc < 0)
					pr_err("%s: i2c_write failed\n", __func__);
				rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client, 0x0A02, j, MSM_CAMERA_I2C_BYTE_DATA);
				if (rc < 0)
					pr_info("%s: i2c_write failed\n", __func__);
				rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client, 0x0A00, 0x01, MSM_CAMERA_I2C_BYTE_DATA);
				if (rc < 0)
					pr_info("%s: i2c_write failed\n", __func__);
				msleep(10);
				for (i=0; i<6; i++) {
					rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client, id_addr[i], &read_data, MSM_CAMERA_I2C_BYTE_DATA);

					if (rc < 0)
						pr_err("%s: i2c_read 0x%x failed\n", __func__, i);

					id_data[i] = read_data & 0xff;
				}
				if (read_data)
					valid_page = j;
				if (valid_page!=-1)
					break;
		}
	}
	rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client, 0x0A00, 0x04, MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0)
		pr_err("%s: i2c_write failed\n", __func__);
	rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client, 0x0A00, 0x00, MSM_CAMERA_I2C_BYTE_DATA);
	if (rc < 0)
		pr_err("%s: i2c_write failed\n", __func__);

	if (cdata != NULL) {
	cdata->cfg.fuse.fuse_id_word1 = 0;
	cdata->cfg.fuse.fuse_id_word2 = id_data[3];
	cdata->cfg.fuse.fuse_id_word3 = id_data[4];
	cdata->cfg.fuse.fuse_id_word4 = id_data[5];

	cdata->lens_id = id_data[1];

	pr_info("s5k5e: fuse->fuse_id : 0x%x 0x%x 0x%x 0x%x\n",
		cdata->cfg.fuse.fuse_id_word1,
		cdata->cfg.fuse.fuse_id_word2,
		cdata->cfg.fuse.fuse_id_word3,
		cdata->cfg.fuse.fuse_id_word4);
	}
	else
	{
		pr_info("s5k5e: first fuse->fuse_id : 0x0 0x%x 0x%x 0x%x\n",
		id_data[3],
		id_data[4],
		id_data[5]);
#ifdef CONFIG_CAMERA_AIT
		front_fusedid[0] = 0;
		front_fusedid[1] = id_data[3];
		front_fusedid[2] = id_data[4];
		front_fusedid[3] = id_data[5];
		front_fuse_id[0] = 0;
		front_fuse_id[1] = id_data[3];
		front_fuse_id[2] = id_data[4];
		front_fuse_id[3] = id_data[5];
#endif
	}
	pr_info("%s: OTP Module vendor = 0x%x\n", __func__,id_data[0]);
	pr_info("%s: OTP LENS = 0x%x\n",          __func__,id_data[1]);
	pr_info("%s: OTP Sensor Version = 0x%x\n",__func__,id_data[2]);

	return rc;
}
#endif

int32_t s5k5e_htc_sensor_match_id(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	int32_t rc1 = 0;
	static int first = 0;
	rc = msm_sensor_match_id(s_ctrl);
	if(rc == 0) {
		if(first == 0) {
			pr_info("%s read_fuseid\n",__func__);
			#ifdef CONFIG_COMPAT
			rc1 = s5k5e_read_fuseid32(NULL, s_ctrl);
			#else
			rc1 = s5k5e_read_fuseid(NULL, s_ctrl);
			#endif
		}
	}
	#ifdef CONFIG_CAMERA_AIT
	if(first == 0)
	{
	    if(rc == 0)
	        match_front_sensor_ID = 1;
	    else
	        match_front_sensor_ID = 0;
	    pr_info("[CAM] %s match_front_sensor_ID=%d\n",__func__, match_front_sensor_ID);
	    pr_info("[CAM] For s5k5e_yuv s5k5e: fuse->fuse_id : 0x0 0x%x 0x%x 0x%x\n",
		front_fusedid[1],
		front_fusedid[2],
		front_fusedid[3]);
	}
	#endif
	first = 1;
	return rc;
}
static struct msm_sensor_fn_t s5k5e_sensor_func_tbl = {
	.sensor_config = msm_sensor_config,
	.sensor_config32 = msm_sensor_config32,
	.sensor_power_up = s5k5e_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
	.sensor_match_id = s5k5e_htc_sensor_match_id,
	.sensor_i2c_read_fuseid = s5k5e_read_fuseid,
	.sensor_i2c_read_fuseid32 = s5k5e_read_fuseid32,
};

static struct msm_sensor_ctrl_t s5k5e_s_ctrl = {
	.sensor_i2c_client = &s5k5e_sensor_i2c_client,
	.power_setting_array.power_setting = s5k5e_power_setting,
	.power_setting_array.size = ARRAY_SIZE(s5k5e_power_setting),
	.power_setting_array.power_down_setting = s5k5e_power_down_setting,
	.power_setting_array.size_down = ARRAY_SIZE(s5k5e_power_down_setting),
	.msm_sensor_mutex = &s5k5e_mut,
	.sensor_v4l2_subdev_info = s5k5e_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(s5k5e_subdev_info),
	.func_tbl = &s5k5e_sensor_func_tbl
};

module_init(s5k5e_init_module);
module_exit(s5k5e_exit_module);
MODULE_DESCRIPTION("s5k5e_htc");
MODULE_LICENSE("GPL v2");
