/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, and the entire permission notice in its entirety,
 *    including the disclaimer of warranties.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * ALTERNATIVELY, this product may be distributed under the terms of
 * the GNU General Public License, version 2, in which case the provisions
 * of the GPL version 2 are required INSTEAD OF the BSD license.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ALL OF
 * WHICH ARE HEREBY DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF NOT ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#ifndef __LINUX_AIT_ISP_H
#define __LINUX_AIT_ISP_H

#include <linux/types.h>
#include <linux/ioctl.h>


#define AIT_ISP_IOCTL_MAGIC 'y'

#define IOCTL_AIT_ISP_GET_INT 1
#define IOCTL_AIT_ISP_ENABLE_INT 2
#define IOCTL_AIT_ISP_EN_MEC_MWB 3
#define IOCTL_AIT_ISP_CONFIG_MEC 4
#define IOCTL_AIT_ISP_CONFIG_MWB 5
#define IOCTL_AIT_ISP_GET_MEC 6
#define IOCTL_AIT_ISP_GET_MWB 7
#define IOCTL_AIT_ISP_GET_CUSTOM_AE_ROI_LUMA 8
#define IOCTL_AIT_ISP_GET_AWB_ROI_ACC 9
#define IOCTL_AIT_ISP_SET_AE_ROI_POSITION 10
#define IOCTL_AIT_ISP_SET_AWB_ROI_POSITION 11
#define IOCTL_AIT_ISP_CUSTOM_AE_ROI_MODE_ENABLE 12
#define IOCTL_AIT_ISP_SET_LUMA_FORMULA 13
#define IOCTL_AIT_ISP_SET_AE_ROI_METERING_TABLE  14
#define IOCTL_AIT_ISP_SET_CUSTOM_AE_ROI_WEIGHT  15
#define IOCTL_AIT_ISP_SET_CUSTOM_AE_ROI_POSITION  16
#define IOCTL_AIT_ISP_GET_AE_ROI_CURRENT_LUMA  17

#define AIT_ISP_GET_INT \
	_IOR(AIT_ISP_IOCTL_MAGIC, IOCTL_AIT_ISP_GET_INT, uint32_t *)

#define AIT_ISP_ENABLE_INT \
	_IOR(AIT_ISP_IOCTL_MAGIC, IOCTL_AIT_ISP_ENABLE_INT, uint32_t *)

#define AIT_ISP_EN_MEC_MWB \
	_IOR(AIT_ISP_IOCTL_MAGIC, IOCTL_AIT_ISP_EN_MEC_MWB, uint32_t *)

#define AIT_ISP_CONFIG_MEC \
	_IOR(AIT_ISP_IOCTL_MAGIC, IOCTL_AIT_ISP_CONFIG_MEC, struct MECData *)

#define AIT_ISP_CONFIG_MWB \
	_IOR(AIT_ISP_IOCTL_MAGIC, IOCTL_AIT_ISP_CONFIG_MWB, struct MWBData *)

#define AIT_ISP_GET_MEC \
	_IOR(AIT_ISP_IOCTL_MAGIC, IOCTL_AIT_ISP_GET_MEC, struct MECData *)

#define AIT_ISP_GET_MWB \
	_IOR(AIT_ISP_IOCTL_MAGIC, IOCTL_AIT_ISP_GET_MWB, struct MWBData *)

#define AIT_ISP_GET_CUSTOM_AE_ROI_LUMA \
	_IOR(AIT_ISP_IOCTL_MAGIC, IOCTL_AIT_ISP_GET_CUSTOM_AE_ROI_LUMA, uint16_t *)

#define AIT_ISP_GET_AWB_ROI_ACC \
	_IOR(AIT_ISP_IOCTL_MAGIC, IOCTL_AIT_ISP_GET_AWB_ROI_ACC, struct AWB_ROI_ACC *)

#define AIT_ISP_SET_AE_ROI_POSITION \
	_IOR(AIT_ISP_IOCTL_MAGIC, IOCTL_AIT_ISP_SET_AE_ROI_POSITION, struct ROI_position *)

#define AIT_ISP_SET_AWB_ROI_POSITION \
	_IOR(AIT_ISP_IOCTL_MAGIC, IOCTL_AIT_ISP_SET_AWB_ROI_POSITION, struct ROI_position *)

