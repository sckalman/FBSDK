/******************************************************************************
 *
 *  Copyright (C) 2009-2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

/******************************************************************************
 *
 *  Filename:      hardware.c
 *
 *  Description:   Contains controller-specific functions, like
 *                      firmware patch download
 *                      low power mode operations
 *
 ******************************************************************************/

#define LOG_TAG "bt_hwcfg"

#include <utils/Log.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>
#include <cutils/properties.h>
#include <stdlib.h>
#include "bt_hci_bdroid.h"
#include "bt_vendor_brcm.h"
#include "userial.h"
#include "userial_vendor.h"
#include "upio.h"

/******************************************************************************
**  Constants & Macros
******************************************************************************/

#ifndef BTHW_DBG
#define BTHW_DBG FALSE
#endif

#if (BTHW_DBG == TRUE)
#define BTHWDBG(param, ...) {ALOGD(param, ## __VA_ARGS__);}
#else
#define BTHWDBG(param, ...) {}
#endif

#define ENABLE_BLUETOOTH_AUTOSLEEP      1
#define ENABLE_BLUETOOTH_WAKEUP         1
#define ENABLE_UART_FLOWCTL             1
#define CHANGE_CPU_TO_52M               0

#define FW_PATCHFILE_EXTENSION      ".hcd"
#define FW_PATCHFILE_EXTENSION_LEN  4
#define FW_PATCHFILE_PATH_MAXLEN    248 /* Local_Name length of return of
                                           HCI_Read_Local_Name */

#define HCI_CMD_MAX_LEN             258

#define HCI_RESET                               0x0C03
#define HCI_VSC_BEKEN_CMD                       0xFCE0
#define HCI_READ_LOCAL_NAME                     0x0C14
#define HCI_READ_LOCAL_BDADDR                   0x1009
#define HCI_VSC_WRITE_BD_ADDR                   0xFC1a

#define HCI_VSC_WRITE_UART_CLOCK_SETTING        0xFC45
#define HCI_VSC_DOWNLOAD_MINIDRV                0xFC2E
#define HCI_VSC_WRITE_SLEEP_MODE                0xFC27
#define HCI_VSC_WRITE_SCO_PCM_INT_PARAM         0xFC1C
#define HCI_VSC_WRITE_PCM_DATA_FORMAT_PARAM     0xFC1E
#define HCI_VSC_WRITE_I2SPCM_INTERFACE_PARAM    0xFC6D
#define HCI_VSC_LAUNCH_RAM                      0xFC4E

#define HCI_EVT_CMD_CMPL_STATUS_RET_BYTE        5
#define HCI_EVT_CMD_CMPL_LOCAL_NAME_STRING      6
#define HCI_EVT_CMD_CMPL_LOCAL_BDADDR_ARRAY     6
#define HCI_EVT_CMD_CMPL_LOCAL_VERSION_STRING   6
#define HCI_EVT_CMD_CMPL_OPCODE                 3
#define LPM_CMD_PARAM_SIZE                      12
#define UPDATE_BAUDRATE_CMD_PARAM_SIZE          0x0B
#define HCI_CMD_PREAMBLE_SIZE                   3
#define HCD_REC_PAYLOAD_LEN_BYTE                2
#define BD_ADDR_LEN                             6
#define LOCAL_NAME_BUFFER_LEN                   32
#define LOCAL_BDADDR_PATH_BUFFER_LEN            256
#define LOCAL_VERSION_BUFFER_LEN                17

#define STREAM_TO_UINT16(u16, p) {u16 = ((uint16_t)(*(p)) + (((uint16_t)(*((p) + 1))) << 8)); (p) += 2;}
#define UINT16_TO_STREAM(p, u16) {*(p)++ = (uint8_t)(u16); *(p)++ = (uint8_t)((u16) >> 8);}
#define UINT32_TO_STREAM(p, u32) {*(p)++ = (uint8_t)(u32); *(p)++ = (uint8_t)((u32) >> 8); *(p)++ = (uint8_t)((u32) >> 16); *(p)++ = (uint8_t)((u32) >> 24);}

/******************************************************************************
**  Local type definitions
******************************************************************************/

/* Hardware Configuration State */
enum {
    HW_CFG_START = 1,
    HW_CFG_SET_UART_CLOCK,
    HW_CFG_SET_UART_BAUD_1,
    HW_CFG_READ_LOCAL_NAME,
    HW_CFG_DL_MINIDRIVER,
    HW_CFG_DL_FW_PATCH,
    HW_CFG_SET_UART_BAUD_2,
    HW_CFG_SET_BD_ADDR
#if (USE_CONTROLLER_BDADDR == TRUE)
    , HW_CFG_READ_BD_ADDR
#endif
};

/* h/w config control block */
typedef struct
{
    uint8_t state;                          /* Hardware configuration state */
    int     fw_fd;                          /* FW patch file fd */
    uint8_t f_set_baud_2;                   /* Baud rate switch state */
    char    local_chip_name[LOCAL_NAME_BUFFER_LEN];
} bt_hw_cfg_cb_t;

/* low power mode parameters */
typedef struct
{
    uint8_t sleep_mode;                     /* 0(disable),1(UART),9(H5) */
    uint8_t host_stack_idle_threshold;      /* Unit scale 300ms/25ms */
    uint8_t host_controller_idle_threshold; /* Unit scale 300ms/25ms */
    uint8_t bt_wake_polarity;               /* 0=Active Low, 1= Active High */
    uint8_t host_wake_polarity;             /* 0=Active Low, 1= Active High */
    uint8_t allow_host_sleep_during_sco;
    uint8_t combine_sleep_mode_and_lpm;
    uint8_t enable_uart_txd_tri_state;      /* UART_TXD Tri-State */
    uint8_t sleep_guard_time;               /* sleep guard time in 12.5ms */
    uint8_t wakeup_guard_time;              /* wakeup guard time in 12.5ms */
    uint8_t txd_config;                     /* TXD is high in sleep state */
    uint8_t pulsed_host_wake;               /* pulsed host wake if mode = 1 */
} bt_lpm_param_t;

/* Firmware re-launch settlement time */
typedef struct {
    const char *chipset_name;
    const uint32_t delay_time;
} fw_settlement_entry_t;


/******************************************************************************
**  Externs
******************************************************************************/

void hw_config_cback(void *p_evt_buf);
extern uint8_t vnd_local_bd_addr[BD_ADDR_LEN];


/******************************************************************************
**  Static variables
******************************************************************************/

static char fw_patchfile_path[256] = FW_PATCHFILE_LOCATION;
static char fw_patchfile_name[128] = { 0 };
#if (VENDOR_LIB_RUNTIME_TUNING_ENABLED == TRUE)
static int fw_patch_settlement_delay = -1;
#endif

static bt_hw_cfg_cb_t hw_cfg_cb;

static bt_lpm_param_t lpm_param =
{
    LPM_SLEEP_MODE,
    LPM_IDLE_THRESHOLD,
    LPM_HC_IDLE_THRESHOLD,
    LPM_BT_WAKE_POLARITY,
    LPM_HOST_WAKE_POLARITY,
    LPM_ALLOW_HOST_SLEEP_DURING_SCO,
    LPM_COMBINE_SLEEP_MODE_AND_LPM,
    LPM_ENABLE_UART_TXD_TRI_STATE,
    0,  /* not applicable */
    0,  /* not applicable */
    0,  /* not applicable */
    LPM_PULSED_HOST_WAKE
};

#if (!defined(SCO_USE_I2S_INTERFACE) || (SCO_USE_I2S_INTERFACE == FALSE))
static uint8_t bt_sco_param[SCO_PCM_PARAM_SIZE] =
{
    SCO_PCM_ROUTING,
    SCO_PCM_IF_CLOCK_RATE,
    SCO_PCM_IF_FRAME_TYPE,
    SCO_PCM_IF_SYNC_MODE,
    SCO_PCM_IF_CLOCK_MODE
};

