

#define NFC_READ_RFSKUID 0
#define NFC_GET_BOOTMODE 1

#define NFC_BOOT_MODE_NORMAL 0
#define NFC_BOOT_MODE_FTM 1
#define NFC_BOOT_MODE_DOWNLOAD 2
#define NFC_BOOT_MODE_OFF_MODE_CHARGING 5

#if defined(CONFIG_MACH_DUMMY)
#define NFC_OFF_MODE_CHARGING_LOAD_SWITCH 1
#else
#define NFC_OFF_MODE_CHARGING_LOAD_SWITCH 0
#endif

int pn544_htc_check_rfskuid(int in_is_alive);

int pn544_htc_get_bootmode(void);


void pn544_htc_parse_dt(struct device *dev);


void pn544_htc_off_mode_charging(void);


