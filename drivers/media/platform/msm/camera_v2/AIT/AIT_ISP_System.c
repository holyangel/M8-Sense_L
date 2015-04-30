#include "AIT_ISP_project.h"
#include "AIT_ISP_API.h"
#include "AIT_ISP_spi_ctl.h"
#include "AIT_ISP_API_hostcmd.h"
#include "AIT_ISP_System.h"

#if 0
#include <stdlib.h>
#endif
extern void HTC_DelayMS(uint16 ms); 
static int gDlFWmode = FW_DRAM_LOAD_DEFAULT_MODE;
#define AIT_FW_BUFFER 0x01040000L

void V3A_SetFW_DLmode(uint8 mode)
{
	gDlFWmode = mode;
}

uint8 V3A_GetFW_DLmode(void)
{
	return gDlFWmode;
}

void VA_DelayMS(uint16 ms)
{
	
	
	#if 1
	HTC_DelayMS(ms);
	#else
	
	#endif
	
}

uint8 V3A_ChipEnable(void)
{
	return 0;
}

uint8 V3A_ChipReset(void)
{
	
	
	return 0;
}

uint8 VA_FirmwareUpdateStatus = 1;
uint8 VA_FirmwareOffset = 0;

uint8 V3_NeedInitSensor = 0;


uint8 V3A_PowerOn(void)
{
	VA_HIGH("V3A_PowerOn : start\n");

	
	V3A_ChipEnable();

	V3A_ChipReset();

	
	V3A_PLLOn();

	VA_HIGH("V3A_PowerOn : end\n");

	return 0;
}

uint8 V3A_PowerOff(void)
{
	VA_HIGH("V3A_PowerOff : start\n");
	
	VA_SetChipPowerOff();
	VA_HIGH("V3A_PowerOff : end\n");

	return 0;
}

uint8 V3A_PowerUp(uint8 *fw_ptr, uint32 fw_size, uint8 *Cali_ptr, uint32 Cali_size)
{
	uint32 timeout_count = 0;

	VA_HIGH("V3A_PowerUp : start\n");

	V3_NeedInitSensor = 1;

#if V3_FORCE_TO_DOWNLOAD_FW
	VA_INFO("+++ force to download FW\n");
	VA_FirmwareUpdateStatus = 1;
#elif V3_FORCE_TO_SKIP_DOWNLOAD_FW
	VA_INFO("+++ force to skip download FW\n");
	VA_FirmwareUpdateStatus = 0;
#elif V3_AUTO_DOWNLOAD_FW
	VA_INFO("+++ auto download FW\n");

	VA_FirmwareUpdateStatus = 0;
	V3A_PLLOn();

	VA_INFO("Wait for booting ... \r\n");
	
	while(((receive_byte_via_SPI(HOST_STATUS_ADDR) & 0x10) == 0) && timeout_count++ < V3_TIMEOUT_CNT)
	{
		VA_INFO("Reg 0x%X = 0x%x\n", HOST_STATUS_ADDR, receive_byte_via_SPI(HOST_STATUS_ADDR));
		VA_DelayMS(5);
	}

	if(timeout_count >= V3_TIMEOUT_CNT)
	{
		VA_ERR("Need to download FW\n");
		VA_FirmwareUpdateStatus = 1;
	}
	else
	{
		return 0;
	}

#endif
	
	if(VA_FirmwareUpdateStatus)
    {
        V3A_FirmwareStart( fw_ptr, fw_size, Cali_ptr, Cali_size);
	}
	else
	{
		V3A_PLLOn();

		VA_MSG("Wait for booting ... \r\n");
		while(((receive_byte_via_SPI(HOST_STATUS_ADDR) & 0x10) == 0) && timeout_count++ < V3_TIMEOUT_CNT)
		{
			VA_INFO("Reg 0X%X = 0x%x\n", HOST_STATUS_ADDR, receive_byte_via_SPI(HOST_STATUS_ADDR));
			VA_DelayMS(5);
		}

		if(timeout_count >= V3_TIMEOUT_CNT)
		{
			VA_ERR("Time out! V3A_PowerUp fail\n");
			return 1;
		}
	}



	VA_HIGH("V3A_PowerUp : end\n");

	return 0;
}


