
#ifndef _AIT_ISP_API_H_
#define _AIT_ISP_API_H_


#include "AIT_ISP_project.h"


#define	HOST_BASE_ADDR						(0x65E0)


#define	HOST_PARAMETER_0					(HOST_BASE_ADDR + 0x10)
#define	HOST_PARAMETER_1					(HOST_BASE_ADDR + 0x11)
#define	HOST_PARAMETER_2					(HOST_BASE_ADDR + 0x12)
#define	HOST_PARAMETER_3					(HOST_BASE_ADDR + 0x13)
#define	HOST_PARAMETER_4					(HOST_BASE_ADDR + 0x14)
#define	HOST_PARAMETER_5					(HOST_BASE_ADDR + 0x15)
#define	HOST_PARAMETER_6					(HOST_BASE_ADDR + 0x16)
#define	HOST_PARAMETER_7					(HOST_BASE_ADDR + 0x17)
#define	HOST_PARAMETER_8					(HOST_BASE_ADDR + 0x18)
#define	HOST_PARAMETER_9					(HOST_BASE_ADDR + 0x19)
#define	HOST_PARAMETER_A					(HOST_BASE_ADDR + 0x1A)
#define	HOST_PARAMETER_B					(HOST_BASE_ADDR + 0x1B)
#define	HOST_PARAMETER_C					(HOST_BASE_ADDR + 0x1C)
#define	HOST_PARAMETER_D					(HOST_BASE_ADDR + 0x1D)

#define HOST_CMD_ADDR0						(HOST_BASE_ADDR + 0x08)
#define HOST_CMD_ADDR1						(HOST_BASE_ADDR + 0x09)
#define HOST_STATUS_ADDR					(HOST_BASE_ADDR + 0x1E)

#define VENUS_CMD_TAIL						(0xE0)


#define VA_PREVIEW_STATUS_STOP				0 
#define VA_PREVIEW_STATUS_START				1 

typedef enum
{
	VENUS_COLOR_FORMAT_YUV422,
	VENUS_COLOR_FORMAT_YUV444,
	VENUS_COLOR_FORMAT_RGB888,
	VENUS_COLOR_FORMAT_RGB555,
	VENUS_COLOR_FORMAT_RGB565,
	VENUS_COLOR_FORMAT_RGB444,
	VENUS_COLOR_FORMAT_BAYERRAW
} VENUS_COLOR_FORMAT;


#define AE_MODE_AUTO						0
#define AE_MODE_MANUAL						1
#define AE_MODE_ISO							2
#define AE_MODE_SHUTTER						3

typedef enum
{
	AE_Level_1								= 0,
	AE_Level_2								= 1,
	AE_Level_3								= 2,
	AE_Level_4								= 3,
	AE_Level_5								= 4
} VENUS_AE_ISO;

typedef enum
{
	EV_Level_1								= 0,
	EV_Level_2								= 1,
	EV_Level_3								= 2,
	EV_Level_4								= 3,
	EV_Level_5								= 4,
	EV_Level_6								= 5,
	EV_Level_7								= 6,
	EV_Level_8								= 7,
	EV_Level_9								= 8
} VENUS_AE_EV;

typedef enum {
	ISP_AWB_MODE_BYPASS				= 0,
	ISP_AWB_MODE_AUTO				= 1,
	ISP_AWB_MODE_MANUAL				= 2,
	ISP_AWB_MODE_NUM
} ISP_AWB_MODE;;

typedef enum
{
	ISP_CUSTOM_WIN_MODE_OFF			= 0,
	ISP_CUSTOM_WIN_MODE_ON			= 1
} ISP_CUSTOM_WIN_MODE;


typedef enum
{
	VENUS_PREVIEW_FORMAT_YUV444					= 0,
	VENUS_PREVIEW_FORMAT_YUV422					= 1,
	VENUS_PREVIEW_FORMAT_YUV420					= 2,
	VENUS_PREVIEW_FORMAT_RGB888					= 3,
	VENUS_PREVIEW_FORMAT_RGB565					= 4,
	VENUS_PREVIEW_FORMAT_YUV422_UYVY			= 9
} VENUS_PREVIEW_FORMAT;

