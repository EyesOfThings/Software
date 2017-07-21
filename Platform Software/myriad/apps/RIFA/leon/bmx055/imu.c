///
/// @file imu.c
/// @copyright All code copyright Movidius Ltd 2014, all rights reserved.
/// For License Warranty see: common/license.txt
///
/// @brief
///

/// System Includes
/// ----------------------------------------------------------------------------
#include "stdio.h"
#include "DrvI2cMaster.h"
#include "DrvIcbDefines.h"
#include "DrvGpio.h"

/// Application Includes
/// ----------------------------------------------------------------------------
#include "imuApi.h"
#include "imuApiDefines.h"

/// 2: Source Specific #defines and types (typedef,enum,struct)
/// ----------------------------------------------------------------------------

#ifndef IMUBMX_CODE_SECTION
#define IMUBMX_CODE_SECTION(x) x
#endif
#ifndef IMUBMX_DATA_SECTION
#define IMUBMX_DATA_SECTION(x) x
#endif

/// IMU Number of Magnetometer reads
#define IMU_BMX055_ONE_MAG_IN 4

/// IMU BMX055 Accelerometer registers address
#define IMU_BMX055_ACC_X_LSB 		0x02
#define IMU_BMX055_ACC_PMU_BW 		0x10
#define IMU_BMX055_ACC_HBW 			0x13
#define IMU_BMX055_ACC_INT_EN_1 	0x17
#define IMU_BMX055_ACC_INT_MAP_1 	0x1A
#define IMU_BMX055_ACC_PMU_RANGE 	0x0F
#define IMU_BMX055_ACC_INT0 		0x22

/// IMU BMX055 Gyro registers address
#define IMU_BMX055_GYRO_RATE_X_LSB 	0x02
#define IMU_BMX055_GYRO_RANGE 		0x0F
#define IMU_BMX055_GYRO_BW 			0x10
#define IMU_BMX055_GYRO_RATE_HBW 	0x13
#define IMU_BMX055_GYRO_INT_EN_0 	0x15
#define IMU_BMX055_GYRO_INT_EN_1 	0x16
#define IMU_BMX055_GYRO_INT_MAP_1 	0x18
#define IMU_BMX055_GYRO_BIST		0x3C /// Built in Self-Test register
#define IMU_BMX055_GYRO_HIGH_TH_Z 	0x26

/// IMU BMX055 Magneto registers address
#define IMU_BMX055_MAG_X_LSB 	0x42
#define IMU_BMX055_MAG_OPMODE	0x4C
#define IMU_BMX055_MAG_AXES_EN 	0x4E

/// IMU BMX055 Accelerometer register values
#define IMU_BMX055_ACC_BGW_CHIPID_VAL 	0xFA
#define IMU_BMX055_ACC_DATA_RDY_INT 	(1<<4)
#define IMU_BMX055_ACC_MAP_TO_INT1 		0x1
#define IMU_BMX055_ACC_INT0_DEF_VAL 	0x9
#define IMU_BMX055_ACC_INT1_DEF_VAL 	0x30

/// IMU BMX055 Accel range
#define IMU_BMX055_ACC_RANGE4G 	0x5
#define IMU_BMX055_ACC_RANGE8G 	0x8
#define IMU_BMX055_ACC_RANGE16G 0xC

/// IMU BMX055 Accel data filter bandwidth (sample rate = 2*bandwidth)
#define IMU_BMX055_ACC_PMU_BW_8HZ 		0x8
#define IMU_BMX055_ACC_PMU_BW_16HZ		0x9
#define IMU_BMX055_ACC_PMU_BW_32HZ 		0xA
#define IMU_BMX055_ACC_PMU_BW_125HZ	 	0xC
#define IMU_BMX055_ACC_PMU_BW_250HZ 	0xD
#define IMU_BMX055_ACC_PMU_BW_500HZ 	0xE
#define IMU_BMX055_ACC_PMU_BW_1000HZ 	0xF

