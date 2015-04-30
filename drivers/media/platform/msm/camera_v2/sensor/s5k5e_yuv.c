/* Copyright (c) 2013, The Linux Foundation. All rights reserved.
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
#ifdef CONFIG_CAMERA_AIT
#include "AIT.h"
#endif
#define S5K5E_SENSOR_NAME "s5k5e_yuv"
#define PLATFORM_DRIVER_NAME "msm_camera_s5k5e_yuv"
#define s5k5e_obj s5k5e_yuv_##obj

#ifdef CONFIG_CAMERA_AIT
int match_front_sensor_ID = 0;
uint16_t front_fusedid[4] = {0,0,0,0};
#endif

#define CONFIG_MSMB_CAMERA_DEBUG
#undef CDBG
#ifdef CONFIG_MSMB_CAMERA_DEBUG
#define CDBG(fmt, args...) pr_info("[CAM]: " fmt, ##args)
#else
#define CDBG(fmt, args...) do { } while (0)
#endif

DEFINE_MSM_MUTEX(s5k5e_yuv_mut);


#define SENSOR_SUCCESS 0
static struct msm_camera_i2c_client s5k5e_sensor_i2c_client;



static struct msm_sensor_ctrl_t s5k5e_s_ctrl;

static struct msm_sensor_power_setting s5k5e_power_setting[] = {
	{
		.seq_type = SENSOR_I2C_MUX,
		.seq_val = 0,
		.config_val = 0,
		.delay = 0,
	},
};

static struct msm_sensor_power_setting s5k5e_power_down_setting[] = {
	{
		.seq_type = SENSOR_I2C_MUX,
		.seq_val = 0,
		.config_val = 0,
		.delay = 0,
	},
};



static struct v4l2_subdev_info s5k5e_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_YUYV8_2X8,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt = 1,
		.order = 0,
	},
};

static const struct i2c_device_id s5k5e_i2c_id[] = {
	{S5K5E_SENSOR_NAME, (kernel_ulong_t)&s5k5e_s_ctrl},
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
		.name = S5K5E_SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client s5k5e_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static const struct of_device_id s5k5e_dt_match[] = {
	{
		.compatible = "htc,s5k5e_yuv",
		.data = &s5k5e_s_ctrl
	},
	{}
};

MODULE_DEVICE_TABLE(of, s5k5e_dt_match);

static int32_t s5k5e_platform_probe(struct platform_device *pdev)
{
	int32_t rc = 0;
	const struct of_device_id *match;
	match = of_match_device(s5k5e_dt_match, &pdev->dev);
	
	if (!match) {
		pr_err("%s:%d match is NULL\n", __func__, __LINE__);
		return -EINVAL;
	}
	
	rc = msm_sensor_platform_probe(pdev, match->data);
	return rc;
}


static struct platform_driver s5k5e_platform_driver = {
	.driver = {
		.name = "htc,s5k5e_yuv",
		.owner = THIS_MODULE,
		.of_match_table = s5k5e_dt_match,
	},
	.probe = s5k5e_platform_probe,
};

static const char *s5k5eVendor = "Samsung";
static const char *s5k5eNAME = "s5k5e_yuv";
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
	pr_info("s5k5e:kobject creat and add\n");
	android_s5k5e = kobject_create_and_add("android_camera1", NULL);
	if (android_s5k5e == NULL) {
		pr_info("s5k5e_sysfs_init: subsystem_register " \
		"failed\n");
		ret = -ENOMEM;
		return ret ;
	}
	pr_info("s5k5e:sysfs_create_file\n");
	ret = sysfs_create_file(android_s5k5e, &dev_attr_sensor.attr);
	if (ret) {
		pr_info("s5k5e_sysfs_init: sysfs_create_file " \
		"failed\n");
		kobject_del(android_s5k5e);
	}

	return 0 ;
}


static int __init s5k5e_init_module(void)
{
	int32_t rc = 0;
	CDBG("%s:%d\n", __func__, __LINE__);
	rc = platform_driver_register(&s5k5e_platform_driver);
	if (!rc) {
		s5k5e_sysfs_init();
		return rc;
	}
	CDBG("%s:%d rc %d\n", __func__, __LINE__, rc);
	return i2c_add_driver(&s5k5e_i2c_driver);
}

static void __exit s5k5e_exit_module(void)
{
	CDBG("%s:%d\n", __func__, __LINE__);
	if (s5k5e_s_ctrl.pdev) {
		msm_sensor_free_sensor_data(&s5k5e_s_ctrl);
		platform_driver_unregister(&s5k5e_platform_driver);
	} else
		i2c_del_driver(&s5k5e_i2c_driver);
	return;
}

int32_t s5k5e_sensor_config(struct msm_sensor_ctrl_t *s_ctrl,
	void __user *argp)
{
	struct sensorb_cfg_data *cdata = (struct sensorb_cfg_data *)argp;
	long rc = 0;
	int32_t i = 0;
	mutex_lock(s_ctrl->msm_sensor_mutex);
	
	
	switch (cdata->cfgtype) {
	case CFG_GET_SENSOR_INFO:
		memcpy(cdata->cfg.sensor_info.sensor_name,
			s_ctrl->sensordata->sensor_name,
			sizeof(cdata->cfg.sensor_info.sensor_name));
		cdata->cfg.sensor_info.session_id =
			s_ctrl->sensordata->sensor_info->session_id;
		for (i = 0; i < SUB_MODULE_MAX; i++) {
			cdata->cfg.sensor_info.subdev_id[i] =
				s_ctrl->sensordata->sensor_info->subdev_id[i];
			cdata->cfg.sensor_info.subdev_intf[i] =
				s_ctrl->sensordata->sensor_info->subdev_intf[i];
		}
		cdata->cfg.sensor_info.is_mount_angle_valid =
			s_ctrl->sensordata->sensor_info->is_mount_angle_valid;
		cdata->cfg.sensor_info.sensor_mount_angle =
			s_ctrl->sensordata->sensor_info->sensor_mount_angle;
		CDBG("%s:%d sensor name %s\n", __func__, __LINE__,
			cdata->cfg.sensor_info.sensor_name);
		CDBG("%s:%d session id %d\n", __func__, __LINE__,
			cdata->cfg.sensor_info.session_id);
		for (i = 0; i < SUB_MODULE_MAX; i++)
			CDBG("%s:%d subdev_id[%d] %d\n", __func__, __LINE__, i,
				cdata->cfg.sensor_info.subdev_id[i]);
		CDBG("%s:%d mount angle valid %d value %d\n", __func__,
			__LINE__, cdata->cfg.sensor_info.is_mount_angle_valid,
			cdata->cfg.sensor_info.sensor_mount_angle);
		break;
	case CFG_SET_INIT_SETTING:
		break;
	case CFG_SET_RESOLUTION:
		break;
	case CFG_SET_STOP_STREAM:
		break;

	case CFG_SET_START_STREAM:
		CDBG("%s, sensor start stream!!", __func__);
		break;
	case CFG_GET_SENSOR_INIT_PARAMS:
		cdata->cfg.sensor_init_params.modes_supported =
			s_ctrl->sensordata->sensor_info->modes_supported;
		cdata->cfg.sensor_init_params.position =
			s_ctrl->sensordata->sensor_info->position;
		cdata->cfg.sensor_init_params.sensor_mount_angle =
			s_ctrl->sensordata->sensor_info->sensor_mount_angle;
		CDBG("%s:%d init params mode %d pos %d mount %d\n", __func__,
			__LINE__,
			cdata->cfg.sensor_init_params.modes_supported,
			cdata->cfg.sensor_init_params.position,
			cdata->cfg.sensor_init_params.sensor_mount_angle);
		break;
	case CFG_SET_SLAVE_INFO: {
		break;
	}
	case CFG_WRITE_I2C_ARRAY: {
		struct msm_camera_i2c_reg_setting conf_array;
		struct msm_camera_i2c_reg_array *reg_setting = NULL;

		if (copy_from_user(&conf_array,
			(void *)cdata->cfg.setting,
			sizeof(struct msm_camera_i2c_reg_setting))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		reg_setting = kzalloc(conf_array.size *
			(sizeof(struct msm_camera_i2c_reg_array)), GFP_KERNEL);
		if (!reg_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(reg_setting, (void *)conf_array.reg_setting,
			conf_array.size *
			sizeof(struct msm_camera_i2c_reg_array))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(reg_setting);
			rc = -EFAULT;
			break;
		}

		conf_array.reg_setting = reg_setting;
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write_table(
			s_ctrl->sensor_i2c_client, &conf_array);
		kfree(reg_setting);
		break;
	}
	case CFG_WRITE_I2C_SEQ_ARRAY: {
		struct msm_camera_i2c_seq_reg_setting conf_array;
		struct msm_camera_i2c_seq_reg_array *reg_setting = NULL;

		if (copy_from_user(&conf_array,
			(void *)cdata->cfg.setting,
			sizeof(struct msm_camera_i2c_seq_reg_setting))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		reg_setting = kzalloc(conf_array.size *
			(sizeof(struct msm_camera_i2c_seq_reg_array)),
			GFP_KERNEL);
		if (!reg_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(reg_setting, (void *)conf_array.reg_setting,
			conf_array.size *
			sizeof(struct msm_camera_i2c_seq_reg_array))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(reg_setting);
			rc = -EFAULT;
			break;
		}

		conf_array.reg_setting = reg_setting;
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
			i2c_write_seq_table(s_ctrl->sensor_i2c_client,
			&conf_array);
		kfree(reg_setting);
		break;
	}

	case CFG_POWER_UP:
		#ifdef CONFIG_CAMERA_AIT
		rc = AIT_ISP_open_init(CAMERA_INDEX_FRONT_S5K5E); 
		#endif
		if (s_ctrl->func_tbl->sensor_power_up)
			rc = s_ctrl->func_tbl->sensor_power_up(s_ctrl);
		else
			rc = -EFAULT;
		break;

	case CFG_POWER_DOWN:
		if (s_ctrl->func_tbl->sensor_power_down)
			rc = s_ctrl->func_tbl->sensor_power_down(
				s_ctrl);
		else
			rc = -EFAULT;

		#ifdef CONFIG_CAMERA_AIT
			AIT_ISP_release();
		#endif
		break;

	case CFG_SET_STOP_STREAM_SETTING: {
		struct msm_camera_i2c_reg_setting *stop_setting =
			&s_ctrl->stop_setting;
		struct msm_camera_i2c_reg_array *reg_setting = NULL;
		if (copy_from_user(stop_setting, (void *)cdata->cfg.setting,
		    sizeof(struct msm_camera_i2c_reg_setting))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		reg_setting = stop_setting->reg_setting;
		stop_setting->reg_setting = kzalloc(stop_setting->size *
			(sizeof(struct msm_camera_i2c_reg_array)), GFP_KERNEL);
		if (!stop_setting->reg_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(stop_setting->reg_setting,
		    (void *)reg_setting, stop_setting->size *
		    sizeof(struct msm_camera_i2c_reg_array))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(stop_setting->reg_setting);
			stop_setting->reg_setting = NULL;
			stop_setting->size = 0;
			rc = -EFAULT;
			break;
		}
		break;
		}
		case CFG_SET_SATURATION: {
		break;
		}
		case CFG_SET_CONTRAST: {
		break;
		}
		case CFG_SET_SHARPNESS: {
		break;
		}
	case CFG_SET_ISO: {
		break;
	}
		case CFG_SET_EXPOSURE_COMPENSATION: {
			break;
	    }
		case CFG_SET_EFFECT: {
			break;
		}
	case CFG_SET_ANTIBANDING: {
		    break;
	}
	case CFG_SET_ISP_CALIBRATION: {
		int32_t mode = 0;
		mode = cdata->ISP_calibration_mode;
		CDBG("%s:%d %s cfgtype = %d, CFG_SET_ISP_CALIBRATION:mode:%d \n", __func__, __LINE__,
		s_ctrl->sensordata->sensor_name, cdata->cfgtype, mode);
		if(mode == ISP_CALIBRATION_AUTO)
		{
		AIT_ISP_enable_calibration(0, NULL, NULL, 0);
		}
		else if(mode == ISP_CALIBRATION_MANUAL)
		{
			struct ISP_roi_local AE_roi;
			struct ISP_roi_local AWB_roi;
			uint16_t GainBase;
			uint32_t ExposureTimeBase;
			AE_roi.width = cdata->AE_roi.width;
			AE_roi.height = cdata->AE_roi.height;
			AE_roi.offsetx = cdata->AE_roi.offsetx;
			AE_roi.offsety = cdata->AE_roi.offsety;
			AWB_roi.width = cdata->AWB_roi.width;
			AWB_roi.height = cdata->AWB_roi.height;
			AWB_roi.offsetx = cdata->AWB_roi.offsetx;
			AWB_roi.offsety = cdata->AWB_roi.offsety;			
			AIT_ISP_enable_calibration(1, &AE_roi, &AWB_roi, cdata->luma_target);
			AIT_ISP_Get_AE_calibration(&(cdata->isp_aec.Gain), &(GainBase), &(cdata->isp_aec.N_parameter), &(ExposureTimeBase), &(cdata->isp_aec.Luma_Value));
			AIT_ISP_Get_AWB_calibration(&(cdata->isp_awb.R_Sum), &(cdata->isp_awb.G_Sum), &(cdata->isp_awb.B_Sum), &(cdata->isp_awb.pixel_count));
		}
		break;
		}
	case CFG_SET_LUMA_TARGET: {
		CDBG("%s:%d %s cfgtype = %d, CFG_SET_LUMA_TARGET \n", __func__, __LINE__,
		s_ctrl->sensordata->sensor_name, cdata->cfgtype);
		AIT_ISP_set_luma_target(cdata->luma_target);
		break;
		}

	case CFG_SET_BESTSHOT_MODE: {
			break;
		}
		case CFG_SET_AUTOFOCUS: {
		
		pr_debug("%s: Setting Auto Focus", __func__);
		break;
		}
		case CFG_SET_WHITE_BALANCE: {
		break;
		}
		case CFG_CANCEL_AUTOFOCUS: {
		
		pr_debug("%s: Cancelling Auto Focus", __func__);
		break;
		}
	case CFG_I2C_IOCTL_R_OTP:
		if (s_ctrl->func_tbl->sensor_i2c_read_fuseid == NULL) {
			rc = -EFAULT;
			break;
		}
		rc = s_ctrl->func_tbl->sensor_i2c_read_fuseid(cdata, s_ctrl);
	break;
		default:
		rc = -EFAULT;
		break;
	}

	mutex_unlock(s_ctrl->msm_sensor_mutex);

	return rc;
}

int32_t s5k5e_yuv_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
    int32_t status;

    CDBG("%s:+\n", __func__);
    s_ctrl->power_setting_array.power_setting = s5k5e_power_setting;
    s_ctrl->power_setting_array.size = ARRAY_SIZE(s5k5e_power_setting);
    status = msm_sensor_power_up(s_ctrl);
    CDBG("%s:-\n", __func__);
    return status;
}

int32_t s5k5e_yuv_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
    int32_t status;
    int i = 0;
    int j = 0;
    int data_size;
    CDBG("%s: +\n", __func__);
    s_ctrl->power_setting_array.power_setting = s5k5e_power_down_setting;
    s_ctrl->power_setting_array.size = ARRAY_SIZE(s5k5e_power_down_setting);

    
    for(i = 0; i < s_ctrl->power_setting_array.size;  i++)
    {
        data_size = sizeof(s5k5e_power_setting[i].data)/sizeof(void *);
        for (j =0; j < data_size; j++ )
        s5k5e_power_down_setting[i].data[j] = s5k5e_power_setting[i].data[j];
    }
    status = msm_sensor_power_down(s_ctrl);
    CDBG("%s: -\n", __func__);
    return status;
}

#ifdef CONFIG_COMPAT
int32_t s5k5e_sensor_config32(struct msm_sensor_ctrl_t *s_ctrl,
	void __user *argp)
{
	struct sensorb_cfg_data32 *cdata = (struct sensorb_cfg_data32 *)argp;
	long rc = 0;
	int32_t i = 0;
	mutex_lock(s_ctrl->msm_sensor_mutex);
	
	
	switch (cdata->cfgtype) {
	case CFG_GET_SENSOR_INFO:
		CDBG("%s:%d %s cfgtype = %d, CFG_GET_SENSOR_INFO+\n", __func__, __LINE__,
		s_ctrl->sensordata->sensor_name, cdata->cfgtype);
		memcpy(cdata->cfg.sensor_info.sensor_name,
			s_ctrl->sensordata->sensor_name,
			sizeof(cdata->cfg.sensor_info.sensor_name));
		cdata->cfg.sensor_info.session_id =
			s_ctrl->sensordata->sensor_info->session_id;
		for (i = 0; i < SUB_MODULE_MAX; i++) {
			cdata->cfg.sensor_info.subdev_id[i] =
				s_ctrl->sensordata->sensor_info->subdev_id[i];
			cdata->cfg.sensor_info.subdev_intf[i] =
				s_ctrl->sensordata->sensor_info->subdev_intf[i];
		}
		cdata->cfg.sensor_info.is_mount_angle_valid =
			s_ctrl->sensordata->sensor_info->is_mount_angle_valid;
		cdata->cfg.sensor_info.sensor_mount_angle =
			s_ctrl->sensordata->sensor_info->sensor_mount_angle;
		cdata->cfg.sensor_info.position =
			s_ctrl->sensordata->sensor_info->position;
		cdata->cfg.sensor_info.modes_supported =
			s_ctrl->sensordata->sensor_info->modes_supported;
		CDBG("%s:%d sensor name %s\n", __func__, __LINE__,
			cdata->cfg.sensor_info.sensor_name);
		CDBG("%s:%d session id %d\n", __func__, __LINE__,
			cdata->cfg.sensor_info.session_id);
		for (i = 0; i < SUB_MODULE_MAX; i++)
			CDBG("%s:%d subdev_id[%d] %d\n", __func__, __LINE__, i,
				cdata->cfg.sensor_info.subdev_id[i]);
		CDBG("%s:%d mount angle valid %d value %d\n", __func__,
			__LINE__, cdata->cfg.sensor_info.is_mount_angle_valid,
			cdata->cfg.sensor_info.sensor_mount_angle);
		
		CDBG("%s:%d %s cfgtype = %d, CFG_GET_SENSOR_INFO-\n", __func__, __LINE__,
		s_ctrl->sensordata->sensor_name, cdata->cfgtype);
		break;
	case CFG_SET_INIT_SETTING:
		CDBG("%s:%d %s cfgtype = %d, CFG_SET_INIT_SETTING \n", __func__, __LINE__,
		s_ctrl->sensordata->sensor_name, cdata->cfgtype);
		#if 1
		AIT_ISP_sensor_init();
		#endif
		break;
	case CFG_SET_RESOLUTION:
	{
		int res = 0;
		CDBG("%s:%d %s cfgtype = %d, CFG_SET_RESOLUTION \n", __func__, __LINE__,
		s_ctrl->sensordata->sensor_name, cdata->cfgtype);
		if (copy_from_user(&res,
			(void *)compat_ptr(cdata->cfg.setting),
			sizeof(int))) {
			CDBG("%s:%d CFG_SET_RESOLUTION failed\n", __func__, __LINE__);
		}
		CDBG("%s: res:%d\n", __func__, res);
		if(res == 0)
		AIT_ISP_sensor_set_resolution(2560, 1920);
		else if(res == 1)
		AIT_ISP_sensor_set_resolution(2560, 1440);
		else if(res == 2)
		AIT_ISP_sensor_set_resolution(1920, 1080);
		else if(res == 3)
		AIT_ISP_sensor_set_resolution(1280, 960);
		else if(res == 4)
		AIT_ISP_sensor_set_resolution(800, 600);
		else
		AIT_ISP_sensor_set_resolution(2560, 1920);
	}
		break;
	case CFG_SET_STOP_STREAM:
		CDBG("%s:%d %s cfgtype = %d, CFG_SET_STOP_STREAM \n", __func__, __LINE__,
		s_ctrl->sensordata->sensor_name, cdata->cfgtype);
		#if 1
		AIT_ISP_sensor_stop_stream();
		#endif
		break;
	case CFG_SET_START_STREAM:
		CDBG("%s:%d %s cfgtype = %d, CFG_SET_START_STREAM \n", __func__, __LINE__,
		s_ctrl->sensordata->sensor_name, cdata->cfgtype);
		#if 1
		AIT_ISP_sensor_start_stream();
		#endif
		break;
	case CFG_GET_SENSOR_INIT_PARAMS:
		CDBG("%s:%d %s cfgtype = %d, CFG_GET_SENSOR_INIT_PARAMS \n", __func__, __LINE__,
		s_ctrl->sensordata->sensor_name, cdata->cfgtype);
		break;
	case CFG_SET_SLAVE_INFO: {
		CDBG("%s:%d %s cfgtype = %d, CFG_SET_SLAVE_INFO \n", __func__, __LINE__,
		s_ctrl->sensordata->sensor_name, cdata->cfgtype);
		break;
	}
	case CFG_WRITE_I2C_ARRAY: {
		struct msm_camera_i2c_reg_setting32 conf_array32;
		struct msm_camera_i2c_reg_setting conf_array;
		struct msm_camera_i2c_reg_array *reg_setting = NULL;
		CDBG("%s:%d %s cfgtype = %d, CFG_WRITE_I2C_ARRAY + remove I2C\n", __func__, __LINE__,
		s_ctrl->sensordata->sensor_name, cdata->cfgtype);
		if (copy_from_user(&conf_array32,
			(void *)compat_ptr(cdata->cfg.setting),
			sizeof(struct msm_camera_i2c_reg_setting32))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -EFAULT;
			break;
		}

		conf_array.addr_type = conf_array32.addr_type;
		conf_array.data_type = conf_array32.data_type;
		conf_array.delay = conf_array32.delay;
		conf_array.size = conf_array32.size;
		conf_array.reg_setting = compat_ptr(conf_array32.reg_setting);

		reg_setting = kzalloc(conf_array.size *
			(sizeof(struct msm_camera_i2c_reg_array)), GFP_KERNEL);
		if (!reg_setting) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			rc = -ENOMEM;
			break;
		}
		if (copy_from_user(reg_setting, (void *)conf_array.reg_setting,
			conf_array.size *
			sizeof(struct msm_camera_i2c_reg_array))) {
			pr_err("%s:%d failed\n", __func__, __LINE__);
			kfree(reg_setting);
			rc = -EFAULT;
			break;
		}

		conf_array.reg_setting = reg_setting;
		
		
		kfree(reg_setting);
		CDBG("%s:%d %s cfgtype = %d, CFG_WRITE_I2C_ARRAY -\n", __func__, __LINE__,
		s_ctrl->sensordata->sensor_name, cdata->cfgtype);
		break;

	}
	case CFG_POWER_UP:
		CDBG("%s:%d %s cfgtype = %d, CFG_POWER_UP+\n", __func__, __LINE__,
		s_ctrl->sensordata->sensor_name, cdata->cfgtype);
		#ifdef CONFIG_CAMERA_AIT
		rc = AIT_ISP_open_init(CAMERA_INDEX_FRONT_S5K5E); 
		#endif
		if (s_ctrl->func_tbl->sensor_power_up)
			rc = s_ctrl->func_tbl->sensor_power_up(s_ctrl);
		else
			rc = -EFAULT;
		CDBG("%s:%d %s cfgtype = %d, CFG_POWER_UP-\n", __func__, __LINE__,
		s_ctrl->sensordata->sensor_name, cdata->cfgtype);
		break;

	case CFG_POWER_DOWN:
		CDBG("%s:%d %s cfgtype = %d, CFG_POWER_DOWN+\n", __func__, __LINE__,
		s_ctrl->sensordata->sensor_name, cdata->cfgtype);
		if (s_ctrl->func_tbl->sensor_power_down)
			rc = s_ctrl->func_tbl->sensor_power_down(
				s_ctrl);
		else
			rc = -EFAULT;
		#ifdef CONFIG_CAMERA_AIT
			AIT_ISP_release();
		#endif
		CDBG("%s:%d %s cfgtype = %d, CFG_POWER_DOWN-\n", __func__, __LINE__,
		s_ctrl->sensordata->sensor_name, cdata->cfgtype);
		break;

		case CFG_SET_SATURATION: {

		break;
		}
		case CFG_SET_CONTRAST: {

		break;
		}
		case CFG_SET_SHARPNESS: {

		break;
		}
	case CFG_SET_ISO: {

		break;
	}
		case CFG_SET_EXPOSURE_COMPENSATION: {
			break;
	    }
		case CFG_SET_EFFECT: {
			break;
		}
	case CFG_SET_ANTIBANDING: {

		    break;
	}
	case CFG_SET_ISP_CALIBRATION: {
		int32_t mode = 0;
		mode = cdata->ISP_calibration_mode;
		CDBG("%s:%d %s cfgtype = %d, CFG_SET_ISP_CALIBRATION:mode:%d \n", __func__, __LINE__,
		s_ctrl->sensordata->sensor_name, cdata->cfgtype, mode);
		if(mode == ISP_CALIBRATION_AUTO)
		{
		AIT_ISP_enable_calibration(0, NULL, NULL, 0);
		}
		else if(mode == ISP_CALIBRATION_MANUAL)
		{
			struct ISP_roi_local AE_roi;
			struct ISP_roi_local AWB_roi;
			uint16_t GainBase;
			uint32_t ExposureTimeBase;
			AE_roi.width = cdata->AE_roi.width;
			AE_roi.height = cdata->AE_roi.height;
			AE_roi.offsetx = cdata->AE_roi.offsetx;
			AE_roi.offsety = cdata->AE_roi.offsety;
			AWB_roi.width = cdata->AWB_roi.width;
			AWB_roi.height = cdata->AWB_roi.height;
			AWB_roi.offsetx = cdata->AWB_roi.offsetx;
			AWB_roi.offsety = cdata->AWB_roi.offsety;			
			AIT_ISP_enable_calibration(1, &AE_roi, &AWB_roi, cdata->luma_target);
			AIT_ISP_Get_AE_calibration(&(cdata->isp_aec.Gain), &(GainBase), &(cdata->isp_aec.N_parameter), &(ExposureTimeBase), &(cdata->isp_aec.Luma_Value));
			AIT_ISP_Get_AWB_calibration(&(cdata->isp_awb.R_Sum), &(cdata->isp_awb.G_Sum), &(cdata->isp_awb.B_Sum), &(cdata->isp_awb.pixel_count));
		}
		break;
		}
	case CFG_SET_LUMA_TARGET: {
		CDBG("%s:%d %s cfgtype = %d, CFG_SET_LUMA_TARGET \n", __func__, __LINE__,
		s_ctrl->sensordata->sensor_name, cdata->cfgtype);
		AIT_ISP_set_luma_target(cdata->luma_target);
		break;
		}
	case CFG_SET_BESTSHOT_MODE: {

			break;
		}
		case CFG_SET_AUTOFOCUS: {
		
		pr_debug("%s: Setting Auto Focus", __func__);
		break;
		}
		case CFG_SET_WHITE_BALANCE: {

		break;
		}
		case CFG_CANCEL_AUTOFOCUS: {
		
		pr_debug("%s: Cancelling Auto Focus", __func__);
		break;
		}
	case CFG_I2C_IOCTL_R_OTP:
		if (s_ctrl->func_tbl->sensor_i2c_read_fuseid32 == NULL) {
			rc = -EFAULT;
			pr_err("%s:%d CFG_I2C_IOCTL_R_OTP fail-\n", __func__, __LINE__);
			break;
		}
		rc = s_ctrl->func_tbl->sensor_i2c_read_fuseid32(cdata, s_ctrl);
	        break;
		default:
		rc = -EFAULT;
		break;
	}

	mutex_unlock(s_ctrl->msm_sensor_mutex);

	return rc;
}
#endif

static int s5k5e_yuv_read_fuseid(struct sensorb_cfg_data *cdata,
	struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;

	cdata->cfg.fuse.fuse_id_word1 = 0;
	cdata->cfg.fuse.fuse_id_word2 = 0;
	cdata->cfg.fuse.fuse_id_word3 = 0;
	cdata->cfg.fuse.fuse_id_word4 = 0;

#ifdef CONFIG_CAMERA_AIT
	cdata->cfg.fuse.fuse_id_word1 = front_fusedid[0];
	cdata->cfg.fuse.fuse_id_word2 = front_fusedid[1];
	cdata->cfg.fuse.fuse_id_word3 = front_fusedid[2];
	cdata->cfg.fuse.fuse_id_word4 = front_fusedid[3];
#endif
	pr_info("[CAM]s5k5e_read_fuseid: fuse->fuse_id : 0x%x 0x%x 0x%x 0x%x\n",
		cdata->cfg.fuse.fuse_id_word1,
		cdata->cfg.fuse.fuse_id_word2,
		cdata->cfg.fuse.fuse_id_word3,
		cdata->cfg.fuse.fuse_id_word4);

	return rc;
}
static int s5k5e_yuv_read_fuseid32(struct sensorb_cfg_data32 *cdata,
	struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;

	cdata->cfg.fuse.fuse_id_word1 = 0;
	cdata->cfg.fuse.fuse_id_word2 = 0;
	cdata->cfg.fuse.fuse_id_word3 = 0;
	cdata->cfg.fuse.fuse_id_word4 = 0;

#ifdef CONFIG_CAMERA_AIT
	cdata->cfg.fuse.fuse_id_word1 = front_fusedid[0];
	cdata->cfg.fuse.fuse_id_word2 = front_fusedid[1];
	cdata->cfg.fuse.fuse_id_word3 = front_fusedid[2];
	cdata->cfg.fuse.fuse_id_word4 = front_fusedid[3];
#endif
	pr_info("[CAM]s5k5e_read_fuseid32: fuse->fuse_id : 0x%x 0x%x 0x%x 0x%x\n",
		cdata->cfg.fuse.fuse_id_word1,
		cdata->cfg.fuse.fuse_id_word2,
		cdata->cfg.fuse.fuse_id_word3,
		cdata->cfg.fuse.fuse_id_word4);

	return rc;
}
int32_t s5k5e_yuv_sensor_match_id(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	CDBG("%s \n",__func__);

#ifdef CONFIG_CAMERA_AIT
	if(match_front_sensor_ID == 1)
	{
	    rc = 0;
	    CDBG("%s match ok\n",__func__);
	}
	else
	{
	    rc = -1;
	    CDBG("%s match fail\n",__func__);
	}
#endif
	return rc;
}
static struct msm_sensor_fn_t sensor_func_tbl = {
	.sensor_config = s5k5e_sensor_config,
#ifdef CONFIG_COMPAT
	.sensor_config32 = s5k5e_sensor_config32,
#endif
	.sensor_power_up = s5k5e_yuv_sensor_power_up,
	.sensor_power_down = s5k5e_yuv_sensor_power_down,
	.sensor_match_id = s5k5e_yuv_sensor_match_id,
	.sensor_i2c_read_fuseid = s5k5e_yuv_read_fuseid,
	.sensor_i2c_read_fuseid32 = s5k5e_yuv_read_fuseid32,
};

static struct msm_sensor_ctrl_t s5k5e_s_ctrl = {
	.sensor_i2c_client = &s5k5e_sensor_i2c_client,
	.power_setting_array.power_setting = s5k5e_power_setting,
	.power_setting_array.size = ARRAY_SIZE(s5k5e_power_setting),
	.power_setting_array.power_down_setting = s5k5e_power_down_setting,
	.power_setting_array.size_down = ARRAY_SIZE(s5k5e_power_down_setting),
	.msm_sensor_mutex = &s5k5e_yuv_mut,
	.sensor_v4l2_subdev_info = s5k5e_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(s5k5e_subdev_info),
	.func_tbl = &sensor_func_tbl,
};

module_init(s5k5e_init_module);
module_exit(s5k5e_exit_module);
MODULE_DESCRIPTION("s5k5e_yuv");
MODULE_LICENSE("GPL v2");