uint8 V3A_PLLOn(void)
{
	uint32 timeout_count = 0;

	
	VA_DelayMS(5);

	
	VA_MSG("Wait DMA complete from SIF to CPU program memory ... ");
	VA_INFO("0x6713=0x%x\n", receive_byte_via_SPI(0x6713));
	
	while(receive_byte_via_SPI(0x6713) != 0 && timeout_count++ < V3_TIMEOUT_CNT)
	{
		VA_DelayMS(1);
	}

	if(timeout_count >= V3_TIMEOUT_CNT)
	{
		VA_ERR("Time out! DMA from SIF to CPU program memory fail.\n");
		return 1;
	}

	VA_HIGH("Done\n");

	return 0;
}


uint8 V3A_FirmwareStart(uint8 *fw_ptr, uint32 fw_size, uint8 *Cali_ptr, uint32 Cali_size)
{
	uint32 timeout_count = 0;

	VA_HIGH("VA Firmware Start : start\n");

	
	V3A_PLLOn();

	
	transmit_byte_via_SPI(0x6901, 0x06);

	
	V3A_DownloadFirmware(fw_ptr, fw_size);

	
    if ((Cali_ptr!=NULL)&&(Cali_size!=0))
	{
		VA_HIGH("download Calibration Table to firmware : start\n");
		V3A_DownloadCaliTable(Cali_ptr, Cali_size);
		VA_HIGH("download Calibration Table to firmware : end\n");
    }

	
	transmit_byte_via_SPI(0x6901, 0x00);

	VA_DelayMS(100);   

	
	VA_HIGH("wait for initialization finished : start\n");
	while(((receive_byte_via_SPI(HOST_STATUS_ADDR) & 0x10) == 0) && timeout_count++ < V3_TIMEOUT_CNT)
	{
		VA_DelayMS(5);
		VA_INFO("Reg HOST_STATUS_ADDR = 0x%x, 0x65FF = 0x%x\n", receive_byte_via_SPI(0x65FE), receive_byte_via_SPI(0x65FF));
	}
	VA_HIGH("wait for initialization finished : end\n");

	if(timeout_count >= V3_TIMEOUT_CNT)
	{
		VA_ERR("Time out! Venus V3 initialization fail\n");
		VA_ERR("0x65F4~F7 = %x %x %x %x\n", receive_byte_via_SPI(0x65F4), receive_byte_via_SPI(0x65F5), receive_byte_via_SPI(0x65F6), receive_byte_via_SPI(0x65F7));
		return 1;
	}

#if V3_FORCE_TO_SKIP_FW_BURNING
	VA_INFO("+++ force to skip FW flash burning\n");
#endif

	VA_HIGH("VA Firmware Start : end\n");

	return 0;
}

void V3A_ResetFWRingBufParameter(void)
{
	transmit_byte_via_SPI(HOST_PARAMETER_4, 0);	
	transmit_byte_via_SPI(HOST_PARAMETER_5, 0);	
	transmit_word_via_SPI(HOST_PARAMETER_6, 0);	
}

void V3A_WaitBootForFWDownload(void)
{
	VA_INFO("Wait CPU for FW download ...\n");
	while(((receive_byte_via_SPI(HOST_STATUS_ADDR) & 0x20) == 0))
	{
		VA_INFO("0x65FF = 0x%x\n", receive_byte_via_SPI(0x65FF));
		VA_DelayMS(100);
#if 0
		
		if(receive_byte_via_SPI(0x65ff) == 0x99)
		{
			uint32 i;
			uint16 errcnt = GetVenusRegW(0x65F0);
			uint32 *paddr = (uint32*)0x10c000;
			uint32 *praddr = (uint32*)(0x10c000 + 1024);
			uint32 *pwaddr = (uint32*)(0x10c000 + 2048);

			VA_MSG("Total err # = 0x%x\n", errcnt);
			for(i = 0; i < errcnt; i++)
			{
				VA_MSG("addr 0x%x, read 0x%x, write 0x%x\n",
				       V3A_GetMemL((uint32)paddr++),
				       V3A_GetMemL((uint32)praddr++),
				       V3A_GetMemL((uint32)pwaddr++));
			}
			Venus_Register();
		}
#endif
	}
}

void V3A_SetFWRingBufSize(uint16 size)
{
	transmit_word_via_SPI(HOST_PARAMETER_6, size);
}

