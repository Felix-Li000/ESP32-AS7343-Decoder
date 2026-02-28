/*
 * Copyright (c) 2014-2021 Rtrobot. <admin@rtrobot.org>
 *  <http://rtrobot.org>
 ***********************************************************************
 */
#include  "Arduino.h"
#include <Wire.h>

#include "AS7343.h"
// http://rtrobot.org

AS7343::AS7343()
{
}

boolean AS7343::begin(void)
{
    _i2caddr = AS7343_I2CADDR;
    Wire.begin();
//    Wire.beginTransmission(_i2caddr);
    SetBank(true);
    // Get AS7343 ID
    if (!Get_Id())
        return false;

    // enable as7343
    SetEnable(true);
    return true;
}

/***************************************************************************************************************
AS7343 Get Device Identity
****************************************************************************************************************/
bool AS7343::Get_Id(void)
{
    uint8_t id = 0x00;
    readCommand(AS7343_REG_ID, &id, 1);
    if (id == 0x81)
    {
        return true;
    }
    else
        return false;
}

/***************************************************************************************************************
The integration time in INT_MODE = “00” and “01” (SPM/SYNS) is set using the ATIME (0x81) and
ASTEP (0xCA, 0xCB) registers. The integration time, in milliseconds, is equal to:
Equation 1: Setting the integration time
𝑡𝑖𝑛𝑡 = (𝐴𝑇𝐼𝑀𝐸 + 1) × (𝐴𝑆𝑇𝐸𝑃 + 1) × 2.78µ𝑠
The reset value for ASTEP is 999 (2.78ms) and the recommended configuration for these two
registers is ASTEP = 599 and ATIME = 29, which results in an integration time of 50ms. It is not
allowed that both settings –ATIME and ASTEP – are set to “0”.
The integration time also defines the full-scale ADC value, which is equal to:
Equation 2: ADC full scale value(1)
𝐴𝐷𝐶𝑓𝑢𝑙𝑙𝑠𝑐𝑎𝑙𝑒 = (𝐴𝑇𝐼𝑀𝐸 + 1) × (𝐴𝑆𝑇𝐸𝑃 + 1)
****************************************************************************************************************/
void AS7343::SetIntegrationTime(uint8_t atime, uint16_t astep)
{
    uint8_t buf[2] = {0x00};
    buf[0] = astep & 0xff;
    buf[1] = astep >> 8;
    writeCommand(AS7343_REG_ATIME, &atime, 1);
    writeCommand(AS7343_REG_ASTEP_L, buf, 2);
}

/***************************************************************************************************************
AS7343 set Gain
****************************************************************************************************************/
void AS7343::SetGain(uint8_t value)
{
    writeCommand(AS7343_REG_CFG1, &value, 1);
}

/***************************************************************************************************************
AS7343 set Enable
****************************************************************************************************************/
void AS7343::SetEnable(bool status)
{
    uint8_t rev_data = 0x00;
    readCommand(AS7343_REG_ENABLE, &rev_data, 1);
    rev_data = (status == true) ? (rev_data | 0x01) : (rev_data & 0xfe);
    writeCommand(AS7343_REG_ENABLE, &rev_data, 1);
}

/***************************************************************************************************************
AS7343 Spectral Measurement
****************************************************************************************************************/
void AS7343::SpectralMeasurement(bool status)
{
    uint8_t rev_data = 0x00;
    readCommand(AS7343_REG_ENABLE, &rev_data, 1);
    rev_data = (status == true) ? (rev_data | 0x02) : (rev_data & 0xfd);
    writeCommand(AS7343_REG_ENABLE, &rev_data, 1);
}

/***************************************************************************************************************
AS7343 set Low Power
****************************************************************************************************************/
void AS7343::SetLowPower(bool status)
{
    uint8_t rev_data = 0x00;
    readCommand(AS7343_REG_ENABLE, &rev_data, 1);
    rev_data = (status == true) ? (rev_data | 0x20) : (rev_data & 0xdf);
    writeCommand(AS7343_REG_ENABLE, &rev_data, 1);
}

