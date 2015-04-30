ccflags-y := -Iinclude/htc

obj-y += htc_sysinfo.o
obj-y += devices_cmdline.o devices_dtb.o

obj-$(CONFIG_HTC_DEBUG_RAMOOPS) += htc_ramoops.o

ifdef CONFIG_HTC_DEBUG
obj-y += htc_debug_tools.o

obj-$(CONFIG_HTC_PROCESS_DEBUG) += htc_process_debug.o
obj-$(CONFIG_HTC_DEBUG_SAVE_BOOT_MSG) += bootmsg.o

endif