#define AIT_ISP_CUSTOM_AE_ROI_MODE_ENABLE \
	_IOR(AIT_ISP_IOCTL_MAGIC, IOCTL_AIT_ISP_CUSTOM_AE_ROI_MODE_ENABLE, int *)

#define AIT_ISP_SET_LUMA_FORMULA \
	_IOR(AIT_ISP_IOCTL_MAGIC, IOCTL_AIT_ISP_SET_LUMA_FORMULA, struct Luma_formula *)

#define AIT_ISP_SET_AE_ROI_METERING_TABLE \
	_IOR(AIT_ISP_IOCTL_MAGIC, IOCTL_AIT_ISP_SET_AE_ROI_METERING_TABLE, uint8_t *)

#define AIT_ISP_SET_CUSTOM_AE_ROI_WEIGHT \
	_IOR(AIT_ISP_IOCTL_MAGIC, IOCTL_AIT_ISP_SET_CUSTOM_AE_ROI_WEIGHT, uint16_t *)

#define AIT_ISP_SET_CUSTOM_AE_ROI_POSITION \
	_IOR(AIT_ISP_IOCTL_MAGIC, IOCTL_AIT_ISP_SET_CUSTOM_AE_ROI_POSITION, struct custom_roi_position *)

#define AIT_ISP_GET_AE_ROI_CURRENT_LUMA \
	_IOR(AIT_ISP_IOCTL_MAGIC, IOCTL_AIT_ISP_GET_AE_ROI_CURRENT_LUMA, uint32_t *)

#define ENABLE_MANUAL_MEC 1
#define ENABLE_MANUAL_MWB 2

#define ENABLE_VIF_FRAME_START_INTERRUPT			0x0001
#define ENABLE_VIF_FRAME_END_INTERRUPT			0x0002
#define ENABLE_IBC_FRAME_START_INTERRUPT			0x0004
#define ENABLE_IBC_FRAME_END_INTERRUPT			0x0008
#define ENABLE_DSI_DATA_ASYNC_FIFO_UNDERRUN		0x0010
#define ENABLE_DSI_DATA_RT_OVERFLOW				0x0020
#define ENABLE_I2CM_SLAVE_NOACK_INTERRUPT			0x0040
#define ENABLE_I2CM_ADDR_WD_FIFO_FULL_INTERRUPT	0x0080
#define ENABLE_I2CM_RD_FIFO_FULL_INTERRUPT		0x0100
#define ENABLE_AE_STABLE_INTERRUPT				0x0200
#define ENABLE_AWB_STABLE_INTERRUPT				0x0400
#define ENABLE_ALL_INTERRUPT						0x07FF

typedef struct{
      uint32_t N_parameter;
      uint16_t Overall_Gain;
} MECData;

typedef struct{
      uint16_t R_Gain;
      uint16_t G_Gain;
      uint16_t B_Gain;
} MWBData;

typedef struct{
      uint32_t R_Sum;
      uint32_t G_Sum;
      uint32_t B_Sum;
      uint32_t pixel_count;
} AWB_ROI_ACC;

typedef struct{
      uint16_t width;
      uint16_t height;
      uint16_t offsetx;
      uint16_t offsety;
} ROI_position;

typedef struct{
      uint16_t paraR;
      uint16_t paraGr;
      uint16_t paraGb;
      uint16_t paraB;
} Luma_formula;

typedef struct{
      uint16_t Start_X_Index;
      uint16_t End_X_Index;
      uint16_t Start_Y_Index;
      uint16_t End_Y_Index;
} custom_roi_position;

struct msm_camera_AIT_info {
	int ISP_reset;
	int ISP_intr;
	int ISP_enable;
	int ISP_APG6;
	int ISP_APG7;
	int ISP_CAM_SEL;
	int ISP_CAM_SEL2;
	int ISP_MCLK_SEL;
	int ISP_V_SR_3V;
	int ISP_gpio_vana;
	int ISP_sub_reset;
	int ISP_front_reset;
	int (*camera_ISP_power_on)(void);
	int (*camera_ISP_power_off)(void);
	uint32_t ISP_1v8_enable;
	uint32_t ISP_1v2_enable;
	uint32_t ISP_A2v8;
	uint32_t ISP_mclk;
	int isp_intr0;
};

#endif 

