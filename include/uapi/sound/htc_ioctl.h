/*
 *
 * Copyright (C) 2012 HTC Corporation.
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


#define ACOUSTIC_IOCTL_MAGIC 'p'
#define TPA9887_IOCTL_MAGIC     'a'

#define ACOUSTIC_SET_Q6_EFFECT                  _IOW(ACOUSTIC_IOCTL_MAGIC, 43, unsigned)
#define ACOUSTIC_GET_HTC_REVISION               _IOR(ACOUSTIC_IOCTL_MAGIC, 44, unsigned)
#define ACOUSTIC_GET_HW_COMPONENT               _IOR(ACOUSTIC_IOCTL_MAGIC, 45, unsigned)
#define ACOUSTIC_GET_DMIC_INFO                  _IOR(ACOUSTIC_IOCTL_MAGIC, 46, unsigned)
#define ACOUSTIC_UPDATE_BEATS_STATUS            _IOW(ACOUSTIC_IOCTL_MAGIC, 47, unsigned)
#define ACOUSTIC_UPDATE_LISTEN_NOTIFICATION     _IOW(ACOUSTIC_IOCTL_MAGIC, 48, unsigned)
#define ACOUSTIC_GET_HW_REVISION                _IOR(ACOUSTIC_IOCTL_MAGIC, 49, struct device_info)
#define ACOUSTIC_CONTROL_WAKELOCK               _IOW(ACOUSTIC_IOCTL_MAGIC, 77, unsigned)
#define ACOUSTIC_DUMMY_WAKELOCK                 _IOW(ACOUSTIC_IOCTL_MAGIC, 78, unsigned)
#define ACOUSTIC_AMP_CTRL                       _IOWR(ACOUSTIC_IOCTL_MAGIC, 50, unsigned)
#define ACOUSTIC_GET_MID                        _IOR(ACOUSTIC_IOCTL_MAGIC, 51, unsigned)
#define ACOUSTIC_RAMDUMP                        _IOW(ACOUSTIC_IOCTL_MAGIC, 99, unsigned)
#define ACOUSTIC_KILL_PID                       _IOW(ACOUSTIC_IOCTL_MAGIC, 88, unsigned)
#define ACOUSTIC_UPDATE_DQ_STATUS               _IOW(ACOUSTIC_IOCTL_MAGIC, 52, unsigned)
#define ACOUSTIC_ADSP_CMD			_IOW(ACOUSTIC_IOCTL_MAGIC, 98, unsigned)

#define ACOUSTIC_GET_TABLES                  _IOW(ACOUSTIC_IOCTL_MAGIC, 33, unsigned)
#define ACOUSTIC_SET_WB_SAMPLE_RATE          _IOW(ACOUSTIC_IOCTL_MAGIC, 38, unsigned int)
#define ACOUSTIC_NOTIFIY_FM_SSR              _IOW(ACOUSTIC_IOCTL_MAGIC, 53, unsigned)
#define TPA9887_ENABLE_DSP                   _IOW(TPA9887_IOCTL_MAGIC, 0x03, unsigned int)
#define TPA9887_KERNEL_LOCK                  _IOW(TPA9887_IOCTL_MAGIC, 0x06, unsigned int)
#define SNDRV_PCM_IOCTL_ENABLE_EFFECT        _IOW('A', 0x70, int)
