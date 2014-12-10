#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include <time.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <sys/param.h>

#include "bt_cfg.h"
#include "bt_rda.h"


unsigned char bt_addr_cmd[10]= {0x01, 0x1a, 0xfc, 0x06, 0x50, 0x6a, 0xb8, 0xf1, 0x76, 0x58};
int RDABT_core_Intialization(int fd);
#define MODEM_DEV_PATH    "/dev/voice_modem"
#define BP_IOCTL_BASE 0x1a
#define BP_IOCTL_GET_BPID       _IOR(BP_IOCTL_BASE, 0x07, int)

static int getBpID(void){
    int bp_fd = -1;
    int biID =-1;
    int err = -1;

    bp_fd = open(MODEM_DEV_PATH, O_RDWR);
    if(bp_fd > 0){
        err = ioctl(bp_fd,BP_IOCTL_GET_BPID,&biID);
        if(err < 0){
           LOGD("biID=%d getBpID failed  ioctrl err =%d bp_fd=%d",biID,err,bp_fd);
           close(bp_fd);
           return -1;
        }else{
            LOGD("biID=%d getBpID sucessed",biID);
            close(bp_fd);
            return biID;
       }
   }

   LOGE("biID=%d getBpID failed bp_fd = ",biID,bp_fd);
   return err;
}

static int rda_bluetooth_incall_set (int fd)
{
    unsigned int i, num_send;
    int id = getBpID();
    
    switch(id)
    {
        case 6: //M0 modem
        {
            unsigned char rda_bt_incall[][14] =
            {
                {0x01,0x02,0xfd,0x0a,0x00,0x01, 0xC4, 0x18, 0x00, 0x80, 0x07, 0x00, 0x0f, 0x00},
                {0x01,0x02,0xfd,0x0a,0x00,0x01, 0x34, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x02},
                {0x01,0x02,0xfd,0x0a,0x00,0x01, 0xCC, 0x18, 0x00, 0x80, 0x00, 0x80, 0x00, 0x00},
            };

            for (i = 0; i < sizeof(rda_bt_incall)/sizeof(rda_bt_incall[0]); i++) 
            {
               num_send = write(fd, rda_bt_incall[i], sizeof(rda_bt_incall[i]));
               if (num_send != sizeof(rda_bt_incall[i])) {
                   printf("num_send = %d (%d)\n", num_send, sizeof(rda_bt_incall[i]));
                   return -1;
               }
               usleep(10000);
            }

        }   
            break;

        case 14: //A85xx modem
        {
            unsigned char rda_bt_incall[][14] =
            {
                {0x01,0x02,0xfd,0x0a,0x00,0x01, 0xC4, 0x18, 0x00, 0x80, 0x07, 0x00, 0x9e, 0x90},
            };

            for (i = 0; i < sizeof(rda_bt_incall)/sizeof(rda_bt_incall[0]); i++)
            {
                num_send = write(fd, rda_bt_incall[i], sizeof(rda_bt_incall[i]));
                if (num_send != sizeof(rda_bt_incall[i])) {
                    printf("num_send = %d (%d)\n", num_send, sizeof(rda_bt_incall[i]));
                    return -1;
                }
              usleep(10000);
            }
        }
            break;

        default: //use default pcm config as M51 M50 aw706 modem
            break;
    }
   return 0;
}

//setup uart flow control, if your uart hardware fifo less than 480 bytes.
int rda_setup_flow_ctl(int fd)
{
    unsigned int i, num_send;
    unsigned char rda_flow_ctl_10[][14] =
    {
        {0x01,0x02,0xfd,0x0a,0x00,0x01,0x44,0x00,0x20,0x40,0x3c,0x00,0x00,0x00},                                                                  
	      {0x01,0x02,0xfd,0x0a,0x00,0x01,0x10,0x00,0x00,0x50,0x22,0x01,0x00,0x00},// flow control
    };
	
    /*Setup flow control */
    for (i = 0; i < sizeof(rda_flow_ctl_10)/sizeof(rda_flow_ctl_10[0]); i++)
    {
	      num_send = write(fd, rda_flow_ctl_10[i], sizeof(rda_flow_ctl_10[i]));
	      if (num_send != sizeof(rda_flow_ctl_10[i]))
	      {
	          printf("num_send = %d (%d)\n", num_send, sizeof(rda_flow_ctl_10[i]));
	          return -1;
	      }
		
	      usleep(5000);
    }

    usleep(50000);
    return 0;
}

