
#ifndef _AIT_ISP_PROJECT_H_
#define _AIT_ISP_PROJECT_H_

#include <linux/kernel.h>

#define FW_DRAM_LOAD_FROM_FLASH				0
#define FW_DRAM_LOAD_FROM_SRAM_ONCE			1
#define FW_DRAM_LOAD_FROM_DRAM				2
#define FW_DRAM_LOAD_FROM_SRAM_BY_RINGBUF	3	

#define FW_DRAM_LOAD_DEFAULT_MODE		FW_DRAM_LOAD_FROM_SRAM_ONCE

#define VENUS_MALLOC_MEM				(0x03000000L)

typedef unsigned char			uint8;
typedef unsigned short			uint16;
#if 1
typedef unsigned int			uint32;
#else
typedef unsigned long			uint32;
#endif
typedef char					int8;
typedef short					int16;
#if 1
typedef int					int32;
#else
typedef long					int32;
#endif



#if 1
#define VA_MSG(...)				
#define VA_INFO(...)			
#define VA_ERR(fmt, args...) pr_err("[CAM][AIT] : " fmt, ##args)
#define VA_HIGH(fmt, args...) pr_info("[CAM][AIT] : " fmt, ##args)
#else
#define VA_MSG(...)				
#define VA_INFO(...)			
#define VA_ERR(...)				
#define VA_HIGH(...)			
#endif
#define VA_CLOCK(...)           


#endif 
