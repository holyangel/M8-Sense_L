
#include "AIT_ISP_API.h"
#include "AIT_ISP_API_hostcmd.h"
#include "AIT_ISP_spi_ctl.h"
#include "AIT_ISP_System.h"


#define MAX_TIMEOUT_CNT			1000


uint8 VA_CheckReadyForCmd(void)
{
	uint8 ret;
	uint32 timeout_count = 0;
	uint16 Reg, Val;

	VA_HIGH("Call VA_CheckReadyForCmd\n");

	while((receive_byte_via_SPI(HOST_STATUS_ADDR) & 0x10) == 0 && timeout_count++ < MAX_TIMEOUT_CNT)
	{

		VA_DelayMS(10);
	}


	if(timeout_count >= MAX_TIMEOUT_CNT)
	{
		
		VA_ERR("VA_CheckReadyForCmd: Venus API Timeout!\n");
		Reg = 0x65F0;
		Val = receive_word_via_SPI(Reg);
		VA_ERR("Reg[%x] %x\n", Reg, Val);
		Reg = 0x65F2;
		Val = receive_word_via_SPI(Reg);
		VA_ERR("Reg[%x] %x\n", Reg, Val);
		Reg = 0x65F4;
		Val = receive_word_via_SPI(Reg);
		VA_ERR("Reg[%x] %x\n", Reg, Val);
		Reg = 0x65F6;
		Val = receive_word_via_SPI(Reg);
		VA_ERR("Reg[%x] %x\n", Reg, Val);
		Reg = 0x65F8;
		Val = receive_word_via_SPI(Reg);
		VA_ERR("Reg[%x] %x\n", Reg, Val);
		Reg = 0x65FA;
		Val = receive_word_via_SPI(Reg);
		VA_ERR("Reg[%x] %x\n", Reg, Val);
		Reg = 0x65FC;
		Val = receive_word_via_SPI(Reg);
		VA_ERR("Reg[%x] %x\n", Reg, Val);
		Reg = 0x65FE;
		Val = receive_word_via_SPI(Reg);
		VA_ERR("Reg[%x] %x\n", Reg, Val);
		Reg = 0x65E8;
		Val = receive_word_via_SPI(Reg);
		VA_ERR("Reg[%x] %x\n", Reg, Val);

		
		if ((receive_byte_via_SPI(0x6900) & 0xC0) != 0xC0)
		{
			VA_ERR("[ErrorMsg] Bootstrapping or SPI Interface error!.\n"); 
		}
		
		
		if(TST_Flow())
		{
			
			transmit_byte_via_SPI(0x6800, 0x00);

			ret = receive_byte_via_SPI(0x6003);
			VA_ERR("[DebugMsg] C_MIPI_RX : %x\n", ret);
			ret = receive_byte_via_SPI(0x6514);
			VA_ERR("[DebugMsg] C_IBC_PI0 : %x\n", ret);
			ret = receive_byte_via_SPI(0x6515);
			VA_ERR("[DebugMsg] C_IBC_PI1 : %x\n", ret);
			ret = receive_byte_via_SPI(0x70C7);
			VA_ERR("[DebugMsg] C_ISP_PROCESSING : %x\n", ret);
			ret = receive_byte_via_SPI(0x4B03);
			VA_ERR("[DebugMsg] C_MIPI_TX : %x\n", ret);

			transmit_byte_via_SPI(0x6001, 0xFF);
			transmit_byte_via_SPI(0x6516, 0xFF);
			transmit_byte_via_SPI(0x6517, 0xFF);
			transmit_byte_via_SPI(0x70C5, 0xFF);

			VA_DelayMS(200);
			
			ret = receive_byte_via_SPI(0x6003);
			VA_ERR("[DebugMsg] C_MIPI_RX : %x\n", ret);
			ret = receive_byte_via_SPI(0x6514);
			VA_ERR("[DebugMsg] C_IBC_PI0 : %x\n", ret);
			ret = receive_byte_via_SPI(0x6515);
			VA_ERR("[DebugMsg] C_IBC_PI1 : %x\n", ret);
			ret = receive_byte_via_SPI(0x70C7);
			VA_ERR("[DebugMsg] C_ISP_PROCESSING : %x\n", ret);
			ret = receive_byte_via_SPI(0x4B03);
			VA_ERR("[DebugMsg] C_MIPI_TX : %x\n", ret);

		
			ret = receive_byte_via_SPI(0x6001);
			VA_ERR("[DebugMsg] H_MIPI_RX : %x\n", ret);
			ret = receive_byte_via_SPI(0x6516);
			VA_ERR("[DebugMsg] H_IBC_PI0 : %x\n", ret);
			ret = receive_byte_via_SPI(0x6517);
			VA_ERR("[DebugMsg] H_IBC_PI1 : %x\n", ret);
			ret = receive_byte_via_SPI(0x70C5);
			VA_ERR("[DebugMsg] H_ISP_PROCESSING : %x\n", ret);
			
		}else{
			VA_ERR("[ErrorMsg] SPI Interface error\n"); 
		}
		

#if 0
		uint16 i;

		for(i = 0; i < 0x46; i += 2)
		{
			Reg = 0x6500 + i;
			Val = receive_word_via_SPI(Reg);
			VA_HIGH("Reg[%x] %x\n", Reg, Val);
		}
		Reg = 0x6580;
		Val = receive_word_via_SPI(Reg);
		VA_ERR("Reg[%x] %x\n", Reg, Val);
		Reg = 0x6582;
		Val = receive_word_via_SPI(Reg);
		VA_ERR("Reg[%x] %x\n", Reg, Val);
		Reg = 0x65A0;
		Val = receive_word_via_SPI(Reg);
		VA_ERR("Reg[%x] %x\n", Reg, Val);
		Reg = 0x65A4;
		Val = receive_word_via_SPI(Reg);
		VA_ERR("Reg[%x] %x\n", Reg, Val);

		for(i = 0; i < 0x62; i += 2)
		{
			Reg = 0x6200 + i;
			Val = receive_word_via_SPI(Reg);
			VA_ERR("Reg[%x] %x\n", Reg, Val);
		}

		for(i = 0; i < 0x38; i += 2)
		{
			Reg = 0x6000 + i;
			Val = receive_word_via_SPI(Reg);
			VA_ERR("Reg[%x] %x\n", Reg, Val);
		}

		for(i = 0; i < 0x3C; i += 2)
		{
			Reg = 0x6F00 + i;
			Val = receive_word_via_SPI(Reg);
			VA_ERR("Reg[%x] %x\n", Reg, Val);
		}

		for(i = 0x50; i < 0x5D; i += 2)
		{
			Reg = 0x6F00 + i;
			Val = receive_word_via_SPI(Reg);
			VA_ERR("Reg[%x] %x\n", Reg, Val);
		}

		Reg = 0x7900;
		Val = receive_word_via_SPI(Reg);
		VA_ERR("Reg[%x] %x\n", Reg, Val);
		Reg = 0x7902;
		Val = receive_word_via_SPI(Reg);
		VA_ERR("Reg[%x] %x\n", Reg, Val);
		Reg = 0x792C;
		Val = receive_word_via_SPI(Reg);
		VA_ERR("Reg[%x] %x\n", Reg, Val);
		Reg = 0x791C;
		Val = receive_word_via_SPI(Reg);
		VA_ERR("Reg[%x] %x\n", Reg, Val);
		Reg = 0x791E;
		Val = receive_word_via_SPI(Reg);
		VA_ERR("Reg[%x] %x\n", Reg, Val);
		
#endif
		return 1;
	}

	return 0;
}


uint8 VA_InitializeSensor(void)
{
	uint8 Result = 0;

	VA_HIGH("VA_InitializeSensor\n");

	Result = VA_CheckReadyForCmd();
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_INIT_SENSOR);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 VA_SetPreviewControl(uint8 status)
{
	uint8 Result = 0;

	uint32 timeout = 0;

	if(status == VA_PREVIEW_STATUS_START)
	{

		VA_HIGH("VA_SetPreviewControl: VA_PREVIEW_STATUS_START\n");

		Result = VA_CheckReadyForCmd();
		transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_PREVIEW_START);
		transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
		while((receive_byte_via_SPI(HOST_STATUS_ADDR) & 0x10) == 0 && timeout++ < 0x1000);

		if(timeout >= 0x1000)
		{
			VA_ERR("VA_SetPreviewControl: timeout\n");
		}

	}
	else     
	{

		VA_HIGH("VA_SetPreviewControl: VA_PREVIEW_STATUS_STOP\n");

		Result = VA_CheckReadyForCmd();
		transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_PREVIEW_STOP);
		transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
		Result = VA_CheckReadyForCmd();
	}

	return Result;
}


uint8 VA_SetPreviewResolution(uint16 width, uint16 height)
{
	uint8 Result = 0;

	VA_HIGH("VA_SetPreviewResolution: (Width x Height) = (%d x %d)\n", width, height);

	Result = VA_CheckReadyForCmd();
	transmit_word_via_SPI(HOST_PARAMETER_0, width);
	transmit_word_via_SPI(HOST_PARAMETER_2, height);
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_SET_PREVIEW_RESOLUTION);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 VA_SetPreviewFormat(VENUS_PREVIEW_FORMAT format)
{
	uint8 Result = 0;
	VA_HIGH("VA_SetPreviewFormat : format = %d\n", format);

	Result = VA_CheckReadyForCmd();
	transmit_byte_via_SPI(HOST_PARAMETER_0, format);
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_SET_PREVIEW_FORMAT);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();

	return Result;
}


