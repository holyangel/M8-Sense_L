/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
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

#include "AIT.h"
#include "AIT_spi.h"
#include <linux/errno.h>
#include "AIT_ISP_API.h"
#include "AIT9882_sub_FW.h"
#include "AIT9882_front_FW.h"
#include "AIT_ISP_System.h"
#define MSM_AIT_ISP_NAME "AIT"

#define AIT_PINCTRL_STATE_SLEEP "AIT_suspend"
#define AIT_PINCTRL_STATE_DEFAULT "AIT_default"
#define AIT_INT

#define AWB_CAL_MAX_SIZE	0x5000U 
extern unsigned char cam_awb_ram[AWB_CAL_MAX_SIZE];

struct qct_isp_struct{
    uint32_t verify;  
    uint32_t isp_caBuff[445];  
    uint32_t fuse_id[4];
    uint32_t check_sum;
};

struct qct_isp_struct *pfront_calibration_data;
struct qct_isp_struct *psub_calibration_data;
int load_calibration_data = 0;
uint16_t sub_fuse_id[4] = {0,0,0,0};
uint16_t front_fuse_id[4] = {0,0,0,0};

uint16_t Ytransform[2][256]={
{ 0,     1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15, 
16,   17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31, 
32,   33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47, 
48,   49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63, 
64,   65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79, 
80,   81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95, 
96,   97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,                               
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,                               
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,                               
160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,                               
176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,                               
192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,                               
208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,                               
224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,                               
240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
},                              
{ 0,   1,   2,   3,   5,   7,   9,  11,  14,  16,  19,  22,  25,  28,  31,  34, 
 37,  39,  42,  45,  48,  50,  53,  55,  58,  60,  63,  66,  69,  71,  73,  75, 
 78,  80,  82,  84,  87,  89,  91,  93,  96,  97,  99, 101, 103, 104, 106, 108, 
110, 111, 113, 115, 117, 118, 120, 121, 123, 124, 126, 127, 129, 130, 131, 132, 
134, 135, 136, 137, 139, 140, 141, 142, 144, 145, 146, 147, 148, 149, 150, 151, 
153, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 164, 165, 166, 
167, 167, 168, 169, 170, 170, 171, 172, 173, 173, 174, 175, 176, 176, 177, 178, 
179, 179, 180, 181, 182, 182, 183, 183, 184, 184, 185, 186, 187, 187, 188, 188, 
189, 189, 190, 191, 192, 192, 193, 194, 195, 195, 196, 196, 197, 197, 198, 199, 
200, 200, 201, 201, 202, 202, 203, 204, 205, 205, 206, 206, 207, 207, 208, 208, 
209, 209, 210, 211, 212, 212, 213, 213, 214, 214, 215, 215, 216, 216, 217, 218, 
219, 219, 220, 220, 221, 221, 222, 222, 223, 223, 224, 224, 225, 225, 226, 226, 
227, 227, 228, 228, 229, 229, 230, 230, 231, 231, 232, 232, 233, 233, 234, 234, 
235, 235, 235, 235, 236, 236, 237, 237, 238, 238, 239, 239, 240, 240, 241, 241, 
242, 242, 242, 242, 243, 243, 244, 244, 245, 245, 246, 246, 247, 247, 247, 247, 
248, 248, 249, 249, 250, 250, 251, 251, 252, 252, 252, 252, 253, 253, 254, 255
}
}; 
#undef CDBG
#define CONFIG_AIT_DEBUG
#ifdef CONFIG_AIT_DEBUG
#define CDBG(fmt, args...) pr_info("[CAM][AIT] : " fmt, ##args)
#else
#define CDBG(fmt, args...) do { } while (0)
#endif


static struct class *AIT_ISP_class;
static dev_t AIT_ISP_devno;

#ifdef AIT_INT
int AIT_intr0;
atomic_t interrupt;
struct AIT_ISP_int_t AIT_ISP_int;
#endif

struct AIT_ISP_int_t {
	spinlock_t AIT_spin_lock;
	wait_queue_head_t AIT_wait;
};

static struct AIT_ISP_ctrl *AIT_ISPCtrl = NULL;
static struct camera_vreg_t ISP_vreg[3];
static void *ISP_vreg_data[3];
unsigned char *pAIT_fw_sub=AIT_fw_sub;
unsigned char *pAIT_fw_front=AIT_fw_front;
static int camera_index = CAMERA_INDEX_SUB_OV2722;
static struct msm_cam_clk_info ISP_clk_info[] = {
	[SENSOR_CAM_MCLK] = {"cam_src_clk", 24000000},
	[SENSOR_CAM_CLK] = {"cam_clk", 0},
};
static int AIT_ISP_fops_open(struct inode *inode, struct file *filp)
{
	struct AIT_ISP_ctrl *raw_dev = container_of(inode->i_cdev,
		struct AIT_ISP_ctrl, cdev);
	filp->private_data = raw_dev;
	return 0;
}
#ifdef AIT_INT
int AIT_ISP_got_INT0(void __user *argp){
	
	uint32_t InterruptCase = 0;
	AIT_ISP_check_interrupt(&InterruptCase);
	CDBG("%s AIT_ISP_got_INT0 AIT_ISP_check_interrupt:%u \n", __func__, InterruptCase);
	enable_irq(AIT_intr0);
	if(copy_to_user((void *)argp, &InterruptCase, sizeof(uint32_t))){
		pr_err("[CAM]%s, copy to user error\n", __func__);
		return -EFAULT;
	}
	return 0;
}

int AIT_ISP_parse_interrupt(void __user *argp){

	int rc = 0;
	if (atomic_read(&AIT_ISPCtrl->check_intr0)) {
		atomic_set(&AIT_ISPCtrl->check_intr0, 0);
		rc = AIT_ISP_got_INT0(argp);
	}

	return rc;
}

static unsigned int AIT_ISP_fops_poll(struct file *filp,
	struct poll_table_struct *pll_table)
{
	int rc = 0;
	unsigned long flags;
	poll_wait(filp, &AIT_ISP_int.AIT_wait, pll_table);
	spin_lock_irqsave(&AIT_ISP_int.AIT_spin_lock, flags);
	if (atomic_read(&interrupt)) {
		atomic_set(&interrupt, 0);
		atomic_set(&AIT_ISPCtrl->check_intr0, 1);
		rc = POLLIN | POLLRDNORM;
	}

	spin_unlock_irqrestore(&AIT_ISP_int.AIT_spin_lock, flags);
	return rc;
}