/// IMU BMX055 Gyro register values
#define IMU_BMX055_GYRO_DATA_RDY_INT 		(1<<7)
#define IMU_BMX055_GYRO_HIGH_TH_Z_DEF 		0x2
#define IMU_BMX055_GYRO_HIGH_DUR_Z_DEF 		0x19
#define IMU_BMX055_GYRO_BIST_RATE_OK		0x10
#define IMU_BMX055_GYRO_BIST_TRIG_MASK 		0x1
#define IMU_BMX055_GYRO_BIST_STAT_MASK 		0x6
#define IMU_BMX055_GYRO_BIST_PASS 			0x2
#define IMU_BMX055_GYRO_INT3_LVL_H 			0x1
#define IMU_BMX055_GYRO_INT3_MAP_DATA_RDY 	0x1

/// IMU BMX055 Magneto register values
#define IMU_BMX055_MAG_CHIP_ID_VAL 			0x32
#define IMU_BMX055_MAG_RATE2 				0x1
#define IMU_BMX055_MAG_RATE6 				0x2
#define IMU_BMX055_MAG_RATE8 				0x3
#define IMU_BMX055_MAG_RATE10				0x0
#define IMU_BMX055_MAG_RATE15 				0x4
#define IMU_BMX055_MAG_RATE20 				0x5
#define IMU_BMX055_MAG_RATE25 				0x6
#define IMU_BMX055_MAG_INT_SET_DEF			0x3F
#define IMU_BMX055_MAG_INT_CTRL_DEF			0x7
#define IMU_BMX055_MAG_SLEEP_MODE 			0x6
#define IMU_BMX055_MAG_NORMAL_SELF_TEST_BIT 0x1
#define IMU_BMX055_MAG_SELF_TEST_PASS 		0x3


/// 3: Global Data (Only if absolutely necessary)
/// ----------------------------------------------------------------------------

/// 4: Static Local Data
/// ----------------------------------------------------------------------------
static I2CM_Device* i2cMDevice;

/// IMU BMX055 I2C read and I2C write protocol
static u8 imuWriteProto[] = I2C_PROTO_WRITE_8BA;
static u8 imuReadProto[] = I2C_PROTO_READ_8BA;

static imuBmx055ActualRegisterData_t imuBmx055ActualRegisterData;

/// User side callback for imu events
static imuBmxCb_t *imuBmxCb;

static struct
{
    u32 Bmx055Acc;
    u32 Bmx055Gyro;
    u32 Bmx055Mag;
} imuCount;

/// 5: Static Function Prototypes
/// ----------------------------------------------------------------------------
static void imuBmxAccRead(u32 unused);
static void imuBmxGyroRead(u32 unused);
static void imuBmxSetConfigRegValues(imuBmx055ConfigData_t* cfg);