uint8 VA_SetSharpness(uint8 Sharpness_parameter)
{
	uint8 Result = 0;

	VA_HIGH("VA_SetSharpness: Sharpness parameter = %d\n", Sharpness_parameter);

	Result = VA_CheckReadyForCmd();
	transmit_word_via_SPI(HOST_PARAMETER_0, Sharpness_parameter);
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_SHAPRNESS_MANUAL);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 VA_SetGamma(uint8 Gamma_parameter)
{
	uint8 Result = 0;

	VA_HIGH("VA_SetGamma: Gamma_parameter = %d\n", Gamma_parameter);

	Result = VA_CheckReadyForCmd();
	transmit_word_via_SPI(HOST_PARAMETER_0, Gamma_parameter);
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_GAMMA_MANUAL);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 VA_SetBrightness(uint8 Brightness_parameter)
{
	uint8 Result = 0;

	VA_HIGH("VA_SetBrightness: Brightness_parameter = %d\n", (uint8)Brightness_parameter);

	Result = VA_CheckReadyForCmd();
	transmit_word_via_SPI(HOST_PARAMETER_0, Brightness_parameter);
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_BRIGHTNESS_MANUAL);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 VA_SetSaturation(uint8 Saturation_parameter)
{
	uint8 Result = 0;

	VA_HIGH("VA_SetSaturation: Saturation_parameter = %d\n", (uint8)Saturation_parameter);

	Result = VA_CheckReadyForCmd();
	transmit_word_via_SPI(HOST_PARAMETER_0, Saturation_parameter);
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_SATURATION_MANUAL);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 VA_SetCaliTable(uint8 *SetCaliStatus)
{
	uint8 Result = 0;

	Result = VA_CheckReadyForCmd();
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_SET_CALIBRATION_DATA);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();

	
	*SetCaliStatus = receive_byte_via_SPI(HOST_PARAMETER_A);
	if((*SetCaliStatus)&0x01)
	{
		VA_HIGH("VA_SetCaliTable : Load Calibration successfully. (%d)\n",(*SetCaliStatus));
	}
	else
	{
		VA_HIGH("VA_SetCaliTable : Fail to load Calibration. (%d)\n",(*SetCaliStatus));
		VA_HIGH("BinCheckSum = %d\n",(((receive_word_via_SPI(HOST_PARAMETER_2) & 0xFFFF) << 16)+(receive_word_via_SPI(HOST_PARAMETER_0) & 0xFFFF)));
		VA_HIGH("FWCheckSum = %d\n",(((receive_word_via_SPI(HOST_PARAMETER_6) & 0xFFFF) << 16)+(receive_word_via_SPI(HOST_PARAMETER_4) & 0xFFFF)));
	}
	
	(*SetCaliStatus) &= 0x01; 

	return Result;
}

uint8 VA_GetLux(uint32 *Lux)
{
	uint8 Result = 0;

	Result = VA_CheckReadyForCmd();
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_GET_LUX);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();

	*Lux = ((receive_word_via_SPI(HOST_PARAMETER_A) & 0xFFFF) << 16);
	*Lux += (receive_word_via_SPI(HOST_PARAMETER_8) & 0xFFFF);
	
	#if 1
	VA_HIGH("VA_GetLux: Lux = %u\n", (uint32)(*Lux));
	#else
	VA_HIGH("VA_GetLux: Lux = %lu\n", (uint32)(*Lux));
	#endif
	
	return Result;
}


uint8 VA_AutoAWBMode(ISP_AWB_MODE AWB_Mode)
{
	

	uint8 Result = 0;

	VA_HIGH("VA_AutoWABMode\n");

	Result = VA_CheckReadyForCmd();
	transmit_byte_via_SPI(HOST_PARAMETER_0, AWB_Mode);
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_WB_MODE);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 VA_GetRGBGain(uint16 *R_Gain, uint16 *G_Gain, uint16 *B_Gain, uint16 *Gain_Base)
{
	uint8 Result = 0;

	VA_HIGH("VA_GetRGBGain\n");

	Result = VA_CheckReadyForCmd();
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_AWB_GET_RGB_GAIN);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();
	*Gain_Base = receive_word_via_SPI(HOST_PARAMETER_4);
	*R_Gain = receive_word_via_SPI(HOST_PARAMETER_6);
	*G_Gain = receive_word_via_SPI(HOST_PARAMETER_8);
	*B_Gain = receive_word_via_SPI(HOST_PARAMETER_A);
	return Result;
}

uint8 VA_SetRGBGain(uint16 R_Gain, uint16 G_Gain, uint16 B_Gain)
{
	uint8 Result = 0;

	VA_HIGH("VA_SetRGBGain\n");

	Result = VA_CheckReadyForCmd();

	if((R_Gain < 1024) || (R_Gain > 4087))
	{
		transmit_word_via_SPI(HOST_PARAMETER_0, 0xFFFF);
		if(R_Gain != 0xFFFF)
		{
			VA_HIGH("R_Gain is invalid and it will keep in original setting.\n");
		}
	}
	else
	{
		transmit_word_via_SPI(HOST_PARAMETER_0, R_Gain);
	}

	if((G_Gain < 1024) || (G_Gain > 4087))
	{
		transmit_word_via_SPI(HOST_PARAMETER_2, 0xFFFF);
		if(G_Gain != 0xFFFF)
		{
			VA_HIGH("G_Gain is invalid and it will keep in original setting.\n");
		}
	}
	else
	{
		transmit_word_via_SPI(HOST_PARAMETER_2, G_Gain);
	}

	if((B_Gain < 1024) || (B_Gain > 4087))
	{
		transmit_word_via_SPI(HOST_PARAMETER_4, 0xFFFF);
		if(B_Gain != 0xFFFF)
		{
			VA_HIGH("B_Gain is invalid and it will keep in original setting.\n");
		}
	}
	else
	{
		transmit_word_via_SPI(HOST_PARAMETER_4, B_Gain);
	}

	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_AWB_SET_RGB_GAIN);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 VA_SetAWBROIPosition(uint16 width, uint16 height, uint16 offsetx, uint16 offsety)
{
	
	uint8 Result = 0;

	if((32 <= width) && (32 <= height) && (0 <= offsetx) && (0 <= offsety))
	{
		VA_HIGH("VA_SetAWBROIPosition\n");

		Result = VA_CheckReadyForCmd();
		transmit_word_via_SPI(HOST_PARAMETER_0, width);
		transmit_word_via_SPI(HOST_PARAMETER_2, height);
		transmit_word_via_SPI(HOST_PARAMETER_4, offsetx);
		transmit_word_via_SPI(HOST_PARAMETER_6, offsety);
		transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_AWB_ROI_SET_POSITION);
		transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	}
	else
	{
		VA_HIGH("The coordinate of AWB ROI is invalid\n");
	}
	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 VA_GetAWBROIACC(uint32 *R_Sum, uint32 *G_Sum, uint32 *B_Sum, uint32 *pixel_count)
{
	
	uint8 Result = 0;
#if 1
	VA_HIGH("VA_GetAWBROIACC\n");
#else
	VA_HIGH("VA_SetAWBROIACC\n");
#endif
	Result = VA_CheckReadyForCmd();
	transmit_byte_via_SPI(HOST_PARAMETER_A, 0x01);
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_AWB_ROI_GET_ACC);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();

	*R_Sum = ((receive_word_via_SPI(HOST_PARAMETER_2) & 0xFFFF) << 16);
	*R_Sum += (receive_word_via_SPI(HOST_PARAMETER_0) & 0xFFFF);

	*G_Sum = ((receive_word_via_SPI(HOST_PARAMETER_6 & 0xFFFF)) << 16);
	*G_Sum += (receive_word_via_SPI(HOST_PARAMETER_4) & 0xFFFF);

	*B_Sum = ((receive_word_via_SPI(HOST_PARAMETER_A) & 0xFFFF) << 16);
	*B_Sum += (receive_word_via_SPI(HOST_PARAMETER_8) & 0xFFFF);

	transmit_byte_via_SPI(HOST_PARAMETER_A, 0x02);
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_AWB_ROI_GET_ACC);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();

	*pixel_count = ((receive_word_via_SPI(HOST_PARAMETER_A) & 0xFFFF) << 16);
	*pixel_count += (receive_word_via_SPI(HOST_PARAMETER_8) & 0xFFFF);

	return Result;
}


uint8 VA_SetFlickerMode(uint8 FlickerMode)
{
	
	uint8 Result = 0;

	if((FlickerMode == 0) || (FlickerMode == 2) || (FlickerMode == 3))
	{
		VA_HIGH("VA_SetFlickerMode: Mode = %d\n", FlickerMode);
		Result = VA_CheckReadyForCmd();
		transmit_byte_via_SPI(HOST_PARAMETER_0, FlickerMode);
		transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_SET_FLICKER_MODE);
		transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	}
	else
	{
		VA_HIGH("The parameter of Flicker Mode is invalid and it can't be worked.\n");
	}
	Result = VA_CheckReadyForCmd();

	return Result;
}