static long AIT_ISP_fops_ioctl(struct file *filp, unsigned int cmd,
	unsigned long arg)
{
	int rc = 0;
	struct AIT_ISP_ctrl *raw_dev = filp->private_data;
	void __user *argp = (void __user *)arg;
	CDBG("%s:%d cmd = %d +\n", __func__, __LINE__, _IOC_NR(cmd));
	mutex_lock(&raw_dev->AIT_ioctl_lock);
	switch (_IOC_NR(cmd)) {
	case IOCTL_AIT_ISP_GET_INT:
		rc = AIT_ISP_parse_interrupt(argp);
		if(rc < 0)
			pr_info("%s:%d cmd = %d, rc =%d\n", __func__, __LINE__, _IOC_NR(cmd), rc);
		break;
	case IOCTL_AIT_ISP_ENABLE_INT:
	{
		uint32_t InterruptCase = 0;
	        if(copy_from_user(&InterruptCase, argp,
			sizeof(uint32_t))){
			pr_err("[CAM] IOCTL_AIT_ISP_ENABLE_INT copy from user error\n");
			rc = -EFAULT;
			        
	        }
	        else
	        {
	        	CDBG("%s + IOCTL_AIT_ISP_ENABLE_INT: InterruptCase:%u\n",__func__, InterruptCase);
	        	AIT_ISP_enable_interrupt(InterruptCase);
	        	CDBG("%s AIT_ISP_enable_interrupt  done\n ", __func__);
	        }

	}
	break;

	case IOCTL_AIT_ISP_EN_MEC_MWB:
	{
		uint32_t manual_EC_EW = 0;
		int disableAWB=0, disableAEC=0;
	        if(copy_from_user(&manual_EC_EW, argp,
			sizeof(uint32_t))){
			pr_err("[CAM] IOCTL_AIT_ISP_EN_MEC_MWB copy from user error\n");
			rc = -EFAULT;
			        
	        }
	        else
	        {
	        	disableAEC = (manual_EC_EW%2);
	        	disableAWB = (manual_EC_EW/2)%2;
	        	CDBG("%s + IOCTL_AIT_ISP_EN_MEC_MWB: manual_EC_EW:%u, disableAEC:%d , disableAWB:%d\n",__func__, manual_EC_EW, disableAEC, disableAWB);
	        	AIT_ISP_enable_MEC_MWB(disableAWB, disableAEC);
	        	CDBG("%s AIT_ISP_enable_MEC_MWB  done\n ", __func__);
	        }

	}
	break;

	case IOCTL_AIT_ISP_CONFIG_MEC:
	{
		MECData mec_data;
	        if(copy_from_user(&mec_data, argp,
			sizeof(MECData))){
			pr_err("[CAM] IOCTL_AIT_ISP_CONFIG_MEC copy from user error\n");
			rc = -EFAULT;
			        
	        }
	        else
	        {
	        	CDBG("%s + mec_data.N_parameter:%u, mec_data.Overall_Gain:%d\n",__func__, mec_data.N_parameter, mec_data.Overall_Gain);
	        	AIT_ISP_config_MEC(mec_data.N_parameter, mec_data.Overall_Gain);
	        	CDBG("%s AIT_ISP_config_MEC  done\n ", __func__);
	        }
	}
	break;
	case IOCTL_AIT_ISP_CONFIG_MWB:
	{
		MWBData mwb_data;
	        if(copy_from_user(&mwb_data, argp,
			sizeof(MWBData))){
			pr_err("[CAM] IOCTL_AIT_ISP_CONFIG_MWB copy from user error\n");
			rc = -EFAULT;
			        
	        }
	        else
	        {
	        	CDBG("%s + RGBGain:(%d, %d, %d)\n",__func__, mwb_data.R_Gain, mwb_data.G_Gain, mwb_data.B_Gain);
	        	AIT_ISP_config_MWB(mwb_data.R_Gain, mwb_data.G_Gain, mwb_data.B_Gain);
	        	CDBG("%s AIT_ISP_config_MWB  done\n ", __func__);
	        }
	}
	break;
	case IOCTL_AIT_ISP_GET_MEC:
	{
		MECData mec_data;
	        uint8_t ret = 0;
	        uint16_t GainBase = 0;
	        uint32_t ExposureTimeBase = 0;
	        ret = VA_GetOverallGain(&(mec_data.Overall_Gain), &GainBase);
	        CDBG("%s + VA_GetOverallGain done, mec_data.Overall_Gain:%d\n",__func__, mec_data.Overall_Gain);
	        ret = VA_GetExposureTime(&(mec_data.N_parameter), &ExposureTimeBase);
	        CDBG("%s + VA_GetExposureTime done,mec_data.N_parameter:%u\n",__func__, mec_data.N_parameter);

	        if(copy_to_user((void *)argp, &mec_data, sizeof(MECData))){
		pr_err("[CAM]%s, IOCTL_AIT_ISP_GET_MEC copy to user error\n", __func__);
		rc = -EFAULT;
	        }
	}
	break;
	case IOCTL_AIT_ISP_GET_MWB:
	{
		MWBData mwb_data;
	        uint16 Gain_Base = 0;
	        uint8_t ret = 0;
	        ret = VA_GetRGBGain (&(mwb_data.R_Gain), &(mwb_data.G_Gain), &(mwb_data.B_Gain), &Gain_Base);
	        CDBG("%s + VA_GetRGBGain (%d, %d, %d)\n",__func__, mwb_data.R_Gain, mwb_data.G_Gain, mwb_data.B_Gain);

	        if(copy_to_user((void *)argp, &mwb_data, sizeof(MWBData))){
		pr_err("[CAM]%s, IOCTL_AIT_ISP_GET_MWB copy to user error\n", __func__);
		rc = -EFAULT;
	        }
	}
	break;
	case IOCTL_AIT_ISP_GET_CUSTOM_AE_ROI_LUMA:
	{
	        uint8_t ret = 0;
	        uint16_t Luma_Value = 0;
	        ret = VA_GetCustomAEROILuma(&Luma_Value);
	        CDBG("%s + VA_GetCustomAEROILuma:%d\n",__func__, Luma_Value);
	        if(copy_to_user((void *)argp, &Luma_Value, sizeof(uint16_t))){
		pr_err("[CAM]%s, IOCTL_AIT_ISP_GET_MEC copy to user error\n", __func__);
		rc = -EFAULT;
	        }
	}
	break;
	case IOCTL_AIT_ISP_GET_AWB_ROI_ACC:
	{
		AWB_ROI_ACC awb_roi_data;
	        uint8_t ret = 0;
	        ret = VA_GetAWBROIACC (&(awb_roi_data.R_Sum), &(awb_roi_data.G_Sum), &(awb_roi_data.B_Sum), &(awb_roi_data.pixel_count));
	        CDBG("%s + VA_GetAWBROIACC (%u, %u, %u, %u)\n",__func__, awb_roi_data.R_Sum, awb_roi_data.G_Sum, awb_roi_data.B_Sum, awb_roi_data.pixel_count);

	        if(copy_to_user((void *)argp, &awb_roi_data, sizeof(AWB_ROI_ACC))){
		pr_err("[CAM]%s, IOCTL_AIT_ISP_GET_AWB_ROI_ACC copy to user error\n", __func__);
		rc = -EFAULT;
	        }
	}
	break;
	case IOCTL_AIT_ISP_SET_AE_ROI_POSITION:
	{
		uint8_t ret = 0;
		ROI_position roi_data;
	        if(copy_from_user(&roi_data, argp,
			sizeof(ROI_position))){
			pr_err("[CAM] IOCTL_AIT_ISP_SET_AE_ROI_POSITION copy from user error\n");
			rc = -EFAULT;
			        
	        }
	        else
	        {
	        	CDBG("%s + AE ROI:(%d, %d, %d, %d)\n",__func__, roi_data.width, roi_data.height, roi_data.offsetx, roi_data.offsety);
	        	ret = VA_SetAEROIPosition(roi_data.width, roi_data.height, roi_data.offsetx, roi_data.offsety);
	        	CDBG("%s VA_SetAEROIPosition  done(%d)\n ", __func__, ret);
	        }
	}
	break;
	case IOCTL_AIT_ISP_SET_AWB_ROI_POSITION:
	{
		uint8_t ret = 0;
		ROI_position roi_data;
	        if(copy_from_user(&roi_data, argp,
			sizeof(ROI_position))){
			pr_err("[CAM] IOCTL_AIT_ISP_SET_AWB_ROI_POSITION copy from user error\n");
			rc = -EFAULT;
			        
	        }
	        else
	        {
	        	CDBG("%s + AE ROI:(%d, %d, %d, %d)\n",__func__, roi_data.width, roi_data.height, roi_data.offsetx, roi_data.offsety);
	        	ret = VA_SetAWBROIPosition(roi_data.width, roi_data.height, roi_data.offsetx, roi_data.offsety);
	        	CDBG("%s VA_SetAWBROIPosition  done(%d)\n ", __func__, ret);
	        }
	}
	break;
	case IOCTL_AIT_ISP_CUSTOM_AE_ROI_MODE_ENABLE:
	{
		uint8_t ret = 0;
		int enable = 0;
	        if(copy_from_user(&enable, argp,
			sizeof(int))){
			pr_err("[CAM] IOCTL_AIT_ISP_CUSTOM_AE_ROI_MODE_ENABLE copy from user error\n");
			rc = -EFAULT;
			        
	        }
	        else
	        {
	        	CDBG("%s + IOCTL_AIT_ISP_CUSTOM_AE_ROI_MODE_ENABLE:(%d)\n",__func__, enable);
	        	if(enable == 0)
	        	ret =VA_CustomAEROIModeEnable(ISP_CUSTOM_WIN_MODE_OFF);
	        	else
	        	ret =VA_CustomAEROIModeEnable(ISP_CUSTOM_WIN_MODE_ON);
	        	CDBG("%s VA_CustomAEROIModeEnable  done(%d)\n ", __func__, ret);
	        }
	}
	break;
	case IOCTL_AIT_ISP_SET_LUMA_FORMULA:
	{
		uint8_t ret = 0;
		Luma_formula luma_formula_data;
	        if(copy_from_user(&luma_formula_data, argp,
			sizeof(Luma_formula))){
			pr_err("[CAM] IOCTL_AIT_ISP_SET_LUMA_FORMULA copy from user error\n");
			rc = -EFAULT;
			        
	        }
	        else
	        {
	        	CDBG("%s + IOCTL_AIT_ISP_SET_LUMA_FORMULA:(%d, %d, %d, %d)\n",__func__, luma_formula_data.paraR, luma_formula_data.paraGr, luma_formula_data.paraGb, luma_formula_data.paraB);
	        	ret = VA_SetLumaFormula(luma_formula_data.paraR, luma_formula_data.paraGr, luma_formula_data.paraGb, luma_formula_data.paraB);
	        	CDBG("%s VA_SetLumaFormula  done(%d)\n ", __func__, ret);
	        }
	}
	break;
	case IOCTL_AIT_ISP_SET_AE_ROI_METERING_TABLE:
	{
		uint8_t ret = 0;
		uint8_t table[128];
	        if(copy_from_user(table, argp,
			128)){
			pr_err("[CAM] IOCTL_AIT_ISP_SET_AE_ROI_METERING_TABLE copy from user error\n");
			rc = -EFAULT;
			        
	        }
	        else
	        {
	        	CDBG("%s + IOCTL_AIT_ISP_SET_AE_ROI_METERING_TABLE +\n",__func__);
	        	ret = VA_SetAEROIMeteringTable(table);
	        	CDBG("%s IOCTL_AIT_ISP_SET_AE_ROI_METERING_TABLE  done(%d)\n ", __func__, ret);
	        }
	}
	break;
	case IOCTL_AIT_ISP_SET_CUSTOM_AE_ROI_WEIGHT:
	{
		uint8_t ret = 0;
		uint16_t weight;
	        if(copy_from_user(&weight, argp,
			sizeof(uint16_t))){
			pr_err("[CAM] IOCTL_AIT_ISP_SET_CUSTOM_AE_ROI_WEIGHT copy from user error\n");
			rc = -EFAULT;
			        
	        }
	        else
	        {
	        	CDBG("%s + IOCTL_AIT_ISP_SET_CUSTOM_AE_ROI_WEIGHT weight:%d\n",__func__, weight);
	        	ret = VA_SetCustomAEROIWeight(weight);
	        	CDBG("%s IOCTL_AIT_ISP_SET_CUSTOM_AE_ROI_WEIGHT done(%d)\n ", __func__, ret);
	        }
	}
	break;
	case IOCTL_AIT_ISP_SET_CUSTOM_AE_ROI_POSITION:
	{
		uint8_t ret = 0;
		custom_roi_position cus_roi_pos;
	        if(copy_from_user(&cus_roi_pos, argp,
			sizeof(custom_roi_position))){
			pr_err("[CAM] IOCTL_AIT_ISP_SET_CUSTOM_AE_ROI_POSITION copy from user error\n");
			rc = -EFAULT;
			        
	        }
	        else
	        {
	        	CDBG("%s + IOCTL_AIT_ISP_SET_CUSTOM_AE_ROI_POSITION:(%d, %d, %d, %d)\n",__func__, cus_roi_pos.Start_X_Index, cus_roi_pos.End_X_Index, cus_roi_pos.Start_Y_Index, cus_roi_pos.End_Y_Index);
	        	ret = VA_SetCustomAEROIPosition(cus_roi_pos.Start_X_Index, cus_roi_pos.End_X_Index, cus_roi_pos.Start_Y_Index, cus_roi_pos.End_Y_Index);
	        	CDBG("%s IOCTL_AIT_ISP_SET_CUSTOM_AE_ROI_POSITION  done(%d)\n ", __func__, ret);
	        }
	}
	break;
	case IOCTL_AIT_ISP_GET_AE_ROI_CURRENT_LUMA:
	{
	        uint32_t luma = 0;
	        uint8_t ret = 0;
	        ret = VA_GetAEROICurrentLuma(&luma);
	        CDBG("%s IOCTL_AIT_ISP_GET_AE_ROI_CURRENT_LUMA:%u \n",__func__, luma);

	        if(copy_to_user((void *)argp, &luma, sizeof(uint32_t))){
		pr_err("[CAM]%s, IOCTL_AIT_ISP_GET_AE_ROI_CURRENT_LUMA copy to user error\n", __func__);
		rc = -EFAULT;
	        }
	}
	break;
	default:
	break;
	}
	mutex_unlock(&raw_dev->AIT_ioctl_lock);
	CDBG("%s:%d cmd = %d -\n", __func__, __LINE__, _IOC_NR(cmd));
	return rc;
}

#endif