static uint8_t bt_pcm_data_fmt_param[PCM_DATA_FORMAT_PARAM_SIZE] =
{
    PCM_DATA_FMT_SHIFT_MODE,
    PCM_DATA_FMT_FILL_BITS,
    PCM_DATA_FMT_FILL_METHOD,
    PCM_DATA_FMT_FILL_NUM,
    PCM_DATA_FMT_JUSTIFY_MODE
};
#else
static uint8_t bt_sco_param[SCO_I2SPCM_PARAM_SIZE] =
{
    SCO_I2SPCM_IF_MODE,
    SCO_I2SPCM_IF_ROLE,
    SCO_I2SPCM_IF_SAMPLE_RATE,
    SCO_I2SPCM_IF_CLOCK_RATE
};
#endif

/*
 * The look-up table of recommended firmware settlement delay (milliseconds) on
 * known chipsets.
 */
static const fw_settlement_entry_t fw_settlement_table[] = {
    {"BCM43241", 200},
    {(const char *) NULL, 100}  // Giving the generic fw settlement delay setting.
};

/******************************************************************************
**  Static functions
******************************************************************************/

/******************************************************************************
**  Controller Initialization Static Functions
******************************************************************************/

/*******************************************************************************
**
** Function        look_up_fw_settlement_delay
**
** Description     If FW_PATCH_SETTLEMENT_DELAY_MS has not been explicitly
**                 re-defined in the platform specific build-time configuration
**                 file, we will search into the look-up table for a
**                 recommended firmware settlement delay value.
**
**                 Although the settlement time might be also related to board
**                 configurations such as the crystal clocking speed.
**
** Returns         Firmware settlement delay
**
*******************************************************************************/
uint32_t look_up_fw_settlement_delay (void)
{
    uint32_t ret_value;
    fw_settlement_entry_t *p_entry;

    if (FW_PATCH_SETTLEMENT_DELAY_MS > 0)
    {
        ret_value = FW_PATCH_SETTLEMENT_DELAY_MS;
    }
#if (VENDOR_LIB_RUNTIME_TUNING_ENABLED == TRUE)
    else if (fw_patch_settlement_delay >= 0)
    {
        ret_value = fw_patch_settlement_delay;
    }
#endif
    else
    {
        p_entry = (fw_settlement_entry_t *)fw_settlement_table;

        while (p_entry->chipset_name != NULL)
        {
            if (strstr(hw_cfg_cb.local_chip_name, p_entry->chipset_name)!=NULL)
            {
                break;
            }

            p_entry++;
        }

        ret_value = p_entry->delay_time;
    }

    BTHWDBG( "Settlement delay -- %d ms", ret_value);

    return (ret_value);
}

/*******************************************************************************
**
** Function        ms_delay
**
** Description     sleep unconditionally for timeout milliseconds
**
** Returns         None
**
*******************************************************************************/
void ms_delay (uint32_t timeout)
{
    struct timespec delay;
    int err;

    if (timeout == 0)
        return;

    delay.tv_sec = timeout / 1000;
    delay.tv_nsec = 1000 * 1000 * (timeout%1000);

    /* [u]sleep can't be used because it uses SIGALRM */
    do {
        err = nanosleep(&delay, &delay);
    } while (err < 0 && errno ==EINTR);
}

/*******************************************************************************
**
** Function        line_speed_to_userial_baud
**
** Description     helper function converts line speed number into USERIAL baud
**                 rate symbol
**
** Returns         unit8_t (USERIAL baud symbol)
**
*******************************************************************************/
uint8_t line_speed_to_userial_baud(uint32_t line_speed)
{
    uint8_t baud;

    if (line_speed == 4000000)
        baud = USERIAL_BAUD_4M;
    else if (line_speed == 3000000)
        baud = USERIAL_BAUD_3M;
    else if (line_speed == 2000000)
        baud = USERIAL_BAUD_2M;
    else if (line_speed == 1500000)
	baud = USERIAL_BAUD_1_5M;
    else if (line_speed == 1000000)
        baud = USERIAL_BAUD_1M;
    else if (line_speed == 921600)
        baud = USERIAL_BAUD_921600;
    else if (line_speed == 460800)
        baud = USERIAL_BAUD_460800;
    else if (line_speed == 230400)
        baud = USERIAL_BAUD_230400;
    else if (line_speed == 115200)
        baud = USERIAL_BAUD_115200;
    else if (line_speed == 57600)
        baud = USERIAL_BAUD_57600;
    else if (line_speed == 19200)
        baud = USERIAL_BAUD_19200;
    else if (line_speed == 9600)
        baud = USERIAL_BAUD_9600;
    else if (line_speed == 1200)
        baud = USERIAL_BAUD_1200;
    else if (line_speed == 600)
        baud = USERIAL_BAUD_600;
    else
    {
        ALOGE( "userial vendor: unsupported baud speed %d", line_speed);
        baud = USERIAL_BAUD_115200;
    }

    return baud;
}


/*******************************************************************************
**
** Function         hw_strncmp
**
** Description      Used to compare two strings in caseless
**
** Returns          0: match, otherwise: not match
**
*******************************************************************************/
static int hw_strncmp (const char *p_str1, const char *p_str2, const int len)
{
    int i;

    if (!p_str1 || !p_str2)
        return (1);

    for (i = 0; i < len; i++)
    {
        if (toupper(p_str1[i]) != toupper(p_str2[i]))
            return (i+1);
    }

    return 0;
}

/*******************************************************************************
**
** Function         hw_config_findpatch
**
** Description      Search for a proper firmware patch file
**                  The selected firmware patch file name with full path
**                  will be stored in the input string parameter, i.e.
**                  p_chip_id_str, when returns.
**
** Returns          TRUE when found the target patch file, otherwise FALSE
**
*******************************************************************************/
static uint8_t hw_config_findpatch(char *p_chip_id_str)
{
    DIR *dirp;
    struct dirent *dp;
    int filenamelen;
    uint8_t retval = FALSE;

    BTHWDBG("Target name = [%s]", p_chip_id_str);

    if (strlen(fw_patchfile_name)> 0)
    {
        /* If specific filepath and filename have been given in run-time
         * configuration /etc/bluetooth/bt_vendor.conf file, we will use them
         * to concatenate the filename to open rather than searching a file
         * matching to chipset name in the fw_patchfile_path folder.
         */
        sprintf(p_chip_id_str, "%s", fw_patchfile_path);
        if (fw_patchfile_path[strlen(fw_patchfile_path)- 1] != '/')
        {
            strcat(p_chip_id_str, "/");
        }
        strcat(p_chip_id_str, fw_patchfile_name);

        ALOGI("FW patchfile: %s", p_chip_id_str);
        return TRUE;
    }

    if ((dirp = opendir(fw_patchfile_path)) != NULL)
    {
        /* Fetch next filename in patchfile directory */
        while ((dp = readdir(dirp)) != NULL)
        {
            /* Check if filename starts with chip-id name */
            if ((hw_strncmp(dp->d_name, p_chip_id_str, strlen(p_chip_id_str)) \
                ) == 0)
            {
                /* Check if it has .hcd extenstion */
                filenamelen = strlen(dp->d_name);
                if ((filenamelen >= FW_PATCHFILE_EXTENSION_LEN) &&
                    ((hw_strncmp(
                          &dp->d_name[filenamelen-FW_PATCHFILE_EXTENSION_LEN], \
                          FW_PATCHFILE_EXTENSION, \
                          FW_PATCHFILE_EXTENSION_LEN) \
                     ) == 0))
                {
                    ALOGI("Found patchfile: %s/%s", \
                        fw_patchfile_path, dp->d_name);

                    /* Make sure length does not exceed maximum */
                    if ((filenamelen + strlen(fw_patchfile_path)) > \
                         FW_PATCHFILE_PATH_MAXLEN)
                    {
                        ALOGE("Invalid patchfile name (too long)");
                    }
                    else
                    {
                        memset(p_chip_id_str, 0, FW_PATCHFILE_PATH_MAXLEN);
                        /* Found patchfile. Store location and name */
                        strcpy(p_chip_id_str, fw_patchfile_path);
                        if (fw_patchfile_path[ \
                            strlen(fw_patchfile_path)- 1 \
                            ] != '/')
                        {
                            strcat(p_chip_id_str, "/");
                        }
                        strcat(p_chip_id_str, dp->d_name);
                        retval = TRUE;
                    }
                    break;
                }
            }
        }

        closedir(dirp);

        if (retval == FALSE)
        {
            /* Try again chip name without revision info */

            int len = strlen(p_chip_id_str);
            char *p = p_chip_id_str + len - 1;

            /* Scan backward and look for the first alphabet
               which is not M or m
            */
            while (len > 3) // BCM****
            {
                if ((isdigit(*p)==0) && (*p != 'M') && (*p != 'm'))
                    break;

                p--;
                len--;
            }

            if (len > 3)
            {
                *p = 0;
                retval = hw_config_findpatch(p_chip_id_str);
            }
        }
    }
    else
    {
        ALOGE("Could not open %s", fw_patchfile_path);
    }

    return (retval);
}

