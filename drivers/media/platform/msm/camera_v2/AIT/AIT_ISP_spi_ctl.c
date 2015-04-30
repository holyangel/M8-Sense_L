
#include "AIT_ISP_spi_ctl.h"


extern void SetVenusRegB(uint16 Addr, uint8 Val);
extern void SetVenusRegW(uint16 Addr, uint16 Val);
extern void SetVenusMultiBytes(uint8* dataPtr, uint16 startAddr, uint16 length);
extern uint8 GetVenusRegB(uint16 Addr);
extern uint16 GetVenusRegW(uint16 Addr);
extern void GetVenusMultiBytes(uint8* dataPtr, uint16 startAddr, uint16 length);

void transmit_byte_via_SPI(uint16 addr, uint8 data)
{
 SetVenusRegB(addr, data); 
}

void transmit_word_via_SPI(uint16 addr, uint16 data)
{
 SetVenusRegW(addr, data);
}

void transmit_multibytes_via_SPI(uint8* ptr, uint16 addr, uint16 length)
{
SetVenusMultiBytes(ptr, addr, length); 
}

uint8 receive_byte_via_SPI(uint16 addr)
{	
    return GetVenusRegB(addr);
}

uint16 receive_word_via_SPI(uint16 addr)
{	
    return GetVenusRegW(addr);
}

void receive_multibytes_via_SPI(uint8* ptr, uint16 addr, uint16 length)
{
GetVenusMultiBytes(ptr, addr, length); 
}
