#include "bt_vendor_lib.h"
#include "bt_rda.h"

extern unsigned char bt_addr_cmd[10];
//===============        I N T E R F A C E S      =======================

int rda_bt_init(const bt_vendor_callbacks_t* p_cb, unsigned char *local_bdaddr)
{
    ALOGD("rda_bt_init\n");
    set_callbacks(p_cb);

    ALOGE("local_bdaddr: %02X:%02X:%02X:%02X:%02X:%02X", 
        local_bdaddr[0], local_bdaddr[1], local_bdaddr[2], local_bdaddr[3], local_bdaddr[4], local_bdaddr[5]);
    /* This is handed over from the stack */
    int i=0, j=6; 
    for(i=0; i<6; i++)
    {
        j--;
        if(j<0) break;
        bt_addr_cmd[i+4] = local_bdaddr[j];
    }
    return 0;
}

int rda_bt_op(bt_vendor_opcode_t opcode, void *param)
{
    int ret = 0;
    
    switch(opcode) 
    {
    	case BT_VND_OP_POWER_CTRL:
    	{
    	    ALOGD("BT_VND_OP_POWER_CTRL\n");
    	    ret = set_bluetooth_power(*((int*)param));
    	    break;
	    }
    	    
    	case BT_VND_OP_USERIAL_OPEN:
    	{
    	    ALOGD("BT_VND_OP_USERIAL_OPEN\n");
    	    
    	    ((int*)param)[0] = init_uart();
    	    ret = 1; // CMD/EVT/ACL-In/ACL-Out via the same fd
    	    break;
	    }
    	    
    	case BT_VND_OP_USERIAL_CLOSE:
    	{
    	    ALOGD("BT_VND_OP_USERIAL_CLOSE\n");
    	    close_uart();
    	    break;
	    }
    	    
    	case BT_VND_OP_FW_CFG:
    	{
    	    ALOGD("BT_VND_OP_FW_CFG\n");
    	    ret = rda_fw_cfg();
    	    break;
	    }
    	    
    	case BT_VND_OP_SCO_CFG:
    	{
    	    ALOGD("BT_VND_OP_SCO_CFG\n");
    	    ret = rda_sco_cfg();
    	    break;
	    }
    	    
    	case BT_VND_OP_GET_LPM_IDLE_TIMEOUT:
    	{
    	    ALOGD("BT_VND_OP_GET_LPM_IDLE_TIMEOUT\n");
    	    *((uint32_t*)param) = 3000; //ms
    	    break;
	    }
    	    
    	case BT_VND_OP_LPM_SET_MODE:
    	{
    	    ALOGD("BT_VND_OP_LPM_SET_MODE\n");
    	    ret = rda_sleep_cfg();
    	    break;
	    }
    	    
    	case BT_VND_OP_LPM_WAKE_SET_STATE:
		{
            uint8_t *state = (uint8_t *) param;
            
            uint8_t wake_assert = (*state == BT_VND_LPM_WAKE_ASSERT) ?  TRUE : FALSE;
            
    	    //ALOGD("BT_VND_OP_LPM_WAKE_SET_STATE,wake_assert:0x%x.\n",wake_assert);

    	    ret = rda_wake_chip(wake_assert);
    	    break;
        }
    	    
    	default:
    	{
    	    ALOGE("Unknown operation %d\n", opcode);
    	    break;
	    }
    }
    
    return ret;
}

void rda_bt_cleanup()
{
    clean_callbacks();
    return;
}

const bt_vendor_interface_t BLUETOOTH_VENDOR_LIB_INTERFACE = {
    sizeof(bt_vendor_interface_t),
    rda_bt_init,
    rda_bt_op,
    rda_bt_cleanup
};


