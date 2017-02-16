#include "DrvEeprom.h"

#include <stdlib.h>
#include <math.h>
#include <registersMyriad.h>
#include <DrvCpr.h>
#include <DrvGpio.h>
#include <DrvRegUtils.h>
#include <DrvSpiDefines.h>

static const u32 SPI_PERIPHERAL = SPI1;
static const u32 BASE_ADDRESS = SPI1_BASE_ADR;
static const u64 SPI_CSS_CLK_MASK = DEV_CSS_SPI0;
static const u32 SPI_SLAVE_SELECT_PIN = 77;

static const drvGpioInitArrayType MYRIAD2_GPIOs =
{
    {74,76  , ACTION_UPDATE_ALL
            ,
              PIN_LEVEL_HIGH
            ,
              D_GPIO_MODE_0            |  // Mode0 SPI1 (mosi, miso, sck)
              D_GPIO_DIR_OUT           |  // Drive out
              D_GPIO_DATA_INV_OFF      |
              D_GPIO_WAKEUP_OFF
            ,
              D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
    },
    {77,77  , ACTION_UPDATE_ALL
            ,
              PIN_LEVEL_HIGH
            ,
              D_GPIO_MODE_7            |  // Mode0 SPI1 (slave select, for the myriad2 backend needs to be mode 7)
              D_GPIO_DIR_OUT           |  // Drive out
              D_GPIO_DATA_INV_OFF      |
              D_GPIO_WAKEUP_OFF
            ,
              D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
    },
    {0,0    , ACTION_TERMINATE_ARRAY      // Do nothing but simply terminate the Array
            ,
              PIN_LEVEL_LOW               // Won't actually be updated
            ,
              D_GPIO_MODE_0               // Won't actually be updated
            ,
              D_GPIO_PAD_DEFAULTS         // Won't actually be updated
    },
};

void DrvEepromInit() {
    // SPI clock in Mhz
    const float SPI_CLK = DrvCprGetSysClockKhz() / 2000.0f;
    // baud rate should be around 15 MHz
    int clkDivider = (int) ceilf( SPI_CLK / 15.0f );
    // clk divider has to be even
    clkDivider += (clkDivider % 2);
    // clock and reset SPI
    DrvCprSysDeviceAction( CSS_DOMAIN, ENABLE_CLKS, SPI_CSS_CLK_MASK );
    DrvCprSysDeviceAction( CSS_DOMAIN, PULSE_RESET, SPI_CSS_CLK_MASK );

    // Configure SPI block as master
    SET_REG_WORD(
            CPR_GEN_CTRL_ADR,
            GET_REG_WORD_VAL(CPR_GEN_CTRL_ADR) | (1 << (SPI_PERIPHERAL + 4)) );

    DrvGpioSetPinHi( SPI_SLAVE_SELECT_PIN );
    // Disable SPI block
    SET_REG_WORD( BASE_ADDRESS + SPI_SSIENR_OFFSET, 0 );
    // Set baud rate = ( SPI CLK / CLK divider ) => should be around 15 MHz
    SET_REG_WORD( BASE_ADDRESS + SPI_BAUDR_OFFSET, clkDivider );
    // Disable DMA, to avoid problems, as the DMA controller might not even be clocked
    SET_REG_WORD( BASE_ADDRESS + SPI_DMACR_OFFSET, 0 );
    // disable all slave selects, so that no transfer will start before we put the control and address bytes into the TX FIFO:
    SET_REG_WORD( BASE_ADDRESS + SPI_SER_OFFSET, 0 );
    // Init GPIO pins
    DrvGpioInitialiseRange( MYRIAD2_GPIOs );
}

static void DrvEepromSendByte( const u8 byte ) {
    // wait for the transmit FIFO to not be full
    while( !(GET_REG_WORD_VAL(BASE_ADDRESS + SPI_SR_OFFSET) & SPI_SR_TFNF) )
        ;
    SET_REG_WORD( BASE_ADDRESS + SPI_DR_OFFSET, byte );
}