/// 6: Functions Implementation
/// ----------------------------------------------------------------------------
s32 IMUBMX_CODE_SECTION(imuComponentInit)(I2CM_Device* i2cDevice, imuBmx055ConfigData_t* cfg)
{
    u8 data[5], tmp[2];
    s32 i2cTransactionResult;
    imuCount.Bmx055Acc = imuCount.Bmx055Gyro = imuCount.Bmx055Mag = 0;

    i2cMDevice = i2cDevice;
    imuBmxSetConfigRegValues(cfg);

    DrvI2cMTransaction(i2cMDevice, IMU_BMX055_GYRO_SLAVE_ADDR,
                       IMU_BMX055_GYRO_CHIP_ID, imuReadProto,
                       tmp, 1);
    DrvI2cMTransaction(i2cMDevice, IMU_BMX055_MAG_SLAVE_ADDR,
                       IMU_BMX055_MAG_CHIP_ID, imuReadProto,
                       tmp,1);
    DrvI2cMTransaction(i2cMDevice, IMU_BMX055_ACC_SLAVE_ADDR,
                       IMU_BMX055_ACC_BGW_CHIP_ID, imuReadProto,
                       tmp, 1);

    /// Settings for accelerometer
    /// Reset accelerometer
    data[0] = IMU_BMX055_ACC_REG_VALUE_SOFT_RESET;
    i2cTransactionResult = DrvI2cMTransaction(
                    i2cMDevice, IMU_BMX055_ACC_SLAVE_ADDR,
                    IMU_BMX055_ACC_REG_BGW_SOFT_RESET_ADDR,
                    imuWriteProto, data, 1);

    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }
    SleepMicro(IMU_BMX055_ACC_STARTUP_TIME_US);

    /// Setting the filter bandwidth (the specs say that the sampling rate is double the filter bandwidth)
    data[0] = imuBmx055ActualRegisterData.accSampleRate;
    i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                              IMU_BMX055_ACC_SLAVE_ADDR,
                                              IMU_BMX055_ACC_PMU_BW,
                                              imuWriteProto, data, 1);

    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }

    /// Setting the range
    data[0] = imuBmx055ActualRegisterData.accRange;
    i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                              IMU_BMX055_ACC_SLAVE_ADDR,
                                              IMU_BMX055_ACC_PMU_RANGE,
                                              imuWriteProto, data, 1);

    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }

    /// Setting filtered mode and active shadowing (reset value is 0 but i'll write it as a precaution)
    data[0] = 0x0;
    i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                              IMU_BMX055_ACC_SLAVE_ADDR,
                                              IMU_BMX055_ACC_HBW, imuWriteProto,
                                              data, 1);

    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }

    /// Enabling the data ready interrupt
    data[0] = IMU_BMX055_ACC_DATA_RDY_INT;
    i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                              IMU_BMX055_ACC_SLAVE_ADDR,
                                              IMU_BMX055_ACC_INT_EN_1,
                                              imuWriteProto, data, 1);

    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }

    /// Mapping the data ready interrupt to INT1 pin
    data[0] = IMU_BMX055_ACC_MAP_TO_INT1;
    i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                              IMU_BMX055_ACC_SLAVE_ADDR,
                                              IMU_BMX055_ACC_INT_MAP_1,
                                              imuWriteProto, data, 1);

    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }

    /// Settings for gyroscope
    /// Leave deep suspend mode
    data[0] = 0;
    i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                              IMU_BMX055_GYRO_SLAVE_ADDR,
                                              IMU_BMX055_GYRO_REG_LPM1_ADDR,
                                              imuWriteProto, data, 1);

    SleepMicro(IMU_BMX055_GYRO_STARTUP_TIME_US);

    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }

    /// Setting the range
    data[0] = imuBmx055ActualRegisterData.gyroRange;
    i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                              IMU_BMX055_GYRO_SLAVE_ADDR,
                                              IMU_BMX055_GYRO_RANGE, /// 2000 grades per s
                                              imuWriteProto, data, 1);

    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }

    /// Setting the bandwidth
    DrvI2cMTransaction(i2cMDevice, IMU_BMX055_GYRO_SLAVE_ADDR,
                       IMU_BMX055_GYRO_BW,
                       imuReadProto, data, 1);

    data[0] = (data[0] | imuBmx055ActualRegisterData.gyroSampleRate);
    i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                              IMU_BMX055_GYRO_SLAVE_ADDR,
                                              IMU_BMX055_GYRO_BW, imuWriteProto,
                                              data, 1);

    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }

    /// Setting filtered mode and shadowing
    data[0] = 0x0;
    i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                              IMU_BMX055_GYRO_SLAVE_ADDR,
                                              IMU_BMX055_GYRO_RATE_HBW,
                                              imuWriteProto, data, 1);

    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }

    /// Setting the data ready interrupt
    data[0] = IMU_BMX055_GYRO_DATA_RDY_INT;
    i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                              IMU_BMX055_GYRO_SLAVE_ADDR,
                                              IMU_BMX055_GYRO_INT_EN_0,
                                              imuWriteProto, data, 1);

    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }

    /// Setting the interrupt pin to push-pull and active level high
    data[0] = IMU_BMX055_GYRO_INT3_LVL_H;
    i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                              IMU_BMX055_GYRO_SLAVE_ADDR,
                                              IMU_BMX055_GYRO_INT_EN_1,
                                              imuWriteProto, data, 1);

    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }

    /// Mapping the data ready interrupt to int3 pin
    data[0] = IMU_BMX055_GYRO_INT3_MAP_DATA_RDY;
    i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                              IMU_BMX055_GYRO_SLAVE_ADDR,
                                              IMU_BMX055_GYRO_INT_MAP_1,
                                              imuWriteProto, data, 1);

    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }

    /// Settings for magnetometer
    /// Setting power control bit
    data[0] = (0x1 << IMU_BMX055_MAG_POWER_CONTROL_OFFSET);
    i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                              IMU_BMX055_MAG_SLAVE_ADDR,
                                              IMU_BMX055_MAG_POWER_RESET,
                                              imuWriteProto, data, 1);

    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }
    SleepMicro(IMU_BMX055_MAG_STARTUP_TIME_US);

    /// Setting the data rate and normal operation mode
    data[0] = (imuBmx055ActualRegisterData.magSampleRate << 3);
    i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                              IMU_BMX055_MAG_SLAVE_ADDR,
                                              IMU_BMX055_MAG_OPMODE,
                                              imuWriteProto, data, 1);

    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }

    return 0;
}

