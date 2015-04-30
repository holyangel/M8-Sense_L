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
 */

#ifndef AIT_SPI_H
#define AIT_SPI_H

#include <linux/module.h>
#include <linux/spi/spi.h>

#define TRUE								1
#define SUCCESS								1
#define FALSE								0
#define FAILURE								0



int spi_AIT_probe(struct spi_device *rawchip);

int AIT_spi_init(void);
uint8_t GetVenusRegB(uint16_t Addr);
uint16_t GetVenusRegW(uint16_t Addr);
void GetVenusMultiBytes(uint8_t* dataPtr, uint16_t startAddr, uint16_t length);
void SetVenusRegB(uint16_t Addr, uint8_t Val);
void SetVenusRegW(uint16_t Addr, uint16_t Val);
void SetVenusMultiBytes(uint8_t* dataPtr, uint16_t startAddr, uint16_t length);

#endif
