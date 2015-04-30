ifeq ("$(origin PRIVATE_RCMS_NAME)", "command line")
KBUILD_RCMS_NAME := $(PRIVATE_RCMS_NAME)
endif

ifeq ("$(origin PRIVATE_SKU_NAME)", "command line")
KBUILD_SKU_NAME := $(PRIVATE_SKU_NAME)
endif

ccflags-$(CONFIG_HTC_RADIO_SMEM) += -DSKU_NAME="\"$(KBUILD_SKU_NAME)\""
ccflags-$(CONFIG_HTC_RADIO_SMEM) += -DRCMS_NAME="\"$(KBUILD_RCMS_NAME)\""
obj-$(CONFIG_HTC_RADIO_SMEM)    += htc_radio_smem.o
obj-y += radio_feedback.o
obj-$(CONFIG_HTC_FEATURES_RIL_PCN0003_HTC_MONITOR) += htc_monitor.o
obj-$(CONFIG_HTC_FEATURES_RIL_PCN0004_HTC_GARBAGE_FILTER) += htc_port_list.o