uint8 VA_SetISO(uint16 ISO_Parameter)
{
	
	uint8 Result = 0;

	if(((ISO_Parameter <= 1600) && (ISO_Parameter >= 50)) || (ISO_Parameter == 0))
	{
		VA_HIGH("VA_SetISO: ISO parameter = %d\n", ISO_Parameter);
		Result = VA_CheckReadyForCmd();
		transmit_word_via_SPI(HOST_PARAMETER_0, ISO_Parameter);
		transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_SET_ISO);
		transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	}
	else
	{
		VA_HIGH("ISO Level is invalid and it can't be worked.\n");
	}
	Result = VA_CheckReadyForCmd();

	return Result;
}


uint8 VA_SetISOLevel(VENUS_AE_ISO ISO_Level)
{
	
	uint8 Result = 0;

	if(ISO_Level < 5)
	{
		VA_HIGH("VA_SetISOLevel: ISO Level = %d\n", (int)(ISO_Level + 1));
		Result = VA_CheckReadyForCmd();
		transmit_word_via_SPI(HOST_PARAMETER_0, (50 + (1550 >> 2)*ISO_Level));
		transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_SET_ISO);
		transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	}
	else
	{
		VA_HIGH("ISO Level is invalid and it can't be worked.\n");
	}
	Result = VA_CheckReadyForCmd();

	return Result;
}


uint8 VA_SetEV(uint16 Exposure_Parameter)
{
	uint8 Result = 0;

	if(Exposure_Parameter < 501)
	{
		VA_HIGH("VA_SetEV: Exposure Parameter = %d\n", Exposure_Parameter);

		Result = VA_CheckReadyForCmd();
		transmit_word_via_SPI(HOST_PARAMETER_0, Exposure_Parameter);
		transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_AE_MANUAL_EV);
		transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	}
	else
	{
		VA_HIGH("EV Level is invalid and it can't be worked.\n");
	}
	Result = VA_CheckReadyForCmd();

	return Result;
}


uint8 VA_GetEV(uint16 *Exposure_Parameter)
{
	uint8 Result = 0;

	VA_HIGH("VA_GetEV\n");

	Result = VA_CheckReadyForCmd();
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_AE_GET_EV);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();
	*Exposure_Parameter = receive_word_via_SPI(HOST_PARAMETER_A);

	Result = VA_CheckReadyForCmd();

	return Result;
}


uint8 VA_SetEVLevel(VENUS_AE_EV Exposure_Level)
{
	
	uint8 Result = 0;

	if(Exposure_Level < 9)
	{
		VA_HIGH("VA_SetEVLevel: Exposure Level = %d\n", Exposure_Level);

		Result = VA_CheckReadyForCmd();
		transmit_word_via_SPI(HOST_PARAMETER_0, (256 >> 3)*Exposure_Level);
		transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_AE_MANUAL_EV);
		transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	}
	else
	{
		VA_HIGH("EV Level is invalid and it can't be worked.\n");
	}
	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 VA_SetExposureMode(VENUS_AE_MODE AE_MODE)
{
	uint8 Result = 0;

	VA_HIGH("VA_SetExposureMode\n");

	Result = VA_CheckReadyForCmd();

	transmit_byte_via_SPI(HOST_PARAMETER_0, AE_MODE);
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_SET_AE_MODE);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);

	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 VA_SetManualExposureTime(uint16 Exposure_Level, uint16 Exposure_Partition)
{
	
	uint8 Result = 0;
	uint16 ISP_ExposureTimeBase;


	VA_HIGH("VA_SetManualExposureTime: Exposure Time p= %d\n", Exposure_Level);

	if(Exposure_Level <= Exposure_Partition)
	{
		
		Result = VA_CheckReadyForCmd();
		transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_AE_GET_EXPOSURE_TIME);
		transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
		Result = VA_CheckReadyForCmd();
		ISP_ExposureTimeBase = receive_word_via_SPI(HOST_PARAMETER_8);

		Result = VA_CheckReadyForCmd();
		transmit_word_via_SPI(HOST_PARAMETER_0, (uint16)(ISP_ExposureTimeBase * Exposure_Level / Exposure_Partition));
		transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_AE_SET_EXPOSURE_TIME);
		transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	}
	else
	{
		VA_HIGH("N_parameter is invalid with ExposureTimeBase and it can't be worked.\n");
	}
	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 VA_SetExposureTime(uint32 N_parameter)
{
	uint8 Result = 0;
	uint32 Custom_ShutterBase = 1048576;	
	uint16 ISP_N_parameter, ISP_ExposureTimeBase;

	
	Result = VA_CheckReadyForCmd();
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_AE_GET_EXPOSURE_TIME);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();
	ISP_ExposureTimeBase = receive_word_via_SPI(HOST_PARAMETER_8);
	ISP_N_parameter = receive_word_via_SPI(HOST_PARAMETER_A);

	VA_HIGH("VA_SetExposureTime: Exposure Time p= %d\n", N_parameter);
	
	ISP_N_parameter = (uint16)(N_parameter /( Custom_ShutterBase / ISP_ExposureTimeBase ));

	Result = VA_CheckReadyForCmd();
	transmit_word_via_SPI(HOST_PARAMETER_0, ISP_N_parameter);
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_AE_SET_EXPOSURE_TIME);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 VA_GetExposureTime(uint32 *N_parameter, uint32 *ExposureTimeBase)
{
	uint8 Result = 0;
	uint32 Custom_ShutterBase = 1048576;	
	uint16 ISP_N_parameter, ISP_ExposureTimeBase;

	VA_HIGH("VA_GetExposureTime\n");

	Result = VA_CheckReadyForCmd();
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_AE_GET_EXPOSURE_TIME);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();
	ISP_ExposureTimeBase = receive_word_via_SPI(HOST_PARAMETER_8);
	ISP_N_parameter = receive_word_via_SPI(HOST_PARAMETER_A);

	
	*N_parameter = (uint32)((Custom_ShutterBase / ISP_ExposureTimeBase) * ISP_N_parameter);
	*ExposureTimeBase = Custom_ShutterBase;

	return Result;
}


uint8 VA_SetOverallGain(uint16 Overall_Gain)
{
	uint8 Result = 0;
	uint16 Custom_OverallGainBase = 256;	
	uint16 ISP_Gain, ISP_GainBase;

	Result = VA_CheckReadyForCmd();
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_AE_GET_OVERALL_GAIN);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();
	
	ISP_GainBase = receive_word_via_SPI(HOST_PARAMETER_8);
	ISP_Gain = receive_word_via_SPI(HOST_PARAMETER_A);

	VA_HIGH("VA_SetOverallGain: Overall Gain = %d\n", Overall_Gain);

	ISP_Gain = (uint16)(ISP_GainBase*Overall_Gain/Custom_OverallGainBase);
	
	Result = VA_CheckReadyForCmd();
	transmit_word_via_SPI(HOST_PARAMETER_0, ISP_Gain);
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_AE_SET_OVERALL_GAIN);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);

	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 VA_GetOverallGain(uint16 *Gain, uint16 *GainBase)
{
	uint8 Result = 0;
	uint16 Custom_OverallGainBase = 256;	
	uint16 ISP_Gain, ISP_GainBase;

	Result = VA_CheckReadyForCmd();
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_AE_GET_OVERALL_GAIN);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();
	
	ISP_GainBase = receive_word_via_SPI(HOST_PARAMETER_8);
	ISP_Gain = receive_word_via_SPI(HOST_PARAMETER_A);

	*GainBase = Custom_OverallGainBase;
	*Gain = (uint16)(Custom_OverallGainBase * ISP_Gain/ISP_GainBase);

	VA_HIGH("VA_GetOverallGain: Overall Gain = %d, Gain Base = %d\n", (uint16)(*Gain), (uint16)(*GainBase));

	return Result;
}


uint8 VA_SetLumaFormula(uint16 paraR, uint16 paraGr, uint16 paraGb, uint16 paraB)
{
	
	uint8 Result = 0;

	VA_HIGH("VA_SetLumaFormula\n");

	Result = VA_CheckReadyForCmd();
	transmit_word_via_SPI(HOST_PARAMETER_0, paraR);
	transmit_word_via_SPI(HOST_PARAMETER_2, paraGr);
	transmit_word_via_SPI(HOST_PARAMETER_4, paraGb);
	transmit_word_via_SPI(HOST_PARAMETER_6, paraB);
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_SET_LUMA_FORMULA);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 VA_SetLumaTargetMode(uint8 Enable)
{
	uint8 Result = 0;
	Result = VA_CheckReadyForCmd();

	transmit_byte_via_SPI(HOST_PARAMETER_0, Enable & 0x01);
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_LUMA_TARGET_MODE);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);

	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 VA_SetAEROILumaTarget(uint16 Luma_Target)
{
	uint8 Result = 0;

	VA_HIGH("VA_SetAEROILumaTarget\n");

	Result = VA_CheckReadyForCmd();
	transmit_word_via_SPI(HOST_PARAMETER_0, Luma_Target);
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_SET_AE_ROI_LUMA_TARGET);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 VA_GetCustomAEROILuma(uint16 *Luma_Value)
{
	uint8 Result = 0;

	Result = VA_CheckReadyForCmd();
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_GET_AE_CUSTOM_ROI_LUMA);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();
	*Luma_Value = receive_word_via_SPI(HOST_PARAMETER_A);
	Result = VA_CheckReadyForCmd();

	VA_HIGH("VA_GetCustomAEROILuma: AEROI Luma = %d\n", (uint16)(*Luma_Value));

	return Result;
}