s32 imuGetDataPacket(imuType_t imuBmxtype, imuPacket_t *read)
{
    s32 i2cTransactionResult;
    u8 data[10];
    u8 check = 0;

    /// Reset this packet to default values
    read[0] = imuPacketDefault;

    /// Assign this packet it's imuBmxtype
    read->imuEnum = imuBmxtype;
    if (IMU_BMX055_ACCEL == read->imuEnum)
    {

        i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                                  IMU_BMX055_ACC_SLAVE_ADDR,
                                                  IMU_BMX055_ACC_X_LSB,
                                                  imuReadProto, data, 7);

        if (i2cTransactionResult == I2CM_STAT_OK)
        {
            read->dataX = (s16) ((data[1] << 8) | data[0]) >> 4;
            read->dataY = (s16) ((data[3] << 8) | data[2]) >> 4;
            read->dataZ = (s16) ((data[5] << 8) | data[4]) >> 4;
            read->dataR = data[6];
            data[0] = data[0] << 7;
            data[2] = data[2] << 7;
            data[4] = data[4] << 7;
            check = data[0] + data[2] + data[4];

            /// We don't have any new data since the last readout
            if (check == 0)
            {
                return 1;
            }
        }
        else
        {
            return i2cTransactionResult;
        }
    }

    if (IMU_BMX055_GYRO == read->imuEnum)
    {
        i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                                  IMU_BMX055_GYRO_SLAVE_ADDR,
                                                  IMU_BMX055_GYRO_RATE_X_LSB,
                                                  imuReadProto, data, 6);

        if (i2cTransactionResult == I2CM_STAT_OK)
        {
            read->dataX = (s16) ((data[1] << 8) | data[0]);
            read->dataY = (s16) ((data[3] << 8) | data[2]);
            read->dataZ = (s16) ((data[5] << 8) | data[4]);
        }
        else
        {
            return i2cTransactionResult;
        }
    }

    if (IMU_BMX055_MAG == read->imuEnum)
    {
        i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                                  IMU_BMX055_MAG_SLAVE_ADDR,
                                                  IMU_BMX055_MAG_X_LSB,
                                                  imuReadProto, data,
                                                  6);

        if (i2cTransactionResult == I2CM_STAT_OK)
        {
            read->dataX = (s16) ((data[1] << 8) | data[0]) >> 3;
            read->dataY = (s16) ((data[3] << 8) | data[2]) >> 3;
            read->dataZ = (s16) ((data[5] << 8) | data[4]) >> 1;
        }
        else
        {
            return i2cTransactionResult;
        }
    }

    return 0;
}

