#include "DrvAudioI2C.h"

#include <DrvI2cMaster.h>
#include <brdMv0182Defines.h>
#include <OsDrvGpio.h>
#include <OsDrvCprDefines.h>
#include <OsDrvCpr.h>
#include <assert.h>

#define TLV_SLAVE_ADDRESS 0x18
#define MAX_VOLUME_VALUE 48
#define MIN_VOLUME_VALUE -126

static I2CM_Device i2cDevice = { 0 };
static u8 writeProtocol[] = I2C_PROTO_WRITE_8BA;
static u8 readProtocol[]  = I2C_PROTO_READ_8BA;
static tyI2cConfig i2c2MasterCfg = {
    .device = IIC3_DEVICE,              // I2C device / connection
    .sclPin = MV0182_I2C2_SCL_PIN,      // GPIO pin used for Serial Clock Line of I2C
    .sdaPin = MV0182_I2C2_SDA_PIN,      // GPIO pin used for Source Data Line of I2C
    .speedKhz = MV0182_I2C2_SPEED_KHZ_DEFAULT,
    .addressSize = MV0182_I2C2_ADDR_SIZE_DEFAULT
};

static const drvGpioInitArrayType MYRIAD_GPIO_AUDIO_CHIP = {
    {
        81, 81,
        ACTION_UPDATE_ALL,
        PIN_LEVEL_HIGH,
        D_GPIO_MODE_2            |
        D_GPIO_DIR_OUT           |
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF,
        D_GPIO_PAD_DEFAULTS
    },
    {
        0, 0,
        ACTION_TERMINATE_ARRAY,      // Do nothing but simply terminate the Array
        PIN_LEVEL_LOW,               // Won't actually be updated
        D_GPIO_MODE_0,               // Won't actually be updated
        D_GPIO_PAD_DEFAULTS          // Won't actually be updated
    },
};


static void isCprClk1Valid( void ) {
    tyClockConfig cpr_clk_1;
    int freq = DrvCprGetClockFreqKhz(AUX_CLK_GPIO1, &cpr_clk_1);
    // AUX_CLK_GPIO1 must be 12Mhz
    assert(12000 == freq || 0 == freq);
    if(0 == freq) {
        // try to set the correct clock frequency
        tyAuxClkDividerCfg auxClkGPIO1[] = {
            // GPIO1: 12MHz
            {
                .auxClockEnableMask     = AUX_CLK_MASK_GPIO1,
                .auxClockSource         = CLK_SRC_REFCLK0,
                .auxClockDivNumerator   = 0,
                .auxClockDivDenominator = 0,
            },
            {0,0,0,0}, // Null Terminated List
        };
        OsDrvCprAuxClockArrayConfig(auxClkGPIO1);
        freq = DrvCprGetClockFreqKhz(AUX_CLK_GPIO1, &cpr_clk_1);
    }
    assert(12000 == freq);
}

/**
 * Initialize the I2C unit #3.
 */
static void DrvAudioI2CInit( void ) {
    // do it only once
    if (i2cDevice.i2cDeviceAddr == 0) {
        isCprClk1Valid();
        // pass the CPR_CLK_1 to the audio chip
        DrvGpioInitialiseRange(MYRIAD_GPIO_AUDIO_CHIP);
        // Initialize I2C device in master configuration
        DrvI2cMInitFromConfig(&i2cDevice, &i2c2MasterCfg);
    }
}

static int WriteToRegister( u32 registerAddress, u8 value ) {
    I2CM_StatusType result;
    result = DrvI2cMTransaction( &i2cDevice,
                        TLV_SLAVE_ADDRESS,
                        registerAddress,
                        writeProtocol,
                        &value,
                        1 );  // write 1 byte
    return result;
}
static int ReadFromRegister( u32 registerAddress, u8* value ) {
    I2CM_StatusType result;
    result = DrvI2cMTransaction( &i2cDevice,
                        TLV_SLAVE_ADDRESS,
                        registerAddress,
                        readProtocol,
                        value,
                        1 );  // read 1 byte
    return result;
}
static int SelectPage( u8 page ) {
    return WriteToRegister( 0x00, page );
}

