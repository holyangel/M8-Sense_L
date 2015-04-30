GCC_VERSION      := $(shell $(CONFIG_SHELL) $(PWD)/scripts/gcc-version.sh $(CROSS_COMPILE)gcc)

EXTRA_CFLAGS += -Idrivers/media/platform/msm/camera_v2/sensor
EXTRA_CFLAGS += -Idrivers/media/platform/msm/camera_v2/sensor/io
EXTRA_CFLAGS += -Idrivers/media/platform/msm/camera_v2

obj-$(CONFIG_CAMERA_AIT) += AIT_ISP.o AIT_spi.o AIT_ISP_API.o AIT_ISP_spi_ctl.o AIT_ISP_System.o