uint8 VA_SetAEROIPosition(uint16 width, uint16 height, uint16 offsetx, uint16 offsety)
{
	
	uint8 Result = 0;

	if((32 <= width) && (32 <= height) && (0 <= offsetx) && (0 <= offsety))
	{
		VA_HIGH("VA_SetAEROIPosition\n");

		Result = VA_CheckReadyForCmd();
		transmit_word_via_SPI(HOST_PARAMETER_0, width);
		transmit_word_via_SPI(HOST_PARAMETER_2, height);
		transmit_word_via_SPI(HOST_PARAMETER_4, offsetx);
		transmit_word_via_SPI(HOST_PARAMETER_6, offsety);
		transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_AE_ROI_SET_POSITION);
		transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	}
	else
	{
		VA_HIGH("The parameter of AEROI Position is invalid in VA_SetAEROIPosition\n");
	}
	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 VA_SetAEROIMeteringTable(uint8 *Metering_Ptr)
{
	uint8 Result;
	uint8 BackUpValue1, BackUpValue2, BackUpValue3;
	uint32 MeteringAddress = 0x00000000;

	VA_CheckReadyForCmd(); 
	transmit_byte_via_SPI(HOST_PARAMETER_A, 0x01);
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_SET_AE_ROI_METERING_TABLE);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	VA_CheckReadyForCmd();

	BackUpValue1 = receive_byte_via_SPI(0x691E);
	BackUpValue2 = receive_byte_via_SPI(0x691D);
	BackUpValue3 = receive_byte_via_SPI(0x691C);
	VA_CheckReadyForCmd();

	MeteringAddress = ((receive_word_via_SPI(HOST_PARAMETER_A) & 0xFFFF) << 16);
	MeteringAddress += (receive_word_via_SPI(HOST_PARAMETER_8) & 0xFFFF);
	
	transmit_byte_via_SPI(0x691E, (MeteringAddress >> 27) & 0x03);
	transmit_byte_via_SPI(0x691D, (MeteringAddress >> 19) & 0xFF);
	transmit_byte_via_SPI(0x691C, (MeteringAddress >> 15) & 0x0F);
	transmit_multibytes_via_SPI(Metering_Ptr, 0x8000 + (MeteringAddress % 0x8000), 128);

	Result = VA_CheckReadyForCmd(); 
	transmit_byte_via_SPI(HOST_PARAMETER_A, 0x03);
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_SET_AE_ROI_METERING_TABLE);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);

	transmit_byte_via_SPI(0x691E, BackUpValue1);
	transmit_byte_via_SPI(0x691D, BackUpValue2);
	transmit_byte_via_SPI(0x691C, BackUpValue3);
	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 VA_GetAEROICurrentLuma(uint32 *AERIO_Luma)
{
	uint8 Result = 0;

	Result = VA_CheckReadyForCmd();

	transmit_word_via_SPI(HOST_PARAMETER_C, VENUS_GET_AVG_LUM);
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_DEBUG_ISP);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();
	*AERIO_Luma = ((receive_word_via_SPI(HOST_PARAMETER_A) & 0xFFFF) << 16);
	*AERIO_Luma += (receive_word_via_SPI(HOST_PARAMETER_8) & 0xFFFF);

	return Result;
}

uint8 VA_CustomAEROIModeEnable(ISP_CUSTOM_WIN_MODE Custom_Enable)
{
	uint8 Result = 0;

	VA_HIGH("VA_CustomAEROIEnable\n");

	Result = VA_CheckReadyForCmd();
	transmit_byte_via_SPI(HOST_PARAMETER_0, Custom_Enable);
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_CUSTOM_AE_ROI_ENABLE);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 VA_SetCustomAEROIPosition(uint16 Start_X_Index, uint16 End_X_Index, uint16 Start_Y_Index, uint16 End_Y_Index)
{
	uint8 Result = 0;

	VA_HIGH("VA_SetCustomAEROIPosition\n");

	Result = VA_CheckReadyForCmd();
	if((Start_X_Index < 16) && (End_X_Index < 16) && (Start_Y_Index < 16) && (End_Y_Index < 16))
	{
		transmit_word_via_SPI(HOST_PARAMETER_0, Start_X_Index);
		transmit_word_via_SPI(HOST_PARAMETER_2, End_X_Index);
		transmit_word_via_SPI(HOST_PARAMETER_4, Start_Y_Index);
		transmit_word_via_SPI(HOST_PARAMETER_6, End_Y_Index);
		transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_CUSTOM_AE_ROI_SET_POSITION);
		transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
		Result = VA_CheckReadyForCmd();
	}

	return Result;
}

uint8 VA_SetCustomAEROIWeight(uint16 weight)
{
	uint8 Result = 0;
	if(weight <= 256)
	{
		VA_HIGH("VA_SetCustomAEROIWeight\n");
		Result = VA_CheckReadyForCmd();
		transmit_word_via_SPI(HOST_PARAMETER_0, weight);
		transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_SET_CUSTOM_AE_ROI_WEIGHT);
		transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	}
	else
	{

		VA_HIGH("Custom AEROI Weight is invalid\n");
	}
	Result = VA_CheckReadyForCmd();

	return Result;
}


uint8 VA_GetIQGainStatus(uint16 *Gain_Status)
{
	
	uint8 Result = 0;

	Result = VA_CheckReadyForCmd();
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_IQ_GAIN_STATUS);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();
	*Gain_Status = receive_word_via_SPI(HOST_PARAMETER_A);

	return Result;
}

uint8 VA_GetIQEnergyStatus(uint16 *Energy_Status)
{
	
	uint8 Result = 0;

	Result = VA_CheckReadyForCmd();
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_IQ_ENERGY_STATUS);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();
	*Energy_Status = receive_word_via_SPI(HOST_PARAMETER_A);

	return Result;
}

uint8 VA_GetIQColortempStatus(uint16 *Colortemp_Status)
{
	
	uint8 Result = 0;

	Result = VA_CheckReadyForCmd();
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_IQ_COLORTEMP_STATUS);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();
	*Colortemp_Status = receive_word_via_SPI(HOST_PARAMETER_A);

	return Result;
}

uint8 VA_GetAEStatus(uint8 *AE_Status)
{
	uint8 Result = 0;

	Result = VA_CheckReadyForCmd();
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_IQ_AE_STATUS);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();
	*AE_Status = receive_byte_via_SPI(HOST_PARAMETER_A);

	return Result;
}

uint8 VA_GetAWBStatus(uint8 *AWB_Status)
{
	uint8 Result = 0;

	Result = VA_CheckReadyForCmd();
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_IQ_AWB_STATUS);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();
	*AWB_Status = receive_byte_via_SPI(HOST_PARAMETER_A);

	return Result;
}

uint8 VA_IQEnable(uint8 IQ_Enable, uint8 IQ_Case)
{
	uint8 Result = 0;
	Result = VA_CheckReadyForCmd();
	transmit_byte_via_SPI(HOST_PARAMETER_0, IQ_Enable);
	transmit_byte_via_SPI(HOST_PARAMETER_A, IQ_Case);
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_IQ_ENABLE);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 VA_GetIQOPR(uint8 *Status_1, uint8 *Status_2)
{
	uint8 Result = 0;
	uint8 BackUpValue1, BackUpValue2, BackUpValue3;

	uint32 Status_Address;

	BackUpValue1 = receive_byte_via_SPI(0x691E);
	BackUpValue2 = receive_byte_via_SPI(0x691D);
	BackUpValue3 = receive_byte_via_SPI(0x691C);

	Status_Address = 0x80007000;
	transmit_byte_via_SPI(0x691E, (Status_Address >> 27) & 0x03);
	transmit_byte_via_SPI(0x691D, (Status_Address >> 19) & 0xFF);
	transmit_byte_via_SPI(0x691C, (Status_Address >> 15) & 0x0F);
	transmit_multibytes_via_SPI(Status_1, 0x8000 + (Status_Address % 0x8000), 1024);

	Status_Address = 0x80000C00;
	transmit_byte_via_SPI(0x691E, (Status_Address >> 27) & 0x03);
	transmit_byte_via_SPI(0x691D, (Status_Address >> 19) & 0xFF);
	transmit_byte_via_SPI(0x691C, (Status_Address >> 15) & 0x0F);
	transmit_multibytes_via_SPI(Status_2, 0x8000 + (Status_Address % 0x8000), 1024);

	transmit_byte_via_SPI(0x691E, BackUpValue1);
	transmit_byte_via_SPI(0x691D, BackUpValue2);
	transmit_byte_via_SPI(0x691C, BackUpValue3);
	Result = VA_CheckReadyForCmd();

	return Result;
}


uint16 VA_GetSensorReg(uint16 sensor_reg_addr)
{
	uint16 reg_value;

	VA_CheckReadyForCmd();
	transmit_word_via_SPI(HOST_PARAMETER_0, sensor_reg_addr);
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_READ_SENSOR);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	VA_CheckReadyForCmd();

	reg_value = receive_word_via_SPI(HOST_PARAMETER_A);

	return reg_value;
}

