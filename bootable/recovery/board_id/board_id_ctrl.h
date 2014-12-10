/*
 * board_id_ctrl.h
 *
 *  Created on: 2013-4-27
 *      Author: mmk
 */

#ifndef BOARD_ID_CTRL_H_
#define BOARD_ID_CTRL_H_
#include <sys/ioctl.h>


enum type_devices{
	DEVICE_TYPE_NULL = 0x0,	

	DEVICE_TYPE_SUM = 0x20,	
	DEVICE_TYPE_AREA = 0x24,	//
	DEVICE_TYPE_OPERATOR = 0x25,	
	DEVICE_TYPE_OPERATOR2 = 0x26,
	DEVICE_TYPE_RESERVE = 0x27,	
	DEVICE_TYPE_STATUS = 0x28,

	DEVICE_TYPE_TP = 0x29,		//one byte size
	DEVICE_TYPE_LCD,	
	DEVICE_TYPE_KEY,	
	DEVICE_TYPE_CODEC,
	DEVICE_TYPE_WIFI,
	DEVICE_TYPE_BT,	
	DEVICE_TYPE_GPS,	
	DEVICE_TYPE_FM,	
	DEVICE_TYPE_MODEM,	
	DEVICE_TYPE_DDR,
	DEVICE_TYPE_FLASH,
	DEVICE_TYPE_HDMI,
	DEVICE_TYPE_BATTERY,
	DEVICE_TYPE_CHARGE,	
	DEVICE_TYPE_BACKLIGHT,
	DEVICE_TYPE_HEADSET,
	DEVICE_TYPE_MICPHONE,
	DEVICE_TYPE_SPEAKER,
	DEVICE_TYPE_VIBRATOR,
	DEVICE_TYPE_TV,	
	DEVICE_TYPE_ECHIP,	//30
	DEVICE_TYPE_HUB,
	DEVICE_TYPE_TPAD,
	
	DEVICE_TYPE_PMIC,
	DEVICE_TYPE_REGULATOR,
	DEVICE_TYPE_RTC,
	DEVICE_TYPE_CAMERA_FRONT,
	DEVICE_TYPE_CAMERA_BACK,	//35
	DEVICE_TYPE_ANGLE,
	DEVICE_TYPE_ACCEL,
	DEVICE_TYPE_COMPASS,
	DEVICE_TYPE_GYRO,
	DEVICE_TYPE_LIGHT,
	DEVICE_TYPE_PROXIMITY,
	DEVICE_TYPE_TEMPERATURE,	
	DEVICE_TYPE_PRESSURE,
	
	DEVICE_NUM_TYPES,
};


