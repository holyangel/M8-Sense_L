/* arch/arm/mach-msm/htc_acoustic_alsa.c
 *
 * Copyright (C) 2012 HTC Corporation
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/device.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/mm.h>
#include <linux/gfp.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/switch.h>
#include <sound/htc_acoustic_alsa.h>
#include <linux/htc_headset_mgr.h>

struct device_info {
	unsigned pcb_id;
	unsigned sku_id;
};
#define AVCS_CMD_ADSP_CRASH                      0x0001FFFF
#define D(fmt, args...) printk(KERN_INFO "[AUD] htc-acoustic: "fmt, ##args)
#define E(fmt, args...) printk(KERN_ERR "[AUD] htc-acoustic: "fmt, ##args)

static DEFINE_MUTEX(api_lock);
static struct acoustic_ops default_acoustic_ops;
static struct acoustic_ops *the_ops = &default_acoustic_ops;
static struct switch_dev sdev_beats;
static struct switch_dev sdev_dq;
static struct switch_dev sdev_fm;
static struct switch_dev sdev_listen_notification;

static DEFINE_MUTEX(hs_amp_lock);
static struct amp_register_s hs_amp = {NULL, NULL};

static DEFINE_MUTEX(spk_amp_lock);
static struct amp_register_s spk_amp[SPK_AMP_MAX] = {{NULL}};

static struct amp_power_ops default_amp_power_ops = {NULL};
static struct amp_power_ops *the_amp_power_ops = &default_amp_power_ops;

static struct wake_lock htc_acoustic_wakelock;
static struct wake_lock htc_acoustic_wakelock_timeout;
static struct wake_lock htc_acoustic_dummy_wakelock;
static struct hs_notify_t hs_plug_nt[HS_N_MAX] = {{0,NULL,NULL}};
static DEFINE_MUTEX(hs_nt_lock);

struct avcs_ctl {
       atomic_t ref_cnt;
       void *apr;
};
static struct avcs_ctl this_avcs;

static int hs_amp_open(struct inode *inode, struct file *file);
static int hs_amp_release(struct inode *inode, struct file *file);
static long hs_amp_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static struct file_operations hs_def_fops = {
	.owner = THIS_MODULE,
	.open = hs_amp_open,
	.release = hs_amp_release,
	.unlocked_ioctl = hs_amp_ioctl,
};

static struct miscdevice hs_amp_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "aud_hs_amp",
	.fops = &hs_def_fops,
};

struct hw_component HTC_AUD_HW_LIST[AUD_HW_NUM] = {
	{
		.name = "TPA2051",
		.id = HTC_AUDIO_TPA2051,
	},
	{
		.name = "TPA2026",
		.id = HTC_AUDIO_TPA2026,
	},
	{
		.name = "TPA2028",
		.id = HTC_AUDIO_TPA2028,
	},
	{
		.name = "A1028",
		.id = HTC_AUDIO_A1028,
	},
	{
		.name = "TPA6185",
		.id = HTC_AUDIO_TPA6185,
	},
	{
		.name = "RT5501",
		.id = HTC_AUDIO_RT5501,
	},
	{
		.name = "TFA9887",
		.id = HTC_AUDIO_TFA9887,
	},
	{
		.name = "TFA9887L",
		.id = HTC_AUDIO_TFA9887L,
	},
};
EXPORT_SYMBOL(HTC_AUD_HW_LIST);

#if 0
extern unsigned int system_rev;
#endif
void htc_acoustic_register_spk_amp(enum SPK_AMP_TYPE type,int (*aud_spk_amp_f)(int, int), struct file_operations* ops)
{
	mutex_lock(&spk_amp_lock);

	if(type < SPK_AMP_MAX) {
		spk_amp[type].aud_amp_f = aud_spk_amp_f;
		spk_amp[type].fops = ops;
	}

	mutex_unlock(&spk_amp_lock);
}

int htc_acoustic_spk_amp_ctrl(enum SPK_AMP_TYPE type,int on, int dsp)
{
	int ret = 0;
	mutex_lock(&spk_amp_lock);

	if(type < SPK_AMP_MAX) {
		if(spk_amp[type].aud_amp_f)
			ret = spk_amp[type].aud_amp_f(on,dsp);
	}

	mutex_unlock(&spk_amp_lock);

	return ret;
}

int htc_acoustic_hs_amp_ctrl(int on, int dsp)
{
	int ret = 0;

	mutex_lock(&hs_amp_lock);
	if(hs_amp.aud_amp_f)
		ret = hs_amp.aud_amp_f(on,dsp);
	mutex_unlock(&hs_amp_lock);
	return ret;
}

void htc_acoustic_register_hs_amp(int (*aud_hs_amp_f)(int, int), struct file_operations* ops)
{
	mutex_lock(&hs_amp_lock);
	hs_amp.aud_amp_f = aud_hs_amp_f;
	hs_amp.fops = ops;
	mutex_unlock(&hs_amp_lock);
}

void htc_acoustic_register_hs_notify(enum HS_NOTIFY_TYPE type, struct hs_notify_t *notify)
{
	if(notify == NULL)
		return;

	mutex_lock(&hs_nt_lock);
	if(hs_plug_nt[type].used) {
		pr_err("%s: hs notification %d is reigstered\n",__func__,(int)type);
	} else {
		hs_plug_nt[type].private_data = notify->private_data;
		hs_plug_nt[type].callback_f = notify->callback_f;
		hs_plug_nt[type].used = 1;
	}
	mutex_unlock(&hs_nt_lock);
}

void htc_acoustic_register_ops(struct acoustic_ops *ops)
{
        D("acoustic_register_ops \n");
	mutex_lock(&api_lock);
	the_ops = ops;
	mutex_unlock(&api_lock);
}

int htc_acoustic_query_feature(enum HTC_FEATURE feature)
{
	int ret = -1;
	mutex_lock(&api_lock);
	switch(feature) {
		case HTC_Q6_EFFECT:
			if(the_ops && the_ops->get_q6_effect)
				ret = the_ops->get_q6_effect();
			break;
		case HTC_AUD_24BIT:
			if(the_ops && the_ops->enable_24b_audio)
				ret = the_ops->enable_24b_audio();
			break;
		default:
			break;

	};
	mutex_unlock(&api_lock);
	return ret;
}

static int acoustic_open(struct inode *inode, struct file *file)
{
	D("open\n");
	return 0;
}

static int acoustic_release(struct inode *inode, struct file *file)
{
	D("release\n");
	return 0;
}

static int32_t avcs_callback(struct apr_client_data *data, void *priv)
{
	if (data->opcode == RESET_EVENTS) {
		pr_debug("%s: Reset event is received: %d %d apr[%p]\n",
				__func__,
				data->reset_event,
				data->reset_proc,
				this_avcs.apr);
		apr_reset(this_avcs.apr);
		atomic_set(&this_avcs.ref_cnt, 0);
		this_avcs.apr = NULL;
		return 0;
	}
	pr_info("%s: avcs command send done.\n", __func__);
	return 0;
}

static long
acoustic_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int rc = 0;
	unsigned char *buf = NULL;
	unsigned int us32_size = 0;
	int s32_value = 0;
	void __user *argp = (void __user *)arg;

	if (_IOC_TYPE(cmd) != ACOUSTIC_IOCTL_MAGIC)
		return -ENOTTY;


	us32_size = _IOC_SIZE(cmd);

	buf = kzalloc(us32_size, GFP_KERNEL);

	if (buf == NULL) {
		E("%s %d: allocate kernel buffer failed.\n", __func__, __LINE__);
		return -EFAULT;
	}

	if (_IOC_DIR(cmd) & _IOC_WRITE) {
		rc = copy_from_user(buf, argp, us32_size);
		if (rc) {
			E("%s %d: copy_from_user fail.\n", __func__, __LINE__);
			rc = -EFAULT;
		}
		else {
			D("%s %d: copy_from_user ok. size=%#x\n", __func__, __LINE__, us32_size);
		}
	}

	mutex_lock(&api_lock);
	switch (_IOC_NR(cmd)) {
	case _IOC_NR(ACOUSTIC_SET_Q6_EFFECT): {
		if(sizeof(s32_value) <= us32_size) {
			memcpy((void*)&s32_value, (void*)buf, sizeof(s32_value));
			D("%s %d: ACOUSTIC_SET_Q6_EFFECT %#x\n", __func__, __LINE__, s32_value);
			if (s32_value < -1 || s32_value > 1) {
				E("unsupported Q6 mode: %d\n", s32_value);
				rc = -EINVAL;
				break;
			}
			if (the_ops->set_q6_effect) {
				the_ops->set_q6_effect(s32_value);
			}
		}
		else {
			E("%s %d: ACOUSTIC_SET_Q6_EFFECT error.\n", __func__, __LINE__);
			rc = -EINVAL;
		}
		break;
	}
	case _IOC_NR(ACOUSTIC_GET_HTC_REVISION):
		if (the_ops->get_htc_revision) {
			s32_value = the_ops->get_htc_revision();
		}
		else {
			s32_value = 1;
		}
		if(sizeof(s32_value) <= us32_size) {
			memcpy((void*)buf, (void*)&s32_value, sizeof(s32_value));
			D("%s %d: ACOUSTIC_GET_HTC_REVISION %#x\n", __func__, __LINE__, s32_value);
		}
		else {
			E("%s %d: ACOUSTIC_GET_HTC_REVISION error.\n", __func__, __LINE__);
			rc = -EINVAL;
		}
		break;
	case _IOC_NR(ACOUSTIC_GET_HW_COMPONENT):
		if (the_ops->get_hw_component) {
			s32_value = the_ops->get_hw_component();
		}
		else {
			s32_value = 0;
		}
		if(sizeof(s32_value) <= us32_size) {
			memcpy((void*)buf, (void*)&s32_value, sizeof(s32_value));
			D("%s %d: ACOUSTIC_GET_HW_COMPONENT %#x\n", __func__, __LINE__, s32_value);
		}
		else {
			E("%s %d: ACOUSTIC_GET_HW_COMPONENT error.\n", __func__, __LINE__);
			rc = -EINVAL;
		}
		break;
	case _IOC_NR(ACOUSTIC_GET_DMIC_INFO):
		if (the_ops->enable_digital_mic) {
			s32_value = the_ops->enable_digital_mic();
		}
		else {
			s32_value = 0;
		}
		if(sizeof(s32_value) <= us32_size) {
			memcpy((void*)buf, (void*)&s32_value, sizeof(s32_value));
			D("%s %d: ACOUSTIC_GET_DMIC_INFO %#x\n", __func__, __LINE__, s32_value);
		}
		else {
			E("%s %d: ACOUSTIC_GET_DMIC_INFO error.\n", __func__, __LINE__);
			rc = -EINVAL;
		}
		break;
	case _IOC_NR(ACOUSTIC_GET_MID): {
		char *mid = NULL;
		if (the_ops->get_mid) {
			mid = the_ops->get_mid();
			if(strlen(mid) <= us32_size) {
				strncpy((char*)buf, (char*)mid, strlen(mid));
				D("%s %d: ACOUSTIC_GET_MID %s\n", __func__, __LINE__, mid);
			}
			else {
				E("%s %d: ACOUSTIC_GET_MID error.\n", __func__, __LINE__);
				rc = -EINVAL;
			}
		}
		else {
			if(strlen("Generic") <= us32_size ) {
				strncpy((char*)buf, (char*)"Generic", strlen("Generic"));
				D("%s %d: ACOUSTIC_GET_MID %s\n", __func__, __LINE__, "Generic");
			}
			else {
				E("%s %d: ACOUSTIC_GET_MID error.\n", __func__, __LINE__);
				rc = -EINVAL;
			}
		}
		break;
	}
	case _IOC_NR(ACOUSTIC_UPDATE_BEATS_STATUS): {
		if(sizeof(s32_value) <= us32_size) {
			memcpy((void*)&s32_value, (void*)buf, sizeof(s32_value));
			D("%s %d: ACOUSTIC_UPDATE_BEATS_STATUS %#x\n", __func__, __LINE__, s32_value);
			if (s32_value < -1 || s32_value > 1) {
				rc = -EINVAL;
				break;
			}
			sdev_beats.state = -1;
			switch_set_state(&sdev_beats, s32_value);
		}
		else {
			E("%s %d: ACOUSTIC_UPDATE_BEATS_STATUS error.\n", __func__, __LINE__);
			rc = -EINVAL;
		}
		break;
	}
	case _IOC_NR(ACOUSTIC_UPDATE_DQ_STATUS): {
		if(sizeof(s32_value) <= us32_size) {
			memcpy((void*)&s32_value, (void*)buf, sizeof(s32_value));
			D("%s %d: ACOUSTIC_UPDATE_DQ_STATUS %#x\n", __func__, __LINE__, s32_value);
			if (s32_value < -1 || s32_value > 1) {
				rc = -EINVAL;
				break;
			}
			sdev_dq.state = -1;
			switch_set_state(&sdev_dq, s32_value);
		}
		else {
			E("%s %d: ACOUSTIC_UPDATE_DQ_STATUS error.\n", __func__, __LINE__);
			rc = -EINVAL;
		}
		break;
	}
	case _IOC_NR(ACOUSTIC_CONTROL_WAKELOCK): {
		if(sizeof(s32_value) <= us32_size) {
			memcpy((void*)&s32_value, (void*)buf, sizeof(s32_value));
			D("%s %d: ACOUSTIC_CONTROL_WAKELOCK %#x\n", __func__, __LINE__, s32_value);
			if (s32_value < -1 || s32_value > 1) {
				rc = -EINVAL;
				break;
			}
			if (s32_value == 1) {
				wake_lock_timeout(&htc_acoustic_wakelock, 60*HZ);
			}
			else {
				wake_lock_timeout(&htc_acoustic_wakelock_timeout, 1*HZ);
				wake_unlock(&htc_acoustic_wakelock);
			}
		}
		else {
			E("%s %d: ACOUSTIC_CONTROL_WAKELOCK error.\n", __func__, __LINE__);
			rc = -EINVAL;
		}
		break;
	}
	case _IOC_NR(ACOUSTIC_DUMMY_WAKELOCK): {
		wake_lock_timeout(&htc_acoustic_dummy_wakelock, 5*HZ);
		break;
	}
	case _IOC_NR(ACOUSTIC_UPDATE_LISTEN_NOTIFICATION): {
		if(sizeof(s32_value) <= us32_size) {
			memcpy((void*)&s32_value, (void*)buf, sizeof(s32_value));
			D("%s %d: ACOUSTIC_UPDATE_LISTEN_NOTIFICATION %#x\n", __func__, __LINE__, s32_value);
			if (s32_value < -1 || s32_value > 1) {
				rc = -EINVAL;
				break;
			}
			sdev_listen_notification.state = -1;
			switch_set_state(&sdev_listen_notification, s32_value);
		}
		else {
			E("%s %d: ACOUSTIC_UPDATE_LISTEN_NOTIFICATION error.\n", __func__, __LINE__);
			rc = -EINVAL;
		}
		break;
	}
	case _IOC_NR(ACOUSTIC_RAMDUMP):
#if 0
		pr_err("trigger ramdump by user space\n");
		if (copy_from_user(&mode, (void *)arg, sizeof(mode))) {
			rc = -EFAULT;
			break;
		}

		if (mode >= 4100 && mode <= 4800) {
			dump_stack();
			pr_err("msgid = %d\n", mode);
			BUG();
		}
#endif
		break;
	case _IOC_NR(ACOUSTIC_GET_HW_REVISION): {
#if 0
			struct device_info info;
			info.pcb_id = system_rev;
			info.sku_id = 0;
			if(sizeof(struct device_info) <= us32_size) {
				memcpy((void*)buf, (void*)&info, sizeof(struct device_info));
				D("%s %d: ACOUSTIC_GET_HW_REVISION pcb_id=%#x, sku_id=%#x\n", __func__, __LINE__, info.pcb_id, info.sku_id);
			}
			else {
				E("%s %d: ACOUSTIC_GET_HW_REVISION error.\n", __func__, __LINE__);
				rc = -EINVAL;
			}
#endif
		break;
	}
	case _IOC_NR(ACOUSTIC_AMP_CTRL): {
		struct amp_ctrl ampctrl;
		int i;
		if(sizeof(struct device_info) <= us32_size) {
			memcpy((void*)&ampctrl, (void*)buf, sizeof(struct amp_ctrl));
			D("%s %d: ACOUSTIC_AMP_CTRL type=%#x\n", __func__, __LINE__, ampctrl.type);
			if(ampctrl.type == AMP_HEADPONE) {
				mutex_lock(&hs_amp_lock);
				if(hs_amp.fops && hs_amp.fops->unlocked_ioctl) {
					hs_amp.fops->unlocked_ioctl(file, cmd, arg);
				}
				mutex_unlock(&hs_amp_lock);
			} else if (ampctrl.type == AMP_SPEAKER) {
				mutex_lock(&spk_amp_lock);
				for(i=0; i<SPK_AMP_MAX; i++) {
					if(spk_amp[i].fops && spk_amp[i].fops->unlocked_ioctl) {
						spk_amp[i].fops->unlocked_ioctl(file, cmd, arg);
					}
				}
				mutex_unlock(&spk_amp_lock);
			}
		}
		else {
			E("%s %d: ACOUSTIC_AMP_CTRL error.\n", __func__, __LINE__);
			rc = -EINVAL;
		}
		break;
	}
	case _IOC_NR(ACOUSTIC_KILL_PID): {
		struct pid *pid_struct = NULL;
		if(sizeof(s32_value) <= us32_size) {
			memcpy((void*)&s32_value, (void*)buf, sizeof(s32_value));
			D("%s %d: ACOUSTIC_KILL_PID %#x\n", __func__, __LINE__, s32_value);
			if (s32_value <= 0) {
				rc = -EINVAL;
				break;
			}
			pid_struct = find_get_pid(s32_value);
			if (pid_struct) {
				kill_pid(pid_struct, SIGKILL, 1);
				D("kill pid: %d", s32_value);
			}
		}
		else {
			E("%s %d: ACOUSTIC_KILL_PID error.\n", __func__, __LINE__);
			rc = -EINVAL;
		}
		break;
	}
	case  _IOC_NR(ACOUSTIC_ADSP_CMD): {
			
			struct avcs_crash_params config;
			config.hdr.hdr_field = APR_HDR_FIELD(APR_MSG_TYPE_SEQ_CMD,
						APR_HDR_LEN(APR_HDR_SIZE), APR_PKT_VER);
			config.hdr.pkt_size = sizeof(struct avcs_crash_params);
			config.hdr.src_port = 0;
			config.hdr.dest_port = 0;
			config.hdr.token = 0;
			config.hdr.opcode = AVCS_CMD_ADSP_CRASH;
			config.crash_type = 0;

			if(sizeof(s32_value) <= us32_size) {
				memcpy((void*)&s32_value, (void *)buf, sizeof(s32_value));
				D("%s %d: ACOUSTIC_ADSP_CMD %#x\n", __func__, __LINE__, s32_value);
				if (s32_value <= 0) {
					pr_err("%s %d: copy to user failed\n", __func__, __LINE__);
					rc = -EFAULT;
					break;
				}
				if ((atomic_read(&this_avcs.ref_cnt) == 0) ||
					(this_avcs.apr == NULL)) {
					this_avcs.apr = apr_register("ADSP", "CORE", avcs_callback,
						0xFFFFFFFF, NULL);

					if (this_avcs.apr == NULL) {
						pr_err("%s: Unable to register apr\n", __func__);
						rc = -ENODEV;
						break;
					}

					atomic_inc(&this_avcs.ref_cnt);
				}

				rc = apr_send_pkt(this_avcs.apr, (uint32_t *)(&config));
				if (rc < 0) {
					pr_err("%s: send ADSP CMD failed, %d.\n", __func__, rc);
				} else {
					pr_info("%s: send ADSP CMD success.\n", __func__);
					rc = 0;
				}
			}
		}
		break;
	case _IOC_NR(ACOUSTIC_NOTIFIY_FM_SSR): {
			int new_state = -1;

			if (copy_from_user(&new_state, (void *)arg, sizeof(new_state))) {
				rc = -EFAULT;
				break;
			}
			D("Update FM SSR Status : %d\n", new_state);
			if (new_state < -1 || new_state > 1) {
				E("Invalid FM status update");
				rc = -EINVAL;
				break;
			}

			sdev_fm.state = -1;
			switch_set_state(&sdev_fm, new_state);
			break;
		}
	default:
		rc= -EINVAL;
		break;
	}

	if(0 == rc) {
		if (_IOC_DIR(cmd) & _IOC_READ) {
			rc = copy_to_user(argp, buf, us32_size);
			if (rc) {
				E("%s %d: copy_to_user fail.\n", __func__, __LINE__);
				rc = -EFAULT;
			}
			else {
				D("%s %d: copy_to_user ok. size=%#x\n", __func__, __LINE__, us32_size);
			}
		}
	}

	kfree(buf);
	mutex_unlock(&api_lock);
	return rc;
}

static ssize_t beats_print_name(struct switch_dev *sdev, char *buf)
{
	return sprintf(buf, "Beats\n");
}

static ssize_t dq_print_name(struct switch_dev *sdev, char *buf)
{
	return sprintf(buf, "DQ\n");
}

static ssize_t fm_print_name(struct switch_dev *sdev, char *buf)
{
	return sprintf(buf, "FM\n");
}

static ssize_t listen_notification_print_name(struct switch_dev *sdev, char *buf)
{
	return sprintf(buf, "Listen_notification\n");
}

static int hs_amp_open(struct inode *inode, struct file *file)
{
	int ret = 0;
	const struct file_operations *old_fops = NULL;

	mutex_lock(&hs_amp_lock);

	if(hs_amp.fops) {
		old_fops = file->f_op;

		file->f_op = fops_get(hs_amp.fops);

		if (file->f_op == NULL) {
			file->f_op = old_fops;
			ret = -ENODEV;
		}
	}
	mutex_unlock(&hs_amp_lock);

	if(ret >= 0) {

		if (file->f_op->open) {
			ret = file->f_op->open(inode, file);
			if (ret) {
				fops_put(file->f_op);
				if(old_fops)
					file->f_op = fops_get(old_fops);
				return ret;
			}
		}

		if(old_fops)
			fops_put(old_fops);
	}

	return ret;
}

static int hs_amp_release(struct inode *inode, struct file *file)
{
	return 0;
}

static long hs_amp_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return 0;
}

void htc_amp_power_register_ops(struct amp_power_ops *ops)
{
	D("%s", __func__);
	the_amp_power_ops = ops;
}

void htc_amp_power_enable(bool enable)
{
	D("%s", __func__);
	if(the_amp_power_ops && the_amp_power_ops->set_amp_power_enable)
		the_amp_power_ops->set_amp_power_enable(enable);
}

static int htc_acoustic_hsnotify(int on)
{
	int i = 0;
	mutex_lock(&hs_nt_lock);
	for(i=0; i<HS_N_MAX; i++) {
		if(hs_plug_nt[i].used && hs_plug_nt[i].callback_f)
			hs_plug_nt[i].callback_f(hs_plug_nt[i].private_data, on);
	}
	mutex_unlock(&hs_nt_lock);

	return 0;
}

static struct file_operations acoustic_fops = {
	.owner = THIS_MODULE,
	.open = acoustic_open,
	.release = acoustic_release,
	.unlocked_ioctl = acoustic_ioctl,
	.compat_ioctl = acoustic_ioctl,
};

static struct miscdevice acoustic_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "htc-acoustic",
	.fops = &acoustic_fops,
};

static int __init acoustic_init(void)
{
	int ret = 0;
	struct headset_notifier notifier;

	ret = misc_register(&acoustic_misc);
	wake_lock_init(&htc_acoustic_wakelock, WAKE_LOCK_SUSPEND, "htc_acoustic");
	wake_lock_init(&htc_acoustic_wakelock_timeout, WAKE_LOCK_SUSPEND, "htc_acoustic_timeout");
	wake_lock_init(&htc_acoustic_dummy_wakelock, WAKE_LOCK_SUSPEND, "htc_acoustic_dummy");

	if (ret < 0) {
		pr_err("failed to register misc device!\n");
		return ret;
	}

	ret = misc_register(&hs_amp_misc);
	if (ret < 0) {
		pr_err("failed to register aud hs amp device!\n");
		return ret;
	}

	sdev_beats.name = "Beats";
	sdev_beats.print_name = beats_print_name;

	ret = switch_dev_register(&sdev_beats);
	if (ret < 0) {
		pr_err("failed to register beats switch device!\n");
		return ret;
	}

	sdev_dq.name = "DQ";
	sdev_dq.print_name = dq_print_name;

	ret = switch_dev_register(&sdev_dq);
	if (ret < 0) {
		pr_err("failed to register DQ switch device!\n");
		return ret;
	}

	sdev_fm.name = "FM";
	sdev_fm.print_name = fm_print_name;

	ret = switch_dev_register(&sdev_fm);
	if (ret < 0) {
		pr_err("failed to register FM switch device!\n");
		return ret;
	}

	sdev_listen_notification.name = "Listen_notification";
	sdev_listen_notification.print_name = listen_notification_print_name;

	ret = switch_dev_register(&sdev_listen_notification);
	if (ret < 0) {
		pr_err("failed to register listen_notification switch device!\n");
		return ret;
	}

	notifier.id = HEADSET_REG_HS_INSERT;
	notifier.func = htc_acoustic_hsnotify;
	headset_notifier_register(&notifier);

	return 0;
}

static void __exit acoustic_exit(void)
{
	misc_deregister(&acoustic_misc);
}

module_init(acoustic_init);
module_exit(acoustic_exit);
