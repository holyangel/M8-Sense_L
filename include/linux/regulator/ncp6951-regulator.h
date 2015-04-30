/* ncp6951-regulator.h
 *
 * Copyright (C) 2014 HTC Corporation.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _NCP6951_REGULATOR_H_
#define _NCP6951_REGULATOR_H_

#include <linux/types.h>

#define NCP6951_I2C_NAME		"ncp6951"
#define NCP6951_VPROGLDO1		0x01
#define NCP6951_VPROGLDO2		0x02
#define NCP6951_VPROGLDO3		0x03
#define NCP6951_VPROGLDO4		0x04
#define NCP6951_VPROGLDO5		0x05
#define NCP6951_VPROGDCDC_V1	0x06
#define NCP6951_VPROGDCDC_V2	0x07
#define NCP6951_ENABLE			0x08
#define NCP6951_STATUS			0x0A

#define NCP6951_LDO1_ENABLE_BIT		0x0
#define NCP6951_LDO2_ENABLE_BIT		0x1
#define NCP6951_LDO3_ENABLE_BIT		0x2
#define NCP6951_LDO4_ENABLE_BIT		0x3
#define NCP6951_LDO5_ENABLE_BIT		0x4
#define NCP6951_DCDC_ENABLE_BIT		0x5
#define NCP6951_DCDC_SWITCH_BIT		0x7

#define NCP6951_ID_LDO1			1
#define NCP6951_ID_LDO2			2
#define NCP6951_ID_LDO3			3
#define NCP6951_ID_LDO4			4
#define NCP6951_ID_LDO5			5
#define NCP6951_ID_DCDC			1

#endif 
