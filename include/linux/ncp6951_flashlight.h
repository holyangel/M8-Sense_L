/*
 * include/linux/ncp6951_flashlight.h - The NCP6951 flashlight header
 *
 * Copyright (C) 2014 HTC Corporation.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef __NCP6951_FLASHLIGHT_H
#define __NCP6951_FLASHLIGHT_H

struct ncp6951_flt_platform_data {
	uint32_t flash_timeout_sw;	
	uint32_t flash_timeout_hw;	
	uint32_t flash_timeout_inhibit; 
	uint32_t flash_timeout_inhibit_en;	
	uint32_t flash_timeout_red_eye_en;	

	uint32_t flash_current_max;	
	uint32_t flash_current_reduced;	
	uint32_t flash_current_red_eye;	
	uint32_t torch_current_max;	

	uint32_t flash_count_red_eye;	

	uint32_t ncp6951_pin_flen;	
	uint32_t ncp6951_pin_flsel;	
};

#undef __NCP6951_FLASHLIGHT_H
#endif