uint16 VA_SetSensorReg(uint16 sensor_reg_addr, uint16 sensor_reg_value)
{
	VA_CheckReadyForCmd();
	transmit_word_via_SPI(HOST_PARAMETER_0, sensor_reg_addr);
	transmit_word_via_SPI(HOST_PARAMETER_2, sensor_reg_value);
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_WRITE_SENSOR);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	VA_CheckReadyForCmd();

	return 0;
}

uint16 VA_ReadSensorOTP(uint8 *OTP_buf)
{
	
	uint8 Result = 0;

	Result = VA_CheckReadyForCmd();
	transmit_byte_via_SPI(HOST_PARAMETER_A, 0x04);
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_SENSOR_READ_OTP);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();

	OTP_buf[0] = receive_byte_via_SPI(HOST_PARAMETER_0);
	OTP_buf[1] = receive_byte_via_SPI(HOST_PARAMETER_1);
	OTP_buf[2] = receive_byte_via_SPI(HOST_PARAMETER_2);
	OTP_buf[3] = receive_byte_via_SPI(HOST_PARAMETER_3);

	return Result;
}


uint8 VA_SetChipStandby(void)
{
	uint8 Result = 0;
	uint32 timeout_count = 0;

	VA_HIGH("VA_SetChipStandby\n");

	while((receive_byte_via_SPI(HOST_STATUS_ADDR) & 0x10) == 0)
	{

		VA_DelayMS(1);

		if(++timeout_count > MAX_TIMEOUT_CNT)
		{
			VA_HIGH("Venus API Timeout!\n");
			return 1;
		}
	}
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_CHIP_STANDBY);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);

	return Result;
}

uint8 VA_SetChipStreaming(void)
{
	uint8 Result = 0;

	VA_HIGH("VA_SetChipStreaming\n");

	Result = VA_CheckReadyForCmd();
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_CHIP_STREAMING);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 VA_SetChipTriState(void)
{
	uint8 Result = 0;

	VA_HIGH("VA_SetChipTriState\n");

	Result = VA_CheckReadyForCmd();
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_CHIP_TRISTATE);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 VA_SetChipPowerOff(void)
{
	uint8 Result = 0;
	VA_HIGH("VA_SetChipPowerOff\n");

	Result = VA_CheckReadyForCmd();
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_CHIP_POWEROFF);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 VA_SetSensorResolutionAsPreview(uint8 sensor_resolution)
{
	uint8 Result = 0;

	VA_HIGH("VA_SetSensorResolutionAsPreview : sensor_resolution = %d\n", sensor_resolution);

	Result = VA_CheckReadyForCmd();
	transmit_byte_via_SPI(HOST_PARAMETER_0, sensor_resolution);
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_SENSOR_PREVIEW_RESOLUTION);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 VA_GetFWVersion(uint8 *version)
{
	uint8 Result = 0;

	Result = VA_CheckReadyForCmd();
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_GET_FW_VERSION);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();

	version[0] = receive_byte_via_SPI(HOST_PARAMETER_0); 
	version[1] = receive_byte_via_SPI(HOST_PARAMETER_1); 
	version[2] = receive_byte_via_SPI(HOST_PARAMETER_2); 
	version[3] = receive_byte_via_SPI(HOST_PARAMETER_3); 

	VA_HIGH("VA_GetFWVersion: Version = 0x%02X.0x%02X.0x%02X.0x%02X\n", version[0], version[1], version[2], version[3]);

	return Result;
}

uint8 VA_HostEvent(uint16 event)
{
	uint8 Result = 0;
	VA_HIGH("VA_HostEvent : event = %d\n", event);

	Result = VA_CheckReadyForCmd();
	transmit_byte_via_SPI(HOST_PARAMETER_0, event & 0xff);
	transmit_byte_via_SPI(HOST_PARAMETER_1, (event >> 8) & 0xff);
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_BUTTON_EVENT);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();

	return Result;
}



uint8 VA_ISPDebug(VENUS_ISP_DEBUG_MODE DebugCase, uint32 *DebugMessage)
{
	uint8 Result = 0;

	Result = VA_CheckReadyForCmd();

	transmit_word_via_SPI(HOST_PARAMETER_C, DebugCase);
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_DEBUG_ISP);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();
	*DebugMessage = receive_word_via_SPI(HOST_PARAMETER_A) << 16;
	*DebugMessage += receive_word_via_SPI(HOST_PARAMETER_8) & 0xFFFF;

	switch(DebugCase)
	{
		case	VENUS_AE_GET_EV_BIAS_PARAMETER:
			VA_HIGH("VENUS_AE_GET_EV_BIAS_PARAMETER: EV Bias Parameter = %d\n", (int)(*DebugMessage - 250));
			break;

		case	VENUS_AE_GET_EV_BIAS_BASE:
			VA_HIGH("VENUS_AE_GET_EV_BIAS_BASE: EV Bias Base = %d\n", (int)(*DebugMessage));
			break;

		case	VENUS_GET_V_SYNC_PARAMETER:
			VA_HIGH("VENUS_GET_V_SYNC_PARAMETER: V-Sync Parameter = %d\n", (int)(*DebugMessage));
			break;

		case	VENUS_GET_V_SYNC_BASE:
			VA_HIGH("VENUS_GET_V_SYNC_BASE: V-Sync Base = %d\n", (int)(*DebugMessage));
			break;

		case	VENUS_GET_EV_TARGET:
			VA_HIGH("VENUS_GET_EV_TARGET: Luma Target = %d\n", (int)(*DebugMessage));
			break;

		case	VENUS_GET_AVG_LUM:
			VA_HIGH("VENUS_ROI_AVG_LUM: ROI Average Luma = %d\n", (int)(*DebugMessage));
			break;

		case	VENUS_AWB_GET_MODE:
			VA_HIGH("VENUS_AWB_GETMODE: AWB Mode (%d)\n", (int)(*DebugMessage));
			break;

		default:
			break;
	}
	return Result;
}


uint8 VA_StreamingDebug(VENUS_STREAMING_DEBUG_MODE DebugCase, uint32 *DebugMessage)
{
	uint8 Result = 0;
	Result = VA_CheckReadyForCmd();
	transmit_word_via_SPI(HOST_PARAMETER_C, DebugCase);
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_DEBUG_STREAMING);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();
	*DebugMessage = (uint32)receive_word_via_SPI(HOST_PARAMETER_A);
	switch(DebugCase)
	{
		case	VENUS_STREAMING_IN_FRAM_COUNT:
			VA_HIGH("VA_StreamingDebug: Input Frame Count= %d\n", (uint16)(*DebugMessage));
			break;

		case	VENUS_STREAMING_OUT_FRAM_COUNT:
			VA_HIGH("VA_StreamingDebug: Output Frame Count= %d\n", (uint16)(*DebugMessage));
			break;
		default:
			break;
	}
	return Result;
}


uint8 VA_SensorDebug(VENUS_SENSOR_DEBUG_MODE DebugCase)
{
	uint8 Result = 0;
	Result = VA_CheckReadyForCmd();
	transmit_word_via_SPI(HOST_PARAMETER_C, DebugCase);
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_DEBUG_SENSOR);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();
	switch(DebugCase)
	{
		case	VENUS_SNR_S5K5E2_READ_ID: 
		case	VENUS_SNR_OV2722_READ_ID:
			if(receive_word_via_SPI(HOST_PARAMETER_8) == 0x9882)
			{
				VA_HIGH("SNR_DEBUG_READ_ID: Sensor ID = 0x%x\n", receive_word_via_SPI(HOST_PARAMETER_A));
			}
			else
			{
				VA_HIGH("SNR_DEBUG_READ_ID: The I2C of sensor has be failed.\n");
			}
			break;

		case	VENUS_SNR_S5K5E2_MODE: 
		case	VENUS_SNR_OV2722_MODE: 
			if(receive_word_via_SPI(HOST_PARAMETER_A) & 0x0001)
			{
				VA_HIGH("SNR_DEBUG_READ_MODE: Streaming Mode\n");
			}
			else
			{
				VA_HIGH("SNR_DEBUG_READ_MODE: Software Standby Mode.\n");
			}
			break;

		case	VENUS_SNR_S5K5E2_FRAME_COUNT: 
			VA_HIGH("SNR_DEBUG_READ_FRAME_COUNT: %d\n", VA_GetSensorReg(0x0005));
			break;

		case	VENUS_SNR_S5K5E2_LANE_NUMBER:
		case	VENUS_SNR_OV2722_LANE_NUMBER:
			VA_HIGH("SNR_DEBUG_READ_LANE_NUMBER: The number of frames is %d\n", receive_word_via_SPI(HOST_PARAMETER_A) + 1);
			break;
		default:
			break;
	}
	return Result;
}