/*******************************************************************************
**
** Function         hw_config_set_bdaddr
**
** Description      Program controller's Bluetooth Device Address
**
** Returns          TRUE, if valid address is sent
**                  FALSE, otherwise
**
*******************************************************************************/
static uint8_t hw_config_set_bdaddr(HC_BT_HDR *p_buf)
{
    uint8_t retval = FALSE;
    uint8_t *p = (uint8_t *) (p_buf + 1);

    ALOGI("Setting local bd addr to %02X:%02X:%02X:%02X:%02X:%02X",
        vnd_local_bd_addr[0], vnd_local_bd_addr[1], vnd_local_bd_addr[2],
        vnd_local_bd_addr[3], vnd_local_bd_addr[4], vnd_local_bd_addr[5]);

    UINT16_TO_STREAM(p, HCI_VSC_WRITE_BD_ADDR);
    *p++ = BD_ADDR_LEN; /* parameter length */
    *p++ = vnd_local_bd_addr[5];
    *p++ = vnd_local_bd_addr[4];
    *p++ = vnd_local_bd_addr[3];
    *p++ = vnd_local_bd_addr[2];
    *p++ = vnd_local_bd_addr[1];
    *p = vnd_local_bd_addr[0];

    p_buf->len = HCI_CMD_PREAMBLE_SIZE + BD_ADDR_LEN;
    hw_cfg_cb.state = HW_CFG_SET_BD_ADDR;

    retval = bt_vendor_cbacks->xmit_cb(HCI_VSC_WRITE_BD_ADDR, p_buf, \
                                 hw_config_cback);

    return (retval);
}

#if (USE_CONTROLLER_BDADDR == TRUE)
/*******************************************************************************
**
** Function         hw_config_read_bdaddr
**
** Description      Read controller's Bluetooth Device Address
**
** Returns          TRUE, if valid address is sent
**                  FALSE, otherwise
**
*******************************************************************************/
static uint8_t hw_config_read_bdaddr(HC_BT_HDR *p_buf)
{
    uint8_t retval = FALSE;
    uint8_t *p = (uint8_t *) (p_buf + 1);

    UINT16_TO_STREAM(p, HCI_READ_LOCAL_BDADDR);
    *p = 0; /* parameter length */

    p_buf->len = HCI_CMD_PREAMBLE_SIZE;
    hw_cfg_cb.state = HW_CFG_READ_BD_ADDR;

    retval = bt_vendor_cbacks->xmit_cb(HCI_READ_LOCAL_BDADDR, p_buf, \
                                 hw_config_cback);

    return (retval);
}
#endif // (USE_CONTROLLER_BDADDR == TRUE)
typedef struct
{
    int init_cmd_len;
    unsigned char *init_cmd;
}init_cmd_struct;


#define BK3512   1
#define BK3513   2
#define BK3515   3

unsigned char read_software_version_cmd[5] =
{
    0x01, 0xE0, 0xFC, 0x01, 0x20
};

unsigned char set_uart_921600_cmd[15] =
{
    0x01, 0xE0, 0xFC, 0x0B, 0x08, 0x80, 0xBA, 0x8C, 0x01, 0x00, 0x10, 0x0E, 0x00, 0x01, 0x01
};

unsigned char set_uart_2000000_cmd[15] =
{
    0x01, 0xE0, 0xFC, 0x0B, 0x08, 0x80, 0xBA, 0x8C, 0x01, 0x80, 0x84, 0x1E, 0x00, 0x01, 0x01
};

unsigned char enable_uart_RX_wakeup_cmd[6] =
{
    0x01, 0xE0, 0xFC, 0x02, 0x0F, 0x01
};

unsigned char disable_uart_RX_wakeup_cmd[6] =
{
    0x01, 0xE0, 0xFC, 0x02, 0x0F, 0x00
};

unsigned char set_uart_RX_wakeup_count_cmd[9] =
{
    0x01, 0xE0, 0xFC, 0x05, 0x0B, 0x01, 0x00, 0x10, 0x00
};

unsigned char enable_close_26M_cmd[6] =
{
    0x01, 0xE0, 0xFC, 0x02, 0x0C, 0x01
};

unsigned char disable_close_26M_cmd[6] =
{
    0x01, 0xE0, 0xFC, 0x02, 0x0C, 0x00
};

unsigned char bk3515_disable_cpu_sleep[6] =
{
    0x01, 0xE0, 0xFC, 0x02, 0x0B, 0x02
};

unsigned char set_BCSP_mode_cmd[6] =
{
    0x01, 0xE0, 0xFC, 0x02, 0x07, 0x01
};

unsigned char disable_hard_error_cmd[6] =
{
    0x01, 0xE0, 0xFC, 0x02, 0x33, 0x00
};

unsigned char set_CPU_speed_52M_cmd[6] =
{
    0x01, 0xE0, 0xFC, 0x02, 0x2F, 0x02
};

unsigned char Enable_wifi_Coexist_cmd[6] =
{
    0x01, 0xE0, 0xFC, 0x02, 0x4b, 0x01
};

unsigned char Disable_ECC_cmd[6] =
{
    0x01, 0xE0, 0xFC, 0x02, 0x2e, 0x00
};

unsigned char Enable_ECC2_cmd[6] =
{
    0x01, 0xE0, 0xFC, 0x02, 0x4c, 0x01
};

unsigned char set_pincode_timeout_cmd[9] =
{
    0x01, 0x66, 0xFC, 0x05, 0x00, 0x30, 0x80, 0x8b, 0x00
};

unsigned char set_sniff_intervel_cmd[7] =
{
    0x01, 0xE0, 0xFC, 0x03, 0x44, 0x08, 0x00
};

unsigned char bk3513_disable_analog_close_cmd[6] =
{
    0x01, 0xE0, 0xFC, 0x02, 0x11, 0x01
};

unsigned char bk3513_enable_analog_close_cmd[6] =
{
    0x01, 0xE0, 0xFC, 0x02, 0x11, 0x00
};

unsigned char bk3515_disable_analog_close_cmd[6] =
{
    0x01, 0xE0, 0xFC, 0x02, 0x0D, 0x00
};

unsigned char bk3515_enable_analog_close_cmd[6] =
{
    0x01, 0xE0, 0xFC, 0x02, 0x0D, 0x01
};

unsigned char bk3513_lower_sniff_current_cmd[6] =
{
    0x01, 0xE0, 0xFC, 0x02, 0x10, 0x00
};

unsigned char bk3515_lower_sniff_current_cmd[6] =
{
    0x01, 0xE0, 0xFC, 0x02, 0x0B, 0x00
};

unsigned char bk3513_enable_AFC_cmd[6] =
{
    0x01, 0xE0, 0xFC, 0x02, 0x09, 0x00
};
unsigned char bk3515_enable_AFC_cmd[6] =
{
    0x01, 0xE0, 0xFC, 0x02, 0x09, 0x01
};

unsigned char bk3513_config_PCM_slave_cmd[8] =
{
    0x01, 0xE0, 0xFC, 0x04, 0x0A, 0x03, 0x00, 0x00
};