enum id_language{
	LANGUAGE_ID_NULL,
	LANGUAGE_ID_EN,// Ӣ��
	LANGUAGE_ID_EN_US,// Ӣ�� (����)
	LANGUAGE_ID_AR,// ��������
	LANGUAGE_ID_AR_AE,// �������� (����������������)
	LANGUAGE_ID_AR_BH,// �������� (����)
	LANGUAGE_ID_AR_DZ,// �������� (����������)
	LANGUAGE_ID_AR_EG,// �������� (����)
	LANGUAGE_ID_AR_IQ,// �������� (������)
	LANGUAGE_ID_AR_JO,// �������� (Լ��)
	LANGUAGE_ID_AR_KW,// �������� (������)
	LANGUAGE_ID_AR_LB,// �������� (�����)
	LANGUAGE_ID_AR_LY,// �������� (������)
	LANGUAGE_ID_AR_MA,// �������� (Ħ���)
	LANGUAGE_ID_AR_OM,// �������� (����)
	LANGUAGE_ID_AR_QA,// �������� (������)
	LANGUAGE_ID_AR_SA,// �������� (ɳ�ذ�����)
	LANGUAGE_ID_AR_SD,// �������� (�յ�)
	LANGUAGE_ID_AR_SY,// �������� (������)
	LANGUAGE_ID_AR_TN,// �������� (ͻ��˹)
	LANGUAGE_ID_AR_YE,// �������� (Ҳ��)
	LANGUAGE_ID_BE,// �׶���˹��
	LANGUAGE_ID_BE_BY,// �׶���˹�� (�׶���˹)
	LANGUAGE_ID_BG,// ����������
	LANGUAGE_ID_BG_BG,// ���������� (��������)
	LANGUAGE_ID_CA,// ��̩��������
	LANGUAGE_ID_CA_ES,// ��̩�������� (������)
	LANGUAGE_ID_CA_ES_EURO,// ��̩�������� (������,EURO)
	LANGUAGE_ID_CS,// �ݿ���
	LANGUAGE_ID_CS_CZ,// �ݿ��� (�ݿ˹��͹�)
	LANGUAGE_ID_DA,// ������
	LANGUAGE_ID_DA_DK,// ������ (����)
	LANGUAGE_ID_DE,// ����
	LANGUAGE_ID_DE_AT,// ���� (�µ���)
	LANGUAGE_ID_DE_AT_EURO,// ���� (�µ���,EURO)
	LANGUAGE_ID_DE_CH,// ���� (��ʿ)
	LANGUAGE_ID_DE_DE,// ���� (�¹�)
	LANGUAGE_ID_DE_DE_EURO,// ���� (�¹�,EURO)
	LANGUAGE_ID_DE_LU,// ���� (¬ɭ��)
	LANGUAGE_ID_DE_LU_EURO,// ���� (¬ɭ��,EURO)
	LANGUAGE_ID_EL,// ϣ����
	LANGUAGE_ID_EL_GR,// ϣ���� (ϣ��)
	LANGUAGE_ID_EN_AU,// Ӣ�� (�Ĵ�����)
	LANGUAGE_ID_EN_CA,// Ӣ�� (���ô�)
	LANGUAGE_ID_EN_GB,// Ӣ�� (Ӣ��)
	LANGUAGE_ID_EN_IE,// Ӣ�� (������)
	LANGUAGE_ID_EN_IE_EURO,// Ӣ�� (������,EURO)
	LANGUAGE_ID_EN_NZ,// Ӣ�� (������)
	LANGUAGE_ID_EN_ZA,// Ӣ�� (�Ϸ�)
	LANGUAGE_ID_ES,// ��������
	LANGUAGE_ID_ES_BO,// �������� (����ά��)
	LANGUAGE_ID_ES_AR,// �������� (����͢)
	LANGUAGE_ID_ES_CL,// �������� (����)
	LANGUAGE_ID_ES_CO,// �������� (���ױ���)
	LANGUAGE_ID_ES_CR,// �������� (��˹�����)
	LANGUAGE_ID_ES_DO,// �������� (������ӹ��͹�)
	LANGUAGE_ID_ES_EC,// �������� (��϶��)
	LANGUAGE_ID_ES_ES,// �������� (������)
	LANGUAGE_ID_ES_ES_EURO,// �������� (������,EURO)
	LANGUAGE_ID_ES_GT,// �������� (Σ������)
	LANGUAGE_ID_ES_HN,// �������� (�鶼��˹)
	LANGUAGE_ID_ES_MX,// �������� (ī����)
	LANGUAGE_ID_ES_NI,// �������� (�������)
	LANGUAGE_ID_ET,// ��ɳ������
	LANGUAGE_ID_ES_PA,// �������� (������)
	LANGUAGE_ID_ES_PE,// �������� (��³)
	LANGUAGE_ID_ES_PR,// �������� (�������)
	LANGUAGE_ID_ES_PY,// �������� (������)
	LANGUAGE_ID_ES_SV,// �������� (�����߶�)
	LANGUAGE_ID_ES_UY,// �������� (������)
	LANGUAGE_ID_ES_VE,// �������� (ί������)
	LANGUAGE_ID_ET_EE,// ��ɳ������ (��ɳ����)
	LANGUAGE_ID_FI,// ������
	LANGUAGE_ID_FI_FI,// ������ (����)
	LANGUAGE_ID_FI_FI_EURO,// ������ (����,EURO)
	LANGUAGE_ID_FR,// ����
	LANGUAGE_ID_FR_BE,// ���� (����ʱ)
	LANGUAGE_ID_FR_BE_EURO,// ���� (����ʱ,EURO)
	LANGUAGE_ID_FR_CA,// ���� (���ô�)
	LANGUAGE_ID_FR_CH,// ���� (��ʿ)
	LANGUAGE_ID_FR_FR,// ���� (����)
	LANGUAGE_ID_FR_FR_EURO,// ���� (����,EURO)
	LANGUAGE_ID_FR_LU,// ���� (¬ɭ��)
	LANGUAGE_ID_FR_LU_EURO,// ���� (¬ɭ��,EURO)
	LANGUAGE_ID_HR,// ���޵�����
	LANGUAGE_ID_HR_HR,// ���޵����� (���޵���)
	LANGUAGE_ID_HU,// ��������
	LANGUAGE_ID_HU_HU,// �������� (������)
	LANGUAGE_ID_IS,// ������
	LANGUAGE_ID_IS_IS,// ������ (����)
	LANGUAGE_ID_IT,// �������
	LANGUAGE_ID_IT_CH,// ������� (��ʿ)
	LANGUAGE_ID_IT_IT,// ������� (�����)
	LANGUAGE_ID_IT_IT_EURO,// ������� (�����,EURO)
	LANGUAGE_ID_IW,// ϣ������
	LANGUAGE_ID_IW_IL,// ϣ������ (��ɫ��)
	LANGUAGE_ID_JA,// ����
	LANGUAGE_ID_JA_JP,// ���� (�ձ�)
	LANGUAGE_ID_KO,// ������
	LANGUAGE_ID_KO_KR,// ������ (�ϳ���)
	LANGUAGE_ID_LT,// ��������
	LANGUAGE_ID_LT_LT,// �������� (������)
	LANGUAGE_ID_LV,// ����ά����(����)
	LANGUAGE_ID_LV_LV,// ����ά����(����) (����ά��)
	LANGUAGE_ID_MK,// �������
	LANGUAGE_ID_MK_MK,// ������� (���������)
	LANGUAGE_ID_NL,// ������
	LANGUAGE_ID_NL_BE,// ������ (����ʱ)
	LANGUAGE_ID_NL_BE_EURO,// ������ (����ʱ,EURO)
	LANGUAGE_ID_NL_NL,// ������ (����)
	LANGUAGE_ID_NL_NL_EURO,// ������ (����,EURO)
	LANGUAGE_ID_NO,// Ų����
	LANGUAGE_ID_NO_NO,// Ų���� (Ų��)
	LANGUAGE_ID_NO_NO_NY,// Ų���� (Ų��,NYNORSK)
	LANGUAGE_ID_PL,// ������
	LANGUAGE_ID_PL_PL,// ������ (����)
	LANGUAGE_ID_PT,// ��������
	LANGUAGE_ID_PT_BR,// �������� (����)
	LANGUAGE_ID_PT_PT,// �������� (������)
	LANGUAGE_ID_PT_PT_EURO,// �������� (������,EURO)
	LANGUAGE_ID_RO,// ����������
	LANGUAGE_ID_RO_RO,// ���������� (��������)
	LANGUAGE_ID_RU,// ����
	LANGUAGE_ID_RU_RU,// ���� (����˹)
	LANGUAGE_ID_SH,// ������˹-���޵�����
	LANGUAGE_ID_SH_YU,// ������˹-���޵����� (��˹����)
	LANGUAGE_ID_SK,// ˹�工����
	LANGUAGE_ID_SK_SK,// ˹�工���� (˹�工��)
	LANGUAGE_ID_SL,// ˹����������
	LANGUAGE_ID_SL_SI,// ˹���������� (˹��������)
	LANGUAGE_ID_SQ,// ������������
	LANGUAGE_ID_SQ_AL,// ������������ (����������)
	LANGUAGE_ID_SR,// ����ά����
	LANGUAGE_ID_SR_YU,// ����ά���� (��˹����)
	LANGUAGE_ID_SV,// �����
	LANGUAGE_ID_SV_SE,// ����� (���)
	LANGUAGE_ID_TH,// ̩��
	LANGUAGE_ID_TH_TH,// ̩�� (̩��)
	LANGUAGE_ID_TR,// ��������
	LANGUAGE_ID_TR_TR,// �������� (������)
	LANGUAGE_ID_UK,// �ڿ�����
	LANGUAGE_ID_UK_UA,// �ڿ����� (�ڿ���)
	LANGUAGE_ID_ZH,// ����
	LANGUAGE_ID_ZH_CN,// ���� (�й�)
	LANGUAGE_ID_ZH_HK,// ���� (���)
	LANGUAGE_ID_ZH_TW,// ���� (̨��)
	LANGUAGE_ID_NUMS,
};