static void ResetAudioChip( void ) {
    // Set register page to 0
    SelectPage( 0 );
    // Initiate SW Reset
    WriteToRegister( 0x01, 0x01 );
    usleep(2000);
}
static void SetupPLLForHighSampleRatePlayback( void ) {
    // Program PLL clock dividers J, D, R, P                    //
    // Set PLL_CLKIN=MCLK(=12MHz), CODEC_CLKIN=PLL_CLK          //
    WriteToRegister( 0x04, 0x03 );  //   3 => 00000011b         //
    // PLL J=7                                                  //
    WriteToRegister( 0x06, 0x07 );  //   7 => 00000111b         //
    // PLL D = 1680 => 0x0690                                   //
    WriteToRegister( 0x07, 0x06 );  // (MSB: 0x06)              //
    WriteToRegister( 0x08, 0x90 );  // (LSB: 0x90)              //
    // Power up PLL, P=1, R=1                                   //
    WriteToRegister( 0x05, 0x91 );  // 145 => 10010001b         //
    usleep(10000);
    // Program and power up NDAC=2                              //
    WriteToRegister( 0x0B, 0x82 );  //   2 => 10000010b         //
    // Program and power up MDAC=7                              // Program Clock Settings
    WriteToRegister( 0x0C, 0x87 );  //   2 => 10000111b         //
    // Program OSR = 128 => 0x0080                              //
    WriteToRegister( 0x0D, 0x00 );  // (MSB: 0x00)              //
    WriteToRegister( 0x0E, 0x80 );  // (LSB: 0x80)              //
    // BCLK_IN = DAC_MOD_CLK == 6.144 MHz                       //
    WriteToRegister( 0x1D, 0x01 );                              //
    // BCLK N = 4 --> 6.144 MHz / 4 == 1.536 MHz == 32 x 48 kHz //
    WriteToRegister( 0x1E, 0x84 );                              //
}
static void SetupPLLForNormalSampleRatePlayback( void ) {
    // Program PLL clock dividers J, D, R, P                    //
    // Set PLL_CLKIN=MCLK(=12MHz), CODEC_CLKIN=PLL_CLK          //
    WriteToRegister( 0x04, 0x03 );  //   3 => 00000011b         //
    // PLL J=7                                                  //
    WriteToRegister( 0x06, 0x07 );  //   7 => 00000111b         //
    // PLL D = 560 => 0x0230                                   //
    WriteToRegister( 0x07, 0x02 );  // (MSB: 0x02)              //
    WriteToRegister( 0x08, 0x30 );  // (LSB: 0x30)              //
    // Power up PLL, P=1, R=1                                   //
    WriteToRegister( 0x05, 0x91 );  // 145 => 10010001b         //
    usleep(10000);
    // Program and power up NDAC=5                              //
    WriteToRegister( 0x0B, 0x85 );  //   2 => 10000101b         //
    // Program and power up MDAC=3                              // Program Clock Settings
    WriteToRegister( 0x0C, 0x83 );  //   2 => 10000011b         //
    // Program OSR = 128 => 0x0080                              //
    WriteToRegister( 0x0D, 0x00 );  // (MSB: 0x00)              //
    WriteToRegister( 0x0E, 0x80 );  // (LSB: 0x80)              //
    // BCLK_IN = DAC_MOD_CLK == 5.6448 MHz                      //
    WriteToRegister( 0x1D, 0x01 );                              //
    // BCLK N = 4 --> 5.6448 MHz/4==1.4112 MHz == 32x44,1 kHz   //
    WriteToRegister( 0x1E, 0x84 );                              //
}
static void SetupPLLForHighSampleRateRecord( void ) {
    // Program PLL clock dividers J, D, R, P
    // Set PLL_CLKIN=MCLK(=12MHz), CODEC_CLKIN=PLL_CLK
    WriteToRegister( 0x04, 0x03 );  //   3 => 00000011b
    // PLL J=7
    WriteToRegister( 0x06, 0x07 );  //   7 => 00000111b
    // PLL D = 1680 => 0x0690
    WriteToRegister( 0x07, 0x06 );  // (MSB: 0x06)
    WriteToRegister( 0x08, 0x90 );  // (LSB: 0x90)
    // Power up PLL, P=1, R=1
    WriteToRegister( 0x05, 0x91 );  // 145 => 10010001b
    usleep(10000);
    // Program and power up NDAC=2
    WriteToRegister( 0x0B, 0x82 );  //   2 => 10000010b
    // Program and power up MDAC=7
    WriteToRegister( 0x0C, 0x87 );  //   2 => 10000111b
    // Program OSR = 128 => 0x0080
    WriteToRegister( 0x0D, 0x00 );  // (MSB: 0x00)
    WriteToRegister( 0x0E, 0x80 );  // (LSB: 0x80)
    // Program and power up NADC=7
    WriteToRegister( 0x12, 0x87 );  //   2 => 10000111b
    // Program and power up MADC=2
    WriteToRegister( 0x13, 0x82 );  //   2 => 10000010b
    // Program OSR = 128 => 0x80
    WriteToRegister( 0x14, 0x80 );
    // BCLK_IN = DAC_MOD_CLK == 6.144 MHz
    WriteToRegister( 0x1D, 0x01 );
    // BCLK N = 4 --> 6.144 MHz / 4 == 1.536 MHz == 32 x 48 kHz
    WriteToRegister( 0x1E, 0x84 );
}