#ifdef CONFIG_COMPAT
static long AIT_ISP_fops_compat_ioctl(struct file *filp, unsigned int cmd,
	unsigned long arg)
{
	int rc = 0;
	struct AIT_ISP_ctrl *raw_dev = filp->private_data;
	void __user *argp = (void __user *)arg;
	CDBG("%s:%d cmd = %d +\n", __func__, __LINE__, _IOC_NR(cmd));
	mutex_lock(&raw_dev->AIT_ioctl_lock);
	switch (_IOC_NR(cmd)) {
	case IOCTL_AIT_ISP_GET_INT:
		rc = AIT_ISP_parse_interrupt(argp);
		if(rc < 0)
			pr_info("%s:%d cmd = %d, rc =%d\n", __func__, __LINE__, _IOC_NR(cmd), rc);
		break;
	case IOCTL_AIT_ISP_ENABLE_INT:
	{
		uint32_t InterruptCase = 0;
	        if(copy_from_user(&InterruptCase, argp,
			sizeof(uint32_t))){
			pr_err("[CAM] IOCTL_AIT_ISP_ENABLE_INT copy from user error\n");
			rc = -EFAULT;
			        
	        }
	        else
	        {
	        	CDBG("%s + IOCTL_AIT_ISP_ENABLE_INT: InterruptCase:%u\n",__func__, InterruptCase);
	        	AIT_ISP_enable_interrupt(InterruptCase);
	        	CDBG("%s AIT_ISP_enable_interrupt  done\n ", __func__);
	        }

	}
	break;

	case IOCTL_AIT_ISP_EN_MEC_MWB:
	{
		uint32_t manual_EC_EW = 0;
		int disableAWB=0, disableAEC=0;
	        if(copy_from_user(&manual_EC_EW, argp,
			sizeof(uint32_t))){
			pr_err("[CAM] IOCTL_AIT_ISP_EN_MEC_MWB copy from user error\n");
			rc = -EFAULT;
			        
	        }
	        else
	        {
	        	disableAEC = (manual_EC_EW%2);
	        	disableAWB = (manual_EC_EW/2)%2;
	        	CDBG("%s + IOCTL_AIT_ISP_EN_MEC_MWB: manual_EC_EW:%u, disableAEC:%d , disableAWB:%d\n",__func__, manual_EC_EW, disableAEC, disableAWB);
	        	AIT_ISP_enable_MEC_MWB(disableAWB, disableAEC);
	        	CDBG("%s AIT_ISP_enable_MEC_MWB  done\n ", __func__);
	        }

	}
	break;

	case IOCTL_AIT_ISP_CONFIG_MEC:
	{
		MECData mec_data;
	        if(copy_from_user(&mec_data, argp,
			sizeof(MECData))){
			pr_err("[CAM] IOCTL_AIT_ISP_CONFIG_MEC copy from user error\n");
			rc = -EFAULT;
			        
	        }
	        else
	        {
	        	CDBG("%s + mec_data.N_parameter:%u, mec_data.Overall_Gain:%d\n",__func__, mec_data.N_parameter, mec_data.Overall_Gain);
	        	AIT_ISP_config_MEC(mec_data.N_parameter, mec_data.Overall_Gain);
	        	CDBG("%s AIT_ISP_config_MEC  done\n ", __func__);
	        }
	}
	break;
	case IOCTL_AIT_ISP_CONFIG_MWB:
	{
		MWBData mwb_data;
	        if(copy_from_user(&mwb_data, argp,
			sizeof(MWBData))){
			pr_err("[CAM] IOCTL_AIT_ISP_CONFIG_MWB copy from user error\n");
			rc = -EFAULT;
			        
	        }
	        else
	        {
	        	CDBG("%s + RGBGain:(%d, %d, %d)\n",__func__, mwb_data.R_Gain, mwb_data.G_Gain, mwb_data.B_Gain);
	        	AIT_ISP_config_MWB(mwb_data.R_Gain, mwb_data.G_Gain, mwb_data.B_Gain);
	        	CDBG("%s AIT_ISP_config_MWB  done\n ", __func__);
	        }
	}
	break;
	case IOCTL_AIT_ISP_GET_MEC:
	{
		MECData mec_data;
	        uint8_t ret = 0;
	        uint16_t GainBase = 0;
	        uint32_t ExposureTimeBase = 0;
	        ret = VA_GetOverallGain(&(mec_data.Overall_Gain), &GainBase);
	        CDBG("%s + VA_GetOverallGain done, mec_data.Overall_Gain:%d\n",__func__, mec_data.Overall_Gain);
	        ret = VA_GetExposureTime(&(mec_data.N_parameter), &ExposureTimeBase);
	        CDBG("%s + VA_GetExposureTime done,mec_data.N_parameter:%u\n",__func__, mec_data.N_parameter);

	        if(copy_to_user((void *)argp, &mec_data, sizeof(MECData))){
		pr_err("[CAM]%s, IOCTL_AIT_ISP_GET_MEC copy to user error\n", __func__);
		rc = -EFAULT;
	        }
	}
	break;
	case IOCTL_AIT_ISP_GET_MWB:
	{
		MWBData mwb_data;
	        uint16 Gain_Base = 0;
	        uint8_t ret = 0;
	        ret = VA_GetRGBGain (&(mwb_data.R_Gain), &(mwb_data.G_Gain), &(mwb_data.B_Gain), &Gain_Base);
	        CDBG("%s + VA_GetRGBGain (%d, %d, %d)\n",__func__, mwb_data.R_Gain, mwb_data.G_Gain, mwb_data.B_Gain);

	        if(copy_to_user((void *)argp, &mwb_data, sizeof(MWBData))){
		pr_err("[CAM]%s, IOCTL_AIT_ISP_GET_MWB copy to user error\n", __func__);
		rc = -EFAULT;
	        }
	}
	break;
	case IOCTL_AIT_ISP_GET_CUSTOM_AE_ROI_LUMA:
	{
	        uint8_t ret = 0;
	        uint16_t Luma_Value = 0;
	        ret = VA_GetCustomAEROILuma(&Luma_Value);
	        CDBG("%s + VA_GetCustomAEROILuma:%d\n",__func__, Luma_Value);
	        if(copy_to_user((void *)argp, &Luma_Value, sizeof(uint16_t))){
		pr_err("[CAM]%s, IOCTL_AIT_ISP_GET_MEC copy to user error\n", __func__);
		rc = -EFAULT;
	        }
	}
	break;
	case IOCTL_AIT_ISP_GET_AWB_ROI_ACC:
	{
		AWB_ROI_ACC awb_roi_data;
	        uint8_t ret = 0;
	        ret = VA_GetAWBROIACC (&(awb_roi_data.R_Sum), &(awb_roi_data.G_Sum), &(awb_roi_data.B_Sum), &(awb_roi_data.pixel_count));
	        CDBG("%s + VA_GetAWBROIACC (%u, %u, %u, %u)\n",__func__, awb_roi_data.R_Sum, awb_roi_data.G_Sum, awb_roi_data.B_Sum, awb_roi_data.pixel_count);

	        if(copy_to_user((void *)argp, &awb_roi_data, sizeof(AWB_ROI_ACC))){
		pr_err("[CAM]%s, IOCTL_AIT_ISP_GET_AWB_ROI_ACC copy to user error\n", __func__);
		rc = -EFAULT;
	        }
	}
	break;
	case IOCTL_AIT_ISP_SET_AE_ROI_POSITION:
	{
		uint8_t ret = 0;
		ROI_position roi_data;
	        if(copy_from_user(&roi_data, argp,
			sizeof(ROI_position))){
			pr_err("[CAM] IOCTL_AIT_ISP_SET_AE_ROI_POSITION copy from user error\n");
			rc = -EFAULT;
			        
	        }
	        else
	        {
	        	CDBG("%s + AE ROI:(%d, %d, %d, %d)\n",__func__, roi_data.width, roi_data.height, roi_data.offsetx, roi_data.offsety);
	        	ret = VA_SetAEROIPosition(roi_data.width, roi_data.height, roi_data.offsetx, roi_data.offsety);
	        	CDBG("%s VA_SetAEROIPosition  done(%d)\n ", __func__, ret);
	        }
	}
	break;
	case IOCTL_AIT_ISP_SET_AWB_ROI_POSITION:
	{
		uint8_t ret = 0;
		ROI_position roi_data;
	        if(copy_from_user(&roi_data, argp,
			sizeof(ROI_position))){
			pr_err("[CAM] IOCTL_AIT_ISP_SET_AWB_ROI_POSITION copy from user error\n");
			rc = -EFAULT;
			        
	        }
	        else
	        {
	        	CDBG("%s + AE ROI:(%d, %d, %d, %d)\n",__func__, roi_data.width, roi_data.height, roi_data.offsetx, roi_data.offsety);
	        	ret = VA_SetAWBROIPosition(roi_data.width, roi_data.height, roi_data.offsetx, roi_data.offsety);
	        	CDBG("%s VA_SetAWBROIPosition  done(%d)\n ", __func__, ret);
	        }
	}
	break;
	case IOCTL_AIT_ISP_CUSTOM_AE_ROI_MODE_ENABLE:
	{
		uint8_t ret = 0;
		int enable = 0;
	        if(copy_from_user(&enable, argp,
			sizeof(int))){
			pr_err("[CAM] IOCTL_AIT_ISP_CUSTOM_AE_ROI_MODE_ENABLE copy from user error\n");
			rc = -EFAULT;
			        
	        }
	        else
	        {
	        	CDBG("%s + IOCTL_AIT_ISP_CUSTOM_AE_ROI_MODE_ENABLE:(%d)\n",__func__, enable);
	        	if(enable == 0)
	        	ret =VA_CustomAEROIModeEnable(ISP_CUSTOM_WIN_MODE_OFF);
	        	else
	        	ret =VA_CustomAEROIModeEnable(ISP_CUSTOM_WIN_MODE_ON);
	        	CDBG("%s VA_CustomAEROIModeEnable  done(%d)\n ", __func__, ret);
	        }
	}
	break;
	case IOCTL_AIT_ISP_SET_LUMA_FORMULA:
	{
		uint8_t ret = 0;
		Luma_formula luma_formula_data;
	        if(copy_from_user(&luma_formula_data, argp,
			sizeof(Luma_formula))){
			pr_err("[CAM] IOCTL_AIT_ISP_SET_LUMA_FORMULA copy from user error\n");
			rc = -EFAULT;
			        
	        }
	        else
	        {
	        	CDBG("%s + IOCTL_AIT_ISP_SET_LUMA_FORMULA:(%d, %d, %d, %d)\n",__func__, luma_formula_data.paraR, luma_formula_data.paraGr, luma_formula_data.paraGb, luma_formula_data.paraB);
	        	ret = VA_SetLumaFormula(luma_formula_data.paraR, luma_formula_data.paraGr, luma_formula_data.paraGb, luma_formula_data.paraB);
	        	CDBG("%s VA_SetLumaFormula  done(%d)\n ", __func__, ret);
	        }
	}
	break;
	case IOCTL_AIT_ISP_SET_AE_ROI_METERING_TABLE:
	{
		uint8_t ret = 0;
		uint8_t table[128];
	        if(copy_from_user(table, argp,
			128)){
			pr_err("[CAM] IOCTL_AIT_ISP_SET_AE_ROI_METERING_TABLE copy from user error\n");
			rc = -EFAULT;
			        
	        }
	        else
	        {
	        	CDBG("%s + IOCTL_AIT_ISP_SET_AE_ROI_METERING_TABLE +\n",__func__);
	        	ret = VA_SetAEROIMeteringTable(table);
	        	CDBG("%s IOCTL_AIT_ISP_SET_AE_ROI_METERING_TABLE  done(%d)\n ", __func__, ret);
	        }
	}
	break;
	case IOCTL_AIT_ISP_SET_CUSTOM_AE_ROI_WEIGHT:
	{
		uint8_t ret = 0;
		uint16_t weight;
	        if(copy_from_user(&weight, argp,
			sizeof(uint16_t))){
			pr_err("[CAM] IOCTL_AIT_ISP_SET_CUSTOM_AE_ROI_WEIGHT copy from user error\n");
			rc = -EFAULT;
			        
	        }
	        else
	        {
	        	CDBG("%s + IOCTL_AIT_ISP_SET_CUSTOM_AE_ROI_WEIGHT weight:%d\n",__func__, weight);
	        	ret = VA_SetCustomAEROIWeight(weight);
	        	CDBG("%s IOCTL_AIT_ISP_SET_CUSTOM_AE_ROI_WEIGHT done(%d)\n ", __func__, ret);
	        }
	}
	break;
	case IOCTL_AIT_ISP_SET_CUSTOM_AE_ROI_POSITION:
	{
		uint8_t ret = 0;
		custom_roi_position cus_roi_pos;
	        if(copy_from_user(&cus_roi_pos, argp,
			sizeof(custom_roi_position))){
			pr_err("[CAM] IOCTL_AIT_ISP_SET_CUSTOM_AE_ROI_POSITION copy from user error\n");
			rc = -EFAULT;
			        
	        }
	        else
	        {
	        	CDBG("%s + IOCTL_AIT_ISP_SET_CUSTOM_AE_ROI_POSITION:(%d, %d, %d, %d)\n",__func__, cus_roi_pos.Start_X_Index, cus_roi_pos.End_X_Index, cus_roi_pos.Start_Y_Index, cus_roi_pos.End_Y_Index);
	        	ret = VA_SetCustomAEROIPosition(cus_roi_pos.Start_X_Index, cus_roi_pos.End_X_Index, cus_roi_pos.Start_Y_Index, cus_roi_pos.End_Y_Index);
	        	CDBG("%s IOCTL_AIT_ISP_SET_CUSTOM_AE_ROI_POSITION  done(%d)\n ", __func__, ret);
	        }
	}
	break;
	case IOCTL_AIT_ISP_GET_AE_ROI_CURRENT_LUMA:
	{
	        uint32_t luma = 0;
	        uint8_t ret = 0;
	        ret = VA_GetAEROICurrentLuma(&luma);
	        CDBG("%s IOCTL_AIT_ISP_GET_AE_ROI_CURRENT_LUMA:%u \n",__func__, luma);

	        if(copy_to_user((void *)argp, &luma, sizeof(uint32_t))){
		pr_err("[CAM]%s, IOCTL_AIT_ISP_GET_AE_ROI_CURRENT_LUMA copy to user error\n", __func__);
		rc = -EFAULT;
	        }
	}
	break;
	default:
	break;
	}
	mutex_unlock(&raw_dev->AIT_ioctl_lock);
	CDBG("%s:%d cmd = %d -\n", __func__, __LINE__, _IOC_NR(cmd));
	return rc;
}
#endif
static  const struct  file_operations AIT_ISP_fops = {
	.owner	  = THIS_MODULE,
	.open	   = AIT_ISP_fops_open,
	.unlocked_ioctl = AIT_ISP_fops_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = AIT_ISP_fops_compat_ioctl,
#endif
#ifdef AIT_INT
	.poll  = AIT_ISP_fops_poll,
#endif
};