struct area_id_name{
	int type;
	int id;
	char country_area[32];		//country or area name such as china
	char locale_language[4];	//locale language name such as zh
	char locale_region[8];		//locale region name such as CN
	char country_geo[20];		//country geographical position such as asia		
	char timezone[32];		//time zone such as Asia/Shanghai
	char user_define[20];		//user-defined name such as A10,A12,A13 
};


struct operator_id_name{
	int type;		//type
	int id;	
	char operator_name[20];	//operator name such as CHINA MOBILE
	char locale_region[8];	//area name such as CN
};

struct reserve_id_name{
	int type;		//type
	int id;			
	char reserve_name[20];	//reserve name	
	char locale_region[20];	
};


struct device_id_name{
	char type;	//device type
	char id;	//board id
	char type_name[14];
	char driver_name[16];
	char dev_name[16];	//name
	char description[30];	// description
	unsigned short device_id;//device_id and only one
	//short select;	// 1:device is selected 0:not
};



#define BOARD_ID_IOCTL_BASE 'b'

//#define BOARD_ID_IOCTL_READ_ALL 			_IOWR(BOARD_ID_IOCTL_BASE, 0x00, struct board_id_private_data)
//#define BOARD_ID_IOCTL_WRITE_ALL 			_IOWR(BOARD_ID_IOCTL_BASE, 0x30, struct board_id_private_data)