static void SetUpWordLength() {
    // Mode=I2S, Word length=16, BCLK, WCLK are outputs         //
    WriteToRegister( 0x1B, 0x0D );  //  13 => 00001101b         //
}
static void SetUpPRBP() {
    // DAC PRB set to PRB_P1                                    //
    WriteToRegister( 0x3C, 0x01 );  //   1 => 00000001b         //
}
static void SetUpPRBR() {
    // ADC PRB set to PRB_R1                                    //
    WriteToRegister( 0x3D, 0x01 );  //   1 => 00000001b         //
}
static void PowerUpPlaybackDriver() {
    // Set register Page to 1
    SelectPage( 0x01 );
    // Disable coarse AV DD generation
    WriteToRegister( 0x01, 0x08 );
    // Enable Master Analog Power Control
    WriteToRegister( 0x02, 0xA1 );
    // Program Common Mode voltage 0.9V
    WriteToRegister( 0x0A, 0x03 );
    // Program PowerTune (PTM) mode PTM_P1
    WriteToRegister( 0x03, 0x02 ); //0x08
    WriteToRegister( 0x04, 0x02 ); //0x08
    // Program Reference fast charging
    WriteToRegister( 0x7B, 0x01 );
    // Program Headphone specific depop settings (in case of headphone driver used)
    WriteToRegister( 0x14, 0x25 );
    // Program routing of DAC output to the output amplifier (headphone or line out)
    // Route Left DAC to HPL
    WriteToRegister( 0x0C, 0x08 );
    // Route Right DAC to HPR
    WriteToRegister( 0x0D, 0x08 );
    // Unmute and set gain of output driver
    // Set the HPL gain to 0dB and unmute
    WriteToRegister( 0x10, 0x00 );
    // Set the HPR gain to 0dB and unmute
    WriteToRegister( 0x11, 0x00 );
    // Power up output driver
    // Power up HPL and HPR drivers
    WriteToRegister( 0x09, 0x30 );
    //read Page 1, Register 63d, D(7:6). When = “11” soft-stepping is complete
    u8 registerValue = 0;
    do {
        // read Page 1, Register 63d, D(7:6).
        // When = “11” soft-stepping is complete
        ReadFromRegister( 63, &registerValue );
    } while( ( registerValue & 0xC0 ) != 0xC0 ); // 11000000b
}
static void PowerUpRecordDriver() {
   // Set register Page to 1
   SelectPage( 0x01 );
   // Disable coarse AV DD generation
   WriteToRegister( 0x01, 0x08 );
   // Enable Master Analog Power Control
   WriteToRegister( 0x02, 0xA1 ); // ? 0x00 // A1 --> 1010 0001b
   // Program Common Mode voltage 0.9V
   WriteToRegister( 0x0A, 0x03 ); // ? 0x00
   // Program PowerTune (PTM) mode PTM_R2
   WriteToRegister( 0x3D, 0xB6 );
   // Set MicPGA startup delay to 6.4ms
   WriteToRegister( 0x47, 0x32 );  // 0x32 --> 0011 0010b
   // Program Reference fast charging
   WriteToRegister( 0x7B, 0x01 );  // 40 ms --> 001b
   // Route IN3L to LEFT_P with 10K input impedance
   WriteToRegister( 0x34, 0x00 ); // 0x20 --> 0010 0000b // external mic. 0x00
   // Route Common Mode to LEFT_M with impedance of 10K
   WriteToRegister( 0x36, 0x40 );
   // Route IN3R to RIGHT_P with input impedance of 10K
   WriteToRegister( 0x37, 0x04 );
   // SCLK/MFP3 control register: disable MFP3 pin
   WriteToRegister( 0x38, 0x00 );
   // Route Common Mode to RIGHT_M with impedance of 10K
   WriteToRegister( 0x39, 0x40 );
   // Unmute Left MICPGA, Channel Gain of 18dB
   WriteToRegister( 0x3B, 0x00 ); // 0x0C --> 0000 1100b
   // Unmute Right MICPGA, Channel Gain of 18dB
   WriteToRegister( 0x3C, 0x12 );
   // MIC BIAS: power up && set 1.7V
   WriteToRegister( 0x33, 0x58);
}
static void EnableHeadphones() {
    //Set register Page to 0
    SelectPage( 0x00 );
    //Power up DAC Channels
    // Set DAC volume to 0dB
    WriteToRegister( 0x41, 0x00 ); // 0.0dB
    WriteToRegister( 0x42, 0x00 ); // 0.0dB
    // Power up the Left and Right DAC Channels with route the Left Audio digital data to
    // Left Channel DAC and Right Audio digital data to Right Channel DAC
    WriteToRegister( 0x3F, 0xD6 );
    // Unmute digital volume control
    // Unmute the DAC digital volume control
    WriteToRegister( 0x40, 0x00 );
}
static void EnableMicrophone() {
    // Set register Page to 0
    SelectPage( 0x00 );
    // Power up Left and Right ADC Channels
    // Set DAC volume to 0dB
    WriteToRegister( 0x51, 0xC0 ); // 0.0dB
    // Unmute Left and Right ADC Digital Volume Control.
    WriteToRegister( 0x52, 0x00 );
}

void DrvAudioI2CPlaybackMode( bool highSamplingRateMode ) {
    DrvAudioI2CInit();
    ResetAudioChip();
    if( highSamplingRateMode ) {
        SetupPLLForHighSampleRatePlayback();
    }
    else {
        SetupPLLForNormalSampleRatePlayback();
    }

    SetUpWordLength();

    SetUpPRBP();
    PowerUpPlaybackDriver();

    EnableHeadphones();
}

void DrvAudioI2CRecordMode( void ) {
    DrvAudioI2CInit();
    ResetAudioChip();

    SetupPLLForHighSampleRateRecord();
    SetUpWordLength();

    SetUpPRBR();
    PowerUpRecordDriver();

    EnableMicrophone();
}

void DrvAudioI2CVolume(float volume) {
    signed char volumeValue = MIN_VOLUME_VALUE*(1.0f-volume) + MAX_VOLUME_VALUE*volume;
    SelectPage( 0x00 );
    WriteToRegister( 0x41, volumeValue );
    WriteToRegister( 0x42, volumeValue );
}