static int msm_camera_pinctrl_init(struct AIT_ISP_ctrl *ctrl)
{
	struct msm_pinctrl_info *sensor_pctrl = NULL;

	sensor_pctrl = &ctrl->AIT_pinctrl;
	sensor_pctrl->pinctrl = devm_pinctrl_get(ctrl->dev);
	if (IS_ERR_OR_NULL(sensor_pctrl->pinctrl)) {
		pr_err("%s:%d Getting pinctrl handle failed\n",
			__func__, __LINE__);
		return -EINVAL;
	}
	sensor_pctrl->gpio_state_active =
		pinctrl_lookup_state(sensor_pctrl->pinctrl,
				AIT_PINCTRL_STATE_DEFAULT);
	if (IS_ERR_OR_NULL(sensor_pctrl->gpio_state_active)) {
		pr_err("%s:%d Failed to get the active state pinctrl handle\n",
			__func__, __LINE__);
		return -EINVAL;
	}
	sensor_pctrl->gpio_state_suspend
		= pinctrl_lookup_state(sensor_pctrl->pinctrl,
				AIT_PINCTRL_STATE_SLEEP);
	if (IS_ERR_OR_NULL(sensor_pctrl->gpio_state_suspend)) {
		pr_err("%s:%d Failed to get the suspend state pinctrl handle\n",
				__func__, __LINE__);
		return -EINVAL;
	}
	return 0;
}
static int setup_AIT_ISP_cdev(void)
{
	int rc = 0;
	struct device *dev;
	CDBG("%s\n", __func__);

	rc = alloc_chrdev_region(&AIT_ISP_devno, 0, 1, MSM_AIT_ISP_NAME);
	if (rc < 0) {
		pr_err("%s: failed to allocate chrdev\n", __func__);
		goto alloc_chrdev_region_failed;
	}

	if (!AIT_ISP_class) {
		AIT_ISP_class = class_create(THIS_MODULE, MSM_AIT_ISP_NAME);
		if (IS_ERR(AIT_ISP_class)) {
			rc = PTR_ERR(AIT_ISP_class);
			pr_err("%s: create device class failed\n",
				__func__);
			goto class_create_failed;
		}
	}

	dev = device_create(AIT_ISP_class, NULL,
		MKDEV(MAJOR(AIT_ISP_devno), MINOR(AIT_ISP_devno)), NULL,
		"%s%d", MSM_AIT_ISP_NAME, 0);
	if (IS_ERR(dev)) {
		pr_err("%s: error creating device\n", __func__);
		rc = -ENODEV;
		goto device_create_failed;
	}

	cdev_init(&AIT_ISPCtrl->cdev, &AIT_ISP_fops);
	AIT_ISPCtrl->cdev.owner = THIS_MODULE;
	AIT_ISPCtrl->cdev.ops   =
		(const struct file_operations *) &AIT_ISP_fops;
	rc = cdev_add(&AIT_ISPCtrl->cdev, AIT_ISP_devno, 1);
	if (rc < 0) {
		pr_err("%s: error adding cdev\n", __func__);
		rc = -ENODEV;
		goto cdev_add_failed;
	}

	return rc;

cdev_add_failed:
	device_destroy(AIT_ISP_class, AIT_ISP_devno);
device_create_failed:
	class_destroy(AIT_ISP_class);
class_create_failed:
	unregister_chrdev_region(AIT_ISP_devno, 1);
alloc_chrdev_region_failed:
	return rc;
}

static void AIT_ISP_tear_down_cdev(void)
{
	CDBG("%s\n", __func__);
	cdev_del(&AIT_ISPCtrl->cdev);
	device_destroy(AIT_ISP_class, AIT_ISP_devno);
	class_destroy(AIT_ISP_class);
	unregister_chrdev_region(AIT_ISP_devno, 1);
}


#ifdef CONFIG_CAMERA_AIT
static struct kobject *AIT_ISP_status_obj;

uint32_t AIT_ISP_id;

static ssize_t probed_AIT_ISP_id_get(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	ssize_t length;
	length = sprintf(buf, "0x%x\n", AIT_ISP_id);
	return length;
}

static DEVICE_ATTR(probed_AIT_ISP_id, 0444,
	probed_AIT_ISP_id_get,
	NULL);

int msm_AIT_ISP_attr_node(void)
{
	int ret = 0;
	CDBG("%s \n", __func__);
	AIT_ISP_status_obj = kobject_create_and_add("camera_AIT_ISP_status", NULL);
	if (AIT_ISP_status_obj == NULL) {
		pr_err("msm_camera: create camera_AIT_ISP_status failed\n");
		ret = -ENOMEM;
		goto error;
	}

	ret = sysfs_create_file(AIT_ISP_status_obj,
		&dev_attr_probed_AIT_ISP_id.attr);
	if (ret) {
		CDBG("%s, sysfs_create_file failed\n", __func__);
		ret = -EFAULT;
		goto error;
	}

	return ret;

error:
	kobject_del(AIT_ISP_status_obj);
	return ret;
}
#endif

void HTC_DelayMS(uint16_t ms)
{
	mdelay(ms);
}

static int ISP_vreg_on(void)
{
	int rc = 0;
	int i = 0;
	int ISP_gpio = 0;
	int ISP_gpio1 = 0;
	int ISP_gpio2 = 0;
	int ret = 0;

	CDBG("%s +\n", __func__);
	ret = msm_camera_pinctrl_init(AIT_ISPCtrl);
	if (ret < 0) {
		pr_err("%s:%d Initialization of pinctrl failed\n",
				__func__, __LINE__);
		AIT_ISPCtrl->AIT_pinctrl_status = 0;
	} else {
		AIT_ISPCtrl->AIT_pinctrl_status = 1;
		
	}
	
	
	for (i = 0 ; i < 2; i++){
		if (i == 0)
			ISP_gpio = AIT_ISPCtrl->pdata->ISP_1v2_enable;
		else
			ISP_gpio = AIT_ISPCtrl->pdata->ISP_1v8_enable;

		if (ISP_vreg[i].type == VREG_TYPE_GPIO) {
			rc = gpio_request(ISP_gpio, "AIT_ISP");
			if (rc < 0) {
			pr_err("GPIO(%d) request failed", ISP_gpio);
			}
			gpio_direction_output(ISP_gpio, 1);
			gpio_free(ISP_gpio);
		} else {
			
			msm_camera_config_single_vreg(AIT_ISPCtrl->dev,
				&ISP_vreg[i],
				(struct regulator **)&ISP_vreg_data[i],
				1);
		}
		mdelay(1);
	}
	
	
	
	ISP_gpio = AIT_ISPCtrl->pdata->ISP_gpio_vana;
	rc = gpio_request(ISP_gpio, "AIT_ISP");
	if (rc < 0) {
		pr_err("GPIO(%d) request failed", ISP_gpio);
	    }
	gpio_direction_output(ISP_gpio, 1);
	mdelay(1);

	    
	
	msm_camera_config_single_vreg(AIT_ISPCtrl->dev,
				&ISP_vreg[2],
				(struct regulator **)&ISP_vreg_data[2],
				1);
	mdelay(1);
	    
	
	
	ISP_gpio = AIT_ISPCtrl->pdata->ISP_enable;
	rc = gpio_request(ISP_gpio, "AIT_ISP");
	if (rc < 0) {
		pr_err("GPIO(%d) request failed", ISP_gpio);
	}
	gpio_direction_output(ISP_gpio, 1);
	gpio_free(ISP_gpio);
	mdelay(1);

	
	if(camera_index == CAMERA_INDEX_SUB_OV2722)
	CDBG("%s sub CAM_SEL low\n", __func__);
	else
	CDBG("%s sub CAM_SEL high\n", __func__);
	ISP_gpio = AIT_ISPCtrl->pdata->ISP_CAM_SEL;
	rc = gpio_request(ISP_gpio, "AIT_ISP");
	if (rc < 0) {
		pr_err("GPIO(%d) request failed", ISP_gpio);
	}
	if(camera_index == CAMERA_INDEX_SUB_OV2722)
	gpio_direction_output(ISP_gpio, 0);
	else
	gpio_direction_output(ISP_gpio, 1);
	gpio_free(ISP_gpio);
	mdelay(1);
	    
	
	
	ISP_gpio = AIT_ISPCtrl->pdata->ISP_CAM_SEL2;
	rc = gpio_request(ISP_gpio, "AIT_ISP");
	if (rc < 0) {
		pr_err("GPIO(%d) request failed", ISP_gpio);
	}
	gpio_direction_output(ISP_gpio, 0);
	gpio_free(ISP_gpio);
	mdelay(1);

	
	if(camera_index == CAMERA_INDEX_SUB_OV2722)
	CDBG("%s ISP_MCLK_SEL high\n", __func__);
	else
	CDBG("%s ISP_MCLK_SEL low\n", __func__);
	ISP_gpio = AIT_ISPCtrl->pdata->ISP_MCLK_SEL;
	rc = gpio_request(ISP_gpio, "AIT_ISP");
	if (rc < 0) {
		pr_err("GPIO(%d) request failed", ISP_gpio);
	}
	
	if(camera_index == CAMERA_INDEX_SUB_OV2722)
	gpio_direction_output(ISP_gpio, 1);
	else
	gpio_direction_output(ISP_gpio, 0);
	
	gpio_free(ISP_gpio);
	mdelay(1);

	
	
	ISP_gpio = AIT_ISPCtrl->pdata->ISP_V_SR_3V;
	rc = gpio_request(ISP_gpio, "AIT_ISP");
	if (rc < 0) {
		pr_err("GPIO(%d) request failed", ISP_gpio);
	}
	gpio_direction_output(ISP_gpio, 1);
	gpio_free(ISP_gpio);
	mdelay(1);

	
	
	rc = gpio_request_one(AIT_ISPCtrl->pdata->ISP_mclk, 1, "CAMIF_MCLK");
	if (rc < 0) {
		pr_err("%s, CAMIF_MCLK (%d) request failed", __func__, AIT_ISPCtrl->pdata->ISP_mclk);
		gpio_free(AIT_ISPCtrl->pdata->ISP_mclk);
	}

	if (AIT_ISPCtrl->AIT_pinctrl_status) {
		ret = pinctrl_select_state(AIT_ISPCtrl->AIT_pinctrl.pinctrl,
			AIT_ISPCtrl->AIT_pinctrl.gpio_state_active);
		if (ret)
			pr_err("%s:%d cannot set pin to active state",
				__func__, __LINE__);
		else
		CDBG("%s  pinctrl_select_state ok\n", __func__);
	}
	
	rc = msm_cam_clk_enable(AIT_ISPCtrl->sensor_dev, ISP_clk_info,
		AIT_ISPCtrl->ISP_clk, ARRAY_SIZE(ISP_clk_info), 1);
	if (rc < 0) {
		pr_err("%s: clk enable failed\n",__func__);
	}
	mdelay(1);
	    
	
	
	ISP_gpio = AIT_ISPCtrl->pdata->ISP_APG6;
	rc = gpio_request_one(ISP_gpio, 0, "AIT_ISP");
	if (rc < 0) {
		pr_err("GPIO(%d) ISP_APG6 gpio_request_one failed", ISP_gpio);
	}
	gpio_direction_output(ISP_gpio, 1);
	
	ISP_gpio1 = AIT_ISPCtrl->pdata->ISP_APG7;
	rc = gpio_request_one(ISP_gpio1, 0, "AIT_ISP");
	if (rc < 0) {
		pr_err("GPIO(%d) ISP_APG7 request failed", ISP_gpio1);
	}
	gpio_direction_output(ISP_gpio1, 1);
	mdelay(1);

	
	
	ISP_gpio2 = AIT_ISPCtrl->pdata->ISP_reset;
	rc = gpio_request(ISP_gpio2, "AIT_ISP");
	if (rc < 0) {
		pr_err("GPIO(%d) request failed", ISP_gpio2);
	}
	gpio_direction_output(ISP_gpio2, 1);
	gpio_free(ISP_gpio2);
	mdelay(1);

	
	
	gpio_direction_output(ISP_gpio, 0);
	gpio_free(ISP_gpio);
	
	gpio_direction_output(ISP_gpio1, 0);
	gpio_free(ISP_gpio1);
	mdelay(5);

	CDBG("%s - \n", __func__);
	return rc;
}