#define BOARD_ID_IOCTL_READ_AREA_ID 			_IOR(BOARD_ID_IOCTL_BASE, 0x80, struct area_id_name)
#define BOARD_ID_IOCTL_READ_OPERATOR_ID 		_IOR(BOARD_ID_IOCTL_BASE, 0x81, struct operator_id_name)
#define BOARD_ID_IOCTL_READ_RESERVE_ID 			_IOR(BOARD_ID_IOCTL_BASE, 0x82, struct reserve_id_name)

#define BOARD_ID_IOCTL_READ_AREA_NAME_BY_ID 		_IOWR(BOARD_ID_IOCTL_BASE, 0x70, struct area_id_name)
#define BOARD_ID_IOCTL_READ_OPERATOR_NAME_BY_ID 	_IOWR(BOARD_ID_IOCTL_BASE, 0x71, struct operator_id_name)
#define BOARD_ID_IOCTL_READ_RESERVE_NAME_BY_ID 		_IOWR(BOARD_ID_IOCTL_BASE, 0x72, struct reserve_id_name)
#define BOARD_ID_IOCTL_READ_DEVICE_NAME_BY_ID 		_IOWR(BOARD_ID_IOCTL_BASE, 0x73, struct device_id_name)



#define BOARD_ID_IOCTL_READ_TP_ID 			_IOR(BOARD_ID_IOCTL_BASE, 0x01, struct device_id_name)
#define BOARD_ID_IOCTL_READ_LCD_ID 			_IOR(BOARD_ID_IOCTL_BASE, 0x02, struct device_id_name)
#define BOARD_ID_IOCTL_READ_KEY_ID 			_IOR(BOARD_ID_IOCTL_BASE, 0x03, struct device_id_name)
#define BOARD_ID_IOCTL_READ_CODEC_ID 			_IOR(BOARD_ID_IOCTL_BASE, 0x04, struct device_id_name)
#define BOARD_ID_IOCTL_READ_WIFI_ID 			_IOR(BOARD_ID_IOCTL_BASE, 0x05, struct device_id_name)
#define BOARD_ID_IOCTL_READ_BT_ID 			_IOR(BOARD_ID_IOCTL_BASE, 0x06, struct device_id_name)	
#define BOARD_ID_IOCTL_READ_GPS_ID 			_IOR(BOARD_ID_IOCTL_BASE, 0x07, struct device_id_name)
#define BOARD_ID_IOCTL_READ_FM_ID 			_IOR(BOARD_ID_IOCTL_BASE, 0x08, struct device_id_name)
#define BOARD_ID_IOCTL_READ_MODEM_ID 			_IOR(BOARD_ID_IOCTL_BASE, 0x09, struct device_id_name)	
#define BOARD_ID_IOCTL_READ_DDR_ID 			_IOR(BOARD_ID_IOCTL_BASE, 0x0a, struct device_id_name)
#define BOARD_ID_IOCTL_READ_FLASH_ID 			_IOR(BOARD_ID_IOCTL_BASE, 0x0b, struct device_id_name)
#define BOARD_ID_IOCTL_READ_HDMI_ID 			_IOR(BOARD_ID_IOCTL_BASE, 0x0c, struct device_id_name)
#define BOARD_ID_IOCTL_READ_BATTERY_ID 			_IOR(BOARD_ID_IOCTL_BASE, 0x0d, struct device_id_name)
#define BOARD_ID_IOCTL_READ_CHARGE_ID 			_IOR(BOARD_ID_IOCTL_BASE, 0x0e, struct device_id_name)	
#define BOARD_ID_IOCTL_READ_BACKLIGHT_ID 		_IOR(BOARD_ID_IOCTL_BASE, 0x0f, struct device_id_name)
#define BOARD_ID_IOCTL_READ_HEADSET_ID 			_IOR(BOARD_ID_IOCTL_BASE, 0x10, struct device_id_name)
#define BOARD_ID_IOCTL_READ_MICPHONE_ID 		_IOR(BOARD_ID_IOCTL_BASE, 0x11, struct device_id_name)
#define BOARD_ID_IOCTL_READ_SPEAKER_ID 			_IOR(BOARD_ID_IOCTL_BASE, 0x12, struct device_id_name)
#define BOARD_ID_IOCTL_READ_VIBRATOR_ID 		_IOR(BOARD_ID_IOCTL_BASE, 0x13, struct device_id_name)
#define BOARD_ID_IOCTL_READ_TV_ID 			_IOR(BOARD_ID_IOCTL_BASE, 0x14, struct device_id_name)
#define BOARD_ID_IOCTL_READ_ECHIP_ID 			_IOR(BOARD_ID_IOCTL_BASE, 0x15, struct device_id_name)		
#define BOARD_ID_IOCTL_READ_HUB_ID 			_IOR(BOARD_ID_IOCTL_BASE, 0x16, struct device_id_name)	
#define BOARD_ID_IOCTL_READ_TPAD_ID 			_IOR(BOARD_ID_IOCTL_BASE, 0x17, struct device_id_name)	