unsigned char bk3513_config_ACL_buf_cmd[7] =
{
    0x01, 0xE0, 0xFC, 0x03, 0x13, 0x80, 0x00
};

unsigned char bk3513_enable_host_wakeup_lowlevel_cmd[9] =
{
    0x01, 0xE0, 0xFC, 0x05, 0x24, 0xA0, 0x0F, 0x00, 0x00
};

unsigned char bk3513_enable_uart_TX_wakeup_cmd[6] =
{
    0x01, 0xE0, 0xFC, 0x02, 0x0D, 0x01
};

unsigned char bk3513_disable_uart_TX_wakeup_cmd[6] =
{
    0x01, 0xE0, 0xFC, 0x02, 0x0D, 0x00
};

unsigned char bk3515_enable_uart_TX_wakeup_cmd[6] =
{
    0x01, 0xE0, 0xFC, 0x02, 0x11, 0x01
};

unsigned char bk3515_disable_uart_TX_wakeup_cmd[6] =
{
    0x01, 0xE0, 0xFC, 0x02, 0x11, 0x00
};

uint8_t chip_id = 0xff;

init_cmd_struct BK3512_init_cmd[] =
{
	{6, bk3513_enable_uart_TX_wakeup_cmd},
	{6, enable_uart_RX_wakeup_cmd},
	{9, set_uart_RX_wakeup_count_cmd},
	{7, set_sniff_intervel_cmd},
	{6, enable_close_26M_cmd},
	{6, bk3513_enable_analog_close_cmd},
	{6, bk3513_lower_sniff_current_cmd},
	{6, bk3513_enable_AFC_cmd},
	{9, set_pincode_timeout_cmd},
};

init_cmd_struct BK3513_wificoexist_cmd[] =
{
	{6, Enable_wifi_Coexist_cmd},
	{6, Disable_ECC_cmd},
	{6, Enable_ECC2_cmd},
};

init_cmd_struct BK3515_init_cmd[] = 
{
#if (!ENABLE_BLUETOOTH_AUTOSLEEP)
	{6, disable_close_26M_cmd},
	{6, bk3515_disable_analog_close_cmd},
	{6, bk3515_disable_cpu_sleep},
#endif
	{7, set_sniff_intervel_cmd},
#if (!ENABLE_BLUETOOTH_WAKEUP)
	{6, bk3515_disable_uart_TX_wakeup_cmd},
#endif
//	{6, bk3515_lower_sniff_current_cmd},
	{6, bk3515_enable_AFC_cmd},
#if CHANGE_CPU_TO_52M
	{6, set_CPU_speed_52M_cmd},
#endif
    {6, Disable_ECC_cmd},
	{9, set_pincode_timeout_cmd},
};


const unsigned int patch_Arr_0831[]=
{
    0x1C04B570 ,0x1C0D8D80 ,0xD1012807 ,0xBD70200C ,
    0x5D0020A2 ,0xD10642A8 ,0x21082200 ,0xF7991C20 ,
    0x2000FC85 ,0x205BBD70 ,0x28005D00 ,0x200CD101 ,
    0x7CE0BD70 ,0xD10A2800 ,0xFA0FF798 ,0xD1062800 ,
    0xF79B1C20 ,0x2801FB5C ,0x200CD101 ,0x1C26BD70 ,
    0x36C136FF ,0x28007AF0 ,0x2301D106 ,0x49551C22 ,
    0xF78D4855 ,0x72F0FB8F ,0x21427CE0 ,0x1C295508 ,
    0xF7971C20 ,0x2000FD35 ,0xB5F3BD70 ,0x9C01B081 ,
    0x309120FF ,0x82A034A0 ,0x9E017808 ,0x07801C0F ,
    0x36D00FC0 ,0x25017330 ,0xF78D1C78 ,0x8220FCF9 ,
    0xF78D1CF8 ,0x8260FCF5 ,0x90001DF8 ,0xFCF0F78D ,
    0x1D7883A0 ,0xF78D1C07 ,0x2801FCEB ,0x2001D203 ,
    0x25008320 ,0x1C38E003 ,0xFCE2F78D ,0x1C388320 ,
    0xFCDEF78D ,0x084F4939 ,0xD9014288 ,0x25008327 ,
    0xF78D9800 ,0x1C39FCD5 ,0xD90142B8 ,0x250083A1 ,
    0x30FF9801 ,0x8A813081 ,0xD00E2900 ,0x428A8A62 ,
    0x7AF1D90B ,0xD1052900 ,0x004B6A81 ,0x00891859 ,
    0xE0018261 ,0x82618AA1 ,0x8A612500 ,0xD10B2900 ,
    0x29007AF1 ,0x6A80D105 ,0x18180043 ,0x82600080 ,
    0x8AA0E001 ,0x25008260 ,0x49218A60 ,0xD50207C2 ,
    0x82604008 ,0x8A202500 ,0xD50207C2 ,0x82204008 ,
    0x8B202500 ,0x08498A61 ,0xD9044288 ,0x72F02006 ,
    0xB0032000 ,0x9801BDF0 ,0x7C802100 ,0xF887F78E ,
    0x01009901 ,0x0FC07CC9 ,0xD1042900 ,0x42817B31 ,
    0x7330D001 ,0x8A212500 ,0x42818A60 ,0x2500D300 ,
    0x42818AE1 ,0x8261D201 ,0x8AA02500 ,0x42888A61 ,
    0x8260D905 ,0x83202004 ,0x83A02001 ,0x1C282500 ,
    0x0000E7D7 ,0x00016B4B ,0x0000BB80 ,0x0000FFFF ,
    0x0000FFFE ,0x00010765 ,0x000107D5 ,0x00006B3F ,
};
const unsigned int patch_Arr_1219[]=
{
	0xF782B508 ,0x490EFE7F ,0x70082001 ,0x490E480D,
	0x28007D80 ,0x1C08D105 ,0x00496809 ,0x60010849,
	0x1C08BD08 ,0x22016809 ,0x431107D2 ,0xE7F76001,
	0xF782B508 ,0x4902FE58 ,0x70082000 ,0x0000BD08,
	0x00406494 ,0x00400380 ,0x00906028 ,0x000107A1,
};

void Write_BK3000_Reg(unsigned int reg, unsigned int v)
{
    unsigned char write_buff[30];
    unsigned int clen;

    clen = 14;
    write_buff[0]=0x01;
    write_buff[1]=0xE0;
    write_buff[2]=0xFC;
    write_buff[3]=0x0A;//(2+4+4*1)
    write_buff[4]=0x02;

    //addr
    write_buff[5]=(reg>>0)&0xff;
    write_buff[6]=(reg>>8)&0xff;
    write_buff[7]=(reg>>16)&0xff;
    write_buff[8]=(reg>>24)&0xff;

    //reg number
    write_buff[9]=0x01;

    write_buff[10]=(v>>0)&0xff;
    write_buff[11]=(v>>8)&0xff;
    write_buff[12]=(v>>16)&0xff;
    write_buff[13]=(v>>24)&0xff;

    userial_vendor_write(write_buff,clen);

    ms_delay(5);

} 

void bk3513_Patch()
{
    unsigned char write_buff[30];
    unsigned char i,j;
    unsigned int addr;
    unsigned int *patch_Arr;
    unsigned int patch_Arr_len;

    patch_Arr=patch_Arr_1219;

    patch_Arr_len=4*5;

    write_buff[0]=0x01;
    write_buff[1]=0xe0;
    write_buff[2]=0xfc;
    write_buff[3]=22;//(2+4+4*4)
    write_buff[4]=0x02;
   
    addr=0x0007f000;

    write_buff[9]=4;

    for(j=0;j<patch_Arr_len;j+=4)
    {
        write_buff[5]=(addr&0xff);
        write_buff[6]=((addr>>8)&0xff);
        write_buff[7]=((addr>>16)&0xff);
        write_buff[8]=((addr>>24)&0xff);
    
        for(i=0;i<4;i++)
        {
            write_buff[10+i*4+0]=patch_Arr[j+i]&0xff;
            write_buff[10+i*4+1]=(patch_Arr[j+i]>>8)&0xff;
            write_buff[10+i*4+2]=(patch_Arr[j+i]>>16)&0xff;
            write_buff[10+i*4+3]=(patch_Arr[j+i]>>24)&0xff;
        }
  
	userial_vendor_write(write_buff,10+16);
    
	ms_delay(5);

        addr+=16;
    }
	ALOGI("Robin bk3513_Patch()");
//52M cpu for SCO 
        Write_BK3000_Reg(0x800004,0xf91ef072);
        Write_BK3000_Reg(0x800000,(0xcdc0)>>2);

        Write_BK3000_Reg(0x80000c,0xf99af073);
        Write_BK3000_Reg(0x800008,(0xbcf8)>>2);

//force HV3 packet
        Write_BK3000_Reg(0x800014,0x43842040);
        Write_BK3000_Reg(0x800010,(0x506c)>>2);
}