uint8 VA_HostIntertuptEnable(uint8 Enable)
{

	uint8 Result = 0;

	if(Enable)
	{
		transmit_byte_via_SPI(0x6800, 0x01);
		transmit_byte_via_SPI(0x6810, 0x21);
		transmit_byte_via_SPI(0x6811, 0x00);
		transmit_byte_via_SPI(0x6812, 0x53);
		transmit_byte_via_SPI(0x6813, 0x00);
	}
	else
	{
		transmit_byte_via_SPI(0x6800, 0x00);
	}

	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 VA_HostIntertuptModeEnable(uint32 InterruptCase)
{
	uint8 Result = 0, ret;

	if(InterruptCase & 0x07FF)
	{
		if(InterruptCase == VENUS_ENABLE_ALL_INTERRUPT)
		{
			transmit_byte_via_SPI(0x6000, 0x04 | 0x02 | 0x08 | 0x20);
			transmit_byte_via_SPI(0x6513, 0x02 | 0x04);
			transmit_byte_via_SPI(0x4B00, 0x40 | 0x80);
			transmit_byte_via_SPI(0x6978, 0x04);
			transmit_byte_via_SPI(0x6979, 0x04 | 0x20);

			Result = VA_CheckReadyForCmd();

			return Result;
		}
		if(InterruptCase & VENUS_ENABLE_VIF_FRAME_START_INTERRUPT)
	{
			ret = receive_byte_via_SPI(0x6000);
			transmit_byte_via_SPI(0x6000, ret | 0x04);
		}
		if(InterruptCase & VENUS_ENABLE_VIF_FRAME_END_INTERRUPT)
		{
			ret = receive_byte_via_SPI(0x6000);
			transmit_byte_via_SPI(0x6000, ret | 0x02);
		}

		if(InterruptCase & VENUS_ENABLE_IBC_FRAME_START_INTERRUPT)
		{
			ret = receive_byte_via_SPI(0x6513);
			transmit_byte_via_SPI(0x6513, ret | 0x02);
		}
		if(InterruptCase & VENUS_ENABLE_IBC_FRAME_END_INTERRUPT)
		{
			ret = receive_byte_via_SPI(0x6513);
			transmit_byte_via_SPI(0x6513, ret | 0x04);
		}
		if(InterruptCase & VENUS_ENABLE_DSI_DATA_ASYNC_FIFO_UNDERRUN)
		{
			ret = receive_byte_via_SPI(0x4B00);
			transmit_byte_via_SPI(0x4B00, ret | 0x40);
		}
		if(InterruptCase & VENUS_ENABLE_DSI_DATA_RT_OVERFLOW)
		{
			ret = receive_byte_via_SPI(0x4B00);
			transmit_byte_via_SPI(0x4B00, ret | 0x80);
		}
		if(InterruptCase & VENUS_ENABLE_I2CM_SLAVE_NOACK_INTERRUPT)
		{
			ret = receive_byte_via_SPI(0x6978);
			transmit_byte_via_SPI(0x6978, ret | 0x04);
		}
		if(InterruptCase & VENUS_ENABLE_I2CM_ADDR_WD_FIFO_FULL_INTERRUPT)
		{
			ret = receive_byte_via_SPI(0x6979);
			transmit_byte_via_SPI(0x6979, ret | 0x04);
		}
		if(InterruptCase & VENUS_ENABLE_I2CM_RD_FIFO_FULL_INTERRUPT)
		{
			ret = receive_byte_via_SPI(0x6979);
			transmit_byte_via_SPI(0x6979, ret | 0x20);
		}
		if(InterruptCase & VENUS_ENABLE_AE_STABLE_INTERRUPT)
		{
			ret = receive_byte_via_SPI(0x6000);
			transmit_byte_via_SPI(0x6000, (uint8)(ret | 0x08));
		}
		if(InterruptCase & VENUS_ENABLE_AWB_STABLE_INTERRUPT)
		{
			ret = receive_byte_via_SPI(0x6000);
			transmit_byte_via_SPI(0x6000, (uint8)(ret | 0x20));
		}
	}

	Result = VA_CheckReadyForCmd();

	return Result;

}

uint8 VA_HostIntertuptModeDisable(uint32 InterruptCase)
{

	uint8 Result = 0, ret;

	if(InterruptCase & 0x07FF)
	{
		if(InterruptCase == VENUS_ENABLE_ALL_INTERRUPT)
		{
			transmit_byte_via_SPI(0x6000, 0x00);
			transmit_byte_via_SPI(0x6513, 0x00);
			transmit_byte_via_SPI(0x4B00, 0x00);
			transmit_byte_via_SPI(0x6978, 0x00);
			transmit_byte_via_SPI(0x6979, 0x00);

			Result = VA_CheckReadyForCmd();

			return Result;
		}

		if(InterruptCase & VENUS_ENABLE_VIF_FRAME_START_INTERRUPT)
	{
			ret = receive_byte_via_SPI(0x6000);
			transmit_byte_via_SPI(0x6000, ret & (~0x04));
		}
		if(InterruptCase & VENUS_ENABLE_VIF_FRAME_END_INTERRUPT)
		{
			ret = receive_byte_via_SPI(0x6000);
			transmit_byte_via_SPI(0x6000, ret & (~0x02));
		}

		if(InterruptCase & VENUS_ENABLE_IBC_FRAME_START_INTERRUPT)
		{
			ret = receive_byte_via_SPI(0x6513);
			transmit_byte_via_SPI(0x6513, ret & (~0x02));
		}
		if(InterruptCase & VENUS_ENABLE_IBC_FRAME_END_INTERRUPT)
		{
			ret = receive_byte_via_SPI(0x6513);
			transmit_byte_via_SPI(0x6513, ret & (~0x04));
		}
		if(InterruptCase & VENUS_ENABLE_DSI_DATA_ASYNC_FIFO_UNDERRUN)
		{
			ret = receive_byte_via_SPI(0x4B00);
			transmit_byte_via_SPI(0x4B00, ret & (~0x40));
		}
		if(InterruptCase & VENUS_ENABLE_DSI_DATA_RT_OVERFLOW)
		{
			ret = receive_byte_via_SPI(0x4B00);
			transmit_byte_via_SPI(0x4B00, ret & (~0x80));
		}
		if(InterruptCase & VENUS_ENABLE_I2CM_SLAVE_NOACK_INTERRUPT)
		{
			ret = receive_byte_via_SPI(0x6978);
			transmit_byte_via_SPI(0x6978, ret & (~0x04));
		}
		if(InterruptCase & VENUS_ENABLE_I2CM_ADDR_WD_FIFO_FULL_INTERRUPT)
		{
			ret = receive_byte_via_SPI(0x6979);
			transmit_byte_via_SPI(0x6979, ret & (~0x04));
		}
		if(InterruptCase & VENUS_ENABLE_I2CM_RD_FIFO_FULL_INTERRUPT)
		{
			ret = receive_byte_via_SPI(0x6979);
			transmit_byte_via_SPI(0x6979, ret & (~0x20));
		}
		if(InterruptCase & VENUS_ENABLE_AE_STABLE_INTERRUPT)
		{
			ret = receive_byte_via_SPI(0x6000);
			transmit_byte_via_SPI(0x6000, ret & (~0x08));
		}
		if(InterruptCase & VENUS_ENABLE_AWB_STABLE_INTERRUPT)
		{
			ret = receive_byte_via_SPI(0x6000);
			transmit_byte_via_SPI(0x6000, ret & (~0x20));

		}

	}

	Result = VA_CheckReadyForCmd();

	return Result;

}

uint8 VA_HostIntertuptModeStatusClear(uint32 InterruptCase)
{

	uint8 Result = 0;

	if(InterruptCase & 0x07FF)
	{
		if(InterruptCase == VENUS_ENABLE_ALL_INTERRUPT)
		{
			Result = VA_CheckReadyForCmd();
			transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_CLEAR_ALL_HOST_INTERRUPT);
			transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);

			Result = VA_CheckReadyForCmd();

			return Result;
		}
		if(InterruptCase & VENUS_ENABLE_VIF_FRAME_START_INTERRUPT)
	{
			transmit_byte_via_SPI(0x6001, 0x04);
		}
		if(InterruptCase & VENUS_ENABLE_VIF_FRAME_END_INTERRUPT)
		{
			transmit_byte_via_SPI(0x6001, 0x02);
		}

		if(InterruptCase & VENUS_ENABLE_IBC_FRAME_START_INTERRUPT)
		{
			transmit_byte_via_SPI(0x6517, 0x02);
		}
		if(InterruptCase & VENUS_ENABLE_IBC_FRAME_END_INTERRUPT)
		{
			transmit_byte_via_SPI(0x6517, 0x04);
		}
		if(InterruptCase & VENUS_ENABLE_DSI_DATA_ASYNC_FIFO_UNDERRUN)
		{
			transmit_byte_via_SPI(0x4B01, 0x40);
		}
		if(InterruptCase & VENUS_ENABLE_DSI_DATA_RT_OVERFLOW)
		{
			transmit_byte_via_SPI(0x4B01, 0x80);
		}
		if(InterruptCase & VENUS_ENABLE_I2CM_SLAVE_NOACK_INTERRUPT)
		{
			transmit_byte_via_SPI(0x697A, 0x04);
		}
		if(InterruptCase & VENUS_ENABLE_I2CM_ADDR_WD_FIFO_FULL_INTERRUPT)
		{
			transmit_byte_via_SPI(0x697B, 0x04);
		}
		if(InterruptCase & VENUS_ENABLE_I2CM_RD_FIFO_FULL_INTERRUPT)
		{
			transmit_byte_via_SPI(0x697B, 0x20);
		}
		if(InterruptCase & VENUS_ENABLE_AE_STABLE_INTERRUPT)
		{
			transmit_byte_via_SPI(0x6001, 0x08);
		}
		if(InterruptCase & VENUS_ENABLE_AWB_STABLE_INTERRUPT)
		{

			transmit_byte_via_SPI(0x6001, 0x20);
		}
	}

	Result = VA_CheckReadyForCmd();

	return Result;

}


