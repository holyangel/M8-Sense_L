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

#include "AIT_spi.h"
#include <linux/vmalloc.h>

#undef CDBG
#define CDBG(fmt, args...) pr_info("[CAM][AIT] : " fmt, ##args)

#define AIT_MAX_ALLOCATE 1000
static DEFINE_MUTEX(spi_lock);
#define MAX_SPI_write 20000
static char tx_buffer[MAX_SPI_write];

static struct spi_device *AIT_dev;

struct AIT_spi_ctrl_blk {
	struct spi_device *spi;
	spinlock_t		spinlock;
};
struct AIT_spi_ctrl_blk *AIT_spi_ctrl;


int spi_AIT_probe(struct spi_device *AIT)
{
	CDBG("%s\n", __func__);

	AIT_dev = AIT;

	
	AIT_spi_ctrl = kzalloc(sizeof(*AIT_spi_ctrl), GFP_KERNEL);
	if (!AIT_spi_ctrl)
		return -ENOMEM;

	AIT_spi_ctrl->spi = AIT;
	spin_lock_init(&AIT_spi_ctrl->spinlock);

	spi_set_drvdata(AIT, AIT_spi_ctrl);

	return 0;
}


static const struct of_device_id AIT_SPI_match_table[] = {
	{.compatible = "AIT_spi", .data = NULL},
	{}
};

MODULE_DEVICE_TABLE(of, AIT_SPI_match_table);

static struct spi_driver spi_AIT = {
	.driver = {
		.name = "AIT_spi",
		.of_match_table = AIT_SPI_match_table,
	},
	.probe = spi_AIT_probe,
};
int spi_read_buf(struct spi_device *spi,u8 *txbuf, u8 *rxbuf, unsigned size){
	int spi_ret;
	struct spi_transfer t ={
		.tx_buf = txbuf,
		.rx_buf = rxbuf,
		.len = size,
	};
	struct spi_message m;

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	spi_ret = spi_sync(spi, &m);
	return spi_ret;
}
#define MAX_READ_SIZE 400
void SetVenusRegW(uint16_t Addr, uint16_t Val)
{
	int rc = 0;
	uint8_t tx[5];
	tx[0] = 0x20;
	tx[1] = (Addr & 0xff00) >> 8;
	tx[2] = Addr & 0x00ff;
	tx[3] = Val & 0x00ff;
	tx[4] = (Val & 0xff00) >> 8;

	mutex_lock(&spi_lock);
	AIT_dev->bits_per_word = 8;
	
	
	rc = spi_write(AIT_dev, tx, 5);
	if (rc < 0) {
		pr_err("[CAM]SetVenusRegW (0x%x,0x%x) failed, rc=%d\n", Addr,Val, rc);
	}
        mutex_unlock(&spi_lock);
}
void SetVenusMultiBytes(uint8_t* dataPtr, uint16_t startAddr, uint16_t length)
{
	int rc = 0;
	mutex_lock(&spi_lock);
	memset(tx_buffer, 0xff, sizeof(tx_buffer));
	if(length > (MAX_SPI_write-3)){
		pr_err("[CAM] not enough buffer length:%d",length);
		mutex_unlock(&spi_lock);
		return ;
	}
	tx_buffer[0]=0x20;
	tx_buffer[1]=(startAddr&0xff00)>>8;
	tx_buffer[2]=(startAddr&0x00ff);

	memcpy(&tx_buffer[3],dataPtr,length);
	AIT_dev->bits_per_word = 8;

	rc = spi_write(AIT_dev, tx_buffer, length+3);
	if (rc < 0) {
		pr_err("[CAM]SetVenusRegW (startAddr:0x%x, length:%d) failed, rc=%d\n", startAddr,length, rc);
	}
	mutex_unlock(&spi_lock);
}
uint16_t GetVenusRegW(uint16_t Addr)
{
        unsigned char tx_buf[5];
	unsigned char rx_buf[5];
	int rc = 0;
	uint16_t Val = 0;
	mutex_lock(&spi_lock);
	tx_buf[0] = 0x01;
	tx_buf[1] = ((Addr & 0xff00) >> 8);
	tx_buf[2] = (Addr & 0xff);
	tx_buf[3] = 0xff;
	tx_buf[4] = 0xff;
	
	
	rx_buf[0] = 0xff;
	rx_buf[1] = 0xff;
	rx_buf[2] = 0xff;
	rx_buf[3] = 0xff;
	rx_buf[4] = 0xff;
	rc = spi_read_buf(AIT_dev, tx_buf, rx_buf, 5);
	if (rc < 0) {
		pr_err("[CAM]GetVenusRegB spi_read_buf failed, rc=%d \n", rc);
		mutex_unlock(&spi_lock);
		return Val;
	}
	Val = (rx_buf[4] << 8) | (rx_buf[3]);
	
	mutex_unlock(&spi_lock);
	return Val;
}
void GetVenusMultiBytes(uint8_t* dataPtr, uint16_t startAddr, uint16_t length)
{
	int rc = 0;
	int remain = length;
	unsigned char tx_buf[16];
	unsigned char rx_buf[16];
	uint16_t startAddr_temp = startAddr;
	uint8_t* dataPtr_temp = dataPtr;
	int read_len = 0;
	mutex_lock(&spi_lock);
	while(remain > 0)
	{
		if(remain > 16)
			read_len = 16;
		else
			read_len = remain;
		
		memset(tx_buf, 0xff, sizeof(tx_buf));
		memset(rx_buf, 0xff, sizeof(rx_buf));
		tx_buf[0] = 0x0 | (read_len - 1);
		tx_buf[1] = ((startAddr_temp & 0xff00) >> 8);
		tx_buf[2] = (startAddr_temp & 0xff);
		
		rc = spi_read_buf(AIT_dev, tx_buf, rx_buf, read_len + 3);
		if (rc < 0) {
		pr_err("[CAM]GetVenusMultiBytes spi_read_buf failed, rc=%d \n", rc);
		mutex_unlock(&spi_lock);
		return;
		}
		memcpy(dataPtr_temp,&rx_buf[3],read_len);
		remain =  remain - read_len;
		dataPtr_temp = dataPtr_temp + read_len;
		startAddr_temp = startAddr_temp + read_len;
	}

        mutex_unlock(&spi_lock);
}

