#ifndef __BT_CFG_H__
#define __BT_CFG_H__

#include "bt_rda.h"

/* SERIAL PORT */
#define CUST_BT_SERIAL_PORT             "/dev/ttyS0"

#define CUST_BT_SERIAL_BOURATE          1500000 //921600,115200;

/* SERIAL BOURATE */
#ifndef CUST_BT_SERIAL_BOURATE
#define CUST_BT_SERIAL_BOURATE           1500000 //default value:1500000
#endif

/* SERIAL FLOW CTRL */
#define CUST_BT_SERIAL_FLOW_CTRL        FLOW_CTL_HW // 0: NOT FLOW CTRL, USE DMA; 1: HW FLOW CTRL; 

#define EXTERNAL_32K                    //Open sleep mode 

#endif