void BK3515_write_init_cmd()
{
	uint8_t i;
	uint8_t init_cmd_num;

#if ENABLE_UART_FLOWCTL
	// enable uart cts/rts
	Write_BK3000_Reg(0x960010,0x00000040);
	Write_BK3000_Reg(0x960014,0x00000040);
	Write_BK3000_Reg(0x930018,0x00017040);
	userial_vendor_enable_CTSRTS(1);
	ALOGI("Robin open CTS/RTS");
#endif

#if 1
	//Write RF parameter
	ALOGI("Write RF parameter\n");
	Write_BK3000_Reg(0x910000+4*0,0x001718C0);
	Write_BK3000_Reg(0x910000+4*1,0xF3C401B0);
	Write_BK3000_Reg(0x910000+4*0x4,0x58EB5844);
	Write_BK3000_Reg(0x910000+4*0x6,0x06508078);
#endif

#if ENABLE_BLUETOOTH_AUTOSLEEP
	ALOGI("Enable bluetooth autosleep\n");
#else
    ALOGI("Disable bluetooth autosleep\n");
#endif

#if ENABLE_BLUETOOTH_WAKEUP
	ALOGI("Enable bluetooth wakeup host\n");
#else
    ALOGI("Disable bluetooth wakeup host\n");
#endif

	init_cmd_num = sizeof(BK3515_init_cmd) / sizeof(init_cmd_struct);
	for (i = 0; i < init_cmd_num; i++)
	{
	    ALOGI("write BK3515_init_cmd[%d]\n", i);
		if (userial_vendor_write(BK3515_init_cmd[i].init_cmd, BK3515_init_cmd[i].init_cmd_len) != BK3515_init_cmd[i].init_cmd_len) {
		    ALOGI("Failed to write init command (%d)", i);
		    perror("Failed to write init command");
		    return -1;
		}
		ms_delay(5);
	}
}

void BK3513_write_init_cmd()
{
	uint8_t init_cmd_num;
	uint8_t i;
	uint8_t is_eut_mode = 0;
	// enable uart cts/rts
	//Write_BK3000_Reg(0x960010,0x00000040);
	//Write_BK3000_Reg(0x960014,0x00000040);
	//Write_BK3000_Reg(0x930018,0x00017040);	
	//userial_vendor_enable_CTSRTS(1);

	ALOGI("Robin BK3513_write_init_cmd() Write RF parameter");
	// Write RF parameter
	Write_BK3000_Reg(0x910000+4*0,0x0017ee00);
	Write_BK3000_Reg(0x910000+4*1,0x900405b0);
	Write_BK3000_Reg(0x910000+4*0x6,0x07408078);
	//Write_BK3000_Reg(0x910000+4*0xa,0x0002E1EF);
	
	if (is_eut_mode==0)
	{
		ALOGI("Enter Normal Mode");
		Write_BK3000_Reg(0x910000+4*0xc,0xd04b0c3f);
		Write_BK3000_Reg(0x910000+4*0x31,0x0002307E);

		Write_BK3000_Reg(0x910000+4*0x28,0x03132333);
  		Write_BK3000_Reg(0x910000+4*0x2a,0x01504042);
		Write_BK3000_Reg(0x910000+4*0x3b,0x09345288);
	}
#if 0
    	if (chip_id == 0x3513)
	{
		init_cmd_num = sizeof(BK3513_wificoexist_cmd) / sizeof(init_cmd_struct);
		for (i = 0; i < init_cmd_num; i++)
		{
			if (write(fd, BK3513_wificoexist_cmd[i].init_cmd, BK3513_wificoexist_cmd[i].init_cmd_len) != BK3513_wificoexist_cmd[i].init_cmd_len) {
			    LOGD("Failed to write init command (%d)", i);
			    perror("Failed to write init command");
			    return -1;
			}
			ms_delay(5);
		}
	}
#endif
	/*Write patch*/
	if (chip_id == BK3513)
	{
		/*init PTA GPIO*/
		//Write_BK3000_Reg(0x960008,0);//GPIO2
		//Write_BK3000_Reg(0x960014,0);//GPIO5
		//Write_BK3000_Reg(0x960010,0x2c);//GPIO4
		bk3513_Patch();
	}
	init_cmd_num = sizeof(BK3512_init_cmd) / sizeof(init_cmd_struct);
	for (i = 0; i < init_cmd_num; i++)
	{
		if (userial_vendor_write(BK3512_init_cmd[i].init_cmd, BK3512_init_cmd[i].init_cmd_len) != BK3512_init_cmd[i].init_cmd_len) {
		    ALOGI("Failed to write init command (%d)", i);
		    perror("Failed to write init command");
		    return -1;
		}
		ms_delay(5);
	}
}

