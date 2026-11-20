/*
 * Copyright (c) 2014-2021 Rtrobot. <admin@rtrobot.org>
 *  <http://rtrobot.org>
 ***********************************************************************
 */
#ifndef AS7343_H
#define AS7343_H
// http://rtrobot.org

#include <Arduino.h>

#define AS7343_I2CADDR 0x39

#define AS7343_REG_AUXID 0x58
#define AS7343_REG_REVID 0x59
#define AS7343_REG_ID 0x5a
#define AS7343_REG_CFG10 0x65
#define AS7343_REG_CFG12 0x66
#define AS7343_REG_GPIO 0x6b
#define AS7343_REG_ENABLE 0x80
#define AS7343_REG_ATIME 0x81
#define AS7343_REG_WTIME 0x83
#define AS7343_REG_SP_TH_L_L 0x84
#define AS7343_REG_SP_TH_L_H 0x85
#define AS7343_REG_SP_TH_H_L 0x86
#define AS7343_REG_SP_TH_H_H 0x87
#define AS7343_REG_STATUS2 0x90
#define AS7343_REG_STATUS3 0x91
#define AS7343_REG_STATUS 0x93
#define AS7343_REG_ASTATUS 0x94
#define AS7343_REG_CH0_DATA_L 0x95
#define AS7343_REG_CH0_DATA_H 0x96
#define AS7343_REG_CH1_DATA_L 0x97
#define AS7343_REG_CH1_DATA_H 0x98
#define AS7343_REG_CH2_DATA_L 0x99
#define AS7343_REG_CH2_DATA_H 0x9A
#define AS7343_REG_CH3_DATA_L 0x9B
#define AS7343_REG_CH3_DATA_H 0x9C
#define AS7343_REG_CH4_DATA_L 0x9D
#define AS7343_REG_CH4_DATA_H 0x9E
#define AS7343_REG_CH5_DATA_L 0x9F
#define AS7343_REG_CH5_DATA_H 0xA0
#define AS7343_REG_CH6_DATA_L 0xA1
#define AS7343_REG_CH6_DATA_H 0xA2
#define AS7343_REG_CH7_DATA_L 0xA3
#define AS7343_REG_CH7_DATA_H 0xA4
#define AS7343_REG_CH8_DATA_L 0xA5
#define AS7343_REG_CH8_DATA_H 0xA6
#define AS7343_REG_CH9_DATA_L 0xA7
#define AS7343_REG_CH9_DATA_H 0xA8
#define AS7343_REG_CH10_DATA_L 0xA9
#define AS7343_REG_CH10_DATA_H 0xAA
#define AS7343_REG_CH11_DATA_L 0xAB
#define AS7343_REG_CH11_DATA_H 0xAC
#define AS7343_REG_CH12_DATA_L 0xAD
#define AS7343_REG_CH12_DATA_H 0xAE
#define AS7343_REG_CH13_DATA_L 0xAF
#define AS7343_REG_CH13_DATA_H 0xB0
#define AS7343_REG_CH14_DATA_L 0xB1
#define AS7343_REG_CH14_DATA_H 0xB2
#define AS7343_REG_CH15_DATA_L 0xB3
#define AS7343_REG_CH15_DATA_H 0xB4
#define AS7343_REG_CH16_DATA_L 0xB5
#define AS7343_REG_CH16_DATA_H 0xB6
#define AS7343_REG_CH17_DATA_L 0xB7
#define AS7343_REG_CH17_DATA_H 0xB8
#define AS7343_REG_STATUS5 0xbb
#define AS7343_REG_STATUS4 0xbc
#define AS7343_REG_CFG0 0xbf
#define AS7343_REG_CFG1 0xc6
#define AS7343_REG_CFG3 0xc7
#define AS7343_REG_CFG8 0xc9
#define AS7343_REG_CFG9 0xca
#define AS7343_REG_PERS 0xcf
#define AS7343_REG_STATUS5 0xbb
#define AS7343_REG_STATUS4 0xbc
#define AS7343_REG_CFG0 0xbf
#define AS7343_REG_CFG1 0xc6
#define AS7343_REG_LED 0xcd
#define AS7343_REG_ASTEP_L 0xd4
#define AS7343_REG_ASTEP_H 0xd5
#define AS7343_REG_CFG20 0xd6
#define AS7343_REG_AGC_GAIN_MAX 0xd7
#define AS7343_REG_AZ_CONFIG 0xde
#define AS7343_REG_FD_CFG0 0xdf
#define AS7343_REG_FD_TIME_1 0xe0
#define AS7343_REG_FD_TIME_2 0xe2
#define AS7343_REG_FD_STATUS 0xe3
#define AS7343_REG_CFG6 0xf5
#define AS7343_REG_INTENAB 0xF9
#define AS7343_REG_CONTROL 0xFA
#define AS7343_REG_FIFO_MAP 0xFC
#define AS7343_REG_LVL 0XFD
#define AS7343_REG_FDATA_L 0XFE
#define AS7343_REG_FDATA_H 0XFF

#define AS7343_SMUX_INIT 0x00
#define AS7343_SMUX_READ 0x08
#define AS7343_SMUX_WRITE 0x10

#define AS7343_GAIN_0_5 0x00
#define AS7343_GAIN_1 0x01
#define AS7343_GAIN_2 0x02
#define AS7343_GAIN_4 0x03
#define AS7343_GAIN_8 0x04
#define AS7343_GAIN_16 0x05
#define AS7343_GAIN_32 0x06
#define AS7343_GAIN_64 0x07
#define AS7343_GAIN_128 0x08
#define AS7343_GAIN_256 0x09
#define AS7343_GAIN_512 0x0A
#define AS7343_GAIN_1024 0x0B
#define AS7343_GAIN_2048 0x0C

class AS7343
{
public:
	AS7343();
	boolean begin(void);
	bool Get_Id(void);
	void SetEnable(bool status);
	void SpectralMeasurement(bool status);
	void ControlLed(bool status, uint8_t value);
	bool WaitDataReady(void);
	void FDConfig(void);
	void ReadAllChannels(uint16_t *readings_buffer);
	uint8_t ReadFlickerData(void);
	void F1F4_Clear_NIR(void);
	void F5F8_Clear_NIR(void);
	void XL_FZ_FY_FD_EXT(void);
	void SetEnableSMUX(void);
	void SetSMUX(uint8_t value);
	void SetBank(bool status);
	void EnableFlickerDetection(bool status);
	void SetIntegrationTime(uint8_t atime, uint16_t astep);
	void SetGain(uint8_t value);
	void SetLowPower(bool status);

private:
	uint8_t _i2caddr;
	boolean readCommand(uint8_t reg, uint8_t *redata, uint8_t redata_length);
	void writeCommand(uint8_t reg, uint8_t *cmd, uint8_t length);
	void writeByte(uint8_t reg, uint8_t cmd);
};
#endif