#define BOARD_ID_IOCTL_READ_PMIC_ID 			_IOR(BOARD_ID_IOCTL_BASE, 0x20, struct device_id_name)
#define BOARD_ID_IOCTL_READ_REGULATOR_ID 		_IOR(BOARD_ID_IOCTL_BASE, 0x21, struct device_id_name)
#define BOARD_ID_IOCTL_READ_RTC_ID 			_IOR(BOARD_ID_IOCTL_BASE, 0x22, struct device_id_name)
#define BOARD_ID_IOCTL_READ_CAMERA_FRONT_ID 		_IOR(BOARD_ID_IOCTL_BASE, 0x23, struct device_id_name)
#define BOARD_ID_IOCTL_READ_CAMERA_BACK_ID 		_IOR(BOARD_ID_IOCTL_BASE, 0x24, struct device_id_name)	
#define BOARD_ID_IOCTL_READ_SENSOR_ANGLE_ID 		_IOR(BOARD_ID_IOCTL_BASE, 0x25, struct device_id_name)
#define BOARD_ID_IOCTL_READ_SENSOR_ACCEL_ID 		_IOR(BOARD_ID_IOCTL_BASE, 0x26, struct device_id_name)
#define BOARD_ID_IOCTL_READ_SENSOR_COMPASS_ID 		_IOR(BOARD_ID_IOCTL_BASE, 0x27, struct device_id_name)
#define BOARD_ID_IOCTL_READ_SENSOR_GYRO_ID 		_IOR(BOARD_ID_IOCTL_BASE, 0x28, struct device_id_name)
#define BOARD_ID_IOCTL_READ_SENSOR_LIGHT_ID 		_IOR(BOARD_ID_IOCTL_BASE, 0x29, struct device_id_name)
#define BOARD_ID_IOCTL_READ_SENSOR_PROXIMITY_ID 	_IOR(BOARD_ID_IOCTL_BASE, 0x2A, struct device_id_name)
#define BOARD_ID_IOCTL_READ_SENSOR_TEMPERATURE_ID 	_IOR(BOARD_ID_IOCTL_BASE, 0x2B, struct device_id_name)	
#define BOARD_ID_IOCTL_READ_SENSOR_PRESSURE_ID 		_IOR(BOARD_ID_IOCTL_BASE, 0x2C, struct device_id_name)