void DrvEepromSendAndReceive( const u8* commandBuffer,
                              const u32 commandBufferSize,
                              const u8 numberOfDummyBytes,
                              const u8* writeBuffer,
                              const u32 writeBufferSize,
                              u8* readBuffer,
                              const u32 readBufferSize ) {
    // enable flash memory
    DrvGpioSetPinLo( SPI_SLAVE_SELECT_PIN );

    SET_REG_WORD( BASE_ADDRESS + SPI_SSIENR_OFFSET, 0 );
    SET_REG_WORD( BASE_ADDRESS + SPI_CTRLR0_OFFSET, SPI_CTRLR0_TMOD_TX_ONLY | // Transmit only
                  SPI_CTRLR0_SRL_NORMAL |// Normal operation
                  SPI_CTRLR0_FRF_MOT_SPI |// Frame Format = Motorola SPI
                  SPI_ONE_BYTE_FRAME_SIZE// Data Frame Size = 8 bits
    );
    // select any slave to start the transfer. Doesn't matter which one, since we control the slave line via a GPIO.
    SET_REG_WORD( BASE_ADDRESS + SPI_SER_OFFSET, 1 );
    SET_REG_WORD( BASE_ADDRESS + SPI_SSIENR_OFFSET, 1 );

    u32 i;
    for( i = 0; i < commandBufferSize; ++i ) {
        DrvEepromSendByte( commandBuffer[i] );
    }
    // during the following dummy bytes no valid data gets returned:
    for( i = 0; i < numberOfDummyBytes; ++i ) {
        DrvEepromSendByte( 0xFF );
    }
    // transmit write data
    for( i = 0; i < writeBufferSize; ++i ) {
        DrvEepromSendByte( writeBuffer[i] );
    }
    // wait for the transfer to finish
    while( ((GET_REG_WORD_VAL(BASE_ADDRESS + SPI_SR_OFFSET) & SPI_SR_TFE) == 0) ||
            ((GET_REG_WORD_VAL(BASE_ADDRESS + SPI_SR_OFFSET ) & SPI_SR_BUSY) != 0) );
    // now switch to transmit & receive:
    SET_REG_WORD( BASE_ADDRESS + SPI_SSIENR_OFFSET, 0 );
    SET_REG_WORD( BASE_ADDRESS + SPI_CTRLR0_OFFSET, SPI_CTRLR0_TMOD_TX_RX | // Transmit and receive.
                  SPI_CTRLR0_SRL_NORMAL | // Normal operation
                  SPI_CTRLR0_FRF_MOT_SPI | // Frame Format = Motorola SPI
                  SPI_ONE_BYTE_FRAME_SIZE // Data Frame Size = 8 bits
    );
    SET_REG_WORD( BASE_ADDRESS + SPI_SSIENR_OFFSET, 1 );

    u32 dummyBytesSent = 0;
    u32 outstandingBytes = 0;
    for( i = 0; i < readBufferSize; ++i ) {
        while( dummyBytesSent < readBufferSize &&
               outstandingBytes < SPI_RX_FIFO_DEPTH &&
               (GET_REG_WORD_VAL(BASE_ADDRESS + SPI_SR_OFFSET) & SPI_SR_TFNF) ) {
            DrvEepromSendByte( 0xFF ); // write dummy byte
            ++dummyBytesSent;
            ++outstandingBytes;
        }

        // wait for the receive FIFO to be not empty
        while( !(GET_REG_WORD_VAL(BASE_ADDRESS + SPI_SR_OFFSET) & SPI_SR_RFNE) );

        readBuffer[i] = GET_REG_WORD_VAL( BASE_ADDRESS + SPI_DR_OFFSET );
        --outstandingBytes;
    }
    // disable flash memory
    DrvGpioSetPinHi( SPI_SLAVE_SELECT_PIN );
}