/***************************************************************************************************************
AS7343 set bank
****************************************************************************************************************/
void AS7343::SetBank(bool status)
{
    uint8_t rev_data = 0x00;
    readCommand(AS7343_REG_CFG0, &rev_data, 1);
    rev_data = (status == true) ? (rev_data | 0x10) : (rev_data & 0xef);
    writeCommand(AS7343_REG_CFG0, &rev_data, 1);
}

/***************************************************************************************************************
SMUX command.
Selects the SMUX command to execute when setting SMUXEN gets set. Do not change during ongoing SMUX operation.
****************************************************************************************************************/
void AS7343::SetSMUX(uint8_t value)
{
    writeCommand(AS7343_REG_CFG6, &value, 1);
}

/***************************************************************************************************************
SMUX Enable.
1: Starts SMUX command
Note: this bit gets cleared automatically as soon as
SMUX operation is finished
****************************************************************************************************************/
void AS7343::SetEnableSMUX(void)
{
    uint8_t rev_data = 0x00;
    readCommand(AS7343_REG_ENABLE, &rev_data, 1);
    rev_data = rev_data | 0x11;
    writeCommand(AS7343_REG_ENABLE, &rev_data, 1);
}

/***************************************************************************************************************
Configure SMUX for sensors F1-4, Clear and NIR
****************************************************************************************************************/
void AS7343::F1F4_Clear_NIR(void)
{
    writeByte(0x00, 0x05);
    writeByte(0x01, 0x02);
    writeByte(0x02, 0x10);
    writeByte(0x03, 0x04);
    writeByte(0x04, 0x55);
    writeByte(0x05, 0x00);
    writeByte(0x06, 0x30);
    writeByte(0x07, 0x05);
    writeByte(0x08, 0x06);
    writeByte(0x09, 0x00);
}

/***************************************************************************************************************
Configure SMUX for sensors F5-8, Clear and NIR
****************************************************************************************************************/
void AS7343::F5F8_Clear_NIR(void)
{
    writeByte(0x00, 0x05);
    writeByte(0x01, 0x30);
    writeByte(0x02, 0x00);
    writeByte(0x03, 0x10);
    writeByte(0x04, 0x55);
    writeByte(0x05, 0x42);
    writeByte(0x06, 0x00);
    writeByte(0x07, 0x05);
    writeByte(0x08, 0x06);
    writeByte(0x09, 0x00);
}

/***************************************************************************************************************
Configure SMUX for sensors XL, Fz, FY, FD, EXT and D
****************************************************************************************************************/
void AS7343::XL_FZ_FY_FD_EXT(void)
{
    writeByte(0x00, 0x10);
    writeByte(0x01, 0x00);
    writeByte(0x02, 0x02);
    writeByte(0x03, 0x00);
    writeByte(0x04, 0x00);
    writeByte(0x05, 0x00);
    writeByte(0x06, 0x03);
    writeByte(0x07, 0x40);
    writeByte(0x08, 0x50);
    writeByte(0x09, 0x06);
}

/***************************************************************************************************************
Configure SMUX for flicker detection
****************************************************************************************************************/
void AS7343::FDConfig(void)
{
    writeByte(0x00, 0x00);
    writeByte(0x01, 0x00);
    writeByte(0x02, 0x00);
    writeByte(0x03, 0x00);
    writeByte(0x04, 0x00);
    writeByte(0x05, 0x00);
    writeByte(0x06, 0x00);
    writeByte(0x07, 0x00);
    writeByte(0x08, 0x00);
    writeByte(0x09, 0x06);
}

/***************************************************************************************************************
Spectral Valid.
Indicates that the spectral measurement has been completed
****************************************************************************************************************/
bool AS7343::WaitDataReady(void)
{
    uint8_t status;
    readCommand(AS7343_REG_STATUS2, &status, 1);
    if ((status & 0x40) >> 6)
        return true;
    else
        return false;
}