uint8 VA_HostIntertuptModeCheckStatus(uint32 * InterruptCase)
{
	uint8 Result = 0;
	if(receive_byte_via_SPI(0x6800) == 0x01)
	{
		if(receive_byte_via_SPI(0x6000)&receive_byte_via_SPI(0x6001) & 0x04)
		{
			*InterruptCase = VENUS_ENABLE_VIF_FRAME_START_INTERRUPT;
		}
		else if(receive_byte_via_SPI(0x6000)&receive_byte_via_SPI(0x6001) & 0x02)
		{
			*InterruptCase = VENUS_ENABLE_VIF_FRAME_END_INTERRUPT;
		}
		else if(receive_byte_via_SPI(0x6513)&receive_byte_via_SPI(0x6517) & 0x02)
		{
			*InterruptCase = VENUS_ENABLE_IBC_FRAME_START_INTERRUPT;
		}
		else if(receive_byte_via_SPI(0x6513)&receive_byte_via_SPI(0x6517) & 0x04)
		{
			*InterruptCase = VENUS_ENABLE_IBC_FRAME_END_INTERRUPT;
		}
		else if(receive_byte_via_SPI(0x4B00)&receive_byte_via_SPI(0x4B01) & 0x40)
		{
			*InterruptCase = VENUS_ENABLE_DSI_DATA_ASYNC_FIFO_UNDERRUN;
		}
		else if(receive_byte_via_SPI(0x4B00)&receive_byte_via_SPI(0x4B01) & 0x80)
		{
			*InterruptCase = VENUS_ENABLE_DSI_DATA_RT_OVERFLOW;
		}
		else if(receive_byte_via_SPI(0x6978)&receive_byte_via_SPI(0x697A) & 0x04)
		{
			*InterruptCase = VENUS_ENABLE_I2CM_SLAVE_NOACK_INTERRUPT;
		}
		else if(receive_byte_via_SPI(0x6979)&receive_byte_via_SPI(0x697B) & 0x04)
		{
			*InterruptCase = VENUS_ENABLE_I2CM_ADDR_WD_FIFO_FULL_INTERRUPT;
		}
		else if(receive_byte_via_SPI(0x6979)&receive_byte_via_SPI(0x697B) & 0x20)
		{
			*InterruptCase = VENUS_ENABLE_I2CM_RD_FIFO_FULL_INTERRUPT;
		}
		else if(receive_byte_via_SPI(0x6000)&receive_byte_via_SPI(0x6001) & 0x08)
		{
			*InterruptCase = VENUS_ENABLE_AE_STABLE_INTERRUPT;
		}
		else if(receive_byte_via_SPI(0x6000)&receive_byte_via_SPI(0x6001) & 0x20)
		{
			*InterruptCase = VENUS_ENABLE_AWB_STABLE_INTERRUPT;
		}
	}

	Result = VA_CheckReadyForCmd();

	return Result;

}

uint8 VA_CheckHostInterruptStatusList(uint32 *Interrupt_Status)
{
	uint8 StstusCount = 0;
	*Interrupt_Status = 0;

	if(receive_byte_via_SPI(0x6000)&receive_byte_via_SPI(0x6001) & 0x04)
	{
		*Interrupt_Status |= 1 << 0;
		VA_HIGH("\n[HostIntEventMsg][%d] : VIF Frame Stasrt\n", StstusCount);
		StstusCount++;
	}
	if(receive_byte_via_SPI(0x6000)&receive_byte_via_SPI(0x6001) & 0x02)
	{
		*Interrupt_Status |= 1 << 1;
		VA_HIGH("\n[HostIntEventMsg][%d] : VIF Frame End\n", StstusCount);
		StstusCount++;
	}

	if(receive_byte_via_SPI(0x6513)&receive_byte_via_SPI(0x6517) & 0x02)
	{
		*Interrupt_Status |= 1 << 2;
		VA_HIGH("\n[HostIntEventMsg][%d] : IBC Frame Stasrt\n", StstusCount);
		StstusCount++;
	}
	if(receive_byte_via_SPI(0x6513)&receive_byte_via_SPI(0x6517) & 0x04)
	{
		*Interrupt_Status |= 1 << 3;
		VA_HIGH("\n[HostIntEventMsg][%d] : IBC Frame End\n", StstusCount);
		StstusCount++;
	}

	if(receive_byte_via_SPI(0x4B00)&receive_byte_via_SPI(0x4B01) & 0x40)
	{
		*Interrupt_Status |= 1 << 4;
		VA_HIGH("\n[HostIntEventMsg][%d] : DSI UnderRun\n", StstusCount);
		StstusCount++;
	}
	if(receive_byte_via_SPI(0x4B00)&receive_byte_via_SPI(0x4B01) & 0x80)
	{
		*Interrupt_Status |= 1 << 5;
		VA_HIGH("\n[HostIntEventMsg][%d] : DSI OverFlow\n", StstusCount);
		StstusCount++;
	}

	if(receive_byte_via_SPI(0x6978)&receive_byte_via_SPI(0x697A) & 0x04)
	{
		*Interrupt_Status |= 1 << 6;
		VA_HIGH("\n[HostIntEventMsg][%d] : I2CM Slave NoACK\n", StstusCount);
		StstusCount++;
	}
	if(receive_byte_via_SPI(0x6979)&receive_byte_via_SPI(0x697B) & 0x04)
	{
		*Interrupt_Status |= 1 << 7;
		VA_HIGH("\n[HostIntEventMsg][%d] : I2CM write data FIFO full\n", StstusCount);
		StstusCount++;
	}
	if(receive_byte_via_SPI(0x6979)&receive_byte_via_SPI(0x697B) & 0x20)
	{
		*Interrupt_Status |= 1 << 8;
		VA_HIGH("\n[HostIntEventMsg][%d] : I2CM read data FIFO full\n", StstusCount);
		StstusCount++;
	}

	if(receive_byte_via_SPI(0x6000)&receive_byte_via_SPI(0x6001) & 0x08)
	{
		*Interrupt_Status |= 1 << 9;
		VA_HIGH("\n[HostIntEventMsg][%d] : AE Stable\n", StstusCount);
		StstusCount++;
	}
	if(receive_byte_via_SPI(0x6000)&receive_byte_via_SPI(0x6001) & 0x20)
	{
		*Interrupt_Status |= 1 << 10;
		VA_HIGH("\n[HostIntEventMsg][%d] : AWB Stable\n", StstusCount);
		StstusCount++;
	}

	return 0;

}

uint8 VA_CustomHostIntertupt0Enable(uint8 Enable)
{
	uint8 Result = 0;

	if(Enable)
	{
		transmit_byte_via_SPI(0x6000, receive_byte_via_SPI(0x6000) | 0x40);
	}
	else
	{
		transmit_byte_via_SPI(0x6000, receive_byte_via_SPI(0x6000) & (~0x40));
	}

	Result = VA_CheckReadyForCmd();

	return Result;
}


uint8 VA_CustomHostIntertupt0ClearStatus(uint8 Enable)
{
	uint8 Result = 0;

	if(Enable)
	{
		transmit_byte_via_SPI(0x6001, 0x40);
	}

	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 VA_CustomHostIntertupt0ModeEnable(uint8 InterruptCase)
{
	uint8 Result = 0;
	
	if(InterruptCase&0x0F)
	{
		transmit_byte_via_SPI(0x6225, ((InterruptCase & 0x0F) << 4));

		transmit_word_via_SPI(HOST_PARAMETER_0, 0x01);
		transmit_word_via_SPI(HOST_PARAMETER_C, InterruptCase&0x0F);
		transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_CUSTOM_HOST_INTERRUPT0);
		transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	}
	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 VA_CustomHostIntertupt0ModeDisable(uint8 InterruptCase)
{
	uint8 Result = 0;
	
	if(InterruptCase&0x0F)
	{
		transmit_byte_via_SPI(0x6225, receive_byte_via_SPI(0x6225) & (~(((InterruptCase & 0x0F) << 4)+(InterruptCase & 0x0F))));

		transmit_word_via_SPI(HOST_PARAMETER_0, 0x00);
		transmit_word_via_SPI(HOST_PARAMETER_C, InterruptCase&0x0F);
		transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_CUSTOM_HOST_INTERRUPT0);
		transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
		
	}

	Result = VA_CheckReadyForCmd();

	return Result;
}


uint8 VA_CustomHostIntertupt0ModeStatusClear(uint8 InterruptCase)
{
	uint8 Result = 0;

	transmit_byte_via_SPI(0x6225, receive_byte_via_SPI(0x6225) & (~(InterruptCase & 0x0F)));
	if(InterruptCase&0x0F)
	{
		transmit_byte_via_SPI(0x6001, 0x40);
	
		transmit_word_via_SPI(HOST_PARAMETER_0, 0x01);
		transmit_word_via_SPI(HOST_PARAMETER_C, InterruptCase&0x0F);
		transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_CUSTOM_HOST_INTERRUPT0);
		transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
		
		transmit_word_via_SPI(0x600A, 0xFFFF);
		transmit_byte_via_SPI(0x6001, 0x40);
	}
	
	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 VA_CustomHostIntertupt0ModeCheckStatus(uint8 *InterruptCase)
{
	uint8 Result = 0;

	*InterruptCase = receive_byte_via_SPI(0x6225) & 0x0F;

	Result = VA_CheckReadyForCmd();

	return Result;

}