s32 IMUBMX_CODE_SECTION(imuI2cTest)(I2CM_Device* dev1)
{
    u8 data[6];
    s32 i2cTransactionResult;

    i2cMDevice = dev1;


    /// Testing ACC
    /// Checking accel chip id
    i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                              IMU_BMX055_ACC_SLAVE_ADDR,
                                              IMU_BMX055_ACC_BGW_CHIP_ID,
                                              imuReadProto, data, 1);

    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }

    if (data[0] != IMU_BMX055_ACC_BGW_CHIP_ID_VAL)
    {
        return 1;
    }

    /// Checking registers that have a default reset value != 0x0
    i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                              IMU_BMX055_ACC_SLAVE_ADDR,
                                              IMU_BMX055_ACC_INT0,
                                              imuReadProto,
                                              data, 2);

    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }

    if (data[0] != IMU_BMX055_ACC_INT0_DEF_VAL)
    {
        return 1;
    }

    if (data[1] != IMU_BMX055_ACC_INT1_DEF_VAL)
    {
        return 1;
    }

    /// Testing GYRO
    /// Checking gyro chip id
    i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                              IMU_BMX055_GYRO_SLAVE_ADDR,
                                              IMU_BMX055_GYRO_CHIP_ID,
                                              imuReadProto, data, 1);

    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }

    if (data[0] != IMU_BMX055_GYRO_CHIP_ID_VAL)
    {
        return 2;
    }

    /// Checking registers that have a default reset value != 0x0
    i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                              IMU_BMX055_GYRO_SLAVE_ADDR,
                                              IMU_BMX055_GYRO_HIGH_TH_Z,
                                              imuReadProto, data, 2);

    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }

    if (data[0] != IMU_BMX055_GYRO_HIGH_TH_Z_DEF)
    {
        return 2;
    }
    if (data[1] != IMU_BMX055_GYRO_HIGH_DUR_Z_DEF)
    {
        return 2;
    }

    /// Testing Magnetometer
    // Setting power control bit
    data[0] = 0x1 << IMU_BMX055_MAG_POWER_CONTROL_OFFSET;

    i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                              IMU_BMX055_MAG_SLAVE_ADDR,
                                              IMU_BMX055_MAG_POWER_RESET,
                                              imuWriteProto, data, 1);

    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }

    SleepMs(3);

    /// Checking magneto chip id
    i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                              IMU_BMX055_MAG_SLAVE_ADDR,
                                              IMU_BMX055_MAG_CHIP_ID,
                                              imuReadProto, data, 1);

    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }

    if (data[0] != IMU_BMX055_MAG_CHIP_ID_VAL)
    {
        return 3;
    }

    /// Checking registers that have a default reset value != 0x0
    i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                              IMU_BMX055_MAG_SLAVE_ADDR,
                                              IMU_BMX055_MAG_INT_SET,
                                              imuReadProto, data, 2);

    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }

    if (data[0] != IMU_BMX055_MAG_INT_SET_DEF)
    {
        return 3;
    }
    if (data[1] != IMU_BMX055_MAG_INT_CTRL_DEF)
    {
        return 3;
    }

    return 0;
}

s32 IMUBMX_CODE_SECTION(imuBmx055SelfTest)(u32* test)
{
    u8 data[6];
    s32 i2cTransactionResult;
    test[0] = 0;

    /// Gyro self-testing
    /// Simple check
    i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                              IMU_BMX055_GYRO_SLAVE_ADDR,
                                              IMU_BMX055_GYRO_BIST,
                                              imuReadProto, data, 1);

    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }

    data[0] = data[0] & IMU_BMX055_GYRO_BIST_RATE_OK;
    /// If the simple check is ok we trigger the BIST test
    if (data[0])
    {
        data[0] = IMU_BMX055_GYRO_BIST_TRIG_MASK;
        i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                                  IMU_BMX055_GYRO_SLAVE_ADDR,
                                                  IMU_BMX055_GYRO_BIST,
                                                  imuWriteProto, data, 1);

        if (i2cTransactionResult != I2CM_STAT_OK)
        {
            return i2cTransactionResult;
        }

        SleepMs(3);

        i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                                  IMU_BMX055_GYRO_SLAVE_ADDR,
                                                  IMU_BMX055_GYRO_BIST,
                                                  imuReadProto, data, 1);

        if (i2cTransactionResult != I2CM_STAT_OK)
        {
            return i2cTransactionResult;
        }

        data[0] = data[0] & IMU_BMX055_GYRO_BIST_STAT_MASK;
        if (data[0] == IMU_BMX055_GYRO_BIST_PASS)
        {
            test[0] = test[0] | IMU_BMX055_GYRO_VALID;
        }
    }

    /// Magneto self-testing
    /// Setting the Sleep mode
    data[0] = IMU_BMX055_MAG_SLEEP_MODE;
    i2cTransactionResult = DrvI2cMTransaction(i2cMDevice, IMU_BMX055_MAG_SLAVE_ADDR,
                              IMU_BMX055_MAG_OPMODE, imuWriteProto, data, 1);

    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }

    SleepMs(3);

    /// Initiating normal self-test mode
    i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                              IMU_BMX055_MAG_SLAVE_ADDR,
                                              IMU_BMX055_MAG_OPMODE,
                                              imuReadProto, data, 1);

    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }

    data[1] = IMU_BMX055_MAG_NORMAL_SELF_TEST_BIT;
    data[0] = (data[0] | data[1]);

    i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                              IMU_BMX055_MAG_SLAVE_ADDR,
                                              IMU_BMX055_MAG_OPMODE,
                                              imuWriteProto, data, 1);

    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }

    SleepMs(5);

    i2cTransactionResult = DrvI2cMTransaction(i2cMDevice,
                                              IMU_BMX055_MAG_SLAVE_ADDR,
                                              IMU_BMX055_MAG_X_LSB,
                                              imuReadProto, data, 6);

    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }

    data[0] = data[0] & 0x1;
    data[2] = data[2] & 0x1;
    data[4] = data[4] & 0x1;
    data[0] = data[0] + data[2] + data[4];
    if (data[0] == IMU_BMX055_MAG_SELF_TEST_PASS)
    {
        test[0] = test[0] | IMU_BMX055_MAG_VALID;
    }

    return 0;
}