typedef enum {
	VENUS_ENABLE_VIF_FRAME_START_INTERRUPT			= 0x0001,
	VENUS_ENABLE_VIF_FRAME_END_INTERRUPT			= 0x0002,
	VENUS_ENABLE_IBC_FRAME_START_INTERRUPT			= 0x0004,
	VENUS_ENABLE_IBC_FRAME_END_INTERRUPT			= 0x0008,
	VENUS_ENABLE_DSI_DATA_ASYNC_FIFO_UNDERRUN		= 0x0010,
	VENUS_ENABLE_DSI_DATA_RT_OVERFLOW				= 0x0020,
	VENUS_ENABLE_I2CM_SLAVE_NOACK_INTERRUPT			= 0x0040,
	VENUS_ENABLE_I2CM_ADDR_WD_FIFO_FULL_INTERRUPT	= 0x0080,
	VENUS_ENABLE_I2CM_RD_FIFO_FULL_INTERRUPT		= 0x0100,
	VENUS_ENABLE_AE_STABLE_INTERRUPT				= 0x0200,
	VENUS_ENABLE_AWB_STABLE_INTERRUPT				= 0x0400,
	
	VENUS_ENABLE_ALL_INTERRUPT						= 0x07FF
} VENUS_INTERRUPT_CASE;

#define VA_MIN(a, b)						(((a) < (b)) ? (a) : (b))
#define VA_MAX(a, b)						(((a) > (b)) ? (a) : (b))
#define VR_ARRSIZE(a)						(sizeof((a)) / sizeof((a[0])))

uint8 VA_InitializeSensor(void);
uint8 VA_SetCaliTable(uint8 *SetCaliStatus);
uint8 VA_SetPreviewControl(uint8 status);

uint8 VA_SetPreviewResolution(uint16 width, uint16 height);

uint8 VA_SetPreviewFormat(VENUS_PREVIEW_FORMAT format);


uint8 VA_SetSharpness(uint8 Sharpness_parameter);

uint8 VA_SetGamma(uint8 Gamma_parameter);

uint8 VA_SetBrightness(uint8 Brightness_parameter);

uint8 VA_SetSaturation(uint8 Saturation_parameter);

uint8 VA_GetLux(uint32 *Lux);

uint8 VA_AutoAWBMode(ISP_AWB_MODE AWB_Mode);

uint8 VA_GetRGBGain(uint16 *R_Gain, uint16 *G_Gain, uint16 *B_Gain, uint16 *Gain_Base);

uint8 VA_SetRGBGain(uint16 RGain, uint16 GGain, uint16 BGain);

uint8 VA_SetAWBROIPosition(uint16 width, uint16 height, uint16 offsetx, uint16 offsety);

uint8 VA_GetAWBROIACC(uint32 *R_Sum, uint32 *G_Sum, uint32 *B_Sum, uint32 *pixel_count);


uint8 VA_SetFlickerMode(uint8 Flicker_Mode);

uint8 VA_SetISO(uint16 ISO_Parameter);

uint8 VA_SetISOLevel(VENUS_AE_ISO ISO_Level);

uint8 VA_SetEV(uint16 Exposure_Parameter);

uint8 VA_GetEV(uint16 *Exposure_Parameter);

uint8 VA_SetEVLevel(VENUS_AE_EV Exposure_Level);


typedef enum
{
    ISP_AE_MODE_P					= 0, 
    ISP_AE_MODE_A					= 1, 
    ISP_AE_MODE_S					= 2, 
    ISP_AE_MODE_M					= 3, 
    ISP_AE_MODE_NUM
} VENUS_AE_MODE;

uint8 VA_SetExposureMode(VENUS_AE_MODE AE_MODE);

uint8 VA_SetManualExposureTime(uint16 Exposure_Levle, uint16 Exposure_Partition);

uint8 VA_GetExposureTime(uint32 *N, uint32 *M);

uint8 VA_SetExposureTime(uint32 N_parameter);

uint8 VA_SetOverallGain(uint16 Overall_Gain);

uint8 VA_GetOverallGain(uint16 *Gain, uint16 *GainBase);


uint8 VA_SetAEROIPosition(uint16 width, uint16 height, uint16 offsetx, uint16 offsety);

uint8 VA_SetAEROIMeteringTable(uint8 *Metering_Ptr);	

uint8 VA_GetAEROICurrentLuma(uint32 *AERIO_Luma);

uint8 VA_CustomAEROIModeEnable(ISP_CUSTOM_WIN_MODE Custom_Enable);

uint8 VA_SetCustomAEROIPosition(uint16 StartX, uint16 EndX, uint16 StartY, uint16 EndY);

uint8 VA_SetCustomAEROIWeight(uint16 weight);			