void V3A_ValidateFWRingBuf(void)
{
	transmit_byte_via_SPI(HOST_PARAMETER_5, 1); 
	VA_DelayMS(10);
	while(receive_byte_via_SPI(HOST_PARAMETER_5));
}

void V3A_FinishFWRingBufDownload(void)
{
	transmit_byte_via_SPI(HOST_PARAMETER_4, 1);	
}


uint8 V3A_DownloadFirmware(uint8 *fw_ptr, uint32 fw_size)
{
#if V3_FW_DOWNLOAD_CHECK
	uint8 *fw_read_ptr = fw_ptr + fw_size;
#endif

	uint8 section_num, section_idx;
	uint32 section_addr;
	uint32 section_size, remain_section_size;

	uint8 part_idx;
	uint32 part_size;

	uint32 i, error_count;

	
	uint8 header[8];
	uint32 offset = 0;
	uint8 ringbuf_download = 0;

	VA_HIGH("V3A_DownloadFirmware : start\n");

	
	for(i = 0 ; i < 8 ; i++)
	{
		header[i] = fw_ptr[i];
	}

	
	if(header[0] == 'A' && header[1] == 'I' && header[2] == 'T' && header[3] == 1)
	{
		offset = (uint32)(header[7] + ((uint32)header[6] << 8) + ((uint32)header[5] << 16));
	}
	if(offset)
	{
		VA_INFO("!! new FW architecture [2010-0108],offset = 0x%x\n", offset);
	}

	header[4] = V3A_GetFW_DLmode();

	fw_ptr += 8; 

	section_num = fw_ptr[0] + (fw_ptr[1] << 8);
	fw_ptr += 2;

	VA_INFO("package name: %s | section number = %d\n", "ait9882fw", section_num);

	VA_INFO("V3_FW_DOWNLOAD_SIZE_PER_PART = %d bytes\n", V3_FW_DOWNLOAD_SIZE_PER_PART);
	VA_INFO("V3_FW_DOWNLOAD_PARTS_PER_BANK = %d\n", V3_FW_DOWNLOAD_PARTS_PER_BANK);

	for(section_idx = 0; section_idx < section_num; section_idx++)
	{

		VA_MSG("  SECTION %d ++++++++++++++++++++++++++++++++++++++++++++++++\n", section_idx);

		section_addr = fw_ptr[0] + (fw_ptr[1] << 8) + (fw_ptr[2] << 16) + (fw_ptr[3] << 24);
		fw_ptr += 4;
		section_size = remain_section_size = fw_ptr[0] + (fw_ptr[1] << 8) + (fw_ptr[2] << 16) + (fw_ptr[3] << 24);
		fw_ptr += 4;

		VA_MSG("  # section addr = 0x%08X\n", section_addr);
		VA_MSG("  # section size = 0x%08X (%d)\n", section_size, section_size);

		
		if(section_addr == 0x01000000)
		{

			
			section_addr = (offset != 0) ? 0x100000 + offset : 0x102000;	

			VA_MSG("  # redirect section address to 0x%08X, since the section is in DRAM)\n", section_addr);

			
			if(header[4] == FW_DRAM_LOAD_FROM_SRAM_BY_RINGBUF)
			{

				V3A_ResetFWRingBufParameter();

				transmit_byte_via_SPI(0x6901, 0x00);		
				VA_DelayMS(100);   							

				V3A_WaitBootForFWDownload();

				ringbuf_download = 1;
			}
		}

		transmit_byte_via_SPI(0x691E, (section_addr >> 27) & 0x03);
		transmit_byte_via_SPI(0x691D, (section_addr >> 19) & 0xFF);

		for(part_idx = 0, error_count = 0; remain_section_size > 0; part_idx++)
		{

			transmit_byte_via_SPI(0x691C, (section_addr >> 15) & 0x0F);

			part_size = VA_MIN(remain_section_size, V3_FW_DOWNLOAD_SIZE_PER_PART);

			
			transmit_multibytes_via_SPI(fw_ptr + part_idx * V3_FW_DOWNLOAD_SIZE_PER_PART, 0x8000 + section_addr % 0x8000, (uint16) part_size);

			
			if(section_addr == 0)
			{
				if(header[0] == 'A' && header[1] == 'I' && header[2] == 'T' && header[3] == 1)
				{
					
					for(i = 0 ; i < 8 ; i++)
					{
						transmit_byte_via_SPI((uint16)(0x8FF8 + i), header[i]);
					}
				}
			}

#if V3_FW_DOWNLOAD_CHECK
			
			receive_multibytes_via_SPI(fw_read_ptr, 0x8000 + section_addr % 0x8000, part_size);

			
			for(i = 0; i < part_size; i++)
			{
				if(fw_ptr[part_idx * V3_FW_DOWNLOAD_SIZE_PER_PART + i] != fw_read_ptr[i])
				{
					VA_MSG("idx 0x%x, read 0x%x, should be 0x%x\n", i, fw_read_ptr[i], fw_ptr[part_idx * V3_FW_DOWNLOAD_SIZE_PER_PART + i]);
					error_count++;
				}
			}
#endif

			
			if(ringbuf_download)
			{
				V3A_SetFWRingBufSize((uint16)part_size);
				V3A_ValidateFWRingBuf();
			}

			remain_section_size -= part_size;

			if(!ringbuf_download)
				section_addr += V3_FW_DOWNLOAD_SIZE_PER_PART;

			VA_MSG("    (0x691E, 0x691D, 0x691C) = (0x%02X, 0x%02X, 0x%02X) | %5d (%3d%%)\n", receive_byte_via_SPI(0x691E), receive_byte_via_SPI(0x691D), receive_byte_via_SPI(0x691C), section_size - remain_section_size, (section_size - remain_section_size) * 100 / section_size);
		}

		fw_ptr += section_size;

#if V3_FW_DOWNLOAD_CHECK
		VA_MSG("  # section error = %d (%d bytes verified)\n", error_count, section_size);
		if(error_count)
			return 1;
#endif
	}

	
	if(ringbuf_download)
	{
		V3A_FinishFWRingBufDownload();
	}

	VA_HIGH("V3A_DownloadFirmware : end\n");

	return 0;
}