uint8 Dump_Opr(uint16 Start_Address, uint16 Dump_Size)
{
	uint8 Result;
	uint16 i = 0;

	Result = VA_CheckReadyForCmd();

	VA_HIGH("[DumpOpr] Start Address : 0x%x && Dump Size : %d\n", Start_Address, Dump_Size);
	if(Start_Address & 0x000F)
	{
		Dump_Size += (Start_Address & 0x000F);
		Start_Address &= (~0x000F);
	}

	if (Dump_Size&0x000F)
	{
		Dump_Size += 0x10;
	}

	for(i = 0; i < Dump_Size; i=i+0x10)
		{
		VA_HIGH("[0x%4x] : 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x\n"
			, (Start_Address + i + 0x00)
			, receive_byte_via_SPI(Start_Address + i + 0x00)
			, receive_byte_via_SPI(Start_Address + i + 0x01)
			, receive_byte_via_SPI(Start_Address + i + 0x02)
			, receive_byte_via_SPI(Start_Address + i + 0x03)
			, receive_byte_via_SPI(Start_Address + i + 0x04)
			, receive_byte_via_SPI(Start_Address + i + 0x05)
			, receive_byte_via_SPI(Start_Address + i + 0x06)
			, receive_byte_via_SPI(Start_Address + i + 0x07)
			, receive_byte_via_SPI(Start_Address + i + 0x08)
			, receive_byte_via_SPI(Start_Address + i + 0x09)
			, receive_byte_via_SPI(Start_Address + i + 0x0A)
			, receive_byte_via_SPI(Start_Address + i + 0x0B)
			, receive_byte_via_SPI(Start_Address + i + 0x0C)
			, receive_byte_via_SPI(Start_Address + i + 0x0D)
			, receive_byte_via_SPI(Start_Address + i + 0x0E)
			, receive_byte_via_SPI(Start_Address + i + 0x0F));
	}

	Result = VA_CheckReadyForCmd();

	return Result;
}


uint8 Debug_ISPStatus(void)
{
	uint8 ret, Result = 0;

	ret = receive_byte_via_SPI(0x6224);

	if(ret & 0x0F)
	{
		VA_HIGH("[ErrorMsg] ISP : Please check the ISP status!. (%x)\n", (ret & 0x0F));
	}
	else
	{
		VA_HIGH("Debug_ISPStatus : %x\n", ret);
	}

	Result = VA_CheckReadyForCmd();

	return Result;
}

uint8 Debug_StreamingStatus(uint8 Enable_FrameCounterDebug, uint8 Enable_HostIntStatus)
{
	uint8 ret, ret0, ret1, ret2, Result;
	uint8 RecordHostInterrupt;
	uint32 Interrupt_Status;

	
	transmit_byte_via_SPI(0x6800, 0x00);
		
	ret = receive_byte_via_SPI(0x6003);
	VA_HIGH("[DebugMsg] C_MIPI_RX : %x\n", ret);
	ret = receive_byte_via_SPI(0x6514);
	VA_HIGH("[DebugMsg] C_IBC_PI0 : %x\n", ret);
	ret = receive_byte_via_SPI(0x6515);
	VA_HIGH("[DebugMsg] C_IBC_PI1 : %x\n", ret);
	ret = receive_byte_via_SPI(0x70C7);
	VA_HIGH("[DebugMsg] C_ISP_PROCESSING : %x\n", ret);
	ret = receive_byte_via_SPI(0x4B03);
	VA_HIGH("[DebugMsg] C_MIPI_TX : %x\n", ret);
	if((ret & 0xC0) || (Enable_FrameCounterDebug == 1))
	{
		
		if(ret & 0xC0)
		{
		VA_HIGH("[ErrorMsg] MIPI_TX : UnderRun and OverFlow\n"); 
		}
		
		if(Enable_HostIntStatus == 0x01)
		{
			
			RecordHostInterrupt = receive_byte_via_SPI(0x6800);

			
			
			transmit_byte_via_SPI(0x6001, 0xFF);
			transmit_byte_via_SPI(0x6516, 0xFF);
			transmit_byte_via_SPI(0x6517, 0xFF);
			transmit_byte_via_SPI(0x70C5, 0xFF);
		}

		
	Result = VA_CheckReadyForCmd();
	transmit_byte_via_SPI(HOST_PARAMETER_C, 0x03);
	transmit_byte_via_SPI(HOST_CMD_ADDR1, VENUS_DEBUG_STREAMING);
	transmit_byte_via_SPI(HOST_CMD_ADDR0, VENUS_CMD_TAIL);
	Result = VA_CheckReadyForCmd();
	ret0 = receive_byte_via_SPI(0x65F8);
	ret1 = receive_byte_via_SPI(0x65F9);
	ret2 = receive_byte_via_SPI(0x65FA);

	if((ret0 == 0) || (ret1 == 0) || (ret2 == 0))
	{
		if(ret0 == 0)
	{
			VA_HIGH("[ErrorMsg] VIF Pipe : Could be error, please check it!.\n"); 
	}
		if(ret1 == 0)
{
			VA_HIGH("[ErrorMsg] ISP Pipe : Could be error, please check it!.\n"); 
}
		if(ret2 == 0)
		{
			VA_HIGH("[ErrorMsg] IBC0 Pipe : Could be error, please check it!.\n"); 
	}
			
			VA_HIGH("\n[Dump Register] MIPI RX(0x6000) :\n");
			Dump_Opr(0x6000, 0x0100);

			VA_HIGH("\n[Dump Register] MIPI RX(0x6100) :\n");
			Dump_Opr(0x6100, 0x0100);
			
#if 0

			VA_HIGH("\n[Dump Register] Reset the the register 0x6180, 0x611F, 0x612F and 0x613F.\n");
			transmit_byte_via_SPI(0x6180, 0x07);
			transmit_byte_via_SPI(0x611F, 0x07);
			transmit_byte_via_SPI(0x612F, 0x07);
			transmit_byte_via_SPI(0x613F, 0x07);

			VA_HIGH("\n[Dump Register] SCALER :\n");
			Dump_Opr(0x6F00, 0x0090);
#endif

			VA_HIGH("\n[Dump Register] IBC (0x6500) :\n");
			Dump_Opr(0x6500, 0x0100);

			VA_HIGH("\n[Dump Register] MIPI TX (0x4B00) :\n");
			Dump_Opr(0x4B00, 0x0100);

	}

		if(Enable_HostIntStatus == 0x01)
		{
			ret = receive_byte_via_SPI(0x6001);
			VA_HIGH("[DebugMsg] H_MIPI_RX : %x\n", ret);
			ret = receive_byte_via_SPI(0x6516);
			VA_HIGH("[DebugMsg] H_IBC_PI0 : %x\n", ret);
			ret = receive_byte_via_SPI(0x6517);
			VA_HIGH("[DebugMsg] H_IBC_PI1 : %x\n", ret);
			ret = receive_byte_via_SPI(0x70C5);
			VA_HIGH("[DebugMsg] H_ISP_PROCESSING : %x\n", ret);

			
			if((RecordHostInterrupt & 0x01) == 0x01)
			{
				transmit_byte_via_SPI(0x6800, RecordHostInterrupt);

				VA_HIGH("\n[Message] During executing Debug_StreamingStatus function :\n");
				VA_HIGH("\n[Message] The Host Interrupt has be triggered as follow :\n");

				VA_CheckHostInterruptStatusList(&Interrupt_Status);
				if(!(Interrupt_Status))
				{
					VA_HIGH("None\n");
				}
			}
		}
		VA_HIGH("\n\n");
	}
	return Result;
}

uint8 TST_RDOP(void)
{
	if((receive_byte_via_SPI(0x6900) & 0xC0) == 0xC0)
		return 1;
	else
		return 0;
}

uint8 TST_WRDA(void)
{
	transmit_word_via_SPI(HOST_PARAMETER_0, 0x3421);
	if(receive_word_via_SPI(HOST_PARAMETER_0) == 0x3421)
		return 1;
	else
		return 0;
}

uint8 TST_Bulk_RDOP_WRDA(void)
{
	uint8 i, TesByte = 8;
	uint8 TestRX[8], TestTX[8];
	TestTX[0] = 0x01;
	TestTX[1] = 0x02;
	for(i = 2; i < 8; i++)
	{
		TestTX[i] = TestTX[i - 1] + TestTX[i - 2];
	}
	transmit_multibytes_via_SPI(TestTX, 0x65F0, TesByte);
	receive_multibytes_via_SPI(TestRX, 0x65F0, TesByte);
	for(i = 0; i < TesByte; i++)
	{
		if(TestRX[i] != TestTX[i])
		{
			return 0;
		}
	}
	return 1;
}

uint8 TST_Flow(void)
{
	if(TST_RDOP())
	{
		if(TST_WRDA())
		{
			if(TST_Bulk_RDOP_WRDA())
			{
				VA_HIGH("SPI Interface is OK!\n");
				return 1;
			}
			else
			{
				VA_ERR("[ErrorMsg] TST_WRDA fail\n");
			}
		}
		else
		{
			VA_ERR("[ErrorMsg] TST_WRDA fail\n");
		}

	}
	else
	{
		VA_ERR("[ErrorMsg] TST_RDOP fail\n");
	}
	return 0;
}

