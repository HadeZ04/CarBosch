#include "main.h"
#include "INA219.h"

extern I2C_HandleTypeDef hi2c1;

uint16_t INA219Read(uint8_t REG)
{
	uint8_t VALUE[2];

	HAL_I2C_Mem_Read(&hi2c1, INA219_I2C_ADDR << 1 | INA219_I2C_READ,
					REG, 1, VALUE, 2, 100);

	return ((VALUE[0] << 8) | VALUE[1]);
}


void INA219Write(uint8_t REG, uint16_t VAL)
{
	uint8_t VALUE[2];
	VALUE[0] = (VAL >> 8) & 0xff;
	VALUE[1] = (VAL >> 0) & 0xff;
	HAL_I2C_Mem_Write(&hi2c1, INA219_I2C_ADDR << 1 | INA219_I2C_WRITE,
						REG, 1, VALUE, 2, 100);
}

uint16_t INA219_ReadBusVoltage()
{
	uint16_t result = INA219Read(INA219_BUSVOLTAGE_REG);

	return ((result >> 3) * 4);
}

int16_t INA219_ReadCurrent_raw()
{
	int16_t result = INA219Read(INA219_CURRENT_REG);

	return result;
}

int16_t INA219_ReadCurrent(int16_t ina219_currentDivider_mA)
{
	int16_t result = INA219_ReadCurrent_raw();

	return (result / ina219_currentDivider_mA);
}

uint16_t INA219_ReadShuntVolage()
{
	uint16_t result = INA219Read(INA219_SHUNTVOLTAGE_REG);

	return (result * 0.01);
}

void INA219SoftwareReset()
{
	INA219Write(INA219_CONFIG_REG, INA219_SOFTWARE_RESET);
	HAL_Delay(1);
}

void INA219_setCalibration(uint16_t CalibrationData)
{
	INA219Write(INA219_CALIBRATION_REG, CalibrationData);
}

uint16_t INA219_getConfig()
{
	uint16_t result = INA219Read(INA219_CONFIG_REG);
	return result;
}

void INA219_setConfig(uint16_t Config)
{
	INA219Write(INA219_CONFIG_REG, Config);
}

//void INA219_setCalibration_32V_2A()
//{
//	uint16_t config = INA219_CONFIG_BVOLTAGERANGE_32V |
//	             INA219_CONFIG_GAIN_8_320MV | INA219_CONFIG_BADCRES_12BIT |
//	             INA219_CONFIG_SADCRES_12BIT_1S_532US |
//	             INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;
//
//	ina219_calibrationValue = 4096;
//	ina219_currentDivider_mA = 10; // Current LSB = 100uA per bit (1000/100 = 10)
//	ina219_powerMultiplier_mW = 2; // Power LSB = 1mW per bit (2/1)
//
//	INA219_setCalibration(ina219, ina219_calibrationValue);
//	INA219_setConfig(ina219, config);
//}
//
//void INA219_setCalibration_32V_1A()
//{
//	uint16_t config = INA219_CONFIG_BVOLTAGERANGE_32V |
//	                    INA219_CONFIG_GAIN_8_320MV | INA219_CONFIG_BADCRES_12BIT |
//	                    INA219_CONFIG_SADCRES_12BIT_1S_532US |
//	                    INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;
//
//	ina219_calibrationValue = 10240;
//	ina219_currentDivider_mA = 25;    // Current LSB = 40uA per bit (1000/40 = 25)
//	ina219_powerMultiplier_mW = 0.8f; // Power LSB = 800uW per bit
//
//	INA219_setCalibration(ina219, ina219_calibrationValue);
//	INA219_setConfig(ina219, config);
//}
//
//void INA219_setCalibration_16V_400mA()
//{
//	uint16_t config = INA219_CONFIG_BVOLTAGERANGE_16V |
//	                    INA219_CONFIG_GAIN_1_40MV | INA219_CONFIG_BADCRES_12BIT |
//	                    INA219_CONFIG_SADCRES_12BIT_1S_532US |
//	                    INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;
//
//	ina219_calibrationValue = 8192;
//	ina219_currentDivider_mA = 20;    // Current LSB = 50uA per bit (1000/50 = 20)
//	ina219_powerMultiplier_mW = 1.0f; // Power LSB = 1mW per bit
//
//	INA219_setCalibration(ina219, ina219_calibrationValue);
//	INA219_setConfig(ina219, config);
//}

//void INA219_setPowerMode(uint8_t Mode)
//{
//	uint16_t config = INA219_getConfig(ina219);
//
//	switch (Mode) {
//		case INA219_CONFIG_MODE_POWERDOWN:
//			config = (config & ~INA219_CONFIG_MODE_MASK) | (INA219_CONFIG_MODE_POWERDOWN & INA219_CONFIG_MODE_MASK);
//			INA219_setConfig(ina219, config);
//			break;
//
//		case INA219_CONFIG_MODE_SANDBVOLT_TRIGGERED:
//			config = (config & ~INA219_CONFIG_MODE_MASK) | (INA219_CONFIG_MODE_SANDBVOLT_TRIGGERED & INA219_CONFIG_MODE_MASK);
//			INA219_setConfig(ina219, config);
//			break;
//
//		case INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS:
//			config = (config & ~INA219_CONFIG_MODE_MASK) | (INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS & INA219_CONFIG_MODE_MASK);
//			INA219_setConfig(ina219, config);
//			break;
//
//		case INA219_CONFIG_MODE_ADCOFF:
//			config = (config & ~INA219_CONFIG_MODE_MASK) | (INA219_CONFIG_MODE_ADCOFF & INA219_CONFIG_MODE_MASK);
//			INA219_setConfig(ina219, config);
//			break;
//	}
//}

uint8_t INA219Init(INA219Config *userConfig)
{
	userConfig->CurrDiv_mA = 0;
	userConfig->PowMul_mW = 0;

	uint8_t isINA219Ready = HAL_I2C_IsDeviceReady(&hi2c1, INA219_I2C_ADDR << 1, 1, 100);

	if(isINA219Ready == HAL_OK)
	{
		/* Check if INA219 is reset */
		uint16_t preConfig = INA219Read(INA219_CONFIG_REG);
		if (preConfig != 0x399f)
		{
			INA219SoftwareReset();
			/* Check again */
			preConfig = INA219Read(INA219_CONFIG_REG);
			if (preConfig != 0x399f)
			{
				/* INA219 error */
				// printf("INA219 can't be reset");
				while (1);
				return 0;
			}
		}
		/* INA219 is reset */
//		printf("INA219 reset");
		/* User configuration */
		//INA219_setCalibration_32V_2A(ina219);
		return 1;
	}

	else
	{
		// printf("INA219 isn't ready");
		return 0;
	}
}
