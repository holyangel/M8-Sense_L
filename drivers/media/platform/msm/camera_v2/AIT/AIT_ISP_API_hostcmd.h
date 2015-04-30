
#ifndef _AIT_ISP_API_HOSTCMD_H_
#define _AIT_ISP_API_HOSTCMD_H_

#define VENUS_CHIP_STANDBY							(0x0B)
#define VENUS_CHIP_STREAMING						(0x0C)
#define VENUS_CHIP_TRISTATE							(0xC0)
#define VENUS_CHIP_POWEROFF							(0xC1)
#define VENUS_BUTTON_EVENT                          (0xC2)
	#define VENUS_BUTTON_HALF_PRESSED               (0x0000)
	#define VENUS_BUTTON_HALF_RELEASED              (0x0001)
#define VENUS_INIT_SENSOR							(0x10)
#define VENUS_WRITE_SENSOR							(0x11)
#define VENUS_READ_SENSOR							(0x12)
#define VENUS_SENSOR_RESOLUTION						(0x32)
#define VENUS_SENSOR_PREVIEW_RESOLUTION				(0x33)
#define VENUS_GET_SENSOR_PREVIEW_RESOLUTION			(0x37)
#define VENUS_WRITE_ADDR							(0x65)
#define VENUS_READ_ADDR								(0x66)
#define VENUS_MOVE_MEM								(0x67)
#define VENUS_EXT_MEM_TEST							(0x60)
#define VENUS_COMPARE_MEM							(0x61)
#define VENUS_GET_FW_VERSION						(0xA2)


#define VENUS_PREVIEW_STOP							(0x0E)
#define VENUS_PREVIEW_START							(0x0F)
#define VENUS_SET_PREVIEW_RESOLUTION				(0x84)
#define VENUS_SET_PREVIEW_FORMAT					(0x85)

#define VENUS_SET_PREVIEW_SHADING_MODE				(0x93)


#define VENUS_WB_MODE								(0x07)


#define VENUS_SET_AE_MODE							(0xA8)

#define VENUS_SET_SATURATION						(0x14)
#define VENUS_SHARPNESS                 			(0x4E)
#define VENUS_GAMMA									(0x4F)
#define VENUS_HUE									(0x51)
#define VENUS_BRIGHTNESS                 			(0x54)
#define VENUS_SET_FLICKER_MODE						(0xAA)
#define VENUS_SET_ISO								(0xAB)



 
#define VENUS_SENSOR_READ_OTP								(0x2C)
#define VENUS_SENSOR_WRITE_OTP								(0x2D)
#define VENUS_SENSOR_SWITCH									(0x2E)
#define VENUS_AWB_ROI_SET_POSITION							(0x2F)

 
#define VENUS_SHAPRNESS_MANUAL								(0x4E)
#define VENUS_GAMMA_MANUAL									(0x4F)
#define VENUS_BRIGHTNESS_MANUAL								(0x54)
#define VENUS_SATURATION_MANUAL								(0x14)

#define VENUS_SET_CALIBRATION_DATA							(0x5A)

#define VENUS_GET_LUX										(0x5E)

 
#define VENUS_AWB_GET_RGB_GAIN								(0x90)
#define VENUS_AWB_SET_RGB_GAIN								(0x91)
#define VENUS_AWB_ROI_GET_ACC								(0x92)
	
 
#define VENUS_DEBUG_SENSOR									(0xD0)
#define VENUS_DEBUG_STREAMING								(0xD1)
#define VENUS_DEBUG_ISP										(0xD2)


#define VENUS_CUSTOM_HOST_INTERRUPT0						(0xD7)
typedef enum {
	VENUS_CUSTOM_HOST_INT0_AE_EFF			=	0x01,
	VENUS_CUSTOM_HOST_INT0_AWB_EFF			=	0x02,
	VENUS_CUSTOM_HOST_INT0_LUMA_EFF			=	0x04,
	VENUS_CUSTOM_HOST_INT0_Event			=	0x08,
	VENUS_CUSTOM_HOST_INT0_ALL				=	0x0F
} VENUS_CUSTOM_HOST_INT0_CASE;
#define VENUS_CUSTOM_HOST_INTERRUPT1						(0xD8)
#define VENUS_CUSTOM_HOST_INTERRUPT2						(0xD9)

 
#define VENUS_AE_CONVERGENCE_SPEED							(0xE3)


#define VENUS_AE_MANUAL_EV									(0xE6)
#define VENUS_SET_AE_ROI_METERING_TABLE						(0xE7)

#define VENUS_AE_GET_EXPOSURE_TIME							(0xE9)
#define VENUS_AE_SET_EXPOSURE_TIME							(0xEA)
#define VENUS_AE_GET_OVERALL_GAIN							(0xEB)
#define VENUS_AE_SET_OVERALL_GAIN							(0xEC)

#define VENUS_AE_GET_EV										(0xED)
#define VENUS_LUMA_TARGET_MODE								(0xEE)

 
#define VENUS_SET_LUMA_FORMULA								(0xF0)
#define VENUS_SET_AE_ROI_LUMA_TARGET					(0xF1)
#define VENUS_GET_AE_CUSTOM_ROI_LUMA						(0xF2)
#define VENUS_AE_ROI_SET_POSITION							(0xF3)

#define VENUS_CUSTOM_AE_ROI_SET_POSITION					(0xF5)
#define VENUS_SET_CUSTOM_AE_ROI_WEIGHT						(0xF6)
#define VENUS_CUSTOM_AE_ROI_ENABLE							(0xF7)


 
#define VENUS_IQ_GAIN_STATUS								(0xF9)
#define VENUS_IQ_ENERGY_STATUS								(0xFA)
#define VENUS_IQ_COLORTEMP_STATUS							(0xFB)
#define VENUS_IQ_AE_STATUS									(0xFC)
#define VENUS_IQ_AWB_STATUS									(0xFD)
#define VENUS_IQ_ENABLE										(0xFE)
#define VENUS_CLEAR_ALL_HOST_INTERRUPT						(0xFF)


#endif 