uint8 V3A_DownloadCaliTable(uint8 *Cali_ptr, uint32 Cali_size)
{
	uint8 Result = 0;
	uint8 BackUpValue1, BackUpValue2, BackUpValue3;
	uint32 Status_Address = 0x0010F800;

	VA_HIGH("V3A_DownloadCaliTable : start\n");
	VA_HIGH("The size of Claibration data is %d\n", Cali_size);

	
	BackUpValue1 = receive_byte_via_SPI(0x691E);
	BackUpValue2 = receive_byte_via_SPI(0x691D);
	BackUpValue3 = receive_byte_via_SPI(0x691C);

	transmit_byte_via_SPI(0x691E, (Status_Address >> 27) & 0x03);
	transmit_byte_via_SPI(0x691D, (Status_Address >> 19) & 0xFF);
	transmit_byte_via_SPI(0x691C, (Status_Address >> 15) & 0x0F);
	transmit_multibytes_via_SPI(Cali_ptr, 0x8000 + (Status_Address % 0x8000), Cali_size);

	transmit_byte_via_SPI(0x691E, BackUpValue1);
	transmit_byte_via_SPI(0x691D, BackUpValue2);
	transmit_byte_via_SPI(0x691C, BackUpValue3);
	
	VA_HIGH("V3A_DownloadCaliTable : end\n");

	return Result;
}


uint8 V3A_ReadMemAddr(uint32 addr)
{
	uint8 ret;

	transmit_byte_via_SPI(0x691E, (addr >> 27) & 0x03);
	transmit_byte_via_SPI(0x691D, (addr >> 19) & 0xFF);
	transmit_byte_via_SPI(0x691C, (addr >> 15) & 0x0F);

	ret = receive_byte_via_SPI(0x8000 + addr % 0x8000);

	return ret;
}

uint8 V3A_WriteMemAddr(uint32 addr, uint8 value)
{

	transmit_byte_via_SPI(0x691E, (addr >> 27) & 0x03);
	transmit_byte_via_SPI(0x691D, (addr >> 19) & 0xFF);
	transmit_byte_via_SPI(0x691C, (addr >> 15) & 0x0F);

	transmit_byte_via_SPI(0x8000 + addr % 0x8000, value);

	return 0;
}

uint8 V3A_GetMemB(uint32 addr)
{
	return V3A_ReadMemAddr(addr);
}