/*******************************************************************************
**
** Function         hw_config_cback
**
** Description      Callback function for controller configuration
**
** Returns          None
**
*******************************************************************************/
void hw_config_cback(void *p_mem)
{
    HC_BT_HDR *p_evt_buf = (HC_BT_HDR *) p_mem;
    char        *p_name, *p_tmp, *p_version;
    uint8_t     *p, status;
    uint16_t    opcode;
    HC_BT_HDR  *p_buf=NULL;
    uint8_t     is_proceeding = FALSE;
    int         i;
#if (USE_CONTROLLER_BDADDR == TRUE)
    const uint8_t null_bdaddr[BD_ADDR_LEN] = {0,0,0,0,0,0};
#endif

    status = *((uint8_t *)(p_evt_buf + 1) + HCI_EVT_CMD_CMPL_STATUS_RET_BYTE);
    p = (uint8_t *)(p_evt_buf + 1) + HCI_EVT_CMD_CMPL_OPCODE;
    STREAM_TO_UINT16(opcode,p);

    /* Ask a new buffer big enough to hold any HCI commands sent in here */
    if ((status == 0 || opcode == HCI_VSC_BEKEN_CMD) && bt_vendor_cbacks)
        p_buf = (HC_BT_HDR *) bt_vendor_cbacks->alloc(BT_HC_HDR_SIZE + \
                                                       HCI_CMD_MAX_LEN);

    if (p_buf != NULL)
    {
        p_buf->event = MSG_STACK_TO_HC_HCI_CMD;
        p_buf->offset = 0;
        p_buf->len = 0;
        p_buf->layer_specific = 0;

        p = (uint8_t *) (p_buf + 1);
	ALOGI("Robin hw_config_cback() state = %d", hw_cfg_cb.state);
        switch (hw_cfg_cb.state)
        {
            case HW_CFG_SET_UART_BAUD_1:
                /* update baud rate of host's UART port */
                ALOGI("bt vendor lib: set UART baud %i", UART_TARGET_BAUD_RATE);
                userial_vendor_set_baud( \
                    line_speed_to_userial_baud(UART_TARGET_BAUD_RATE) \
                );

                /* read local name */
                UINT16_TO_STREAM(p, HCI_READ_LOCAL_NAME);
                *p = 0; /* parameter length */

                p_buf->len = HCI_CMD_PREAMBLE_SIZE;
                hw_cfg_cb.state = HW_CFG_READ_LOCAL_NAME;

                is_proceeding = bt_vendor_cbacks->xmit_cb(HCI_READ_LOCAL_NAME, \
                                                    p_buf, hw_config_cback);
                break;

            case HW_CFG_READ_LOCAL_NAME:
                p_tmp = p_name = (char *) (p_evt_buf + 1) + \
                         HCI_EVT_CMD_CMPL_LOCAL_NAME_STRING;

                for (i=0; (i < LOCAL_NAME_BUFFER_LEN)||(*(p_name+i) != 0); i++)
                    *(p_name+i) = toupper(*(p_name+i));

                if ((p_name = strstr(p_name, "BEKEN")) != NULL)
                {
                    strncpy(hw_cfg_cb.local_chip_name, p_name, \
                            LOCAL_NAME_BUFFER_LEN-1);
                }
                else
                {
                    strncpy(hw_cfg_cb.local_chip_name, "UNKNOWN", \
                            LOCAL_NAME_BUFFER_LEN-1);
                    p_name = p_tmp;
                }

                hw_cfg_cb.local_chip_name[LOCAL_NAME_BUFFER_LEN-1] = 0;

                BTHWDBG("Chipset %s", hw_cfg_cb.local_chip_name);
                if (is_proceeding == FALSE)
                {
                    is_proceeding = hw_config_set_bdaddr(p_buf);
                }
                break;

            case HW_CFG_DL_MINIDRIVER:
                /* give time for placing firmware in download mode */
                ms_delay(50);
                hw_cfg_cb.state = HW_CFG_DL_FW_PATCH;
                /* fall through intentionally */
            case HW_CFG_DL_FW_PATCH:
                p_buf->len = read(hw_cfg_cb.fw_fd, p, HCI_CMD_PREAMBLE_SIZE);
                if (p_buf->len > 0)
                {
                    if ((p_buf->len < HCI_CMD_PREAMBLE_SIZE) || \
                        (opcode == HCI_VSC_LAUNCH_RAM))
                    {
                        ALOGW("firmware patch file might be altered!");
                    }
                    else
                    {
                        p_buf->len += read(hw_cfg_cb.fw_fd, \
                                           p+HCI_CMD_PREAMBLE_SIZE,\
                                           *(p+HCD_REC_PAYLOAD_LEN_BYTE));
                        STREAM_TO_UINT16(opcode,p);
                        is_proceeding = bt_vendor_cbacks->xmit_cb(opcode, \
                                                p_buf, hw_config_cback);
                        break;
                    }
                }

                close(hw_cfg_cb.fw_fd);
                hw_cfg_cb.fw_fd = -1;

                /* Normally the firmware patch configuration file
                 * sets the new starting baud rate at 115200.
                 * So, we need update host's baud rate accordingly.
                 */
                ALOGI("bt vendor lib: set UART baud 115200");
                userial_vendor_set_baud(USERIAL_BAUD_115200);

                /* Next, we would like to boost baud rate up again
                 * to desired working speed.
                 */
                hw_cfg_cb.f_set_baud_2 = TRUE;

                /* Check if we need to pause a few hundred milliseconds
                 * before sending down any HCI command.
                 */
                ms_delay(look_up_fw_settlement_delay());

                /* fall through intentionally */
            case HW_CFG_START:
                p_tmp = p_version = (char *) (p_evt_buf + 1) + \
                         HCI_EVT_CMD_CMPL_LOCAL_VERSION_STRING;
                if (strncmp(p_version, "BK3000V4", 8) == 0)
                {
                    chip_id = BK3513;
                }
                else if (strncmp(p_version, "BK3000V5", 8) == 0)
                {
                    chip_id = BK3515;
                }
   		ALOGI("Robin hw_config_cback() chip_id = %d", chip_id);
                //break;
            //case HW_CFG_SET_UART_CLOCK:
#if 1
            {
                u_int8_t Beken_change_uart_1M_cmd[15] = {0x01, 0xe0, 0xfc, 0x0b, 0x08, 0x80, 0xba, 0x8c, 0x01, 0x40, 0x42, 0x0f, 0x00, 0x00, 0x00};  //1000000
		//u_int8_t Beken_change_uart_1_5M_cmd[15] = {0x01, 0xE0, 0xFC, 0x0B, 0x08, 0x80, 0xBA, 0x8C, 0x01, 0x60, 0xe3, 0x16, 0x00, 0x01, 0x01};  //1500000
                //u_int8_t Beken_change_uart_2000000_cmd[15] = {0x01, 0xe0, 0xfc, 0x0b, 0x08, 0x80, 0xba, 0x8c, 0x01, 0x80, 0x84, 0x1e, 0x00, 0x00, 0x00}; //2000000
                userial_vendor_write((uint8_t *)Beken_change_uart_1M_cmd, 15);
                /* update baud rate of host's UART port */
                ALOGI("Robin bt vendor lib: set UART baud %d", UART_TARGET_BAUD_RATE);
                ms_delay(50);
                userial_vendor_set_baud( \
                                    line_speed_to_userial_baud(UART_TARGET_BAUD_RATE) \
                                    );
                ALOGI("Robin bt vendor lib baud = %d", userial_vendor_get_baud());
		/*write init parameter*/
                if (chip_id == BK3515)
                {
                    BK3515_write_init_cmd();
                }
                else if (chip_id == BK3513)
                {
                    BK3513_write_init_cmd();
                }
                /* read local name */
                UINT16_TO_STREAM(p, HCI_READ_LOCAL_NAME);
                *p = 0; /* parameter length */

                p_buf->len = HCI_CMD_PREAMBLE_SIZE;
                hw_cfg_cb.state = HW_CFG_READ_LOCAL_NAME;
                ms_delay(50);
                is_proceeding = bt_vendor_cbacks->xmit_cb(HCI_READ_LOCAL_NAME, \
                                                            p_buf, hw_config_cback);
            }
#else
                /* set controller's UART baud rate to 3M */
                UINT16_TO_STREAM(p, HCI_VSC_BEKEN_CMD);
                *p++ = UPDATE_BAUDRATE_CMD_PARAM_SIZE; /* parameter length */
                *p++ = 0x08;                            /* Set baudrate command */
                UINT32_TO_STREAM(p, UART_TARGET_CLOCK); /* Set baudrate clock */
                UINT32_TO_STREAM(p, UART_TARGET_BAUD_RATE); /*Set baudrate*/
                *p++ = 0x01;                            /*parity Enable*/
                *p++ = 0x01;                            /*even parity*/

                p_buf->len = HCI_CMD_PREAMBLE_SIZE + \
                             UPDATE_BAUDRATE_CMD_PARAM_SIZE;
                hw_cfg_cb.state = (hw_cfg_cb.f_set_baud_2) ? \
                            HW_CFG_SET_UART_BAUD_2 : HW_CFG_SET_UART_BAUD_1;

                is_proceeding = bt_vendor_cbacks->xmit_cb(HCI_VSC_BEKEN_CMD, \
                                                    p_buf, hw_config_cback);
#endif
                break;

            case HW_CFG_SET_UART_BAUD_2:
                /* update baud rate of host's UART port */
                ALOGI("bt vendor lib: set UART baud %i", UART_TARGET_BAUD_RATE);
                userial_vendor_set_baud( \
                    line_speed_to_userial_baud(UART_TARGET_BAUD_RATE) \
                );

#if (USE_CONTROLLER_BDADDR == TRUE)
                if ((is_proceeding = hw_config_read_bdaddr(p_buf)) == TRUE)
                    break;
#else
                if ((is_proceeding = hw_config_set_bdaddr(p_buf)) == TRUE)
                    break;
#endif
                /* fall through intentionally */
            case HW_CFG_SET_BD_ADDR:
                ALOGI("vendor lib fwcfg completed");
                bt_vendor_cbacks->dealloc(p_buf);
                bt_vendor_cbacks->fwcfg_cb(BT_VND_OP_RESULT_SUCCESS);

                hw_cfg_cb.state = 0;

                if (hw_cfg_cb.fw_fd != -1)
                {
                    close(hw_cfg_cb.fw_fd);
                    hw_cfg_cb.fw_fd = -1;
                }

                is_proceeding = TRUE;
                break;

#if (USE_CONTROLLER_BDADDR == TRUE)
            case HW_CFG_READ_BD_ADDR:
                p_tmp = (char *) (p_evt_buf + 1) + \
                         HCI_EVT_CMD_CMPL_LOCAL_BDADDR_ARRAY;

                if (memcmp(p_tmp, null_bdaddr, BD_ADDR_LEN) == 0)
                {
                    // Controller does not have a valid OTP BDADDR!
                    // Set the BTIF initial BDADDR instead.
                    if ((is_proceeding = hw_config_set_bdaddr(p_buf)) == TRUE)
                        break;
                }
                else
                {
                    ALOGI("Controller OTP bdaddr %02X:%02X:%02X:%02X:%02X:%02X",
                        *(p_tmp+5), *(p_tmp+4), *(p_tmp+3),
                        *(p_tmp+2), *(p_tmp+1), *p_tmp);
                }

                ALOGI("vendor lib fwcfg completed");
                bt_vendor_cbacks->dealloc(p_buf);
                bt_vendor_cbacks->fwcfg_cb(BT_VND_OP_RESULT_SUCCESS);

                hw_cfg_cb.state = 0;

                if (hw_cfg_cb.fw_fd != -1)
                {
                    close(hw_cfg_cb.fw_fd);
                    hw_cfg_cb.fw_fd = -1;
                }

                is_proceeding = TRUE;
                break;
#endif // (USE_CONTROLLER_BDADDR == TRUE)
        } // switch(hw_cfg_cb.state)
    } // if (p_buf != NULL)

    /* Free the RX event buffer */
    if (bt_vendor_cbacks)
        bt_vendor_cbacks->dealloc(p_evt_buf);

    if (is_proceeding == FALSE)
    {
        ALOGE("vendor lib fwcfg aborted!!!");
        if (bt_vendor_cbacks)
        {
            if (p_buf != NULL)
                bt_vendor_cbacks->dealloc(p_buf);

            bt_vendor_cbacks->fwcfg_cb(BT_VND_OP_RESULT_FAIL);
        }

        if (hw_cfg_cb.fw_fd != -1)
        {
            close(hw_cfg_cb.fw_fd);
            hw_cfg_cb.fw_fd = -1;
        }

        hw_cfg_cb.state = 0;
    }
}