static int ISP_vreg_off(void)
{

	int rc = 0;
	int i = 0;
	int ISP_gpio = 0;
	int ret = 0;

	CDBG("%s +\n", __func__);
	
	
	
	
	
	ISP_gpio = AIT_ISPCtrl->pdata->ISP_reset;
	rc = gpio_request(ISP_gpio, "AIT_ISP");
	if (rc < 0) {
		pr_err("GPIO(%d) ISP_reset request failed", ISP_gpio);
	}
	gpio_direction_output(ISP_gpio, 0);
	gpio_free(ISP_gpio);
	mdelay(1);

	
	
	ISP_gpio = AIT_ISPCtrl->pdata->ISP_enable;
	rc = gpio_request(ISP_gpio, "AIT_ISP");
	if (rc < 0) {
		pr_err("GPIO(%d) request failed", ISP_gpio);
	}
	gpio_direction_output(ISP_gpio, 0);
	gpio_free(ISP_gpio);
	mdelay(1);

	
	
	rc = msm_cam_clk_enable(AIT_ISPCtrl->sensor_dev, ISP_clk_info,
		AIT_ISPCtrl->ISP_clk, ARRAY_SIZE(ISP_clk_info), 0);
	if (rc < 0)
		pr_err("disable MCLK failed\n");

	if (AIT_ISPCtrl->AIT_pinctrl_status) {
		ret = pinctrl_select_state(AIT_ISPCtrl->AIT_pinctrl.pinctrl,
				AIT_ISPCtrl->AIT_pinctrl.gpio_state_suspend);
		if (ret)
			pr_err("%s:%d cannot set pin to suspend state",
				__func__, __LINE__);
		devm_pinctrl_put(AIT_ISPCtrl->AIT_pinctrl.pinctrl);
	}
	AIT_ISPCtrl->AIT_pinctrl_status = 0;
	
	gpio_free(AIT_ISPCtrl->pdata->ISP_mclk);
	mdelay(1);

	
	
	ISP_gpio = AIT_ISPCtrl->pdata->ISP_APG6;
	rc = gpio_request_one(ISP_gpio, 0, "AIT_ISP");
	if (rc < 0) {
		pr_err("GPIO(%d) ISP_APG6 gpio_request_one failed", ISP_gpio);
	}
	gpio_direction_output(ISP_gpio, 0);
	gpio_free(ISP_gpio);

	ISP_gpio = AIT_ISPCtrl->pdata->ISP_APG7;
	rc = gpio_request_one(ISP_gpio, 0, "AIT_ISP");
	if (rc < 0) {
		pr_err("GPIO(%d) ISP_APG7 gpio_request_one failed", ISP_gpio);
	}
	gpio_direction_output(ISP_gpio, 0);
	gpio_free(ISP_gpio);
	mdelay(1);

	
	
	msm_camera_config_single_vreg(AIT_ISPCtrl->dev,
				&ISP_vreg[2],
				(struct regulator **)&ISP_vreg_data[2],
				0);
	mdelay(1);

	
	
	gpio_direction_output(AIT_ISPCtrl->pdata->ISP_gpio_vana, 0);
	gpio_free(AIT_ISPCtrl->pdata->ISP_gpio_vana);
	mdelay(1);
	    
	
	for (i = 1 ; i >= 0; i--){
		if (i == 0)
			ISP_gpio = AIT_ISPCtrl->pdata->ISP_1v2_enable;
		else
			ISP_gpio = AIT_ISPCtrl->pdata->ISP_1v8_enable;

		if (ISP_vreg[i].type == VREG_TYPE_GPIO) {
			rc = gpio_request(ISP_gpio, "AIT_ISP");
			if (rc < 0) {
				pr_err("GPIO(%d) request failed", ISP_gpio);
			}
			gpio_direction_output(ISP_gpio, 0);
			gpio_free(ISP_gpio);
		} else {
			
			msm_camera_config_single_vreg(AIT_ISPCtrl->dev,
				&ISP_vreg[i],
				(struct regulator **)&ISP_vreg_data[i],
				0);
		}
		mdelay(1);
	}
	CDBG("%s - \n", __func__);
	return rc;
}

int AIT_ISP_power_up(const struct msm_camera_AIT_info *pdata)
{
	int rc = 0;
	
	struct device *dev = NULL;
	dev = AIT_ISPCtrl->dev;
	
	CDBG("[CAM] %s\n", __func__);

	if (pdata->camera_ISP_power_on == NULL) {
		pr_err("AIT_ISP power on platform_data didn't register\n");
		return -EIO;
	}
	rc = pdata->camera_ISP_power_on();
	if (rc < 0) {
		pr_err("AIT_ISP power on failed\n");
	}
	return rc;
}

int AIT_ISP_power_down(const struct msm_camera_AIT_info *pdata)
{
	int rc = 0;
	
	struct device *dev = NULL;
	dev = AIT_ISPCtrl->dev;
	
	CDBG("%s\n", __func__);

	if (pdata->camera_ISP_power_off == NULL) {
		pr_err("AIT_ISP power off platform_data didn't register\n");
		return -EIO;
	}

	rc = pdata->camera_ISP_power_off();
	if (rc < 0)
		pr_err("AIT_ISP power off failed\n");

	return rc;
}

int AIT_ISP_match_id(void)
{
	int rc = 0;
	unsigned char read_id =0;
	uint16_t read =0;
	read_id = GetVenusRegB(0x6900);
	if(read_id != 0xd0)
	    pr_err("AIT_ISP_match_id read 0x6900 failure:0x%x\n", read_id);
	
	
	read_id = GetVenusRegB(0x6902);
	if(read_id != 0x5)
	    pr_err("AIT_ISP_match_id read 0x6902 failure:0x%x\n", read_id);
	
	
	SetVenusRegB(0x69F0, 0x01);
	read = GetVenusRegW(0x69FE);
	if(read != 0x8980)
	{
	    pr_err("AIT_ISP_match_id read 0x69FE failure:0x%x\n", read);
	    rc = -1;
	}
	else
	    CDBG("%s  match ok\n", __func__);
	
	return rc;
}