void DrvEepromPowerUp( void ) {
    u8 powerUpCommand[] = { 0xAB };
    DrvEepromSendAndReceive( powerUpCommand, 1, 0, NULL, 0, NULL, 0 );
}

void DrvEepromPowerDown( void ) {
    u8 powerDownCommand[] = { 0xB9 };
    DrvEepromSendAndReceive( powerDownCommand, 1, 0, NULL, 0, NULL, 0 );
}

void DrvEepromSetWriteEnable( void ) {
    u8 writeEnableCommand[] = { 0x06 };
    DrvEepromSendAndReceive( writeEnableCommand, 1, 0, NULL, 0, NULL, 0 );
}
void DrvEepromSetWriteDisable( void ) {
    u8 writeDisableCommand[] = { 0x04 };
    DrvEepromSendAndReceive( writeDisableCommand, 1, 0, NULL, 0, NULL, 0 );
}

void DrvEepromWriteStatusRegister( const u8 statusRegister ) {
    u8 writeStatusRegisterCommand[] = { 0x01 };
    DrvEepromSendAndReceive( writeStatusRegisterCommand, 1, 0, &statusRegister,
                             1, NULL, 0 );
}

u8 DrvEepromReadStatusRegister( void ) {
    u8 statusRegister;
    u8 readStatusRegisterCommand[] = { 0x05 };
    DrvEepromSendAndReceive( readStatusRegisterCommand, 1, 0, NULL, 0,
                             &statusRegister, 1 );
    return statusRegister;
}

void DrvEepromWaitWriteInProgress( void ) {
    while( DrvEepromReadStatusRegister() & 1 );
}

void DrvEepromChipErase( void ) {
    u8 chipEraseCommand[] = { 0xC7 };
    DrvEepromSendAndReceive( chipEraseCommand, 1, 0, NULL, 0, NULL, 0 );
}

void DrvEepromBlockErase( const u32 address ) {
    u8 blockEraseCommand[] = { 0xD8,
                               (u8) (address >> 16),
                               (u8) (address >> 8),
                               (u8) (address/*>>0*/) };
    DrvEepromSendAndReceive( blockEraseCommand, 4, 0, NULL, 0, NULL, 0 );
}

void DrvEepromSectorErase( const u32 address ) {
    u8 sectorEraseCommand[] = { 0x20,
                                (u8) (address >> 16),
                                (u8) (address >> 8),
                                (u8) (address/*>>0*/) };
    DrvEepromSendAndReceive( sectorEraseCommand, 4, 0, NULL, 0, NULL, 0 );
}

void DrvEepromReadId( u8* readIdBuffer, const u8 readIdBufferSize ) {
    u8 readIdCommand[] = { 0x9F };
    DrvEepromSendAndReceive( readIdCommand, 1, 0, NULL, 0, readIdBuffer,
                             readIdBufferSize );
}

void DrvEepromFastRead( const u32 address, u8* readBuffer,
                        const u32 readBufferSize ) {
    u8 fastReadCommand[] = { 0x0B,
                             (u8) (address >> 16),
                             (u8) (address >> 8),
                             (u8) (address/*>>0*/) };
    DrvEepromSendAndReceive( fastReadCommand, 4, 1, NULL, 0, readBuffer,
                             readBufferSize );
}

void DrvEepromPageProgram( const u32 address, const u8* writeBuffer,
                           const u32 writeBufferSize ) {
    u8 pageProgramCommand[] = { 0x02,
                                (u8) (address >> 16),
                                (u8) (address >> 8),
                                (u8) (address/*>>0*/) };
    DrvEepromSendAndReceive( pageProgramCommand, 4, 0, writeBuffer,
                             writeBufferSize, NULL, 0 );
}

void DrvEepromClose() {
    // Disable SPI block
    SET_REG_WORD( BASE_ADDRESS + SPI_SSIENR_OFFSET, 0 );
}
