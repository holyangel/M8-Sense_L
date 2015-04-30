/*
 *  m1120.c - Linux kernel modules for hall switch
 *
 *  Copyright (C) 2013 Seunghwan Park <seunghwan.park@magnachip.com>
 *  Copyright (C) 2014 MagnaChip Semiconductor.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/m1120.h>
#include <linux/of_gpio.h>
#include <linux/async.h>
#include <linux/hall_sensor.h>


#define DRIVER_NAME "HL"

#define M1120_DETECTION_MODE				M1120_DETECTION_MODE_INTERRUPT
#define M1120_INTERRUPT_TYPE				M1120_VAL_INTSRS_INTTYPE_BESIDE
static u8 M1120_SENSITIVITY_TYPE;
#define M1120_PERSISTENCE_COUNT				M1120_VAL_PERSINT_COUNT(2)
#define M1120_OPERATION_FREQUENCY			M1120_VAL_OPF_FREQ_20HZ
#define M1120_OPERATION_RESOLUTION			M1120_VAL_OPF_BIT_10
#define M1120_RESULT_STATUS_A				(0x00)	
#define M1120_RESULT_STATUS_B				(0x01)	
#define M1120_EVENT_DATA_CAPABILITY_MIN			(-32768)
#define M1120_EVENT_DATA_CAPABILITY_MAX			(32767)


#ifdef M1120_DBG_ENABLE
#define dbg(fmt, args...)	HL_LOG("(L%04d) : " fmt "\n", __LINE__, ##args)
#define dbgn(fmt, args...)	printk(fmt, ##args)
#else
#define dbg(fmt, args...)
#define dbgn(fmt, args...)
#endif 


static m1120_data_t *p_m1120_data;

static int	m1120_i2c_read(struct i2c_client *client, u8 reg, u8* rdata, u8 len);
static int	m1120_i2c_get_reg(struct i2c_client *client, u8 reg, u8* rdata);
static int	m1120_i2c_write(struct i2c_client *client, u8 reg, u8* wdata, u8 len);
static int	m1120_i2c_set_reg(struct i2c_client *client, u8 reg, u8 wdata);
static void	report_cover_event(m1120_data_t* p_data);
static void	m1120_work_func(struct work_struct *work);
static irqreturn_t m1120_irq_handler(int irq, void *dev_id);
static void	m1120_get_reg(struct device *dev, int* regdata);
static void	m1120_set_reg(struct device *dev, int* regdata);
static int	m1120_get_enable(struct device *dev);
static void	m1120_set_enable(struct device *dev, int enable);
static int	m1120_get_delay(struct device *dev);
static void	m1120_set_delay(struct device *dev, int delay);
static int	m1120_get_debug(struct device *dev);
static void	m1120_set_debug(struct device *dev, int debug);
static int	m1120_clear_interrupt(struct device *dev);
static int	m1120_update_interrupt_threshold(struct device *dev, short raw);
static int	m1120_set_operation_mode(struct device *dev, int mode);
static int	m1120_set_detection_mode(struct device *dev, u8 mode);
static int	m1120_init_device(struct device *dev);
static int	m1120_reset_device(struct device *dev);
static int	m1120_set_calibration(struct device *dev);
static int	m1120_get_calibrated_data(struct device *dev, int* data);
static int	m1120_measure(m1120_data_t *p_data, short *raw);
static int	m1120_get_result_status(m1120_data_t* p_data, int raw);


#define M1120_I2C_BUF_SIZE				(17)
static int m1120_i2c_read(struct i2c_client* client, u8 reg, u8* rdata, u8 len)
{
	int rc;

	struct i2c_msg msg[] = {
		{
			.addr = client->addr,
			.flags = 0,
			.len = 1,
			.buf = &reg,
		},
		{
			.addr = client->addr,
			.flags = I2C_M_RD,
			.len = len,
			.buf = rdata,
		},
	};

	if ( client == NULL ) {
		HL_ERR("client is NULL");
		return -ENODEV;
	}

	rc = i2c_transfer(client->adapter, msg, 2);
	if(rc<0) {
		HL_ERR("i2c_transfer was failed(%d)", rc);
		return rc;
	}

	return 0;
}

static int	m1120_i2c_get_reg(struct i2c_client *client, u8 reg, u8* rdata)
{
	return m1120_i2c_read(client, reg, rdata, 1);
}

static int m1120_i2c_write(struct i2c_client* client, u8 reg, u8* wdata, u8 len)
{
	m1120_data_t *p_data = i2c_get_clientdata(client);
	u8  buf[M1120_I2C_BUF_SIZE];
	int rc;
	int i;
	struct i2c_msg msg[] = {
		{
			.addr = client->addr,
			.flags = 0,
			.len = len+1,
			.buf = buf,
		},
	};

	if ( client == NULL ) {
		HL_ERR("i2c client is NULL.\n");
		return -ENODEV;
	}

	buf[0] = reg;
	if (len > M1120_I2C_BUF_SIZE) {
		HL_ERR("i2c buffer size must be less than %d", M1120_I2C_BUF_SIZE);
		return -EIO;
	}
	for( i=0 ; i<len; i++ ) buf[i+1] = wdata[i];

	rc = i2c_transfer(client->adapter, msg, 1);
	if(rc< 0) {
		HL_ERR("i2c_transfer was failed (%d)", rc);
		return rc;
	}

	if(len==1) {
		switch(reg){
		case M1120_REG_PERSINT:
			p_data->reg.map.persint = wdata[0];
			break;
		case M1120_REG_INTSRS:
			p_data->reg.map.intsrs = wdata[0];
			break;
		case M1120_REG_LTHL:
			p_data->reg.map.lthl = wdata[0];
			break;
		case M1120_REG_LTHH:
			p_data->reg.map.lthh = wdata[0];
			break;
		case M1120_REG_HTHL:
			p_data->reg.map.hthl = wdata[0];
			break;
		case M1120_REG_HTHH:
			p_data->reg.map.hthh = wdata[0];
			break;
		case M1120_REG_I2CDIS:
			p_data->reg.map.i2cdis = wdata[0];
			break;
		case M1120_REG_SRST:
			p_data->reg.map.srst = wdata[0];
			msleep(1);
			break;
		case M1120_REG_OPF:
			p_data->reg.map.opf = wdata[0];
			break;
		}
	}

	for(i=0; i<len; i++) dbg("reg=0x%02X data=0x%02X", buf[0]+(u8)i, buf[i+1]);

	return 0;
}

static int m1120_i2c_set_reg(struct i2c_client *client, u8 reg, u8 wdata)
{
	return m1120_i2c_write(client, reg, &wdata, sizeof(wdata));
}


static ssize_t read_att(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int pos = 0;
	char string[100] = {0};
	m1120_data_t *p_data = p_m1120_data;

	if(p_data->first_boot)
	{
		report_cover_event(p_data);
	}
	p_data->first_boot = 0;
	HL_LOG("ATT(%d):ATT_N=%d, ATT_S=%d", 2, !p_data->prev_val_n, !p_data->prev_val_s);
	pos += snprintf(string+pos, sizeof(string), "ATT(%d):ATT_N=%d, ATT_S=%d", 2, !p_data->prev_val_n, !p_data->prev_val_s);

	pos = snprintf(buf, pos + 2, "%s", string);
	return pos;
}

static ssize_t write_att(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	m1120_data_t *p_data = p_m1120_data;
	if (buf[0] == '0' || buf[0] == '1') {
		if ((m1120_get_enable(&p_data->client->dev) == 1) && (buf[0] == '0'))
		{
			m1120_set_enable(&p_data->client->dev, 0);
			HL_LOG("Disable hall sensor interrupts\n");
		}
		else if ((m1120_get_enable(&p_data->client->dev) == 0) && (buf[0] == '1'))
		{
			m1120_set_enable(&p_data->client->dev, 1);
			HL_LOG("Enable hall sensor interrupts\n");
		}
		else
			HL_LOG("Invalid paramater(0:Disable 1:Enable)\n");
	} else
		HL_LOG("Parameter Error\n");

	return count;
}

static DEVICE_ATTR(read_att, S_IRUGO | S_IWUSR , read_att, write_att);

static struct kobject *android_cover_kobj;

static int hall_cover_sysfs_init(int init)
{
	int ret = 0;

	if (init) {
		android_cover_kobj = kobject_create_and_add("android_cover", NULL);
		if (android_cover_kobj == NULL) {
			HL_ERR("subsystem_register_failed");
			ret = -ENOMEM;
			return ret;
		}

		ret = sysfs_create_file(android_cover_kobj, &dev_attr_read_att.attr);
		if (ret) {
			HL_ERR("sysfs_create_file read_att failed\n");
			return ret;
		}
		dbg("attribute file register Done");
	} else {
		sysfs_remove_file(android_cover_kobj, &dev_attr_read_att.attr);
		kobject_del(android_cover_kobj);
	}
	return 0;
}


static void report_cover_event(m1120_data_t* p_data)
{
	if (p_data->last_data == M1120_RESULT_STATUS_B) {
		if (p_data->last_raw >= 0) {
			HL_LOG("att_s[Near]");
			input_report_key(p_data->input_dev, HALL_S_POLE, p_data->last_data);
			p_data->prev_val_s = 1;
			blocking_notifier_call_chain(&hallsensor_notifier_list, (1 << 1) | p_data->last_data, NULL);
		} else {
			HL_LOG("att_n[Near]");
			input_report_key(p_data->input_dev, HALL_N_POLE, p_data->last_data);
			p_data->prev_val_n = 1;
			blocking_notifier_call_chain(&hallsensor_notifier_list, (0 << 1) | p_data->last_data, NULL);
		}
	} else {
		if (p_data->prev_val_s) {
			HL_LOG("att_s[Far]");
			input_report_key(p_data->input_dev, HALL_S_POLE, p_data->last_data);
			p_data->prev_val_s = 0;
			blocking_notifier_call_chain(&hallsensor_notifier_list, (1 << 1) | p_data->last_data, NULL);
		}
		if (p_data->prev_val_n) {
			HL_LOG("att_n[Far]");
			input_report_key(p_data->input_dev, HALL_N_POLE, p_data->last_data);
			p_data->prev_val_n = 0;
			blocking_notifier_call_chain(&hallsensor_notifier_list, (0 << 1) | p_data->last_data, NULL);
		}
	}

	input_sync(p_data->input_dev);
}

static void m1120_work_func(struct work_struct *work)
{
	m1120_data_t* p_data = container_of((struct delayed_work *)work, m1120_data_t, work);
	unsigned long delay = msecs_to_jiffies(m1120_get_delay(&p_data->client->dev));
	short raw = 0;
	int err = 0;

	dbg("+++");
	err = m1120_measure(p_data, &raw);

	if(!err) {

		if(p_data->reg.map.intsrs & M1120_DETECTION_MODE_INTERRUPT) {
			p_data->last_data = m1120_get_result_status(p_data, raw);
		} else {
			p_data->last_data = (int)raw;
		}
		p_data->last_raw = raw;

		report_cover_event(p_data);
	}

	if( p_data->reg.map.intsrs & M1120_DETECTION_MODE_INTERRUPT) {
		dbg("run update_interrupt_threshold");
		m1120_update_interrupt_threshold(&p_data->client->dev, raw);
	} else {
		schedule_delayed_work(&p_data->work, delay);
		dbg("run schedule_delayed_work");
	}
}


static irqreturn_t m1120_irq_handler(int irq, void *dev_id)
{
	m1120_data_t* p_data = dev_id;
	unsigned long delay = msecs_to_jiffies(m1120_get_delay(&p_data->client->dev));
	short raw = 0;
	int err = 0;

	HL_LOG_TIME("interrupt trigger");
	if(p_m1120_data != NULL) {
		err = m1120_measure(p_data, &raw);

		if(!err) {

			if(p_data->reg.map.intsrs & M1120_DETECTION_MODE_INTERRUPT) {
				p_data->last_data = m1120_get_result_status(p_data, raw);
			} else {
				p_data->last_data = (int)raw;
			}
			p_data->last_raw = raw;

			report_cover_event(p_data);
		}

		if( p_data->reg.map.intsrs & M1120_DETECTION_MODE_INTERRUPT) {
			dbg("run update_interrupt_threshold");
			m1120_update_interrupt_threshold(&p_data->client->dev, raw);
		} else {
			schedule_delayed_work(&p_data->work, delay);
			dbg("run schedule_delayed_work");
		}
	}
	return IRQ_HANDLED;
}



static void m1120_get_reg(struct device *dev, int* regdata)
{
	struct i2c_client *client = to_i2c_client(dev);
	int err;

	u8 rega = (((*regdata) >> 8) & 0xFF);
	u8 regd = 0;
	err = m1120_i2c_get_reg(client, rega, &regd);

	*regdata = 0;
	*regdata |= (err==0) ? 0x0000 : 0xFF00;
	*regdata |= regd;
}

static void m1120_set_reg(struct device *dev, int* regdata)
{
	struct i2c_client *client = to_i2c_client(dev);
	int err;

	u8 rega = (((*regdata) >> 8) & 0xFF);
	u8 regd = *regdata&0xFF;
	err = m1120_i2c_set_reg(client, rega, regd);

	*regdata = 0;
	*regdata |= (err==0) ? 0x0000 : 0xFF00;
}


static int m1120_get_enable(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	m1120_data_t *p_data = i2c_get_clientdata(client);

	return atomic_read(&p_data->atm.enable);
}

static void m1120_set_enable(struct device *dev, int enable)
{
	struct i2c_client *client = to_i2c_client(dev);
	m1120_data_t *p_data = i2c_get_clientdata(client);
	int delay = m1120_get_delay(dev);

	HL_LOG("enable=%d", enable);
	mutex_lock(&p_data->mtx.enable);

	if (enable) {                   
		if (!atomic_cmpxchg(&p_data->atm.enable, 0, 1)) {
			m1120_set_detection_mode(dev, p_data->reg.map.intsrs & M1120_DETECTION_MODE_INTERRUPT);
			m1120_set_operation_mode(&p_m1120_data->client->dev, OPERATION_MODE_MEASUREMENT);
			if( ! (p_data->reg.map.intsrs & M1120_DETECTION_MODE_INTERRUPT)) {
				schedule_delayed_work(&p_data->work, msecs_to_jiffies(delay));
			}
		}
	} else {                        
		if (atomic_cmpxchg(&p_data->atm.enable, 1, 0)) {
			cancel_delayed_work_sync(&p_data->work);
			m1120_set_operation_mode(&p_m1120_data->client->dev, OPERATION_MODE_POWERDOWN);
		}
	}
	atomic_set(&p_data->atm.enable, enable);

	mutex_unlock(&p_data->mtx.enable);
}

static int m1120_get_delay(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	m1120_data_t *p_data = i2c_get_clientdata(client);

	int delay = 0;

	delay = atomic_read(&p_data->atm.delay);

	return delay;
}

static void m1120_set_delay(struct device *dev, int delay)
{
	struct i2c_client *client = to_i2c_client(dev);
	m1120_data_t *p_data = i2c_get_clientdata(client);

	if(delay<M1120_DELAY_MIN) delay = M1120_DELAY_MIN;
	atomic_set(&p_data->atm.delay, delay);

	mutex_lock(&p_data->mtx.enable);

	if (m1120_get_enable(dev)) {
		if( ! (p_data->reg.map.intsrs & M1120_DETECTION_MODE_INTERRUPT)) {
			cancel_delayed_work_sync(&p_data->work);
			schedule_delayed_work(&p_data->work, msecs_to_jiffies(delay));
		}
	}

	mutex_unlock(&p_data->mtx.enable);
}

static int m1120_get_debug(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	m1120_data_t *p_data = i2c_get_clientdata(client);

	return atomic_read(&p_data->atm.debug);
}

static void m1120_set_debug(struct device *dev, int debug)
{
	struct i2c_client *client = to_i2c_client(dev);
	m1120_data_t *p_data = i2c_get_clientdata(client);

	atomic_set(&p_data->atm.debug, debug);
}

static int m1120_clear_interrupt(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	m1120_data_t *p_data = i2c_get_clientdata(client);
	int ret = 0;

	ret = m1120_i2c_set_reg(p_data->client, M1120_REG_PERSINT, p_data->reg.map.persint | 0x01);

	return ret;
}

void m1120_convdata_short_to_2byte(u8 opf, short x, unsigned char *hbyte, unsigned char *lbyte)
{
	if( (opf & M1120_VAL_OPF_BIT_8) == M1120_VAL_OPF_BIT_8) {
		
		if(x<-128) x=-128;
		else if(x>127) x=127;

		if(x>=0) {
			*lbyte = x & 0x7F;
		} else {
			*lbyte = ( (0x80 - (x*(-1))) & 0x7F ) | 0x80;
		}
		*hbyte = 0x00;
	} else {
		
		if(x<-512) x=-512;
		else if(x>511) x=511;

		if(x>=0) {
			*lbyte = x & 0xFF;
			*hbyte = (((x&0x100)>>8)&0x01) << 6;
		} else {
			*lbyte = (0x0200 - (x*(-1))) & 0xFF;
			*hbyte = ((((0x0200 - (x*(-1))) & 0x100)>>8)<<6) | 0x80;
		}
	}
}

short m1120_convdata_2byte_to_short(u8 opf, unsigned char hbyte, unsigned char lbyte)
{
	short x;

	if( (opf & M1120_VAL_OPF_BIT_8) == M1120_VAL_OPF_BIT_8) {
		
		x = lbyte & 0x7F;
		if(lbyte & 0x80) {
			x -= 0x80;
		}
	} else {
		
		x = ( ( (hbyte & 0x40) >> 6) << 8 ) | lbyte;
		if(hbyte&0x80) {
			x -= 0x200;
		}
	}

	return x;
}

static int m1120_update_interrupt_threshold(struct device *dev, short raw)
{
	struct i2c_client *client = to_i2c_client(dev);
	m1120_data_t *p_data = i2c_get_clientdata(client);
	u8 lthh=0, lthl=0, hthh=0, hthl=0, data=0;
	int err = -1;

	if(p_data->reg.map.intsrs & M1120_DETECTION_MODE_INTERRUPT) {

		dbg("reg.map.intsrs = 0x%02X", p_data->reg.map.intsrs);
		dbg("raw = %d", raw);
		data = M1120_DETECTION_MODE | M1120_SENSITIVITY_TYPE;
		if (p_data->last_data == M1120_RESULT_STATUS_B) {
			m1120_convdata_short_to_2byte(p_data->reg.map.opf, p_data->thrhigh_l, &hthh, &hthl);
			m1120_convdata_short_to_2byte(p_data->reg.map.opf, p_data->thrlow_h, &lthh, &lthl);
			data |= M1120_VAL_INTSRS_INTTYPE_WITHIN;
		} else {
			m1120_convdata_short_to_2byte(p_data->reg.map.opf, p_data->thrhigh, &hthh, &hthl);
			m1120_convdata_short_to_2byte(p_data->reg.map.opf, p_data->thrlow, &lthh, &lthl);
			data |= M1120_VAL_INTSRS_INTTYPE_BESIDE;
		}

		err = m1120_i2c_set_reg(p_data->client, M1120_REG_INTSRS, data);
		if(err) return err;
		err = m1120_i2c_set_reg(p_data->client, M1120_REG_HTHH, hthh);
		if(err) return err;
		err = m1120_i2c_set_reg(p_data->client, M1120_REG_HTHL, hthl);
		if(err) return err;
		err = m1120_i2c_set_reg(p_data->client, M1120_REG_LTHH, lthh);
		if(err) return err;
		err = m1120_i2c_set_reg(p_data->client, M1120_REG_LTHL, lthl);
		if(err) return err;

		dbg("threshold : (0x%02X%02X, 0x%02X%02X) INT: %02X\n", hthh, hthl, lthh, lthl, data);

		err = m1120_clear_interrupt(dev);
		if(err) return err;
	}

	return err;
}

static int m1120_set_operation_mode(struct device *dev, int mode)
{
	struct i2c_client *client = to_i2c_client(dev);
	m1120_data_t *p_data = i2c_get_clientdata(client);
	u8 opf = p_data->reg.map.opf;
	int err = -1;

	switch(mode) {
		case OPERATION_MODE_POWERDOWN:
			if(p_data->irq_enabled) {

				
				disable_irq(p_data->irq);
				free_irq(p_data->irq, NULL);
				p_data->irq_enabled = 0;
			}
			opf &= (0xFF - M1120_VAL_OPF_HSSON_ON);
			err = m1120_i2c_set_reg(client, M1120_REG_OPF, opf);
			HL_LOG("operation mode was chnaged to OPERATION_MODE_POWERDOWN");
			break;
		case OPERATION_MODE_MEASUREMENT:
			opf &= (0xFF - M1120_VAL_OPF_EFRD_ON);
			opf |= M1120_VAL_OPF_HSSON_ON;
			err = m1120_i2c_set_reg(client, M1120_REG_OPF, opf);
			if(p_data->reg.map.intsrs & M1120_DETECTION_MODE_INTERRUPT) {
				if(!p_data->irq_enabled) {
					
					err = request_threaded_irq(p_data->irq,
							NULL, m1120_irq_handler,
							IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
							M1120_IRQ_NAME, p_data);
					if(err) {
						HL_ERR("request_threaded_irq was failed, ret = %d", err);
						return err;
					}
					enable_irq_wake(p_data->irq);
					HL_LOG("request_threaded_irq was success");
					
					p_data->irq_enabled = 1;
				}
			}
			HL_LOG("operation mode was chnaged to OPERATION_MODE_MEASUREMENT");
			break;
		case OPERATION_MODE_FUSEROMACCESS:
			opf |= M1120_VAL_OPF_EFRD_ON;
			opf |= M1120_VAL_OPF_HSSON_ON;
			err = m1120_i2c_set_reg(client, M1120_REG_OPF, opf);
			HL_LOG("operation mode was chnaged to OPERATION_MODE_FUSEROMACCESS");
			break;
	}

	return err;
}

static int m1120_set_detection_mode(struct device *dev, u8 mode)
{
	struct i2c_client *client = to_i2c_client(dev);
	m1120_data_t *p_data = i2c_get_clientdata(client);
	u8 data;
	int err = 0;

	if(mode & M1120_DETECTION_MODE_INTERRUPT) {

		
		m1120_update_interrupt_threshold(dev, p_data->last_data);

		
		data = p_data->reg.map.intsrs | M1120_DETECTION_MODE_INTERRUPT;
		err = m1120_i2c_set_reg(p_data->client, M1120_REG_INTSRS, data);
		if(err) return err;

	} else {

		
		data = p_data->reg.map.intsrs & (0xFF - M1120_DETECTION_MODE_INTERRUPT);
		err = m1120_i2c_set_reg(p_data->client, M1120_REG_INTSRS, data);
		if(err) return err;
	}

	return err;
}


static int m1120_init_device(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	m1120_data_t *p_data = i2c_get_clientdata(client);
	int err = -1;

	
	atomic_set(&p_data->atm.enable, 0);
	atomic_set(&p_data->atm.delay, M1120_DELAY_MIN);
#ifdef M1120_DBG_ENABLE
	atomic_set(&p_data->atm.debug, 1);
#else
	atomic_set(&p_data->atm.debug, 0);
#endif
	p_data->calibrated_data = 0;
	p_data->last_data = 0;
	p_data->irq_enabled = 0;
	p_data->irq_first = 1;
	p_data->thrlow = p_data->threshold[0];
	p_data->thrlow_h = p_data->threshold[1];
	p_data->thrhigh_l = p_data->threshold[2];
	p_data->thrhigh = p_data->threshold[3];

	m1120_set_delay(&client->dev, M1120_DELAY_MAX);
	m1120_set_debug(&client->dev, 0);

	
	err = m1120_reset_device(dev);
	if(err) {
		HL_ERR("m1120_reset_device was failed (%d)", err);
		return err;
	}

	HL_LOG("initializing device was success");

	return 0;
}

static int m1120_reset_device(struct device *dev)
{
	int	err = 0;
	u8	id = 0xFF, data = 0x00;

	struct i2c_client *client = to_i2c_client(dev);
	m1120_data_t *p_data = i2c_get_clientdata(client);

	if( (p_data == NULL) || (p_data->client == NULL) ) return -ENODEV;

	
	err = m1120_i2c_set_reg(p_data->client, M1120_REG_SRST, M1120_VAL_SRST_RESET);
	if(err) {
		HL_ERR("sw-reset was failed(%d)", err);
		return err;
	}
	msleep(5); 
	dbg("wait 5ms after vdd power up");

	
	err = m1120_i2c_get_reg(p_data->client, M1120_REG_DID, &id);
	if (err < 0) return err;
	if (id != M1120_VAL_DID) {
		HL_ERR("current device id(0x%02X) is not M1120 device id(0x%02X)", id, M1120_VAL_DID);
		return -ENXIO;
	}

	
	
	data = M1120_PERSISTENCE_COUNT;
	err = m1120_i2c_set_reg(p_data->client, M1120_REG_PERSINT, data);
	
	data = M1120_DETECTION_MODE | M1120_SENSITIVITY_TYPE;
	if(data & M1120_DETECTION_MODE_INTERRUPT) {
		data |= M1120_INTERRUPT_TYPE;
	}
	err = m1120_i2c_set_reg(p_data->client, M1120_REG_INTSRS, data);
	
	data = M1120_OPERATION_FREQUENCY | M1120_OPERATION_RESOLUTION;
	err = m1120_i2c_set_reg(p_data->client, M1120_REG_OPF, data);

	
	err = m1120_set_detection_mode(dev, M1120_DETECTION_MODE);
	if(err) {
		HL_ERR("m1120_set_detection_mode was failed(%d)", err);
		return err;
	}

	
	err = m1120_set_operation_mode(dev, OPERATION_MODE_POWERDOWN);
	if(err) {
		HL_ERR("m1120_set_detection_mode was failed(%d)", err);
		return err;
	}

	return err;
}


static int m1120_set_calibration(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	m1120_data_t *p_data = i2c_get_clientdata(client);
	int retrycnt = 10, cnt = 0;

	short raw = 0;
	int err;

	m1120_set_operation_mode(dev, OPERATION_MODE_MEASUREMENT);
	for(cnt=0; cnt<retrycnt; cnt++) {
		msleep(M1120_DELAY_FOR_READY);
		err = m1120_measure(p_data, &raw);
		if(!err) {
			p_data->calibrated_data = raw;
			break;
		}
	}
	if(!m1120_get_enable(dev)) m1120_set_operation_mode(dev, OPERATION_MODE_POWERDOWN);

	return err;
}

static int m1120_get_calibrated_data(struct device *dev, int* data)
{
	struct i2c_client *client = to_i2c_client(dev);
	m1120_data_t *p_data = i2c_get_clientdata(client);

	int err = 0;

	if(p_data == NULL) err = -ENODEV;
	else *data = p_data->calibrated_data;

	return err;
}

static int m1120_measure(m1120_data_t *p_data, short *raw)
{
	struct i2c_client *client = p_data->client;
	int err;
	u8 buf[3];
	int st1_is_ok = 0;

	
	err = m1120_i2c_read(client, M1120_REG_ST1, buf, sizeof(buf));
	if(err) return err;

	
	if(p_data->reg.map.intsrs & M1120_VAL_INTSRS_INT_ON) {
		
		if( ! (buf[0] & 0x10) ) {
			st1_is_ok = 1;
		}
	} else {
		
		if(buf[0] & 0x01) {
			st1_is_ok = 1;
		}
	}

	if(st1_is_ok) {
		*raw = m1120_convdata_2byte_to_short(p_data->reg.map.opf, buf[2], buf[1]);
	} else {
		HL_ERR("st1(0x%02X) is not DRDY", buf[0]);
		err = -1;
	}

	
		HL_LOG("raw data (%d)\n", *raw);
	

	return err;
}


static int m1120_get_result_status(m1120_data_t* p_data, int raw)
{
	int status;

	dbg("raw: %d reg.map.intsrs = (%02X)\n", raw, p_data->reg.map.intsrs);
	if(p_data->reg.map.intsrs & M1120_VAL_INTSRS_INTTYPE_WITHIN) {
		dbg("within\n");
		if ((p_data->thrhigh >= raw) && (p_data->thrlow <= raw)) {
			status = M1120_RESULT_STATUS_A;
		} else {
			status = M1120_RESULT_STATUS_B;
		}
	} else {
		dbg("beside\n");
		if ((p_data->thrhigh_l >= raw) && (p_data->thrlow_h <= raw)) {
			status = M1120_RESULT_STATUS_A;
		} else {
			status = M1120_RESULT_STATUS_B;
		}
	}

	switch(status) {
		case M1120_RESULT_STATUS_A:
			HL_LOG("Result is status [A]\n");
			break;
		case M1120_RESULT_STATUS_B:
			HL_LOG("Result is status [B]\n");
			break;
	}

	return status;
}


static int m1120_input_dev_init(m1120_data_t *p_data)
{
	struct input_dev *dev;
	int err;

	dev = input_allocate_device();
	if (!dev) {
		return -ENOMEM;
	}
	dev->name = M1120_DRIVER_NAME;
	dev->id.bustype = BUS_I2C;

	input_set_drvdata(dev, p_data);

	set_bit(EV_SYN, dev->evbit);
	set_bit(EV_KEY, dev->evbit);

	input_set_capability(dev, EV_KEY, HALL_N_POLE);
	input_set_capability(dev, EV_KEY, HALL_S_POLE);

	err = input_register_device(dev);
	if (err < 0) {
		input_free_device(dev);
		return err;
	}

	p_data->input_dev = dev;

	return 0;
}

static void m1120_input_dev_terminate(m1120_data_t *p_data)
{
	struct input_dev *dev = p_data->input_dev;

	input_unregister_device(dev);
	input_free_device(dev);
}






static int m1120_misc_dev_open( struct inode*, struct file* );
static int m1120_misc_dev_release( struct inode*, struct file* );
static long m1120_misc_dev_ioctl(struct file* file, unsigned int cmd, unsigned long arg);
static ssize_t m1120_misc_dev_read( struct file *filp, char *buf, size_t count, loff_t *ofs );
static ssize_t m1120_misc_dev_write( struct file *filp, const char *buf, size_t count, loff_t *ofs );
static unsigned int m1120_misc_dev_poll( struct file *filp, struct poll_table_struct *pwait );

static struct file_operations m1120_misc_dev_fops =
{
	.owner = THIS_MODULE,
	.open = m1120_misc_dev_open,
	.unlocked_ioctl = m1120_misc_dev_ioctl,
	.release = m1120_misc_dev_release,
	.read = m1120_misc_dev_read,
	.write = m1120_misc_dev_write,
	.poll = m1120_misc_dev_poll,
};

static struct miscdevice m1120_misc_dev =
{
	.minor = MISC_DYNAMIC_MINOR,
	.name = M1120_DRIVER_NAME,
	.fops = &m1120_misc_dev_fops,
};
static int m1120_misc_dev_open( struct inode* inode, struct file* file)
{
	return 0;
}

static int m1120_misc_dev_release( struct inode* inode, struct file* file)
{
	return 0;
}

static long m1120_misc_dev_ioctl(struct file* file, unsigned int cmd, unsigned long arg)
{
	long ret = 0;

	void __user *argp = (void __user *)arg;
	int kbuf = 0;
	int caldata = 0;

	switch( cmd ) {
	case M1120_IOCTL_SET_ENABLE:
		if(copy_from_user(&kbuf, argp, sizeof(kbuf))) return -EFAULT;
		dbg("M1120_IOCTL_SET_ENABLE(%d)\n", kbuf);
		m1120_set_enable(&p_m1120_data->client->dev, kbuf);
		break;
	case M1120_IOCTL_GET_ENABLE:
		kbuf = m1120_get_enable(&p_m1120_data->client->dev);
		dbg("M1120_IOCTL_GET_ENABLE(%d)\n", kbuf);
		if(copy_to_user(argp, &kbuf, sizeof(kbuf))) return -EFAULT;
		break;
	case M1120_IOCTL_SET_DELAY:
		if(copy_from_user(&kbuf, argp, sizeof(kbuf))) return -EFAULT;
		dbg("M1120_IOCTL_SET_DELAY(%d)\n", kbuf);
		m1120_set_delay(&p_m1120_data->client->dev, kbuf);
		break;
	case M1120_IOCTL_GET_DELAY:
		kbuf = m1120_get_delay(&p_m1120_data->client->dev);
		dbg("M1120_IOCTL_GET_DELAY(%d)\n", kbuf);
		if(copy_to_user(argp, &kbuf, sizeof(kbuf))) return -EFAULT;
		break;
	case M1120_IOCTL_SET_CALIBRATION:
		dbg("M1120_IOCTL_SET_CALIBRATION\n");
		kbuf = m1120_set_calibration(&p_m1120_data->client->dev);
		if(copy_to_user(argp, &kbuf, sizeof(kbuf))) return -EFAULT;
		break;
	case M1120_IOCTL_GET_CALIBRATED_DATA:
		dbg("M1120_IOCTL_GET_CALIBRATED_DATA\n");
		kbuf = m1120_get_calibrated_data(&p_m1120_data->client->dev, &caldata);
		if(copy_to_user(argp, &caldata, sizeof(caldata))) return -EFAULT;
		dbg("calibrated data (%d)\n", caldata);
		break;
	case M1120_IOCTL_SET_REG:
		if(copy_from_user(&kbuf, argp, sizeof(kbuf))) return -EFAULT;
		dbg("M1120_IOCTL_SET_REG([0x%02X] %02X", (u8)((kbuf>>8)&0xFF), (u8)(kbuf&0xFF));
		m1120_set_reg(&p_m1120_data->client->dev, &kbuf);
		dbgn(" (%s))\n", (kbuf&0xFF00)?"Not Ok":"Ok");
		if(copy_to_user(argp, &kbuf, sizeof(kbuf))) return -EFAULT;
		break;
	case M1120_IOCTL_GET_REG:
		if(copy_from_user(&kbuf, argp, sizeof(kbuf))) return -EFAULT;
		dbg("M1120_IOCTL_GET_REG([0x%02X]", (u8)((kbuf>>8)&0xFF) );
		m1120_get_reg(&p_m1120_data->client->dev, &kbuf);
		dbgn(" 0x%02X (%s))\n", (u8)(kbuf&0xFF), (kbuf&0xFF00)?"Not Ok":"Ok");
		if(copy_to_user(argp, &kbuf, sizeof(kbuf))) return -EFAULT;
		break;
	case M1120_IOCTL_SET_INTERRUPT:
		if(copy_from_user(&kbuf, argp, sizeof(kbuf))) return -EFAULT;
		dbg("M1120_IOCTL_SET_INTERRUPT(%d)\n", kbuf);
		if(kbuf) {
			m1120_set_detection_mode(&p_m1120_data->client->dev, M1120_DETECTION_MODE_INTERRUPT);
		} else {
			m1120_set_detection_mode(&p_m1120_data->client->dev, M1120_DETECTION_MODE_POLLING);
		}
		break;
	case M1120_IOCTL_GET_INTERRUPT:
		kbuf = (p_m1120_data->reg.map.intsrs & M1120_DETECTION_MODE_INTERRUPT) ? 1 : 0 ;
		dbg("M1120_IOCTL_GET_INTERRUPT(%d)\n", kbuf);
		if(copy_to_user(argp, &kbuf, sizeof(kbuf)));
		break;
	case M1120_IOCTL_SET_THRESHOLD_HIGH:
		if(copy_from_user(&kbuf, argp, sizeof(kbuf))) return -EFAULT;
		dbg("M1120_IOCTL_SET_THRESHOLD_HIGH(%d)\n", kbuf);
		p_m1120_data->thrhigh = kbuf;
		break;
	case M1120_IOCTL_GET_THRESHOLD_HIGH:
		kbuf = p_m1120_data->thrhigh;
		dbg("M1120_IOCTL_GET_THRESHOLD_HIGH(%d)\n", kbuf);
		if(copy_to_user(argp, &kbuf, sizeof(kbuf)));
		break;
	case M1120_IOCTL_SET_THRESHOLD_LOW:
		if(copy_from_user(&kbuf, argp, sizeof(kbuf))) return -EFAULT;
		dbg("M1120_IOCTL_SET_THRESHOLD_LOW(%d)\n", kbuf);
		p_m1120_data->thrlow = kbuf;
		break;
	case M1120_IOCTL_GET_THRESHOLD_LOW:
		kbuf = p_m1120_data->thrlow;
		dbg("M1120_IOCTL_GET_THRESHOLD_LOW(%d)\n", kbuf);
		if(copy_to_user(argp, &kbuf, sizeof(kbuf)));
		break;
	default:
		return -ENOTTY;
	}

	return ret;
}

static ssize_t m1120_misc_dev_read( struct file *filp, char *buf, size_t count, loff_t *ofs )
{
	return 0;
}

static ssize_t m1120_misc_dev_write( struct file *filp, const char *buf, size_t count, loff_t *ofs )
{
	return 0;
}

static unsigned int m1120_misc_dev_poll( struct file *filp, struct poll_table_struct *pwait )
{
	return 0;
}






static ssize_t m1120_enable_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", m1120_get_enable(dev));
}

static ssize_t m1120_enable_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	unsigned long enable = simple_strtoul(buf, NULL, 10);

	if ((enable == 0) || (enable == 1)) {
		m1120_set_enable(dev, enable);
	}

	return count;
}

static ssize_t m1120_delay_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", m1120_get_delay(dev));
}

static ssize_t m1120_delay_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	unsigned long delay = simple_strtoul(buf, NULL, 10);

	if (delay > M1120_DELAY_MAX) {
		delay = M1120_DELAY_MAX;
	}

	m1120_set_delay(dev, delay);

	return count;
}

static ssize_t m1120_debug_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", m1120_get_debug(dev));
}

static ssize_t m1120_debug_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	unsigned long debug = simple_strtoul(buf, NULL, 10);

	m1120_set_debug(dev, debug);

	return count;
}

static ssize_t m1120_wake_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	return 0;
}

static DEVICE_ATTR(enable,	S_IRUGO|S_IWUSR|S_IWGRP, m1120_enable_show, m1120_enable_store);
static DEVICE_ATTR(delay,	S_IRUGO|S_IWUSR|S_IWGRP, m1120_delay_show,	m1120_delay_store);
static DEVICE_ATTR(debug,	S_IRUGO|S_IWUSR|S_IWGRP, m1120_debug_show,	m1120_debug_store);
static DEVICE_ATTR(wake,	S_IWUSR|S_IWGRP,		 NULL,				m1120_wake_store);

static struct attribute *m1120_attributes[] = {
	&dev_attr_enable.attr,
	&dev_attr_delay.attr,
	&dev_attr_debug.attr,
	&dev_attr_wake.attr,
	NULL
};

static struct attribute_group m1120_attribute_group = {
	.attrs = m1120_attributes
};

static int m1120_dt_parser(struct device_node *dt, m1120_platform_data_t *p_platform)
{
	int ret = 0, i = 0;
	const char *parser_st[] = {"interrupt_gpio", "threshold", "sensitivity"};
	uint32_t gpio_att = 0;

	gpio_att = of_get_named_gpio(dt, parser_st[0], 0);
	if (!gpio_is_valid(gpio_att))  {
		HL_LOG("DT: %s parser failue, ret=%d", parser_st[0], p_platform->interrupt_gpio);
		ret = p_platform->interrupt_gpio;
		goto parser_failed;
	} else {
		p_platform->interrupt_gpio = gpio_att;
		p_platform->interrupt_irq = gpio_to_irq(gpio_att);
		HL_LOG("DT:%s: (%d)[%d] read",
				parser_st[0],
				p_platform->interrupt_gpio,
				p_platform->interrupt_irq);
	}

	ret = of_property_read_u32_array(dt, parser_st[1],
			p_platform->threshold, THRESHOLD_SIZE);
	if(ret) {
		HL_LOG("DT: %s parser failue, ret=%d", parser_st[1], ret);
		goto parser_failed;
	} else {
		printk("[HL] DT:%s: ", parser_st[1]);
		for(i = 0 ; i < THRESHOLD_SIZE; i++)
			printk("%d " , p_platform->threshold[i]);
		printk("read\n");
	}

	ret = of_property_read_u8(dt, parser_st[2], &M1120_SENSITIVITY_TYPE);
	if(ret) {
		M1120_SENSITIVITY_TYPE = M1120_VAL_INTSRS_SRS_10BIT_0_010mT;
		HL_LOG("DT: %s parser failue, ret=%d, use M1120_VAL_INTSRS_SRS_10BIT_0_010mT",
			parser_st[2], ret);
		goto parser_failed;
	} else {
		M1120_SENSITIVITY_TYPE &= 0x07;
		HL_LOG("DT:%s: 0x%X", parser_st[2], M1120_SENSITIVITY_TYPE);
	}
	return 0;
parser_failed:
	return ret;
}



int m1120_i2c_drv_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	m1120_platform_data_t *p_platform;
	m1120_data_t *p_data;
	int i = 0;
	int err = 0;
	int ret = 0;

	HL_LOG("+++");

	
	p_data = kzalloc(sizeof(m1120_data_t), GFP_KERNEL);
	if (!p_data) {
		HL_ERR("kernel memory alocation was failed");
		err = -ENOMEM;
		goto error_0;
	}

	
	mutex_init(&p_data->mtx.enable);
	mutex_init(&p_data->mtx.data);

	
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		HL_ERR("i2c_check_functionality was failed");
		err = -ENODEV;
		goto error_1;
	}
	i2c_set_clientdata(client, p_data);
	p_data->client = client;

	
	if (client->dev.of_node) {
		p_platform = kzalloc(sizeof(*p_platform), GFP_KERNEL);
		if (!p_data)
		{
			HL_ERR("platform_data alloc memory fail");
		}
		ret = m1120_dt_parser(client->dev.of_node, p_platform);
		if (ret < 0) {
			ret = -ENOMEM;
			HL_ERR("m1120_dt_parser was failed(%d)", err);
		}
	} else {
		p_platform = client->dev.platform_data;
		dbgn("[HL] p_platform: vi2c = %d, vdd = %d, gpio = %d, irq = %d, threshold = [",
				p_platform->power_vi2c, p_platform->power_vdd,
				p_platform->interrupt_gpio, p_platform->interrupt_irq);

		for(i = 0 ; i < THRESHOLD_SIZE; i++){
			dbgn("%d " , p_platform->threshold[i]);
		}
		dbgn("]\n");
	}

	if(p_platform) {
		p_data->power_vi2c	= p_platform->power_vi2c;
		p_data->power_vdd	= p_platform->power_vdd;
		p_data->igpio		= p_platform->interrupt_gpio;
		p_data->irq		= p_platform->interrupt_irq;
		printk("[HL] vi2c = %d, vdd = %d, gpio = %d, irq = %d, threshold = [",
				p_data->power_vi2c, p_data->power_vdd,
				p_data->igpio, p_data->irq);

		for(i = 0 ; i < THRESHOLD_SIZE; i++){
			p_data->threshold[i] = p_platform->threshold[i];
			printk("%d " , p_data->threshold[i]);
		}
		printk("]\n");
	} else {
		p_data->power_vi2c = -1;
		p_data->power_vdd = -1;
		p_data->igpio = -1;
		p_data->irq = -1;
		HL_ERR("platform unexist\n");
	}

	p_data->prev_val_n = 0;
	p_data->prev_val_s = 0;
	p_data->first_boot = 1;

	
	if(p_data->igpio != -1) {
		err = gpio_request(p_data->igpio, "hall_m1120_irq");
		if (err){
			HL_ERR("gpio_request was failed(%d)", err);
			goto error_1;
		}
		HL_LOG("gpio_request was success");
		err = gpio_direction_input(p_data->igpio);
		if (err < 0) {
			HL_ERR("gpio_direction_input was failed(%d)", err);
			goto error_2;
		}
		HL_LOG("gpio_direction_input was success");
	}


	
	err = m1120_init_device(&p_data->client->dev);
	if(err) {
		HL_ERR("m1120_init_device was failed(%d)", err);
		goto error_1;
	}
	HL_LOG("%s was found", id->name);

	
	INIT_DELAYED_WORK(&p_data->work, m1120_work_func);

	
	err = m1120_input_dev_init(p_data);
	if(err) {
		HL_ERR("m1120_input_dev_init was failed(%d)", err);
		goto error_1;
	}
	HL_LOG("%s was initialized", M1120_DRIVER_NAME);

	
	err = sysfs_create_group(&p_data->input_dev->dev.kobj, &m1120_attribute_group);
	if(err) {
		HL_ERR("sysfs_create_group was failed(%d)", err);
		goto error_3;
	}

	
	err = misc_register(&m1120_misc_dev);
	if(err) {
		HL_ERR("misc_register was failed(%d)", err);
		goto error_4;
	}

	
	p_m1120_data = p_data;
	err = hall_cover_sysfs_init(1);
	if(err) {
		HL_ERR("hall_cover_sysfs_init was failed(%d)", err);
		goto error_5;
	}

	m1120_set_enable(&client->dev, 1);
	dbg("%s was probed.\n", M1120_DRIVER_NAME);

	return 0;

error_5:
	misc_deregister(&m1120_misc_dev);
error_4:
	sysfs_remove_group(&p_data->input_dev->dev.kobj, &m1120_attribute_group);

error_3:
	m1120_input_dev_terminate(p_data);

error_2:
	if(p_data->igpio != -1) {
		gpio_free(p_data->igpio);
	}

error_1:
	kfree(p_data);

error_0:

	return err;
}

static int m1120_i2c_drv_remove(struct i2c_client *client)
{
	m1120_data_t *p_data = i2c_get_clientdata(client);

	m1120_set_enable(&client->dev, 0);
	misc_deregister(&m1120_misc_dev);
	hall_cover_sysfs_init(0);
	sysfs_remove_group(&p_data->input_dev->dev.kobj, &m1120_attribute_group);
	m1120_input_dev_terminate(p_data);
	if(p_data->igpio!= -1) {
		gpio_free(p_data->igpio);
	}
	kfree(p_data);

	return 0;
}

static int m1120_i2c_drv_suspend(struct i2c_client *client, pm_message_t mesg)
{
	m1120_data_t *p_data = i2c_get_clientdata(client);

	HL_LOG("+++");

	disable_irq(p_data->irq);
#if 0
	mutex_lock(&p_data->mtx.enable);

	if (m1120_get_enable(&client->dev)) {
		if(p_data->reg.map.intsrs & M1120_DETECTION_MODE_INTERRUPT) {
			m1120_set_operation_mode(&client->dev, OPERATION_MODE_MEASUREMENT);
		} else {
			cancel_delayed_work_sync(&p_data->work);
			m1120_set_detection_mode(&client->dev, M1120_DETECTION_MODE_INTERRUPT);
		}
	}

	mutex_unlock(&p_data->mtx.enable);
#endif

	return 0;
}

static int m1120_i2c_drv_resume(struct i2c_client *client)
{
	m1120_data_t *p_data = i2c_get_clientdata(client);

	HL_LOG("+++");
#if 0
	mutex_lock(&p_data->mtx.enable);

	if (m1120_get_enable(&client->dev)) {
		if(p_data->reg.map.intsrs & M1120_DETECTION_MODE_INTERRUPT) {
			m1120_set_detection_mode(&client->dev, M1120_DETECTION_MODE_POLLING);
			schedule_delayed_work(&p_data->work, msecs_to_jiffies(m1120_get_delay(&client->dev)));
		}
	}

	mutex_unlock(&p_data->mtx.enable);
#endif
	enable_irq(p_data->irq);

	return 0;
}

static const struct i2c_device_id m1120_i2c_drv_id_table[] = {
	{"m1120", 0 },
	{ }
};

static const struct of_device_id m1120_mttable[] = {
	{ .compatible = "hall_sensor,m1120"},
	{ },
};

static struct i2c_driver m1120_driver = {
	.driver = {
		.owner  = THIS_MODULE,
		.name	= "m1120",
		.of_match_table = m1120_mttable,
	},
	.probe		= m1120_i2c_drv_probe,
	.remove		= m1120_i2c_drv_remove,
	.id_table	= m1120_i2c_drv_id_table,
	.suspend		= m1120_i2c_drv_suspend,
	.resume		= m1120_i2c_drv_resume,
};

static void __init m1120_driver_init_async(void *unused, async_cookie_t cookie)
{
	HL_LOG();
	i2c_add_driver(&m1120_driver);
}

static int __init m1120_driver_init(void)
{
	async_schedule(m1120_driver_init_async, NULL);
	return 0;
}
module_init(m1120_driver_init);

static void __exit m1120_driver_exit(void)
{
	HL_LOG();
	i2c_del_driver(&m1120_driver);
}
module_exit(m1120_driver_exit);

MODULE_AUTHOR("shpark <seunghwan.park@magnachip.com>");
MODULE_VERSION(M1120_DRIVER_VERSION);
MODULE_DESCRIPTION("M1120 hallswitch driver");
MODULE_LICENSE("GPL");