int rda_init(int fd)
{
    int ret = -1;
	
    ret = RDABT_core_Intialization(fd);
    if(ret < 0)
    {
    	ALOGE("[###### TCC BT #######]rda_init:fails: %s(%d)\n",strerror(errno), errno);
        return ret;
    }
    return 0;
}

int hostwake_up_irq(int on)
{
    int sz;
    int fd = -1;
    int ret = -1;
    const uint32_t buf = (on ? 1 : 0);
    
    fd = open(TCC_BT_DEVICE_PATH, O_RDWR);
    
    if (fd < 0) 
    {
        ALOGE("[###### TCC BT #######]Open %s to set BT power fails: %s(%d)", TCC_BT_DEVICE_PATH, strerror(errno), errno);
        goto out;
    }
    
    ret = ioctl(fd, IOCTL_BT_SET_EINT, &buf);
    
    if(ret < 0) 
    {
        ALOGE("[###### TCC BT #######]Set BT power %d fails: %s(%d)\n", buf, strerror(errno), errno);
        goto out;
    }

out:
    if (fd >= 0)
        close(fd);
        
    return ret;
}


/*此函数实现拉高LDO_DON,并延时50ms*/
void RDA_pin_to_high(int fd)
{
    int bt_on_off = 1;
    ioctl(fd, IOCTL_BT_DEV_POWER, &bt_on_off);	
}

/*此函数实现拉低LDO_DON,并延时50ms*/
void RDA_pin_to_low(int fd)
{
    int bt_on_off = 0;
    ioctl(fd, IOCTL_BT_DEV_POWER, &bt_on_off);		
}

void rdabt_write_memory(int fd,__u32 addr,__u32 *data,__u8 len,__u8 memory_type)
{
   __u16 num_to_send;	
   __u16 i,j;
   __u8 data_to_send[256]={0};
   __u32 address_convert;
   
   data_to_send[0] = 0x01;
   data_to_send[1] = 0x02;
   data_to_send[2] = 0xfd;
   data_to_send[3] = (__u8)(len*4+6);
   data_to_send[4] = (memory_type+0x80);  // add the event display
   data_to_send[5] = len;
   
   if(memory_type == 0x01)
   {
      address_convert = addr*4+0x200;
      
      data_to_send[6] = (__u8)address_convert;
      data_to_send[7] = (__u8)(address_convert>>8);
      data_to_send[8] = (__u8)(address_convert>>16);
      data_to_send[9] = (__u8)(address_convert>>24);	  
   }
   else
   {
      data_to_send[6] = (__u8)addr;
      data_to_send[7] = (__u8)(addr>>8);
      data_to_send[8] = (__u8)(addr>>16);
      data_to_send[9] = (__u8)(addr>>24);
   }
   
   for(i=0;i<len;i++,data++)
   {
       j=10+i*4;
       data_to_send[j] =  (__u8)(*data);
       data_to_send[j+1] = (__u8)((*data)>>8);
       data_to_send[j+2] = (__u8)((*data)>>16);
       data_to_send[j+3] = (__u8)((*data)>>24);
   }
   
   num_to_send = 4+data_to_send[3];

   write(fd,&(data_to_send[0]),num_to_send);   	
}



void RDA_uart_write_array(int fd,__u32 buf[][2],__u16 len,__u8 type)
{
   __u32 i;
   
   for(i=0;i<len;i++)
   {
      rdabt_write_memory(fd,buf[i][0],&buf[i][1],1,type);
      
      usleep(12000);//12ms?
   }	
}

