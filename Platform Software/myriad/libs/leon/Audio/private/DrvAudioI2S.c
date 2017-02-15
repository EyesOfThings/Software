#include "DrvAudioI2S.h"

#include <DrvGpio.h>
#include <DrvCpr.h>
#include <DrvIcbDefines.h>
#include <DrvIcb.h>
#include <DrvRegUtilsDefines.h>
#include <bsp.h>

static const u32 I2S_TRANSMITTER_BASE = I2S1_BASE_ADR;
static const drvGpioInitArrayType MYRIAD_GPIO_I2S = {
    // sck, ws
    {
        28, 29,
        ACTION_UPDATE_ALL,
        PIN_LEVEL_LOW,
        D_GPIO_MODE_3            |
        D_GPIO_DIR_IN           |
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF,
        D_GPIO_PAD_DEFAULTS
    },
    // in sd0
    {
        30, 30,
        ACTION_UPDATE_ALL,
        PIN_LEVEL_LOW,
        D_GPIO_MODE_3            |
        D_GPIO_DIR_IN            |
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF,
        D_GPIO_PAD_DEFAULTS
    },
    // out sd0
    {
        34, 34,
        ACTION_UPDATE_ALL,
        PIN_LEVEL_LOW,
        D_GPIO_MODE_3            |
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
static I2SIrqParam irqI2S0Handler = {
        .status = 0,
        .irqVector = 0
};

void DrvAudioI2SInit( void (irqCallback)(void* arg) ) {
    if(0 == irqI2S0Handler.status) {
        irqI2S0Handler.irqVector = IRQ_I2S_1;
        irqI2S0Handler.status = 1;
        BSP_Clear_interrupt(irqI2S0Handler.irqVector);
        BSP_Set_interrupt_type_priority(irqI2S0Handler.irqVector,
                                        POS_LEVEL_INT,
                                        8);
        BSP_interrupt_register(irqI2S0Handler.irqVector,
                               "I2S0",
                               irqCallback,
                               &irqI2S0Handler);
    }
    DrvGpioInitialiseRange(MYRIAD_GPIO_I2S);

    // reset I2S module
    SET_REG_WORD( I2S_TRANSMITTER_BASE+I2S_IER, 0 );
    SET_REG_WORD( I2S_TRANSMITTER_BASE+I2S_IER, 1 );

    // set WWS to 16 clocks and No SCLKG
    SET_REG_WORD( I2S_TRANSMITTER_BASE+I2S_CCR, 0 ); // 00 000b
    // set trigger level
    SET_REG_WORD( I2S_TRANSMITTER_BASE+I2S_TFCR0, 4 );
    SET_REG_WORD( I2S_TRANSMITTER_BASE+I2S_RFCR0, 12);
    // channel data resolution: 16 bit
    SET_REG_WORD( I2S_TRANSMITTER_BASE+I2S_TCR0, 2 ); // 010b
    SET_REG_WORD( I2S_TRANSMITTER_BASE+I2S_RCR0, 2 );

    // flush and enable transmitter block again
    SET_REG_WORD( I2S_TRANSMITTER_BASE+I2S_TXFFR, 1);

    // fill buffer
    for(int i = 0; i < 16; ++i) {
        // write left stereo data LTHR0
        SET_REG_HALF( I2S_TRANSMITTER_BASE+I2S_LTHR0, 0 );
        // write right stereo data RTHR0
        SET_REG_HALF( I2S_TRANSMITTER_BASE+I2S_RTHR0, 0 );
    }

    // enable transmitter channel
    SET_REG_WORD( I2S_TRANSMITTER_BASE+I2S_TER0, 1 );
    SET_REG_WORD( I2S_TRANSMITTER_BASE+I2S_TER1, 0 );
    SET_REG_WORD( I2S_TRANSMITTER_BASE+I2S_TER2, 0 );
    SET_REG_WORD( I2S_TRANSMITTER_BASE+I2S_ITER, 1 );

    // flush and enable receiver block again
    SET_REG_WORD( I2S_TRANSMITTER_BASE+I2S_RXFFR, 1 );
    SET_REG_WORD( I2S_TRANSMITTER_BASE+I2S_RER0, 1 );
    SET_REG_WORD( I2S_TRANSMITTER_BASE+I2S_RER1, 0 );
    SET_REG_WORD( I2S_TRANSMITTER_BASE+I2S_RER2, 0 );
    SET_REG_WORD( I2S_TRANSMITTER_BASE+I2S_IRER, 1 );

    // slave mode
    SET_REG_WORD( I2S_TRANSMITTER_BASE+I2S_CER, 0 );
}

void DrvAudioI2SEnableFIFOEmptyInterrupt() {
    int interruptMaskRegister = 0x33;
    GET_REG_WORD(I2S_TRANSMITTER_BASE+I2S_IMR0, interruptMaskRegister );
    interruptMaskRegister = interruptMaskRegister & 0x23;
    SET_REG_WORD( I2S_TRANSMITTER_BASE+I2S_IMR0, interruptMaskRegister ); // x000xxb
}
void DrvAudioI2SDisableFIFOEmptyInterrupt() {
    int interruptMaskRegister = 0x33;
    GET_REG_WORD(I2S_TRANSMITTER_BASE+I2S_IMR0, interruptMaskRegister );
    interruptMaskRegister = interruptMaskRegister | 0x10;
    SET_REG_WORD( I2S_TRANSMITTER_BASE+I2S_IMR0, interruptMaskRegister ); // x100xxb
}
void DrvAudioI2SEnableFIFODataAvailableInterrupt() {
    int interruptMaskRegister = 0x33;
    GET_REG_WORD(I2S_TRANSMITTER_BASE+I2S_IMR0, interruptMaskRegister );
    interruptMaskRegister = interruptMaskRegister & 0x32;
    SET_REG_WORD( I2S_TRANSMITTER_BASE+I2S_IMR0, interruptMaskRegister ); // xx00x0b
}
void DrvAudioI2SDisableFIFODataAvailableInterrupt() {
    int interruptMaskRegister = 0x33;
    GET_REG_WORD(I2S_TRANSMITTER_BASE+I2S_IMR0, interruptMaskRegister );
    interruptMaskRegister = interruptMaskRegister | 0x01;
    SET_REG_WORD( I2S_TRANSMITTER_BASE+I2S_IMR0, interruptMaskRegister ); // xx00x1b
}
void DrvAudioI2SAddAudioData(int16_t leftChannel, int16_t rightChannel) {
    // write left stereo data LTHR0
    SET_REG_HALF( I2S_TRANSMITTER_BASE+I2S_LTHR0, leftChannel  );
    // write right stereo data RTHR0
    SET_REG_HALF( I2S_TRANSMITTER_BASE+I2S_RTHR0, rightChannel );
}
void DrvAudioI2SGetAudioData(int16_t* leftChannel, int16_t* rightChannel) {
    // read left stereo data LRHR0
    GET_REG_HALF( I2S_TRANSMITTER_BASE+I2S_LRBR0, *leftChannel );
    // read right stereo data RRHR0
    GET_REG_HALF( I2S_TRANSMITTER_BASE+I2S_RRBR0, *rightChannel );
}

bool DrvAudioI2SIsEmpty() {
    u32 status = 0;
    GET_REG_WORD( I2S_TRANSMITTER_BASE+I2S_ISR0, status );
    return status & 0x10;
}
bool DrvAudioI2SIsDataAvailable() {
    u32 status = 0;
    GET_REG_WORD( I2S_TRANSMITTER_BASE+I2S_ISR0, status );
    return status & 0x01;
}
