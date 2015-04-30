
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/types.h>
#include "pn544_htc.h"

#if NFC_GET_BOOTMODE
#include <htc/devices_cmdline.h>
#endif 



#define D(x...)	\
	if (is_debug) \
		printk(KERN_DEBUG "[NFC] " x)
#define I(x...) printk(KERN_INFO "[NFC] " x)
#define E(x...) printk(KERN_ERR "[NFC] [Err] " x)


#if NFC_OFF_MODE_CHARGING_LOAD_SWITCH
static unsigned int   pvdd_gpio;
#endif 


int pn544_htc_check_rfskuid(int in_is_alive){
	return in_is_alive;
}


int pn544_htc_get_bootmode(void) {
	int bootmode = NFC_BOOT_MODE_NORMAL;
#if NFC_GET_BOOTMODE
	bootmode = board_mfg_mode();
	if (bootmode == MFG_MODE_OFFMODE_CHARGING) {
		I("%s: Check bootmode done NFC_BOOT_MODE_OFF_MODE_CHARGING\n",__func__);
		return NFC_BOOT_MODE_OFF_MODE_CHARGING;
	} else {
		I("%s: Check bootmode done NFC_BOOT_MODE_NORMAL mode = %d\n",__func__,bootmode);
		return NFC_BOOT_MODE_NORMAL;
	}
#else
	return bootmode;
#endif  
}


void pn544_htc_parse_dt(struct device *dev) {
#if NFC_OFF_MODE_CHARGING_LOAD_SWITCH
	struct device_node *dt = dev->of_node;
	pvdd_gpio = of_get_named_gpio_flags(dt, "nxp,pvdd-gpio",0, NULL);
	I("%s: pvdd_gpio:%d\n", __func__, pvdd_gpio);
#endif
}

void pn544_htc_off_mode_charging (void) {
#if NFC_OFF_MODE_CHARGING_LOAD_SWITCH
	int ret;
	I("%s: Turn off NFC_PVDD \n", __func__);
	ret = gpio_request(pvdd_gpio, "nfc_pvdd");
	if (ret) {
		E("%s : request pvdd gpio%d fail\n", __func__, pvdd_gpio);
	} else {
		gpio_set_value(pvdd_gpio, 0);
	}
#endif
}