static void IMUBMX_CODE_SECTION(imuBmxSetConfigRegValues)(imuBmx055ConfigData_t* cfg)
{
    imuBmx055ActualRegisterData = imuBmx055RegDefaultConfiguration;
    switch (cfg->accRange)
    {
    case 2:
        imuBmx055ActualRegisterData.accRange = IMU_BMX055_ACC_RANGE2G;
        break;
    case 4:
        imuBmx055ActualRegisterData.accRange = IMU_BMX055_ACC_RANGE4G;
        break;
    case 8:
        imuBmx055ActualRegisterData.accRange = IMU_BMX055_ACC_RANGE8G;
        break;
    case 16:
        imuBmx055ActualRegisterData.accRange = IMU_BMX055_ACC_RANGE16G;
        break;
    }

    switch (cfg->accSampleRate)
    {
    case 16:
        imuBmx055ActualRegisterData.accSampleRate =
                        IMU_BMX055_ACC_PMU_BW_8HZ;
        break;
    case 32:
        imuBmx055ActualRegisterData.accSampleRate =
                        IMU_BMX055_ACC_PMU_BW_16HZ;
        break;
    case 62:
        imuBmx055ActualRegisterData.accSampleRate =
                        IMU_BMX055_ACC_PMU_BW_32HZ;
        break;
    case 125:
        imuBmx055ActualRegisterData.accSampleRate =
                        IMU_BMX055_ACC_PMU_BW_62HZ;
        break;
    case 250:
        imuBmx055ActualRegisterData.accSampleRate =
                        IMU_BMX055_ACC_PMU_BW_125HZ;
        break;
    case 500:
        imuBmx055ActualRegisterData.accSampleRate =
                        IMU_BMX055_ACC_PMU_BW_500HZ;
        break;
    case 1000:
        imuBmx055ActualRegisterData.accSampleRate =
                        IMU_BMX055_ACC_PMU_BW_1000HZ;
        break;
    }

    switch (cfg->gyroSampleRate)
    {
    case 100:
        imuBmx055ActualRegisterData.gyroSampleRate =
                        IMU_BMX055_GYRO_100HZ_BW_32HZ;
        break;
    case 200:
        imuBmx055ActualRegisterData.gyroSampleRate =
                        IMU_BMX055_GYRO_200HZ_BW_64HZ;
        break;
    case 400:
        imuBmx055ActualRegisterData.gyroSampleRate =
                        IMU_BMX055_GYRO_400HZ_BW_47HZ;
        break;
    case 1000:
        imuBmx055ActualRegisterData.gyroSampleRate =
                        IMU_BMX055_GYRO_1000HZ_BW_116HZ;
        break;
    case 2000:
        imuBmx055ActualRegisterData.gyroSampleRate =
                        IMU_BMX055_GYRO_2000HZ_BW_230HZ;
        break;
    }

    switch (cfg->gyroRange)
    {
    case 2000:
        imuBmx055ActualRegisterData.gyroRange = IMU_BMX055_GYRO_RANGE2000;
        break;
    case 1000:
        imuBmx055ActualRegisterData.gyroRange = IMU_BMX055_GYRO_RANGE1000;
        break;
    case 500:
        imuBmx055ActualRegisterData.gyroRange = IMU_BMX055_GYRO_RANGE500;
        break;
    case 250:
        imuBmx055ActualRegisterData.gyroRange = IMU_BMX055_GYRO_RANGE250;
        break;
    case 125:
        imuBmx055ActualRegisterData.gyroRange = IMU_BMX055_GYRO_RANGE125;
        break;
    }

    switch (cfg->magSampleRate)
    {
    case 2:
        imuBmx055ActualRegisterData.magSampleRate = IMU_BMX055_MAG_RATE2;
        break;
    case 6:
        imuBmx055ActualRegisterData.magSampleRate = IMU_BMX055_MAG_RATE6;
        break;
    case 8:
        imuBmx055ActualRegisterData.magSampleRate = IMU_BMX055_MAG_RATE8;
        break;
    case 10:
        imuBmx055ActualRegisterData.magSampleRate = IMU_BMX055_MAG_RATE10;
        break;
    case 15:
        imuBmx055ActualRegisterData.magSampleRate = IMU_BMX055_MAG_RATE15;
        break;
    case 20:
        imuBmx055ActualRegisterData.magSampleRate = IMU_BMX055_MAG_RATE20;
        break;
    case 25:
        imuBmx055ActualRegisterData.magSampleRate = IMU_BMX055_MAG_RATE25;
        break;
    case 30:
        imuBmx055ActualRegisterData.magSampleRate = IMU_BMX055_MAG_RATE30;
        break;
    }
}