#define BOARD_ID_IOCTL_WRITE_AREA_ID 		_IOW(BOARD_ID_IOCTL_BASE, 0x90, struct area_id_name)
#define BOARD_ID_IOCTL_WRITE_OPERATOR_ID 		_IOW(BOARD_ID_IOCTL_BASE, 0x91, struct operator_id_name)
#define BOARD_ID_IOCTL_WRITE_RESERVE_ID 		_IOW(BOARD_ID_IOCTL_BASE, 0x92, struct reserve_id_name)


#define BOARD_ID_IOCTL_WRITE_TP_ID 			_IOW(BOARD_ID_IOCTL_BASE, 0x31, struct device_id_name)
#define BOARD_ID_IOCTL_WRITE_LCD_ID 			_IOW(BOARD_ID_IOCTL_BASE, 0x32, struct device_id_name)
#define BOARD_ID_IOCTL_WRITE_KEY_ID 			_IOW(BOARD_ID_IOCTL_BASE, 0x33, struct device_id_name)
#define BOARD_ID_IOCTL_WRITE_CODEC_ID 			_IOW(BOARD_ID_IOCTL_BASE, 0x34, struct device_id_name)
#define BOARD_ID_IOCTL_WRITE_WIFI_ID 			_IOW(BOARD_ID_IOCTL_BASE, 0x35, struct device_id_name)
#define BOARD_ID_IOCTL_WRITE_BT_ID 			_IOW(BOARD_ID_IOCTL_BASE, 0x36, struct device_id_name)	
#define BOARD_ID_IOCTL_WRITE_GPS_ID 			_IOW(BOARD_ID_IOCTL_BASE, 0x37, struct device_id_name)
#define BOARD_ID_IOCTL_WRITE_FM_ID 			_IOW(BOARD_ID_IOCTL_BASE, 0x38, struct device_id_name)
#define BOARD_ID_IOCTL_WRITE_MODEM_ID 			_IOW(BOARD_ID_IOCTL_BASE, 0x39, struct device_id_name)	
#define BOARD_ID_IOCTL_WRITE_DDR_ID 			_IOW(BOARD_ID_IOCTL_BASE, 0x3a, struct device_id_name)
#define BOARD_ID_IOCTL_WRITE_FLASH_ID 			_IOW(BOARD_ID_IOCTL_BASE, 0x3b, struct device_id_name)
#define BOARD_ID_IOCTL_WRITE_HDMI_ID 			_IOW(BOARD_ID_IOCTL_BASE, 0x3c, struct device_id_name)
#define BOARD_ID_IOCTL_WRITE_BATTERY_ID 		_IOW(BOARD_ID_IOCTL_BASE, 0x3d, struct device_id_name)
#define BOARD_ID_IOCTL_WRITE_CHARGE_ID 			_IOW(BOARD_ID_IOCTL_BASE, 0x3e, struct device_id_name)	
#define BOARD_ID_IOCTL_WRITE_BACKLIGHT_ID 		_IOW(BOARD_ID_IOCTL_BASE, 0x3f, struct device_id_name)
#define BOARD_ID_IOCTL_WRITE_HEADSET_ID 		_IOW(BOARD_ID_IOCTL_BASE, 0x40, struct device_id_name)
#define BOARD_ID_IOCTL_WRITE_MICPHONE_ID 		_IOW(BOARD_ID_IOCTL_BASE, 0x41, struct device_id_name)
#define BOARD_ID_IOCTL_WRITE_SPEAKER_ID 		_IOW(BOARD_ID_IOCTL_BASE, 0x42, struct device_id_name)
#define BOARD_ID_IOCTL_WRITE_VIBRATOR_ID 		_IOW(BOARD_ID_IOCTL_BASE, 0x43, struct device_id_name)
#define BOARD_ID_IOCTL_WRITE_TV_ID 			_IOW(BOARD_ID_IOCTL_BASE, 0x44, struct device_id_name)
#define BOARD_ID_IOCTL_WRITE_ECHIP_ID 			_IOW(BOARD_ID_IOCTL_BASE, 0x45, struct device_id_name)		
#define BOARD_ID_IOCTL_WRITE_HUB_ID 			_IOW(BOARD_ID_IOCTL_BASE, 0x46, struct device_id_name)		
#define BOARD_ID_IOCTL_WRITE_TPAD_ID 			_IOW(BOARD_ID_IOCTL_BASE, 0x47, struct device_id_name)		