__u32 rdabt_rf_init_12[][2] = 
{   
    {0x0000003f,0x00000000},//
    {0x00000001,0x00001FFF},//                                                               
    {0x00000006,0x000007F7},//padrv_set,increase the power.                                  
    {0x00000008,0x000001E7},//                                                               
    {0x00000009,0x00000520},//                                                               
    {0x0000000B,0x000003DF},//filter_cap_tuning<3:0>1101                                     
    {0x0000000C,0x000085E8},//                                                               
    {0x0000000F,0x00000DBC},// 0FH,16'h1D8C; 0FH,16'h1DBC;adc_clk_sel=1 20110314 ;adc_digi_pw
    {0x00000012,0x000007F7},//padrv_set,increase the power.                                  
    {0x00000013,0x00000327},//agpio down pullen .                                            
    {0x00000014,0x00000CCC},//h0CFE; bbdac_cm 00=vdd/2.                                      
    {0x00000015,0x00000526},//Pll_bypass_ontch:1,improve ACPR.                               
    {0x00000016,0x00008918},//add div24 20101126                                             
    {0x00000018,0x00008800},//add div24 20101231                                             
    {0x00000019,0x000010C8},//pll_adcclk_en=1 20101126                                       
    {0x0000001A,0x00009128},//Mdll_adcclk_out_en=0                                           
    {0x0000001B,0x000080C0},//1BH,16'h80C2                                                   
    {0x0000001C,0x00003613},//                                                               
    {0x0000001D,0x000013E3},//Pll_cp_bit_tx<3:0>1110;13D3                                    
    {0x0000001E,0x0000300C},//Pll_lpf_gain_tx<1:0> 00;304C                                   
    {0x00000023,0x00002222},//                                                               
    {0x00000024,0x0000359F},//                                                               
    {0x00000027,0x00000011},//                                                               
    {0x00000028,0x0000124F},//                                                               
    {0x00000039,0x0000A5FC},//      
    {0x0000003f,0x00000001},//                                                        
    {0x00000000,0x0000043F},//agc                                                            
    {0x00000001,0x0000467F},//agc                                                            
    {0x00000002,0x000028FF},//agc//2011032382H,16'h68FF;agc                                  
    {0x00000003,0x000067FF},//agc                                                            
    {0x00000004,0x000057FF},//agc                                                            
    {0x00000005,0x00007BFF},//agc                                                            
    {0x00000006,0x00003FFF},//agc                                                            
    {0x00000007,0x00007FFF},//agc                                                            
    {0x00000018,0x0000F3F5},//                                                               
    {0x00000019,0x0000F3F5},//                                                               
    {0x0000001A,0x0000E7F3},//                                                               
    {0x0000001B,0x0000F1FF},//                                                               
    {0x0000001C,0x0000FFFF},//                                                               
    {0x0000001D,0x0000FFFF},//                                                               
    {0x0000001E,0x0000FFFF},//                                                               
    {0x0000001F,0x0000FFFF},//padrv_gain;9FH,16'hFFEC;padrv_gain20101103;improve ACPR;       
#ifdef EXTERNAL_32K
    {0x00000023,0x00004224},//;ext32k
#endif
    {0x00000024,0x00000110},
    {0x00000025,0x000043E1},//ldo_vbit:110,1.96v                                             
    {0x00000026,0x00004BB5},//reg_ibit:101,reg_vbit:110,1.12v,reg_vbit_deepsleep:110,750mV   
    {0x00000032,0x00000079},//TM mod                                                         
    {0x0000003f,0x00000000},//
};

__u32 RDA5876_ENABLE_SPI[][2] =
{
    {0x40240000,0x2004f39c},                               
};

__u32 RDA5876_DISABLE_SPI[][2] = 
{
	{0x40240000,0x2000f29c},
};

__u32 RDA5876_PSKEY_RF[][2] =
{
    {0x40240000,0x2004f39c},
    {0x800000C0,0x00000021},
    {0x800000C4,0x003F0000},
    {0x800000C8,0x00414003},
    {0x800000CC,0x004225BD},
    {0x800000D0,0x004908E4},
    {0x800000D4,0x0043B074},
    {0x800000D8,0x0044D01A},
    {0x800000DC,0x004A0800},
    {0x800000E0,0x0054A020},
    {0x800000E4,0x0055A020},
    {0x800000E8,0x0056A542},
    {0x800000EC,0x00574C18},
    {0x800000F0,0x003F0001},
    {0x800000F4,0x00410900},
    {0x800000F8,0x0046033F},
    {0x800000FC,0x004C0000},
    {0x80000100,0x004D0015},
    {0x80000104,0x004E002B},
    {0x80000108,0x004F0042},
    {0x8000010C,0x0050005A},
    {0x80000110,0x00510073},
    {0x80000114,0x0052008D},
    {0x80000118,0x005300A7},
    {0x8000011C,0x005400C4},
    {0x80000120,0x005500E3},
    {0x80000124,0x00560103},
    {0x80000128,0x00570127},
    {0x8000012C,0x0058014E},
    {0x80000130,0x00590178},
    {0x80000134,0x005A01A1},
    {0x80000138,0x005B01CE},
    {0x8000013C,0x005C01FF},
    {0x80000140,0x003F0000},
    {0x80000144,0x00000000}, //;         PSKEY: Page 0
    {0x80000040,0x10000000},
};