/******************************************************************************
**   LPM Static Functions
******************************************************************************/

/*******************************************************************************
**
** Function         hw_lpm_ctrl_cback
**
** Description      Callback function for lpm enable/disable rquest
**
** Returns          None
**
*******************************************************************************/
void hw_lpm_ctrl_cback(void *p_mem)
{
    HC_BT_HDR *p_evt_buf = (HC_BT_HDR *) p_mem;
    bt_vendor_op_result_t status = BT_VND_OP_RESULT_FAIL;

    if (*((uint8_t *)(p_evt_buf + 1) + HCI_EVT_CMD_CMPL_STATUS_RET_BYTE) == 0)
    {
        status = BT_VND_OP_RESULT_SUCCESS;
    }

    if (bt_vendor_cbacks)
    {
        bt_vendor_cbacks->lpm_cb(status);
        bt_vendor_cbacks->dealloc(p_evt_buf);
    }
}


#if (SCO_CFG_INCLUDED == TRUE)
/*****************************************************************************
**   SCO Configuration Static Functions
*****************************************************************************/

/*******************************************************************************
**
** Function         hw_sco_cfg_cback
**
** Description      Callback function for SCO configuration rquest
**
** Returns          None
**
*******************************************************************************/
void hw_sco_cfg_cback(void *p_mem)
{
    HC_BT_HDR *p_evt_buf = (HC_BT_HDR *) p_mem;
    uint8_t     *p;
    uint16_t    opcode;
    HC_BT_HDR  *p_buf=NULL;

    p = (uint8_t *)(p_evt_buf + 1) + HCI_EVT_CMD_CMPL_OPCODE;
    STREAM_TO_UINT16(opcode,p);

    /* Free the RX event buffer */
    if (bt_vendor_cbacks)
        bt_vendor_cbacks->dealloc(p_evt_buf);

#if (!defined(SCO_USE_I2S_INTERFACE) || (SCO_USE_I2S_INTERFACE == FALSE))
    if (opcode == HCI_VSC_WRITE_SCO_PCM_INT_PARAM)
    {
        uint8_t ret = FALSE;

        /* Ask a new buffer to hold WRITE_PCM_DATA_FORMAT_PARAM command */
        if (bt_vendor_cbacks)
            p_buf = (HC_BT_HDR *) bt_vendor_cbacks->alloc(BT_HC_HDR_SIZE + \
                                                HCI_CMD_PREAMBLE_SIZE + \
                                                PCM_DATA_FORMAT_PARAM_SIZE);
        if (p_buf)
        {
            p_buf->event = MSG_STACK_TO_HC_HCI_CMD;
            p_buf->offset = 0;
            p_buf->layer_specific = 0;
            p_buf->len = HCI_CMD_PREAMBLE_SIZE + PCM_DATA_FORMAT_PARAM_SIZE;

            p = (uint8_t *) (p_buf + 1);
            UINT16_TO_STREAM(p, HCI_VSC_WRITE_PCM_DATA_FORMAT_PARAM);
            *p++ = PCM_DATA_FORMAT_PARAM_SIZE;
            memcpy(p, &bt_pcm_data_fmt_param, PCM_DATA_FORMAT_PARAM_SIZE);

            if ((ret = bt_vendor_cbacks->xmit_cb(HCI_VSC_WRITE_PCM_DATA_FORMAT_PARAM,\
                                           p_buf, hw_sco_cfg_cback)) == FALSE)
            {
                bt_vendor_cbacks->dealloc(p_buf);
            }
            else
                return;
        }
    }
#endif  // !SCO_USE_I2S_INTERFACE

if (bt_vendor_cbacks)
    bt_vendor_cbacks->scocfg_cb(BT_VND_OP_RESULT_SUCCESS);
}
#endif // SCO_CFG_INCLUDED

/*****************************************************************************
**   Hardware Configuration Interface Functions
*****************************************************************************/


/*******************************************************************************
**
** Function        hw_config_start
**
** Description     Kick off controller initialization process
**
** Returns         None
**
*******************************************************************************/
void hw_config_start(void)
{
    HC_BT_HDR  *p_buf = NULL;
    uint8_t     *p;

    hw_cfg_cb.state = 0;
    hw_cfg_cb.fw_fd = -1;
    hw_cfg_cb.f_set_baud_2 = FALSE;

    /* Start from sending HCI_RESET */
    ALOGI("Robin hw_config_start()");
    if (bt_vendor_cbacks)
    {
        p_buf = (HC_BT_HDR *) bt_vendor_cbacks->alloc(BT_HC_HDR_SIZE + \
                                                       HCI_CMD_PREAMBLE_SIZE);
    }

    if (p_buf)
    {
        p_buf->event = MSG_STACK_TO_HC_HCI_CMD;
        p_buf->offset = 0;
        p_buf->layer_specific = 0;
        p_buf->len = HCI_CMD_PREAMBLE_SIZE + 1;

        p = (uint8_t *) (p_buf + 1);
        UINT16_TO_STREAM(p, HCI_VSC_BEKEN_CMD);
        *p++ = 0x01; /* parameter length */
	*p = 0x20;    /*get version */

        hw_cfg_cb.state = HW_CFG_START;

        bt_vendor_cbacks->xmit_cb(HCI_VSC_BEKEN_CMD, p_buf, hw_config_cback);
    }
    else
    {
        if (bt_vendor_cbacks)
        {
            ALOGE("vendor lib fw conf aborted [no buffer]");
            bt_vendor_cbacks->fwcfg_cb(BT_VND_OP_RESULT_FAIL);
        }
    }
}