void AIT_ISP_release(void)
{
	struct msm_camera_AIT_info *pdata = AIT_ISPCtrl->pdata;
	CDBG("[CAM]%s\n", __func__);
#ifdef AIT_INT
	CDBG("[CAM]%s: YushanII free irq", __func__);
	free_irq(pdata->isp_intr0, 0);
#endif
	AIT_ISP_power_down(pdata);
}
#ifdef AIT_INT
static irqreturn_t AIT_ISP_irq_handler(int irq, void *dev_id){

	unsigned long flags;
	CDBG("%s after detect INT0, interrupt:%d %d\n",
		__func__, atomic_read(&interrupt),__LINE__);
	disable_irq_nosync(AIT_ISPCtrl->pdata->isp_intr0);
	spin_lock_irqsave(&AIT_ISP_int.AIT_spin_lock,flags);
	atomic_set(&interrupt, 1);
	CDBG("%s after detect INT0, interrupt:%d\n",__func__, atomic_read(&interrupt));
	wake_up(&AIT_ISP_int.AIT_wait);
	spin_unlock_irqrestore(&AIT_ISP_int.AIT_spin_lock,flags);
	return IRQ_HANDLED;
}
#endif
int AIT_load_calibration_data(void)
{
	int status = 0;
	CDBG("%s, cam_index:%d\n", __func__, camera_index);
	if(camera_index == CAMERA_INDEX_FRONT_S5K5E)
	{
	    pfront_calibration_data = (struct qct_isp_struct *)(&(cam_awb_ram[0x4800]));
	    CDBG("%s, cam_index:%d, verify:%u, \n", __func__, camera_index, pfront_calibration_data->verify);
	    CDBG("%s, cam_index:%d, fuse_id:(%u, %u, %u, %u) \n", __func__, camera_index, pfront_calibration_data->fuse_id[0], pfront_calibration_data->fuse_id[1], pfront_calibration_data->fuse_id[2], pfront_calibration_data->fuse_id[3]);
	    CDBG("%s, cam_index:%d, check_sum:%u, \n", __func__, camera_index, pfront_calibration_data->check_sum);
	    if((pfront_calibration_data->verify == 3099))
	    {
	    	if((pfront_calibration_data->fuse_id[0] == 0 
	    	&& pfront_calibration_data->fuse_id[1] == 0
	    	&& pfront_calibration_data->fuse_id[2] == 0
	    	&& pfront_calibration_data->fuse_id[3] == 0)
	    	||
	    	(pfront_calibration_data->fuse_id[0] == front_fuse_id[0]
	    	&& pfront_calibration_data->fuse_id[1] == front_fuse_id[1]
	    	&& pfront_calibration_data->fuse_id[2] == front_fuse_id[2]
	    	&& pfront_calibration_data->fuse_id[3] == front_fuse_id[3])
	    	)
	        status= 1;
	    }
	}
	else
	{
	    psub_calibration_data = (struct qct_isp_struct *)(&cam_awb_ram[0x4000]);
	    CDBG("%s, cam_index:%d, verify:%u, \n", __func__, camera_index, psub_calibration_data->verify);
	    CDBG("%s, cam_index:%d, fuse_id:(%u, %u, %u, %u) \n", __func__, camera_index, psub_calibration_data->fuse_id[0], psub_calibration_data->fuse_id[1], psub_calibration_data->fuse_id[2], psub_calibration_data->fuse_id[3]);
	    CDBG("%s, cam_index:%d, check_sum:%u, \n", __func__, camera_index, psub_calibration_data->check_sum);
	    if(psub_calibration_data->verify == 3099)
	    {
	    	if((psub_calibration_data->fuse_id[0] == 0 
	    	&& psub_calibration_data->fuse_id[1] == 0
	    	&& psub_calibration_data->fuse_id[2] == 0
	    	&& psub_calibration_data->fuse_id[3] == 0)
	    	||
	    	(psub_calibration_data->fuse_id[0] == sub_fuse_id[0]
	    	&& psub_calibration_data->fuse_id[1] == sub_fuse_id[1]
	    	&& psub_calibration_data->fuse_id[2] == sub_fuse_id[2]
	    	&& psub_calibration_data->fuse_id[3] == sub_fuse_id[3])
	    	)
	        status= 1;
	    }
	}
	
	if(status == 1)
	{
	CDBG("%s, EMMC data ok \n", __func__);
	}
	else
	{
	CDBG("%s, EMMC data fail \n", __func__);
	}
	return status;
}

void AIT_ISP_read_subcam_OTP(uint8_t *otp_buf)
{
	uint16_t rc = 0;
        int i = 0;
        CDBG("%s +\n", __func__);
        rc = VA_ReadSensorOTP(otp_buf);
        for(i = 0; i < 4; i++)
        {
            sub_fuse_id[i] = otp_buf[i];
        }
        CDBG("%s rc:%d -\n", __func__, rc);
}

int AIT_ISP_read_subcam_sensor_ID(void)
{
	int rc = 0;
	uint16 sensor_id1 = 0;
	uint16 sensor_id2 = 0;
	CDBG("%s, camera_index:%d, V3A_FirmwareStart\n", __func__, camera_index);
	rc = V3A_FirmwareStart(pAIT_fw_sub, sizeof(AIT_fw_sub), NULL, 0);
	CDBG("%s, camera_index:%d, VA_InitializeSensor\n", __func__, camera_index);
	rc = VA_InitializeSensor();
	CDBG("%s, camera_index:%d, read 0x300A\n", __func__, camera_index);
	sensor_id1 = VA_GetSensorReg(0x300A);
	CDBG("%s, camera_index:%d, read 0x300A = 0x%x\n", __func__, camera_index, sensor_id1);
	sensor_id2 = VA_GetSensorReg(0x300B);
	CDBG("%s, camera_index:%d, read 0x300B = 0x%x\n", __func__, camera_index, sensor_id2);
	if(sensor_id1 == 0x27 && sensor_id2 == 0x22)
	{
	    CDBG("%s, match ID ok\n", __func__);
	    rc = 0;
	}
	else
	{
	    CDBG("%s, match ID fail\n", __func__);
	    rc = -1;
	}
	return rc;
}
int AIT_ISP_open_init(int cam_index)
{
	int rc = 0;
	struct msm_camera_AIT_info *pdata = AIT_ISPCtrl->pdata;
	int read_id_retry = 0;
	uint8_t SetCaliStatus = 0;

	CDBG("%s, cam_index:%d\n", __func__, cam_index);
        camera_index = cam_index;
open_read_id_retry:
	rc = AIT_ISP_power_up(pdata);
	if (rc < 0)
		return rc;
	rc = AIT_ISP_match_id();
	if (rc < 0) {
		if (read_id_retry < 3) {
			CDBG("%s, retry read AIT_ISP ID: %d\n", __func__, read_id_retry);
			read_id_retry++;
			AIT_ISP_power_down(pdata);
			goto open_read_id_retry;
		}
		goto open_init_failed;
	}
	load_calibration_data = AIT_load_calibration_data();
	if(camera_index == CAMERA_INDEX_FRONT_S5K5E)
	{
		CDBG("%s: V3A_FirmwareStart: sizeof(AIT_fw_front):%lu \n", __func__, sizeof(AIT_fw_sub));
		if(load_calibration_data == 1)
		{
		    CDBG("%s: V3A_FirmwareStart: sizeof(pfront_calibration_data->isp_caBuff):%lu \n", __func__, sizeof(pfront_calibration_data->isp_caBuff));
		    rc = V3A_FirmwareStart(pAIT_fw_front, sizeof(AIT_fw_front), (uint8_t *)(pfront_calibration_data->isp_caBuff), sizeof(pfront_calibration_data->isp_caBuff));
		    rc = VA_SetCaliTable(&SetCaliStatus);
		    CDBG("%s VA_SetCaliTable:(rc:%d, stauts:%d)\n", __func__, rc, SetCaliStatus);
		}
		else
		rc = V3A_FirmwareStart(pAIT_fw_front, sizeof(AIT_fw_front), NULL, 0);
	}
	else
	{
		CDBG("%s: V3A_FirmwareStart: sizeof(AIT_fw_sub):%lu \n", __func__, sizeof(AIT_fw_sub));
		if(load_calibration_data == 1)
		{
		    CDBG("%s: V3A_FirmwareStart: sizeof(psub_calibration_data->isp_caBuff):%lu \n", __func__, sizeof(psub_calibration_data->isp_caBuff));
		    rc = V3A_FirmwareStart(pAIT_fw_sub, sizeof(AIT_fw_sub), (uint8_t *)(psub_calibration_data->isp_caBuff), sizeof(psub_calibration_data->isp_caBuff));
		    rc = VA_SetCaliTable(&SetCaliStatus);
		    CDBG("%s VA_SetCaliTable:(rc:%d, stauts:%d)\n", __func__, rc, SetCaliStatus);
		}
		else
		rc = V3A_FirmwareStart(pAIT_fw_sub, sizeof(AIT_fw_sub), NULL, 0);
	}
#ifdef AIT_INT
	init_waitqueue_head(&AIT_ISP_int.AIT_wait);
	spin_lock_init(&AIT_ISP_int.AIT_spin_lock);
	
	atomic_set(&interrupt, 0);

	
	rc = request_irq(pdata->isp_intr0, AIT_ISP_irq_handler,
		IRQF_TRIGGER_HIGH, "AIT_ISP_irq", 0);
	if (rc < 0) {
		pr_err("[CAM]request irq intr0 failed\n");
		goto open_init_failed;
	}
	atomic_set(&AIT_ISPCtrl->check_intr0, 0);
	AIT_intr0 = pdata->isp_intr0;	
#endif
        CDBG("%s -\n", __func__);
	return rc;
#if 1
open_init_failed:
	pr_err("%s: AIT_ISP_open_init failed\n", __func__);
	AIT_ISP_power_down(pdata);
#endif
	return rc;
}

int AIT_ISP_probe_init(struct device *dev, int cam_index)
{
	int rc = 0;
	struct msm_camera_AIT_info *pdata = NULL;
	int read_id_retry = 0;

	CDBG("%s, cam_index:%d\n", __func__, cam_index);
        camera_index = cam_index;
	if (AIT_ISPCtrl == NULL) {
		pr_err("already failed in __msm_AIT_ISP_probe\n");
		return -EINVAL;
	} else
		pdata = AIT_ISPCtrl->pdata;

	AIT_ISPCtrl->sensor_dev = dev;
probe_read_id_retry:
	rc = AIT_ISP_power_up(pdata);
	if (rc < 0)
		return rc;

	rc = AIT_ISP_match_id();

	if (rc < 0) {
		if (read_id_retry < 3) {
			CDBG("%s, retry read AIT_ISP ID: %d\n", __func__, read_id_retry);
			read_id_retry++;
			AIT_ISP_power_down(pdata);
			goto probe_read_id_retry;
		}
		goto probe_init_fail;
	}
	
	CDBG("%s: probe_success\n", __func__);
	return rc;

probe_init_fail:
	pr_err("%s: AIT_ISP_probe_init failed\n", __func__);
	AIT_ISP_power_down(pdata);
	return rc;

}

void AIT_ISP_probe_deinit(void)
{
	struct msm_camera_AIT_info *pdata = AIT_ISPCtrl->pdata;
	CDBG("%s\n", __func__);

	AIT_ISP_power_down(pdata);
}


void AIT_ISP_sensor_init(void)
{
	int rc = 0;
	CDBG("%s +\n", __func__);
	rc = VA_InitializeSensor();
	CDBG("%s -\n", __func__);
}

void AIT_ISP_sensor_set_resolution(int width, int height)
{
	int rc = 0;
	CDBG("%s (%d x %d)\n", __func__, width, height);
	CDBG("%s VA_SetPreviewResolution+\n", __func__);
	rc = VA_SetPreviewResolution( width, height );
	CDBG("%s VA_SetPreviewResolution-\n", __func__);
	
	CDBG("%s VA_SetPreviewFormat+\n", __func__);
	rc = VA_SetPreviewFormat(VENUS_PREVIEW_FORMAT_YUV422);
	CDBG("%s VA_SetPreviewFormat-\n", __func__);
}

void AIT_ISP_Dump_status(void)
{
        Debug_ISPStatus();
        Debug_StreamingStatus(1, 1);
}

void AIT_ISP_enable_interrupt(uint32_t InterruptCase)
{
	uint8_t interrupt_status = 0;
	
	interrupt_status = VA_HostIntertuptEnable(1);
	CDBG("%s VA_HostIntertuptEnable:%d\n", __func__, interrupt_status);
	interrupt_status = VA_HostIntertuptModeEnable(InterruptCase);
	CDBG("%s VA_HostIntertuptModeEnable:%d\n", __func__, interrupt_status);
	interrupt_status = VA_HostIntertuptModeStatusClear(InterruptCase);
	CDBG("%s VA_HostIntertuptModeStatusClear:%d\n", __func__, interrupt_status);
}

void AIT_ISP_check_interrupt(uint32_t* InterruptCase)
{
	uint8_t interrupt_status = 0;
	interrupt_status = VA_CheckHostInterruptStatusList(InterruptCase);
	CDBG("%s VA_HostIntertuptModeCheckStatus (interrupt_status:%d, InterruptCase:%u)\n", __func__, interrupt_status, *InterruptCase);
	interrupt_status = VA_HostIntertuptModeStatusClear(*InterruptCase);
	CDBG("%s VA_HostIntertuptModeStatusClear:%d\n", __func__, interrupt_status);

}