__u32 RDA5876_DCCAL[][2]=
{
    {0x0000003f,0x00000000},
    {0x00000030,0x00000129},
    {0x00000030,0x0000012B},
};

__u32 RDA5876_PSKEY_MISC[][2] =
{
    {0x800004ec,0xf88dffff}, ///disable edr   
    {0x800004f0,0x83793b98}, ///disable 3m esco ev4 ev5
#if (defined(CUST_BT_SERIAL_BOURATE) && CUST_BT_SERIAL_BOURATE == 1500000)
    {0x80000060,0x0016e360},//baud 1500000
#elif (defined(CUST_BT_SERIAL_BOURATE) && CUST_BT_SERIAL_BOURATE == 921600)
    {0x80000060,0x000e1000},//baud 921600
#else 
    {0x80000060,0x0001c200},//baud 115200
#endif

    {0x80000070,0x00002000},
#ifdef EXTERNAL_32K
    {0x80000074,0xa5025010},
#else
    {0x80000074,0x05025010},//0x05025010 for no sleep ; 0xa5025010 for sleep
#endif


    {0x80000078,0x0f054001},
    {0x8000007c,0xb530b530},
    //{0x800000a0,0000000000},
    //{0x800000a4,0x08a05014},   ////hostwake pull up for 10ms 
    {0x800000a4,0x08a19024},   ////hostwake pull up for 10ms 
    {0x800000a8,0x0Bbaba30},
    // {0x80000040,0x0400f000},//PSKEY: modify flag
    {0x4024002c,0x00b81000},
    {0x80002be4,0x00010a02},
    {0x80000040,0x0600f100},//PSKEY: modify flag
    
};

__u32 RDA5876_TRAP[][2] = 
{
    {0x40180100,0x000068b8},//inc power
	{0x40180120,0x000069f4},
	
	{0x40180104,0x000066b8},//dec power
	{0x40180124,0x000069f4},

	
	{0x40180108,0x0001544c},//esco w
	{0x40180128,0x0001568c},
	
	{0x80000300,0xe3a0700f}, ///2ev3 ev3 hv3
	{0x4018010c,0x0000bae8},//esco packet
	{0x4018012c,0x80000300}, 

  {0x40180114,0x0000f8c4},///all rxon
  {0x40180134,0x00026948},

  {0x40180118,0x000130b8},///qos PRH_CHN_QUALITY_MIN_NUM_PACKETS
  {0x40180138,0x0001cbb4},  
  
  {0x4018011c,0x0000bac8},
  {0x4018013c,0x0000bae4},
  
  {0x80000254,0x1586701c},
  {0x80000258,0x13a00001},
  {0x8000025C,0x15c6002e},
  {0x80000260,0xe3a00cef},
  {0x80000264,0xe280f020},
  {0x80000000,0xea000093},
  {0x40180004,0x0000ef1c},
  {0x40180024,0x00032d14},
  
  {0x80000200,0xe59f5014},//sco sniff
  {0x80000204,0xe5d55000},
  {0x80000208,0xe3550001},
  {0x8000020C,0x03a00000},
  {0x80000210,0x11a00006},
  {0x80000214,0xe3a06bb7},
  {0x80000218,0xe286ffa8},
  {0x8000021c,0x80000989},
  {0x80000004,0xea00007d},
  {0x40180008,0x0002de9c},
  {0x40180028,0x00032d18},
  
  {0x40180010,0x0002d04c},//connection patch   
  {0x40180030,0x00000014},                     
  {0x40180014,0x0000badc},//link               
  {0x40180034,0x0001dc04},                     
  {0x40180018,0x0001a338},//encrpyt            
  {0x40180038,0x0001ac1c},        
  
  {0x80000008,0xea00003c},//EDR 1
  {0x800000f8,0xe51ff004},
  {0x800000fc,0x00021bbc},
  {0x80000100,0xe59f0028},
  {0x80000104,0xebfffffb},
  {0x80000108,0xe3a00000},
  {0x8000010c,0xe3a0e0a2},
  {0x80000110,0xe28ef090},
  {0x4018001c,0x0000a28c},
  {0x4018003c,0x00032d1c},

  {0x8000000C,0xea000043},//EDR 2
  {0x80000118,0xe51ff004},
  {0x8000011c,0x00021bd4},
  {0x80000120,0xe59f0008},
  {0x80000124,0xebfffffb},
  {0x80000128,0xe1a00006},
  {0x8000012c,0xe8bd80f8},
  {0x40180020,0x0000a2d8},
  {0x40180040,0x00032d20},
                      
  {0x80000130,0x80000134},
  {0x80000134,0x00000000},

	{0x40180000,0x0000ffff},
};