/*******************************************************************************
**
** Function        hw_lpm_enable
**
** Description     Enalbe/Disable LPM
**
** Returns         TRUE/FALSE
**
*******************************************************************************/
uint8_t hw_lpm_enable(uint8_t turn_on)
{
    HC_BT_HDR  *p_buf = NULL;
    uint8_t     *p;
    uint8_t     ret = FALSE;
#if 0
    if (bt_vendor_cbacks)
        p_buf = (HC_BT_HDR *) bt_vendor_cbacks->alloc(BT_HC_HDR_SIZE + \
                                                       HCI_CMD_PREAMBLE_SIZE + \
                                                       LPM_CMD_PARAM_SIZE);

    if (p_buf)
    {
        p_buf->event = MSG_STACK_TO_HC_HCI_CMD;
        p_buf->offset = 0;
        p_buf->layer_specific = 0;
        p_buf->len = HCI_CMD_PREAMBLE_SIZE + LPM_CMD_PARAM_SIZE;

        p = (uint8_t *) (p_buf + 1);
        UINT16_TO_STREAM(p, HCI_VSC_WRITE_SLEEP_MODE);
        *p++ = LPM_CMD_PARAM_SIZE; /* parameter length */

        if (turn_on)
        {
            memcpy(p, &lpm_param, LPM_CMD_PARAM_SIZE);
            upio_set(UPIO_LPM_MODE, UPIO_ASSERT, 0);
        }
        else
        {
            memset(p, 0, LPM_CMD_PARAM_SIZE);
            upio_set(UPIO_LPM_MODE, UPIO_DEASSERT, 0);
        }

        if ((ret = bt_vendor_cbacks->xmit_cb(HCI_VSC_WRITE_SLEEP_MODE, p_buf, \
                                        hw_lpm_ctrl_cback)) == FALSE)
        {
            bt_vendor_cbacks->dealloc(p_buf);
        }
    }
#endif
	if (turn_on)
        {
            //memcpy(p, &lpm_param, LPM_CMD_PARAM_SIZE);
            upio_set(UPIO_LPM_MODE, UPIO_ASSERT, 0);
        }
        else
        {
            //memset(p, 0, LPM_CMD_PARAM_SIZE);
            upio_set(UPIO_LPM_MODE, UPIO_DEASSERT, 0);
        }
    //if ((ret == FALSE) && bt_vendor_cbacks)
        bt_vendor_cbacks->lpm_cb(BT_VND_OP_RESULT_SUCCESS);

    return ret;
}

/*******************************************************************************
**
** Function        hw_lpm_get_idle_timeout
**
** Description     Calculate idle time based on host stack idle threshold
**
** Returns         idle timeout value
**
*******************************************************************************/
uint32_t hw_lpm_get_idle_timeout(void)
{
    uint32_t timeout_ms;

    /* set idle time to be LPM_IDLE_TIMEOUT_MULTIPLE times of
     * host stack idle threshold (in 300ms/25ms)
     */
    timeout_ms = (uint32_t)lpm_param.host_stack_idle_threshold \
                            * LPM_IDLE_TIMEOUT_MULTIPLE;

    if (strstr(hw_cfg_cb.local_chip_name, "BCM4325") != NULL)
        timeout_ms *= 25; // 12.5 or 25 ?
    else
        timeout_ms *= 300;

    return timeout_ms;
}

/*******************************************************************************
**
** Function        hw_lpm_set_wake_state
**
** Description     Assert/Deassert BT_WAKE
**
** Returns         None
**
*******************************************************************************/
void hw_lpm_set_wake_state(uint8_t wake_assert)
{
    uint8_t state = (wake_assert) ? UPIO_ASSERT : UPIO_DEASSERT;

    upio_set(UPIO_BT_WAKE, state, lpm_param.bt_wake_polarity);
}

#if (SCO_CFG_INCLUDED == TRUE)
/*******************************************************************************
**
** Function         hw_sco_config
**
** Description      Configure SCO related hardware settings
**
** Returns          None
**
*******************************************************************************/
void hw_sco_config(void)
{
    HC_BT_HDR  *p_buf = NULL;
    uint8_t     *p, ret;

#if (!defined(SCO_USE_I2S_INTERFACE) || (SCO_USE_I2S_INTERFACE == FALSE))
    uint16_t cmd_u16 = HCI_CMD_PREAMBLE_SIZE + SCO_PCM_PARAM_SIZE;
#else
    uint16_t cmd_u16 = HCI_CMD_PREAMBLE_SIZE + SCO_I2SPCM_PARAM_SIZE;
#endif
#if 0
    if (bt_vendor_cbacks)
        p_buf = (HC_BT_HDR *) bt_vendor_cbacks->alloc(BT_HC_HDR_SIZE+cmd_u16);
 
    if (p_buf)
    {
        p_buf->event = MSG_STACK_TO_HC_HCI_CMD;
        p_buf->offset = 0;
        p_buf->layer_specific = 0;
        p_buf->len = cmd_u16;

        p = (uint8_t *) (p_buf + 1);
#if (!defined(SCO_USE_I2S_INTERFACE) || (SCO_USE_I2S_INTERFACE == FALSE))
        UINT16_TO_STREAM(p, HCI_VSC_WRITE_SCO_PCM_INT_PARAM);
        *p++ = SCO_PCM_PARAM_SIZE;
        memcpy(p, &bt_sco_param, SCO_PCM_PARAM_SIZE);
        cmd_u16 = HCI_VSC_WRITE_SCO_PCM_INT_PARAM;
        ALOGI("SCO PCM configure {%d, %d, %d, %d, %d}",
           bt_sco_param[0], bt_sco_param[1], bt_sco_param[2], bt_sco_param[3], \
           bt_sco_param[4]);

#else
        UINT16_TO_STREAM(p, HCI_VSC_WRITE_I2SPCM_INTERFACE_PARAM);
        *p++ = SCO_I2SPCM_PARAM_SIZE;
        memcpy(p, &bt_sco_param, SCO_I2SPCM_PARAM_SIZE);
        cmd_u16 = HCI_VSC_WRITE_I2SPCM_INTERFACE_PARAM;
        ALOGI("SCO over I2SPCM interface {%d, %d, %d, %d}",
           bt_sco_param[0], bt_sco_param[1], bt_sco_param[2], bt_sco_param[3]);
#endif

        if ((ret=bt_vendor_cbacks->xmit_cb(cmd_u16, p_buf, hw_sco_cfg_cback)) \
             == FALSE)
        {
            bt_vendor_cbacks->dealloc(p_buf);
        }
        else
            return;
    }

    if (bt_vendor_cbacks)
    {
        ALOGE("vendor lib scocfg aborted");
        bt_vendor_cbacks->scocfg_cb(BT_VND_OP_RESULT_FAIL);
    }
#endif
    if (bt_vendor_cbacks)
    {
        bt_vendor_cbacks->scocfg_cb(BT_VND_OP_RESULT_SUCCESS);
    }
}
#endif  // SCO_CFG_INCLUDED

/*******************************************************************************
**
** Function        hw_set_patch_file_path
**
** Description     Set the location of firmware patch file
**
** Returns         0 : Success
**                 Otherwise : Fail
**
*******************************************************************************/
int hw_set_patch_file_path(char *p_conf_name, char *p_conf_value, int param)
{

    strcpy(fw_patchfile_path, p_conf_value);

    return 0;
}

/*******************************************************************************
**
** Function        hw_set_patch_file_name
**
** Description     Give the specific firmware patch filename
**
** Returns         0 : Success
**                 Otherwise : Fail
**
*******************************************************************************/
int hw_set_patch_file_name(char *p_conf_name, char *p_conf_value, int param)
{

    strcpy(fw_patchfile_name, p_conf_value);

    return 0;
}

#if (VENDOR_LIB_RUNTIME_TUNING_ENABLED == TRUE)
/*******************************************************************************
**
** Function        hw_set_patch_settlement_delay
**
** Description     Give the specific firmware patch settlement time in milliseconds
**
** Returns         0 : Success
**                 Otherwise : Fail
**
*******************************************************************************/
int hw_set_patch_settlement_delay(char *p_conf_name, char *p_conf_value, int param)
{
    fw_patch_settlement_delay = atoi(p_conf_value);

    return 0;
}
#endif  //VENDOR_LIB_RUNTIME_TUNING_ENABLED

