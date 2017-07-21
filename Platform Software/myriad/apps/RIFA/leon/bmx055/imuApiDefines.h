///
/// @file imuApiDefines.h
/// @copyright All code copyright Movidius Ltd 2013, all rights reserved.
/// For License Warranty see: common/license.txt
///
/// @brief
///

#ifndef IMUAPIDEFINES_H_
#define IMUAPIDEFINES_H_

/// System Includes
/// ----------------------------------------------------------------------------

/// Application Includes
/// ----------------------------------------------------------------------------

/// Source Specific #defines and types (typedef,enum,struct)
/// ----------------------------------------------------------------------------

/// IMU BMX055 Valid flags
#define IMU_BMX055_ACC_VALID 0x0001
#define IMU_BMX055_GYRO_VALID 0x0002
#define IMU_BMX055_MAG_VALID 0x0004

/// IMU BMX055 slave addresses
#define IMU_BMX055_ACC_SLAVE_ADDR_R2 0x18
#define IMU_BMX055_ACC_SLAVE_ADDR_R3 0x19

#define IMU_BMX055_MAG_SLAVE_ADDR_R2 0x10
#define IMU_BMX055_MAG_SLAVE_ADDR_R3 0x11

#define IMU_BMX055_ACC_SLAVE_ADDR IMU_BMX055_ACC_SLAVE_ADDR_R3
#define IMU_BMX055_MAG_SLAVE_ADDR IMU_BMX055_MAG_SLAVE_ADDR_R3

#define IMU_BMX055_GYRO_SLAVE_ADDR 0x68
//#define IMU_BMX055_ACC_SLAVE_ADDR 0x18
//#define IMU_BMX055_MAG_SLAVE_ADDR 0x10

/// Gyroscope range
#define IMU_BMX055_GYRO_RANGE2000 0x0
#define IMU_BMX055_GYRO_RANGE1000 0x1
#define IMU_BMX055_GYRO_RANGE500 0x2
#define IMU_BMX055_GYRO_RANGE250 0x3
#define IMU_BMX055_GYRO_RANGE125 0x4

/// Gyroscope rate and filter bandwidth
#define IMU_BMX055_GYRO_100HZ_BW_32HZ 0x7
#define IMU_BMX055_GYRO_200HZ_BW_64HZ 0x6
#define IMU_BMX055_GYRO_400HZ_BW_47HZ 0x3
#define IMU_BMX055_GYRO_1000HZ_BW_116HZ 0x2
#define IMU_BMX055_GYRO_2000HZ_BW_230HZ 0x1

/// Accelerometer Config registers
#define IMU_BMX055_ACC_REG_PMU_LPW_ADDR 0x11
#define IMU_BMX055_ACC_REG_BGW_SOFT_RESET_ADDR 0x14

#define IMU_BMX055_ACC_DEEP_SUSPEND_BIT_OFFSET 5
#define IMU_BMX055_ACC_PMU_BW_62HZ 0x0B
#define IMU_BMX055_ACC_STARTUP_TIME_US 1300

/// Gyroscope Config registers
#define IMU_BMX055_GYRO_REG_LPM1_ADDR 0x11
#define IMU_BMX055_GYRO_REG_BGW_SOFTRESET_ADDR 0x14

#define IMU_BMX055_GYRO_DEEP_SUSPEND_BIT_OFFSET 5
#define IMU_BMX055_GYRO_STARTUP_TIME_US 30000

/// Magnetometer Config registers
#define IMU_BMX055_MAG_STARTUP_TIME_US 3000
#define IMU_BMX055_MAG_POWER_CONTROL_OFFSET 0

#define IMU_BMX055_ACC_REG_VALUE_SOFT_RESET 0xB6
#define IMU_BMX055_ACC_BGW_CHIP_ID_VAL 0xFA
#define IMU_BMX055_GYRO_CHIP_ID_VAL 0x0F
#define IMU_BMX055_MAG_RATE30 0x07

/// IMU BMX055 Accelerometer range
#define IMU_BMX055_ACC_RANGE2G 0x3

#define IRQ_SOURCE_0 0
#define IRQ_SOURCE_1 1
typedef enum imuType_t
{
    IMU_INVALID_PACKET_TYPE,
    IMU_BMX055_ACCEL,
    IMU_BMX055_GYRO,
    IMU_BMX055_MAG
} imuType_t;

typedef struct imuPacket_t
{
    u64 timestamp;
    imuType_t imuEnum;
    s16 dataX;
    s16 dataY;
    s16 dataZ;
    s16 dataR;
} imuPacket_t;

/// The typedef of the callback function
// that is needed as a parameter for ConfigureInterruptHandlers function
typedef void (imuBmxCb_t) (imuType_t);

/// Imu configuration data for initialisation
typedef struct imuBmx055ConfigData_t
{
    u16 accSampleRate;
    u16 gyroSampleRate;
    u16 magSampleRate;
    u16 accRange;
    u16 gyroRange;
} imuBmx055ConfigData_t;

/// The struct to keep the hex register values for the config data
/// !Note: This is generated automatically from the values in Bmx055_Config_Data
typedef struct imuBmx055ActualRegisterData_t
{
    u8 accSampleRate;
    u8 gyroSampleRate;
    u8 magSampleRate;
    u8 accRange;
    u8 gyroRange;
} imuBmx055ActualRegisterData_t;

/// Imu interrupt handler initialisation struct
typedef struct imuBmx055InterruptConfig_t
{
    u32 gpioInt1;
    u32 gpioInt3;
    u8 handlerPriority :4;
} imuBmx055InterruptConfig_t;

/// Imu default configuration
static const imuBmx055ConfigData_t imuBmx055DefaultConfiguration =
{
    .accSampleRate 	= 125,
    .gyroSampleRate = 100,
    .magSampleRate 	= 30,
    .accRange 		= 2,
    .gyroRange 		= 2000
};

static const imuPacket_t imuPacketDefault =
{
    .timestamp 	= 0x0,
    .imuEnum 	= 0x0,
    .dataX 		= 0x0,
    .dataY 		= 0x0,
    .dataZ 		= 0x0,
    .dataR 		= 0x0
};

static const imuBmx055ActualRegisterData_t imuBmx055RegDefaultConfiguration =
{
    .accSampleRate 	 = IMU_BMX055_ACC_PMU_BW_62HZ,
    .gyroSampleRate	 = 0x5, //IMU_BMX055_GYRO_100HZ_BW_32HZ,
    .magSampleRate	 = IMU_BMX055_MAG_RATE30,
    .accRange		 = IMU_BMX055_ACC_RANGE2G,
    .gyroRange		 = IMU_BMX055_GYRO_RANGE2000
};

#endif /* IMUAPIDEFINES_H_ */