void SetVenusRegB(uint16_t Addr, uint8_t Val)
{
	int rc = 0;
	uint8_t tx[4];
	tx[0] = 0x20;
	tx[1] = (Addr & 0xff00) >> 8;
	tx[2] = Addr & 0x00ff;
	tx[3] = Val;
	mutex_lock(&spi_lock);
	AIT_dev->bits_per_word = 8;
	
	rc = spi_write(AIT_dev, tx, 4);
	if (rc < 0) {
		pr_err("[CAM]SetVenusRegB (0x%x,0x%x) failed, rc=%d\n", Addr,Val, rc);
	}
        mutex_unlock(&spi_lock);
}
uint8_t GetVenusRegB(uint16_t Addr)
{
	unsigned char tx_buf[4];
	unsigned char rx_buf[4];
	int rc = 0;
	mutex_lock(&spi_lock);
	tx_buf[0] = 0;
	tx_buf[1] = ((Addr & 0xff00) >> 8);
	tx_buf[2] = (Addr & 0xff);
	tx_buf[3] = 0xff;
	
	
	rx_buf[0] = 0xff;
	rx_buf[1] = 0xff;
	rx_buf[2] = 0xff;
	rx_buf[3] = 0xff;
	rc = spi_read_buf(AIT_dev, tx_buf, rx_buf, 4);
	if (rc < 0) {
		pr_err("[CAM]GetVenusRegB spi_read_buf failed, rc=%d \n", rc);
		mutex_unlock(&spi_lock);
		return rc;
	}
	
	mutex_unlock(&spi_lock);
	return rx_buf[3];
}
int AIT_spi_init(void)
{
	int rc = -1;

	CDBG("%s +\n", __func__);

	rc = spi_register_driver(&spi_AIT);
	if (rc < 0) {
		pr_err("[CAM]%s:failed to register \
			spi driver(%d) for camera\n", __func__, rc);
		return -EINVAL;
	}

	CDBG("%s rc:%d, -\n", __func__, rc);
	return 0;
}