uint8 VA_SetLumaFormula(uint16 paraR, uint16 paraGr, uint16 paraGb, uint16 paraB);

uint8 VA_SetLumaTargetMode(uint8 Enable);

uint8 VA_SetAEROILumaTarget(uint16 Luma_Target);

uint8 VA_GetCustomAEROILuma(uint16 *Luma_Value);


uint16 VA_GetSensorReg(uint16 sensor_reg_addr);

uint16 VA_ReadSensorOTP(uint8 *OTP_buf);

uint8 VA_SENSOR_READ_OTP(uint8 page, uint16 Address);


uint8 VA_GetIQGainStatus(uint16 *Gain_Status);

uint8 VA_GetIQEnergyStatus(uint16 *Energy_Status);

uint8 VA_GetIQColortempStatus(uint16 *Colortemp_Status);

uint8 VA_GetAEStatus(uint8 *AE_Status);

uint8 VA_GetAWBStatus(uint8 *AWB_Status);

uint8 VA_IQEnable(uint8 IQ_Enable, uint8 IQ_Case);

uint8 VA_GetIQOPR (uint8 *Status_1, uint8 *Status_2);



uint8 VA_SetChipStandby(void);

uint8 VA_SetChipStreaming(void);

uint8 VA_SetChipTriState(void);

uint8 VA_SetChipPowerOff(void);

uint8 VA_GetFWVersion(uint8 *version);

uint8 VA_HostEvent(uint16 event);


typedef enum
{
	VENUS_STREAMING_IN_FRAM_COUNT,
	VENUS_STREAMING_OUT_FRAM_COUNT
} VENUS_STREAMING_DEBUG_MODE;

uint8 VA_StreamingDebug(VENUS_STREAMING_DEBUG_MODE DebugCase, uint32 *DebugMessage);


typedef enum
{
	VENUS_SNR_S5K5E2_READ_ID,
	VENUS_SNR_OV2722_READ_ID,
	VENUS_SNR_S5K5E2_MODE,
	VENUS_SNR_OV2722_MODE,
	VENUS_SNR_S5K5E2_FRAME_COUNT,
	VENUS_SNR_S5K5E2_LANE_NUMBER,
	VENUS_SNR_OV2722_LANE_NUMBER
} VENUS_SENSOR_DEBUG_MODE;

uint8 VA_SensorDebug(VENUS_SENSOR_DEBUG_MODE DebugCase);

typedef enum
{
	VENUS_AE_GET_EV_BIAS_PARAMETER,
	VENUS_AE_GET_EV_BIAS_BASE,
	VENUS_GET_V_SYNC_PARAMETER,
	VENUS_GET_V_SYNC_BASE,
	VENUS_GET_EV_TARGET, 
	VENUS_GET_AVG_LUM,
	VENUS_AWB_GET_MODE
} VENUS_ISP_DEBUG_MODE;

uint8 VA_ISPDebug(VENUS_ISP_DEBUG_MODE DebugCase, uint32 *DebugMessage);


uint8 VA_HostIntertuptEnable(uint8 Enable);

uint8 VA_HostIntertuptModeEnable(uint32 InterruptCase);

uint8 VA_HostIntertuptModeDisable(uint32 InterruptCase);

uint8 VA_HostIntertuptModeStatusClear(uint32 InterruptCase);

uint8 VA_HostIntertuptModeCheckStatus(uint32 *InterruptCase);

uint8 VA_CheckHostInterruptStatusList(uint32 *Interrupt_Status);

uint8 VA_CustomHostIntertupt0Enable(uint8 Enable);

uint8 VA_CustomHostIntertupt0ClearStatus(uint8 Enable);

uint8 VA_CustomHostIntertupt0ModeEnable(uint8 InterruptCase);

uint8 VA_CustomHostIntertupt0ModeDisable(uint8 InterruptCase);

uint8 VA_CustomHostIntertupt0ModeStatusClear(uint8 InterruptCase);

uint8 VA_CustomHostIntertupt0ModeCheckStatus(uint8 *InterruptCase);

uint8 Dump_Opr(uint16 Start_Address, uint16 Dump_Size);

uint8 Debug_ISPStatus(void);

uint8 Debug_StreamingStatus(uint8 Enable_FrameCounterDebug, uint8 Enable_HostIntStatus);


uint8 TST_RDOP(void);

uint8 TST_WRDA(void);

uint8 TST_Bulk_RDOP_WRDA(void);

uint8 TST_Flow(void);

#endif 