void AIT_ISP_sensor_start_stream(void)
{
	int rc = 0;
	CDBG("%s \n +", __func__);
	CDBG("%s VA_SetPreviewControl(VA_PREVIEW_STATUS_START) +\n", __func__);
	rc = VA_SetPreviewControl(VA_PREVIEW_STATUS_START);
	CDBG("%s VA_SetPreviewControl(VA_PREVIEW_STATUS_START) -\n", __func__);
	mdelay(50);
	Debug_ISPStatus();
        Debug_StreamingStatus(0, 1);
	CDBG("%s -\n", __func__);
}

void AIT_ISP_sensor_stop_stream(void)
{
	int rc = 0;
	CDBG("%s \n", __func__);
	CDBG("%s AIT_ISP_sensor_stop_stream+\n", __func__);
	rc = VA_SetPreviewControl(VA_PREVIEW_STATUS_STOP);
	CDBG("%s AIT_ISP_sensor_stop_stream-\n", __func__);
}

void AIT_ISP_config_MEC(uint32_t N_parameter, uint16_t Overall_Gain)
{
	uint8_t ret = 0;
	CDBG("%s + N_parameter:%u, Overall_Gain:%d\n", __func__, N_parameter, Overall_Gain);
	ret = VA_SetExposureTime(N_parameter);
	CDBG("%s VA_SetExposureMode done, ret:%d\n", __func__, ret);
	ret = VA_SetOverallGain(Overall_Gain);
	CDBG("%s VA_SetOverallGain done, ret:%d\n", __func__, ret);	
	CDBG("%s -\n", __func__);
}

void AIT_ISP_config_MWB(uint16_t R_Gain, uint16_t G_Gain, uint16_t B_Gain)
{
	uint8_t ret = 0;
	CDBG("%s + RGB(%d, %d, %d)\n", __func__, R_Gain, G_Gain, B_Gain);
	ret = VA_SetRGBGain(R_Gain, G_Gain, B_Gain);
	CDBG("%s VA_SetRGBGain done, ret:%d\n", __func__, ret);
	CDBG("%s -\n", __func__);
}

void AIT_ISP_enable_MEC_MWB(int disableAWB, int disableAEC)
{
	CDBG("%s disableAWB:%d, disableAEC:%d \n", __func__, disableAWB, disableAEC);
	if(disableAWB == 1)
	{
	    VA_AutoAWBMode(0);
	}
	else
	    VA_AutoAWBMode(1);
	CDBG("%s VA_AutoAWBMode done\n", __func__);
	if(disableAEC == 1)
	{
	    VA_SetExposureMode(2);
	}
	else
	    VA_SetExposureMode(0);
	CDBG("%s VA_SetExposureMode done\n", __func__);
}
void AIT_ISP_enable_calibration(int mode, struct ISP_roi_local *pAE_roi, struct ISP_roi_local *pAWB_roi, uint16_t luma_target)
{
	uint8_t ret = 0;
	int i = 0;
	uint8_t AE_Status = 0;
	uint16_t final_luma_target;
	uint32_t DebugMessage;
    
	CDBG("%s cam:%d, mode: %d +\n", __func__, camera_index, mode);
	mdelay(250);
	CDBG("%s cam:%d, mode: %d , delay 250 ms done\n", __func__, camera_index, mode);
	AIT_ISP_Dump_status();
	if(camera_index == CAMERA_INDEX_SUB_OV2722 && mode == 1)
	{
		ret = VA_IQEnable(0, 1);
		CDBG("%s VA_IQEnable (0, 1) done (%d)\n", __func__, ret);
		AIT_ISP_Dump_status();
		CDBG("%s luma_target (%d)\n", __func__, luma_target);
		for(i =0; i < 256 ; i++)
		{
		    if(Ytransform[1][i] > luma_target)
		    break;
		}
		CDBG("%s (%d)luma_target (%d), input (%d), output(%d)\n", __func__,i-1 , luma_target, Ytransform[0][i-1],Ytransform[1][i-1]);
		final_luma_target = Ytransform[0][i-1] * 256;
		CDBG("%s final_luma_target(0x%x)(%d)\n", __func__,final_luma_target, final_luma_target);
		AIT_ISP_Dump_status();
		CDBG("%s AE ROI(%d, %d, %d, %d)\n", __func__,pAE_roi->width, pAE_roi->height,pAE_roi->offsetx,pAE_roi->offsety  );
		ret = VA_SetAEROIPosition(pAE_roi->width, pAE_roi->height, pAE_roi->offsetx, pAE_roi->offsety);
		CDBG("%s VA_SetAEROIPosition done (%d)\n", __func__, ret);
		AIT_ISP_Dump_status();
		CDBG("%s AWB ROI(%d, %d, %d, %d)\n", __func__,pAWB_roi->width, pAWB_roi->height,pAWB_roi->offsetx,pAWB_roi->offsety  );
		ret = VA_SetAWBROIPosition(pAWB_roi->width, pAWB_roi->height, pAWB_roi->offsetx, pAWB_roi->offsety);
		CDBG("%s VA_SetAWBROIPosition done (%d)\n", __func__, ret);
		AIT_ISP_Dump_status();
		ret = VA_SetLumaFormula(0, 128, 128, 0);
		CDBG("%s VA_SetLumaFormula done (%d)\n", __func__, ret);
		AIT_ISP_Dump_status();
		ret = VA_SetAEROILumaTarget(final_luma_target);
		CDBG("%s VA_SetAEROILumaTarget (%d) done.\n", __func__, ret);
		ret = VA_SetLumaTargetMode(1);
		CDBG("%s VA_SetLumaTargetMode done (%d)\n", __func__, ret);
		AIT_ISP_Dump_status();
		for(i = 0; i < 200; i++)
		{
		    ret = VA_GetAEStatus(&AE_Status);
		    CDBG("%s manual(%d)VA_GetAEStatus:%d\n", __func__, i, ret);
		    AIT_ISP_Dump_status();
		    VA_ISPDebug( VENUS_GET_EV_TARGET, &DebugMessage);
		    VA_ISPDebug( VENUS_GET_AVG_LUM, &DebugMessage);

		    if(AE_Status == 1)
		    {
		        CDBG("%s (%d)VA_GetAEStatus done\n", __func__, i);
		        break;
		    }
		    mdelay(20);
		}
		if(i == 200)
		{
		    CDBG("%s (%d)VA_GetAEStatus fail\n", __func__, i);
		}
	}
	else
	{
		ret = VA_IQEnable(0, 0);
		CDBG("%s VA_IQEnable (0, 0) done (%d)\n", __func__, ret);
		AIT_ISP_Dump_status();		    
		ret = VA_SetAEROILumaTarget(0x6500);
		CDBG("%s VA_SetAEROILumaTarget 0x6500 (%d) done.\n", __func__, ret);
		AIT_ISP_Dump_status();
		ret = VA_SetLumaTargetMode(1);
		CDBG("%s VA_SetLumaTargetMode done (%d)\n", __func__, ret);
		AIT_ISP_Dump_status();
		
		for(i = 0; i < 200; i++)
		{
		    ret = VA_GetAEStatus(&AE_Status);
		    CDBG("%s auto (%d)VA_GetAEStatus:%d\n", __func__, i, ret);
		    AIT_ISP_Dump_status();
		    VA_ISPDebug( VENUS_GET_EV_TARGET, &DebugMessage);
		    VA_ISPDebug( VENUS_GET_AVG_LUM, &DebugMessage);
		    if(AE_Status == 1)
		    {
			    	CDBG("%s (%d)VA_GetAEStatus done\n", __func__, i);
			    	break;
		    }
		    mdelay(20);
		}
		if(i == 200)
		{
		    CDBG("%s (%d)VA_GetAEStatus fail\n", __func__, i);
		}
	}
	CDBG("%s cam:%d, mode: %d -\n", __func__, camera_index, mode);

}

void AIT_ISP_set_luma_target(uint16_t luma_target)
{
	uint8_t ret = 0;
	int i = 0;
	uint8_t AE_Status = 0;
	uint16_t final_luma_target;
	CDBG("%s cam:%d +\n", __func__, camera_index);
	CDBG("%s luma_target (%d)\n", __func__, luma_target);
	for(i =0; i < 256 ; i++)
	{
		    if(Ytransform[1][i] > luma_target)
		    break;
	}
	CDBG("%s (%d)luma_target (%d), input (%d), output(%d)\n", __func__,i-1 , luma_target, Ytransform[0][i-1],Ytransform[1][i-1]);
	final_luma_target = Ytransform[0][i-1] * 256;
	CDBG("%s final_luma_target(0x%x)(%d)\n", __func__,final_luma_target, final_luma_target);
	ret = VA_SetAEROILumaTarget(final_luma_target);
	CDBG("%s VA_SetAEROILumaTarget (%d) done.\n", __func__, ret);
	ret = VA_SetLumaTargetMode(1);
	CDBG("%s VA_SetLumaTargetMode done (%d)\n", __func__, ret);
	for(i = 0; i < 200; i++)
	{
	    ret = VA_GetAEStatus(&AE_Status);
	    if(AE_Status == 1)
	    {
			    	CDBG("%s (%d)VA_GetAEStatus done\n", __func__, i);
			    	break;
	    }
	    mdelay(10);
	}
	if(i == 200)
	{
	    CDBG("%s (%d)VA_GetAEStatus fail\n", __func__, i);
	}
	CDBG("%s cam:%d -\n", __func__, camera_index);
}

void AIT_ISP_Get_AE_calibration(uint16_t *Gain, uint16_t *GainBase, uint32_t *N_parameter, uint32_t* ExposureTimeBase, uint16_t* Luma_Value)
{
	uint8_t ret = 0;
	uint32_t luma_temp = 0;
	CDBG("%s +\n", __func__);
	ret = VA_GetAEROICurrentLuma(&luma_temp);
	*Luma_Value = (uint16_t)luma_temp;
	CDBG("%s VA_GetCustomAEROILuma done (%d)luma_temp:0x%x(0x%x)\n", __func__, ret, luma_temp, *Luma_Value);
	
	ret = VA_GetOverallGain(Gain, GainBase);
	CDBG("%s VA_GetOverallGain done (%d)(0x%x, 0x%x)\n", __func__, ret, *Gain, *GainBase);
	
	ret = VA_GetExposureTime(N_parameter, ExposureTimeBase);
	CDBG("%s VA_GetExposureTime done (%d)(0x%x, 0x%x)\n", __func__, ret, *N_parameter, *ExposureTimeBase);
	
	CDBG("%s -\n", __func__);
	
}

void AIT_ISP_Get_AWB_calibration(uint32_t *R_Sum, uint32_t *G_Sum, uint32_t *B_Sum, uint32_t *pixel_count)
{
	uint8_t ret = 0;
	CDBG("%s +\n", __func__);
	ret = VA_GetAWBROIACC(R_Sum, G_Sum, B_Sum, pixel_count);
	CDBG("%s VA_GetAWBROIACC done (%d)(%u, %u, %u, %u)\n", __func__, ret, *R_Sum, *G_Sum, *B_Sum, *pixel_count);
	
	CDBG("%s -\n", __func__);
}