uint16 V3A_GetMemW(uint32 addr)
{
	uint16 data16;

	data16 = (uint16)(V3A_ReadMemAddr(addr + 1) << 8) + (uint16)(V3A_ReadMemAddr(addr));

	return data16;
}

uint32 V3A_GetMemL(uint32 addr)
{
	uint32 data32;

	data32 = (uint32)(V3A_ReadMemAddr(addr + 3) << 24) +
	         (uint32)(V3A_ReadMemAddr(addr + 2) << 16) +
	         (uint32)(V3A_ReadMemAddr(addr + 1) << 8) +
	         (uint32)(V3A_ReadMemAddr(addr));

	return data32;
}

void V3A_SetMemB(uint32 addr, uint8 data)
{
	V3A_WriteMemAddr(addr, data);
}

void V3A_SetMemW(uint32 addr, uint16 data16)
{
	V3A_WriteMemAddr(addr + 1, (uint8)(data16 >> 8));
	V3A_WriteMemAddr(addr  , (uint8)(data16));
}

void V3A_SetMemL(uint32 addr, uint32 data32)
{
	V3A_WriteMemAddr(addr + 3, (uint8)(data32 >> 24));
	V3A_WriteMemAddr(addr + 2, (uint8)(data32 >> 16));
	V3A_WriteMemAddr(addr + 1, (uint8)(data32 >>  8));
	V3A_WriteMemAddr(addr  , (uint8)(data32));
}
#if 0
void V3A_GetMemMultiBytes(uint32 host_dst, uint32 fw_src, uint32 len)
{
#define MULTIBYTES		0x1000
	uint32 part_idx, part_size, section_addr, remain_section_size;
	uint8 *mem_ptr = (uint8*)host_dst;

#if 0

	for(i = 0; i < len; i++)
	{
		mem_ptr[i] = V3A_GetMemB(src + i);
	}

#else

	section_addr = fw_src;
	remain_section_size = len;

	transmit_byte_via_SPI(0x691E, (section_addr >> 27) & 0x03);
	transmit_byte_via_SPI(0x691D, (section_addr >> 19) & 0xFF);
	for(part_idx = 0; remain_section_size > 0; part_idx++)
	{
		transmit_byte_via_SPI(0x691C, (section_addr >> 15) & 0x0F);
		part_size = VA_MIN(remain_section_size, MULTIBYTES);

		receive_multibytes_via_SPI(mem_ptr, 0x8000 + section_addr % 0x8000, (uint16) part_size);

		remain_section_size -= part_size;
		section_addr += part_size;
		mem_ptr += part_size;
	}

#endif
}

void V3A_SetMemMultiBytes(uint32 fw_dst, uint32 host_src, uint32 len)
{
#define MULTIBYTES		0x1000
	uint32 part_idx, part_size, section_addr, remain_section_size;
	uint8 *mem_ptr = (uint8*)host_src;
	section_addr = fw_dst;
	remain_section_size = len;

	transmit_byte_via_SPI(0x691E, (section_addr >> 27) & 0x03);
	transmit_byte_via_SPI(0x691D, (section_addr >> 19) & 0xFF);
	for(part_idx = 0; remain_section_size > 0; part_idx++)
	{
		transmit_byte_via_SPI(0x691C, (section_addr >> 15) & 0x0F);
		part_size = VA_MIN(remain_section_size, MULTIBYTES);

		transmit_multibytes_via_SPI(mem_ptr, 0x8000 + section_addr % 0x8000, (uint16) part_size);

		remain_section_size -= part_size;
		section_addr += part_size;
		mem_ptr += part_size;
	}
}
#endif
void V3A_ClearMemMultiBytes(uint32 dst, uint32 len)
{
#define MULTIBYTES		0x1000
	uint32 part_idx, part_size, section_addr, remain_section_size;

	section_addr = dst;
	remain_section_size = len;

	transmit_byte_via_SPI(0x691E, (section_addr >> 27) & 0x03);
	transmit_byte_via_SPI(0x691D, (section_addr >> 19) & 0xFF);
	for(part_idx = 0; remain_section_size > 0; part_idx++)
	{
		transmit_byte_via_SPI(0x691C, (section_addr >> 15) & 0x0F);
		part_size = VA_MIN(remain_section_size, MULTIBYTES);

		

		remain_section_size -= part_size;
		section_addr += part_size;
	}
}

