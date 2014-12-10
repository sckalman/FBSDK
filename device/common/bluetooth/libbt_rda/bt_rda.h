

#ifndef _BT_RDA_H_
#define _BT_RDA_H_

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/uio.h>

#include <cutils/log.h>
#include "bt_hci_bdroid.h"
#include "bt_vendor_lib.h"

#define TCC_BT_DEVICE_PATH              "/dev/tcc_bt_dev"  

#define BT_DEV_MAJOR_NUM                234

#define IOCTL_BT_DEV_POWER              _IO(BT_DEV_MAJOR_NUM, 100)
#define IOCTL_BT_SET_EINT       	    _IO(BT_DEV_MAJOR_NUM, 101)


#define LOGV(fmt, args...)              ALOGV(fmt, ##args)
#define LOGD(fmt, args...)              ALOGD(fmt, ##args)
#define LOGI(fmt, args...)              ALOGI(fmt, ##args)
#define LOGW(fmt, args...)              ALOGW(fmt, ##args)
#define LOGE(fmt, args...)              ALOGE(fmt, ##args)
#define LOGE_IF(cond, fmt, args...)     ALOGE_IF(cond, fmt, ##args

#define FLOW_CTL_HW         0x0001
#define FLOW_CTL_SW         0x0002
#define FLOW_CTL_NONE       0x0000
#define FLOW_CTL_MASK       0x0003

/*****************    APIs    ******************/
void set_callbacks(const bt_vendor_callbacks_t* p_cb);
void clean_callbacks(void);
int set_bluetooth_power(int on);
int init_uart(void);
void close_uart(void);
int rda_fw_cfg(void);
int rda_sco_cfg(void);
int rda_sleep_cfg(void);
int rda_wake_chip(uint8_t wake_assert);

#endif