/***************************************************************************************************************
AS7343 Control Led
status:open led or close led
value: 1 ~ 127
****************************************************************************************************************/
void AS7343::ControlLed(bool status, uint8_t value)
{
    uint8_t rev_data;
    SetBank(true);
    rev_data = value;
    rev_data = (status == true) ? (rev_data | 0x80) : (rev_data & 0x7f);
    writeCommand(AS7343_REG_LED, &rev_data, 1);
    SetBank(false);
}

/***************************************************************************************************************
AS7343 Enable Flicker Detection
****************************************************************************************************************/
void AS7343::EnableFlickerDetection(bool status)
{
    uint8_t rev_data = 0x00;
    readCommand(AS7343_REG_ENABLE, &rev_data, 1);
    rev_data = (status == true) ? (rev_data | 0x40) : (rev_data & 0xBF);
    writeCommand(AS7343_REG_ENABLE, &rev_data, 1);
}

/***************************************************************************************************************
AS7343 Read Flicker Data
****************************************************************************************************************/
uint8_t AS7343::ReadFlickerData(void)
{
    uint8_t flicker;
    SetBank(false);
    SpectralMeasurement(false);
    SetSMUX(AS7343_SMUX_WRITE);
    FDConfig();
    SetEnableSMUX();
    SpectralMeasurement(true);
    EnableFlickerDetection(true);
    delay(10);
    readCommand(AS7343_REG_FD_STATUS, &flicker, 1);
    EnableFlickerDetection(false);
    return flicker;
}

/***************************************************************************************************************
AS7343 Read All Channels
****************************************************************************************************************/
void AS7343::ReadAllChannels(uint16_t *readings_buffer)
{
    uint8_t rev_data[12] = {0};
    int i = 0;
    SetBank(false);
    SpectralMeasurement(false);
    SetSMUX(AS7343_SMUX_WRITE);

    F1F4_Clear_NIR();
    SetEnableSMUX();
    SpectralMeasurement(true);
    while (!WaitDataReady())
    {
        delay(1);
    }

    readCommand(AS7343_REG_CH0_DATA_L, rev_data, 12);

    for (i = 0; i < 4; i++)
        readings_buffer[i] = rev_data[i * 2 + 1] << 8 | rev_data[i * 2];

    SetBank(false);
    SpectralMeasurement(false);
    SetSMUX(AS7343_SMUX_WRITE);
    F5F8_Clear_NIR();
    SetEnableSMUX();
    SpectralMeasurement(true);
    while (!WaitDataReady())
    {
        delay(2);
    }
    readCommand(AS7343_REG_CH0_DATA_L, rev_data, 12);
    for (i = 0; i < 6; i++)
        readings_buffer[4 + i] = rev_data[i * 2 + 1] << 8 | rev_data[i * 2];
    SetBank(false);
    SpectralMeasurement(false);
    SetSMUX(AS7343_SMUX_WRITE);
    XL_FZ_FY_FD_EXT();
    SetEnableSMUX();
    SpectralMeasurement(true);
    while (!WaitDataReady())
    {
        delay(2);
    }
    readCommand(AS7343_REG_CH0_DATA_L, rev_data, 12);
    for (i = 0; i < 6; i++)
        readings_buffer[10 + i] = rev_data[i * 2 + 1] << 8 | rev_data[i * 2];
}

boolean AS7343::readCommand(uint8_t reg, uint8_t *redata, uint8_t redata_length)
{
    Wire.beginTransmission(_i2caddr);
//    Serial.print("ReadCommand");
    Wire.write(reg);
    Wire.endTransmission(false);
    delay(2);
    while (Wire.requestFrom(_i2caddr, redata_length) != redata_length)
        ;
    for (uint8_t i = 0; i < redata_length; i++)
        redata[i] = Wire.read();
    return true;
}

void AS7343::writeCommand(uint8_t reg, uint8_t *cmd, uint8_t length)
{
    Wire.beginTransmission(_i2caddr);
    Wire.write(reg);
    delay(2);
    Wire.write(cmd, length);
    Wire.endTransmission();
}

void AS7343::writeByte(uint8_t reg, uint8_t cmd)
{
    Wire.beginTransmission(_i2caddr);
    Wire.write(reg);
    delay(2);
    Wire.write(&cmd, 1);
    Wire.endTransmission();
}
