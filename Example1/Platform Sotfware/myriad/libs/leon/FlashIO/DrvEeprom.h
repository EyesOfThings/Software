#ifndef _DRV_EEPROM_H_
#define _DRV_EEPROM_H_

#include <mv_types.h>

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * Initialize the SPI device 0 as master and configure it for communication with
     * the SPI EEPROM chip.
     */
    void DrvEepromInit( void );
    /**
     * Send a command to and receive the response from the EEPROM chip.
     * Maximum transfer size: 64 KBytes.
     *
     * @param[in]  commandBuffer A pointer to a buffer containing the command bytes.
     * @param[in]  commandBufferSize The number of bytes to transmit.
     * @param[in]  numberOfDummyBits The number of bytes.
     * @param[in]  writeBuffer A pointer to a buffer containing the write bytes.
     * @param[in]  writeBufferSize The number of bytes to transmit.
     * @param[out] readBuffer A pointer to the buffer to which the received data is
     *     written to.
     * @param[in]  readBufferSize The number of received bytes.
     */
    void DrvEepromSendAndReceive( const u8* commandBuffer,
                                  const u32 commandBufferSize,
                                  const u8 numberOfDummyBytes,
                                  const u8* writeBuffer,
                                  const u32 writeBufferSize,
                                  u8* readBuffer,
                                  const u32 readBufferSize );
    /**
     * Send the power up command.
     */
    void DrvEepromPowerUp( void );
    /**
     * Send the power down command.
     */
    void DrvEepromPowerDown( void );
    /**
     * Send the write enable command.
     */
    void DrvEepromSetWriteEnable( void );
    /**
     * Send the write disable command.
     */
    void DrvEepromSetWriteDisable( void );
    /**
     * Write to status register.
     */
    void DrvEepromWriteStatusRegister( const u8 statusRegister );
    /**
     * Read from status register.
     */
    u8 DrvEepromReadStatusRegister( void );
    /**
     * Wait until write progress is over.
     */
    void DrvEepromWaitWriteInProgress( void );
    /**
     * Send erase chip command.
     */
    void DrvEepromChipErase( void );
    /**
     * Send erase block command.
     */
    void DrvEepromBlockErase( const u32 address );
    /**
     * Send erase sector command.
     */
    void DrvEepromSectorErase( const u32 address );
    /**
     * Send the read id command.
     */
    void DrvEepromReadId( u8* readIdBuffer,
                          const u8 readIdBufferSize );
    /**
     * Send a read command
     */
    void DrvEepromFastRead( const u32 address,
                            u8* readBuffer,
                            const u32 readBufferSize );
    /**
     * Send a page program command
     */
    void DrvEepromPageProgram( const u32 address,
                               const u8* writeBuffer,
                               const u32 writeBufferSize );
    /**
     * Release the master interface (SPI).
     */
    void DrvEepromClose( void );

#ifdef __cplusplus
}
#endif

#endif /* _DRV_EEPROM_H_ */