#define BOARD_ID_IOCTL_WRITE_PMIC_ID 			_IOW(BOARD_ID_IOCTL_BASE, 0x50, struct device_id_name)
#define BOARD_ID_IOCTL_WRITE_REGULATOR_ID 		_IOW(BOARD_ID_IOCTL_BASE, 0x51, struct device_id_name)
#define BOARD_ID_IOCTL_WRITE_RTC_ID 			_IOW(BOARD_ID_IOCTL_BASE, 0x52, struct device_id_name)
#define BOARD_ID_IOCTL_WRITE_CAMERA_FRONT_ID 		_IOW(BOARD_ID_IOCTL_BASE, 0x53, struct device_id_name)
#define BOARD_ID_IOCTL_WRITE_CAMERA_BACK_ID 		_IOW(BOARD_ID_IOCTL_BASE, 0x54, struct device_id_name)	
#define BOARD_ID_IOCTL_WRITE_SENSOR_ANGLE_ID 		_IOW(BOARD_ID_IOCTL_BASE, 0x55, struct device_id_name)
#define BOARD_ID_IOCTL_WRITE_SENSOR_ACCEL_ID 		_IOW(BOARD_ID_IOCTL_BASE, 0x56, struct device_id_name)
#define BOARD_ID_IOCTL_WRITE_SENSOR_COMPASS_ID 		_IOW(BOARD_ID_IOCTL_BASE, 0x57, struct device_id_name)
#define BOARD_ID_IOCTL_WRITE_SENSOR_GYRO_ID 		_IOW(BOARD_ID_IOCTL_BASE, 0x58, struct device_id_name)
#define BOARD_ID_IOCTL_WRITE_SENSOR_LIGHT_ID 		_IOW(BOARD_ID_IOCTL_BASE, 0x59, struct device_id_name)
#define BOARD_ID_IOCTL_WRITE_SENSOR_PROXIMITY_ID 	_IOW(BOARD_ID_IOCTL_BASE, 0x5A, struct device_id_name)
#define BOARD_ID_IOCTL_WRITE_SENSOR_TEMPERATURE_ID 	_IOW(BOARD_ID_IOCTL_BASE, 0x5B, struct device_id_name)	
#define BOARD_ID_IOCTL_WRITE_SENSOR_PRESSURE_ID 	_IOW(BOARD_ID_IOCTL_BASE, 0x5C, struct device_id_name)

#define BOARD_ID_IOCTL_WRITE_AREA_FLASH 		_IOW(BOARD_ID_IOCTL_BASE, 0x60, struct area_id_name)
#define BOARD_ID_IOCTL_WRITE_DEVICE_FLASH 		_IOW(BOARD_ID_IOCTL_BASE, 0x61, struct device_id_name)
#define BOARD_ID_IOCTL_READ_STATUS 			_IOR(BOARD_ID_IOCTL_BASE, 0x62,	char)
#define BOARD_ID_IOCTL_READ_VENDOR_DATA 		_IOR(BOARD_ID_IOCTL_BASE, 0x63,	char[DEVICE_NUM_TYPES])


int board_id_open_device(void);
int board_id_close_device(void);

int board_id_get_locale_region(enum type_devices type, char *country_area, char *locale_language, char *locale_region, char *country_geo,  char *timezone, char *user_define);
int board_id_get_operator_name(enum type_devices type, char *locale_region, char *operator_name);
int board_id_get_reserve_name(enum type_devices type, char *locale_region, char *reserve_name);
int board_id_get_device_name(enum type_devices type, char *type_name, char *dev_name);

int board_id_get_locale_region_by_id(enum type_devices type, char *id, char *country_area, char *locale_language, char *locale_region, char *country_geo,  char *timezone, char *user_define);
int board_id_get_operator_name_by_id(enum type_devices type, char *id, char *locale_region, char *operator_name);
int board_id_get_reserve_name_by_id(enum type_devices type, char *id, char *locale_region, char *reserve_name);
int board_id_get_device_name_by_id(enum type_devices type, char *id, char *type_name, char *dev_name);
int board_id_get(char *id);
#endif /* BOARD_ID_CTRL_H_ */
