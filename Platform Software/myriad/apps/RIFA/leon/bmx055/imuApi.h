///
/// @file imuApi.h
/// @copyright All code copyright Movidius Ltd 2013, all rights reserved.
/// For License Warranty see: common/license.txt
///
/// @brief
///

#ifndef IMUAPI_H_
#define IMUAPI_H_

/// System Includes
/// ----------------------------------------------------------------------------
#include "mv_types.h"
#include "DrvI2cMasterDefines.h"
#include "DrvTimer.h"

/// Application Includes
/// ----------------------------------------------------------------------------
#include "imuApiDefines.h"

/// Source Specific #defines and types (typedef,enum,struct)
/// ----------------------------------------------------------------------------
/// Accel registers address
#define IMU_BMX055_ACC_BGW_CHIP_ID 0x00
#define IMU_BMX055_GYRO_CHIP_ID 0x00
#define IMU_BMX055_MAG_POWER_RESET 0x4B
#define IMU_BMX055_MAG_CHIP_ID 0x40
#define IMU_BMX055_MAG_INT_SET 0x4D

// Global function prototypes
// ----------------------------------------------------------------------------
/**
* Component Brief:
* This component handles the BMX055 sensor data.
* The Accel and Gyro data are read when their data ready interrupt occurs.
* The Magnetometer data is read every 4th Accel sample during the Accel data ready interrupt.
*
* Durations:
* For one simple Accel or Gyro sample the interrupt takes ~300 us
* For Accel + Magneto and Gyro + Baro the interrupt take ~ 570 us
* The total interrupt duration of all the interrupts, with the sensors set with default values, is ~80000 us per second
*/


/**
* @brief Initializes the i2c communication and the IMU chip
* @param[in] dev1 i2c device handler to be used
* @param[in] Cfg the struct with the values to configure the Imu chip
* @return 0 if successful, or the appropriate i2c error code if unsuccessful.
*/
s32 imuComponentInit(I2CM_Device* i2cDevice, imuBmx055ConfigData_t* cfg);


/**
* @brief Reads a data packet from the IMU chip
* @param[in] type the type of packet to read
* @param[out] read the struct in which to save the imu data
* @return 0 if successful, or the appropriate error code if unsuccessful.
*/
s32 imuGetDataPacket(imuType_t imuBmxtype, imuPacket_t *read);

/**
* @brief Performs the sensor integrated self-tests. On success/error, the self-test returns a mask
* representing the sensor(s)that passed.
* For each bit, a one (1) represents a "pass" case; conversely,
* a zero (0) indicates a failure.
* The mask is defined as follows:
* ACCEL_VALID 0x0001 (self test not implemented)
* GYRO_VALID 0x0002
* MAG_VALID 0x0004
* @param[out] test the mask that will tell which sensor passed/failed the test
* @return 0 if successful, or the appropriate error code if unsuccessful.
*/
s32 imuBmx055SelfTest(u32* test);

/**
* @brief A simple I2C communication verification
* @param[in] dev1 i2c device handler to be used
* @return 0 if successful, or the appropriate i2c error code if unsuccessful.
*/
s32 imuI2cTest(I2CM_Device* dev1);

/**
* @brief This function configures and starts the Leon interrupt handlers to watch for the Bmx055 interrupt pins.
* Once a data sample is collected it is sent as a parameter to the function given at DataPlacer.
* @param[in] IntCfg the config struct that has the numbers of the gpio's to which the int1, int3 and int5 are connected
* and the priority of this interrupts in leon (same for all)
* @param[out] pointer to callback function that will handle the data packets
* @return void
*/
void imuBmx055SetIsr(imuBmx055InterruptConfig_t* IntCfg, imuBmxCb_t* imuCb);

/**
* @brief The function to clear and disable the interrupt handlers. Call this to stop the leon from looking for imu interrupts
* @return void
*/
void imuBmx055DisableInterrupts(void);

/**
* @brief This function sets the Bmx055 to one of the following modes: DeepSuspend, Normal.
* After entering normal mode, all register configuration will be lost.
* @param[in] PowerMode the mode in which the sensor will enter
* @return The function returns 0 in case of success.
*/
s32 imuBmxEnterLowPowerState(void);

#endif /* IMUAPI_H_ */