static int AIT_ISP_driver_probe(struct platform_device *pdev)
{
	int rc = 0;
	int rc1 = 0;
	int i = 0;
	uint32_t tmp_array[5];
	CDBG("%s +\n", __func__);

	AIT_ISPCtrl = kzalloc(sizeof(struct AIT_ISP_ctrl), GFP_ATOMIC);
	if (!AIT_ISPCtrl) {
		pr_err("%s: could not allocate mem for AIT_ISP_dev\n", __func__);
		return -ENOMEM;
	}

	rc = setup_AIT_ISP_cdev();
	if (rc < 0) {
		kfree(AIT_ISPCtrl);
		return rc;
	}

	mutex_init(&AIT_ISPCtrl->AIT_ioctl_lock);

	if (!pdev->dev.of_node) {
		pr_err("%s:%d of_node NULL \n", __func__, __LINE__);
		return -EINVAL;
	}

	AIT_ISPCtrl->pdata = kzalloc(sizeof(struct msm_camera_AIT_info),
		GFP_ATOMIC);
	if (!AIT_ISPCtrl->pdata) {
		pr_err("%s:%d failed no memory\n", __func__, __LINE__);
		return -ENOMEM;
	}
	memset(ISP_vreg, 0x0, sizeof(ISP_vreg));

	AIT_ISPCtrl->pdata->ISP_reset = of_get_named_gpio((&pdev->dev)->of_node, "ISP_reset",0);
	CDBG("ISP_reset %d\n", AIT_ISPCtrl->pdata->ISP_reset);
	if (AIT_ISPCtrl->pdata->ISP_reset < 0) {
		pr_err("%s:%d ISP_reset rc %d\n", __func__, __LINE__, AIT_ISPCtrl->pdata->ISP_reset);
	}

	AIT_ISPCtrl->pdata->ISP_enable = of_get_named_gpio((&pdev->dev)->of_node, "ISP_enable",0);
	CDBG("ISP_enable %d\n", AIT_ISPCtrl->pdata->ISP_enable);
	if (AIT_ISPCtrl->pdata->ISP_enable < 0) {
		pr_err("%s:%d ISP_enable rc %d\n", __func__, __LINE__, AIT_ISPCtrl->pdata->ISP_enable);
	}

	AIT_ISPCtrl->pdata->ISP_APG6 = of_get_named_gpio((&pdev->dev)->of_node, "ISP_APG6",0);
	CDBG("ISP_APG6 %d\n", AIT_ISPCtrl->pdata->ISP_APG6);
	if (AIT_ISPCtrl->pdata->ISP_APG6 < 0) {
		pr_err("%s:%d ISP_APG6 rc %d\n", __func__, __LINE__, AIT_ISPCtrl->pdata->ISP_APG6);
	}

	AIT_ISPCtrl->pdata->ISP_APG7 = of_get_named_gpio((&pdev->dev)->of_node, "ISP_APG7",0);
	CDBG("ISP_APG7 %d\n", AIT_ISPCtrl->pdata->ISP_APG7);
	if (AIT_ISPCtrl->pdata->ISP_APG7 < 0) {
		pr_err("%s:%d ISP_APG7 rc %d\n", __func__, __LINE__, AIT_ISPCtrl->pdata->ISP_APG7);
	}


	AIT_ISPCtrl->pdata->ISP_mclk = of_get_named_gpio((&pdev->dev)->of_node, "ISP_mclk",0);
	CDBG("ISP_mclk %d\n", AIT_ISPCtrl->pdata->ISP_mclk);
	if (AIT_ISPCtrl->pdata->ISP_mclk < 0) {
		pr_err("%s:%d ISP_mclk rc %d\n", __func__, __LINE__, AIT_ISPCtrl->pdata->ISP_mclk);
	}
	
	AIT_ISPCtrl->pdata->ISP_CAM_SEL = of_get_named_gpio((&pdev->dev)->of_node, "ISP_CAM_SEL",0);
	CDBG("ISP_CAM_SEL %d\n", AIT_ISPCtrl->pdata->ISP_CAM_SEL);
	if (AIT_ISPCtrl->pdata->ISP_CAM_SEL < 0) {
		pr_err("%s:%d ISP_CAM_SEL rc %d\n", __func__, __LINE__, AIT_ISPCtrl->pdata->ISP_CAM_SEL);
	}

	AIT_ISPCtrl->pdata->ISP_CAM_SEL2 = of_get_named_gpio((&pdev->dev)->of_node, "ISP_CAM_SEL2",0);
	CDBG("ISP_CAM_SEL2 %d\n", AIT_ISPCtrl->pdata->ISP_CAM_SEL2);
	if (AIT_ISPCtrl->pdata->ISP_CAM_SEL2 < 0) {
		pr_err("%s:%d ISP_CAM_SEL2 rc %d\n", __func__, __LINE__, AIT_ISPCtrl->pdata->ISP_CAM_SEL2);
	}

	AIT_ISPCtrl->pdata->ISP_MCLK_SEL = of_get_named_gpio((&pdev->dev)->of_node, "ISP_MCLK_SEL",0);
	CDBG("ISP_MCLK_SEL %d\n", AIT_ISPCtrl->pdata->ISP_MCLK_SEL);
	if (AIT_ISPCtrl->pdata->ISP_MCLK_SEL < 0) {
		pr_err("%s:%d ISP_MCLK_SEL rc %d\n", __func__, __LINE__, AIT_ISPCtrl->pdata->ISP_MCLK_SEL);
	}


	AIT_ISPCtrl->pdata->ISP_V_SR_3V = of_get_named_gpio((&pdev->dev)->of_node, "ISP_V_SR_3V",0);
	CDBG("ISP_V_SR_3V %d\n", AIT_ISPCtrl->pdata->ISP_V_SR_3V);
	if (AIT_ISPCtrl->pdata->ISP_V_SR_3V < 0) {
		pr_err("%s:%d ISP_V_SR_3V rc %d\n", __func__, __LINE__, AIT_ISPCtrl->pdata->ISP_V_SR_3V);
	}

	AIT_ISPCtrl->pdata->ISP_gpio_vana = of_get_named_gpio((&pdev->dev)->of_node, "ISP_gpio_vana",0);
	CDBG("ISP_gpio_vana %d\n", AIT_ISPCtrl->pdata->ISP_gpio_vana);
	if (AIT_ISPCtrl->pdata->ISP_gpio_vana < 0) {
		pr_err("%s:%d ISP_gpio_vana rc %d\n", __func__, __LINE__, AIT_ISPCtrl->pdata->ISP_gpio_vana);
	}
#ifdef AIT_INT
	AIT_ISPCtrl->pdata->isp_intr0 = platform_get_irq_byname(pdev,"AIT_intr");
	CDBG("isp_intr0 %d\n", AIT_ISPCtrl->pdata->isp_intr0);
	if (AIT_ISPCtrl->pdata->isp_intr0 < 0) {
		pr_err("%s:%d isp_intr0 rc %d\n", __func__, __LINE__, AIT_ISPCtrl->pdata->isp_intr0);
	}
#endif
	ISP_vreg[0].reg_name = "ISP_1v2";
	ISP_vreg[1].reg_name = "ISP_1v8";
	ISP_vreg[2].reg_name = "ISP_A2v8";
	ISP_vreg_data[0] = NULL;
	ISP_vreg_data[1] = NULL;
	ISP_vreg_data[2] = NULL;

	AIT_ISPCtrl->pdata->camera_ISP_power_on = ISP_vreg_on;
	AIT_ISPCtrl->pdata->camera_ISP_power_off = ISP_vreg_off;

	rc = of_property_read_u32_array((&pdev->dev)->of_node, "ISP-vreg-type",
		tmp_array, 3);
	if (rc < 0) {
		pr_err("%s %d, ISP-vreg-type failed \n", __func__, __LINE__);
	}
	for (i = 0; i < 3; i++) {
		ISP_vreg[i].type = tmp_array[i];
		CDBG("%s ISP_vreg[%d].type = %d\n", __func__, i,
			ISP_vreg[i].type);
	}

	if (ISP_vreg[1].type == 2){
	    AIT_ISPCtrl->pdata->ISP_1v8_enable= of_get_named_gpio((&pdev->dev)->of_node, "ISP_1v8-gpio",0);
	    CDBG("ISP_1v8 %d\n", AIT_ISPCtrl->pdata->ISP_1v8_enable);
	    if (AIT_ISPCtrl->pdata->ISP_1v8_enable < 0) {
		pr_err("%s:%d ISP_1v2 rc %d\n", __func__, __LINE__, AIT_ISPCtrl->pdata->ISP_1v8_enable);
	    }
	}

	if (ISP_vreg[0].type == 2){
	    AIT_ISPCtrl->pdata->ISP_1v2_enable= of_get_named_gpio((&pdev->dev)->of_node, "ISP_1v2-gpio",0);
	    CDBG("ISP_1v2 %d\n", AIT_ISPCtrl->pdata->ISP_1v2_enable);
	    if (AIT_ISPCtrl->pdata->ISP_1v2_enable < 0) {
		pr_err("%s:%d ISP_1v2 rc %d\n", __func__, __LINE__, AIT_ISPCtrl->pdata->ISP_1v2_enable);
	    }
	}

	if (ISP_vreg[2].type == 2){
	    AIT_ISPCtrl->pdata->ISP_A2v8= of_get_named_gpio((&pdev->dev)->of_node, "ISP_A2v8-gpio",0);
	    CDBG("ISP_A2v8 %d\n", AIT_ISPCtrl->pdata->ISP_A2v8);
	    if (AIT_ISPCtrl->pdata->ISP_A2v8 < 0) {
		pr_err("%s:%d ISP_A2v8 rc %d\n", __func__, __LINE__, AIT_ISPCtrl->pdata->ISP_A2v8);
	    }
	}


	
	AIT_ISPCtrl->dev = &pdev->dev;
	

	rc1 = msm_AIT_ISP_attr_node();
	CDBG("%s -\n", __func__);

	return rc;
}

static int AIT_ISP_driver_remove(struct platform_device *pdev)
{
	CDBG("%s\n", __func__);
	AIT_ISP_tear_down_cdev();

	mutex_destroy(&AIT_ISPCtrl->AIT_ioctl_lock);

	kfree(AIT_ISPCtrl);

	return 0;
}

static const struct of_device_id AIT_dt_match[] = {
	{.compatible = "AIT_ISP", .data = NULL},
	{}
};

MODULE_DEVICE_TABLE(of, AIT_dt_match);



static struct  platform_driver AIT_ISP_driver = {
	.remove =AIT_ISP_driver_remove,
	.driver = {
		.name = "AIT_ISP",
		.owner = THIS_MODULE,
		.of_match_table = AIT_dt_match,
	},
};

static int __init AIT_ISP_driver_init(void)
{
	int rc;
	CDBG("%s + \n", __func__);
	rc = AIT_spi_init();
	if (rc < 0) {
		pr_err("%s: failed to register spi driver\n", __func__);
		return rc;
	}
	rc = platform_driver_probe(&AIT_ISP_driver, AIT_ISP_driver_probe);
	CDBG("%s - \n", __func__);
	return rc;
}

static void __exit AIT_ISP_driver_exit(void)
{
	CDBG("%s\n", __func__);
	platform_driver_unregister(&AIT_ISP_driver);
}

MODULE_DESCRIPTION("AIT_ISP driver");
MODULE_VERSION("AIT_ISP 0.1");

module_init(AIT_ISP_driver_init);
module_exit(AIT_ISP_driver_exit);

