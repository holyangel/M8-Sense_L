#
# Makefile for miscellaneous I2C chip drivers.
#
# Do not add new drivers to this directory! It is DEPRECATED.
#
# Device drivers are better grouped according to the functionality they
# implement rather than to the bus they are connected to. In particular:
# * Hardware monitoring chip drivers go to drivers/hwmon
# * RTC chip drivers go to drivers/rtc
# * I/O expander drivers go to drivers/gpio
#
obj-$(CONFIG_SENSORS_NFC_PN544) 	+= pn544.o
obj-$(CONFIG_SENSORS_NFC_PN544)         += pn544_htc.o
#obj-$(CONFIG_SENSORS_NFC_PN544) 	+= pn544.o
#obj-$(CONFIG_TI_TCA6418)      		+= tca6418_ioexpander.o
#obj-$(CONFIG_BOSCH_BMA250)      	+= bma250_bosch.o
obj-$(CONFIG_KIONIX_KXTJ2)      	+= kxtj2.o
#obj-$(CONFIG_YAMAHA_YAS532)      	+= yas532.o
obj-$(CONFIG_AMP_TFA9887)       	+= tfa9895.o
obj-$(CONFIG_AMP_TFA9887L)      	+= tfa9895l.o
obj-$(CONFIG_AMP_RT5506)		+= rt5506.o
#obj-$(CONFIG_SENSORS_AK8963_DOE_PLUS)	+= akm8963_doe_plus.o
obj-$(CONFIG_SENSORS_AK09911)    	+= akm09911.o
#obj-$(CONFIG_BOSCH_BMA250)      	+= bma250_bosch.o
#obj-$(CONFIG_AMP_TPA6130A2)       	+= tpa6130a2.o
obj-$(CONFIG_INPUT_CWSTM32)    	+= CwMcuSensor.o
#CFLAGS_CwMcuSensor.o := -DDEBUG

ifeq ($(CONFIG_INPUT_CAPELLA_CM36686),y)
proximity-objs := cm36686.o
obj-$(CONFIG_INPUT_CAPELLA_CM36686)	+= proximity.o
endif

ifeq ($(CONFIG_I2C_DEBUG_CHIP),y)
EXTRA_CFLAGS += -DDEBUG
endif