void RDA5876_RfInit(int fd)
{
    RDA_uart_write_array(fd,RDA5876_ENABLE_SPI,sizeof(RDA5876_ENABLE_SPI)/sizeof(RDA5876_ENABLE_SPI[0]),0);
    RDA_uart_write_array(fd,rdabt_rf_init_12,sizeof(rdabt_rf_init_12)/sizeof(rdabt_rf_init_12[0]),1);
    usleep(50000);//50ms?
}

void RDA5876_Pskey_RfInit(int fd)
{
    RDA_uart_write_array(fd,RDA5876_PSKEY_RF,sizeof(RDA5876_PSKEY_RF)/sizeof(RDA5876_PSKEY_RF[0]),0);
}

void RDA5876_Dccal(int fd)
{
    RDA_uart_write_array(fd,RDA5876_DCCAL,sizeof(RDA5876_DCCAL)/sizeof(RDA5876_DCCAL[0]),1);
    RDA_uart_write_array(fd,RDA5876_DISABLE_SPI,sizeof(RDA5876_DISABLE_SPI)/sizeof(RDA5876_DISABLE_SPI[0]),0);
}

void RDA5876_Pskey_Misc(int fd)
{
    RDA_uart_write_array(fd,RDA5876_PSKEY_MISC,sizeof(RDA5876_PSKEY_MISC)/sizeof(RDA5876_PSKEY_MISC[0]),0);
}

void RDA5876_Trap(int fd)
{
    RDA_uart_write_array(fd,RDA5876_TRAP,sizeof(RDA5876_TRAP)/sizeof(RDA5876_TRAP[0]),0);
}

int rda_write_bdaddr(int fd)
{
    unsigned int num_send;

    ALOGE("rda_bdaddr: %02X:%02X:%02X:%02X:%02X:%02X",
        bt_addr_cmd[4], bt_addr_cmd[5], bt_addr_cmd[6], bt_addr_cmd[7], bt_addr_cmd[8], bt_addr_cmd[9]);
    num_send = write(fd, bt_addr_cmd, 10);
    if(num_send != sizeof(bt_addr_cmd))
    {
        ALOGE("Failed to write RDA Address \n");
    }
    usleep(5000);

    return 0;
}

int RDABT_core_Intialization(int fd)
{
    int bt_fd = -1;	

	bt_fd = open(TCC_BT_DEVICE_PATH, O_RDWR);
	
	if( bt_fd < 0 ) 
	{
		perror("[###### TCC BT #######] open error");
		return -1;
	}

    RDA_pin_to_high(bt_fd);
    RDA5876_RfInit(fd);
    RDA5876_Pskey_RfInit(fd);

    RDA_pin_to_low(bt_fd);
    RDA_pin_to_high(bt_fd); 
    usleep(50000);

    RDA5876_RfInit(fd);   
    RDA5876_Pskey_RfInit(fd);

    RDA5876_Dccal(fd);	
    if(rda_bluetooth_incall_set(fd) < 0)
    {
        ALOGE("[###### TCC BT #######]rda_bluetooth_incall_set: %s(%d)\n",strerror(errno), errno);
    }
    RDA5876_Trap(fd);       

    usleep(50000);
    rda_write_bdaddr(fd);       
    RDA5876_Pskey_Misc(fd);  
    if(bt_fd>0)
        close(bt_fd);

    return 0;
}