void IMUBMX_CODE_SECTION(imuBmx055SetIsr)(imuBmx055InterruptConfig_t* IntCfg, imuBmxCb_t* imuCb)
{
    /// Record the Callback function
    imuBmxCb = imuCb;

    swcLeonSetPIL(1);
    if (DrvGpioIrqConfigAnyGpio(IntCfg->gpioInt1, IRQ_SOURCE_0, POS_EDGE_INT,
                                IntCfg->handlerPriority,
                                   imuBmxAccRead)
                                   != 0 )
        printf("IRQ ERROR!\n");
    if (DrvGpioIrqConfigAnyGpio(IntCfg->gpioInt3, IRQ_SOURCE_1, POS_EDGE_INT,
                               IntCfg->handlerPriority,
                                   imuBmxGyroRead)
                                   != 0 )
        printf("IRQ ERROR!\n");

}

void IMUBMX_CODE_SECTION(imuBmx055DisableInterrupts)(void)
{
    DrvIcbDisableIrq(IRQ_GPIO_0);
    DrvIcbIrqClear(IRQ_GPIO_0);
    DrvIcbDisableIrq(IRQ_GPIO_1);
    DrvIcbIrqClear(IRQ_GPIO_1);
}

static void imuBmxAccRead(u32 unused)
{
    imuBmxCb(IMU_BMX055_ACCEL);

    if ((imuCount.Bmx055Acc % IMU_BMX055_ONE_MAG_IN) == 0)
    {
        imuBmxCb(IMU_BMX055_MAG);
    }

    DrvIcbIrqClear(IRQ_GPIO_0);
    imuCount.Bmx055Acc++;
}

static void imuBmxGyroRead(u32 unused)
{
    imuBmxCb(IMU_BMX055_GYRO);

    DrvIcbIrqClear(IRQ_GPIO_1);

    imuCount.Bmx055Gyro++;
}

s32 IMUBMX_CODE_SECTION(imuBmxEnterLowPowerState)()
{
    u8 data[2];
    s32 i2cTransactionResult;

    /// Accelerometer enters deep suspend state
    data[0] = 1 << IMU_BMX055_ACC_DEEP_SUSPEND_BIT_OFFSET;
    i2cTransactionResult = DrvI2cMTransaction(i2cMDevice, IMU_BMX055_ACC_SLAVE_ADDR,
                                IMU_BMX055_ACC_REG_PMU_LPW_ADDR,
                                imuWriteProto,
                                data,
                                1);
    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }

    /// Gyro enters deep suspend state
    data[0] = 1 << IMU_BMX055_GYRO_DEEP_SUSPEND_BIT_OFFSET;
    i2cTransactionResult = DrvI2cMTransaction(i2cMDevice, IMU_BMX055_GYRO_SLAVE_ADDR,
                                IMU_BMX055_GYRO_REG_LPM1_ADDR, imuWriteProto,
                                data, 1);
    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }

    /// Magnetometer enters suspend state
    data[0] = 0;
    i2cTransactionResult = DrvI2cMTransaction(i2cMDevice, IMU_BMX055_MAG_SLAVE_ADDR,
                                IMU_BMX055_MAG_POWER_RESET, imuWriteProto, data,
                                1);
    if (i2cTransactionResult != I2CM_STAT_OK)
    {
        return i2cTransactionResult;
    }

    return 0;
}
