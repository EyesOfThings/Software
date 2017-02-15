/**
 * @file DrvAudioI2S.h
 *
 * @brief This file provides functions for sending and receiving audio to/from the audio chip.
 */
#ifndef LEON_DRVAUDIOI2S_H_
#define LEON_DRVAUDIOI2S_H_

#include <stdbool.h>
#include <stdint.h>
#include <rtems.h>

#ifdef __cplusplus
extern "C" {
#endif
    typedef struct __sI2SIrqParam {
        uint32_t status;
        rtems_vector_number irqVector;
    } I2SIrqParam;
    /**
     * @brief Initialize the I2S unit.
     * @param irqCallback function pointer to the ISR
     */
    void DrvAudioI2SInit( void (irqCallback)(void* arg) );
    /**
     * @brief Unmasks the FIFO Empty interrupt.
     */
    void DrvAudioI2SEnableFIFOEmptyInterrupt();
    /**
     * @brief Masks the FIFO Empty interrupt.
     */
    void DrvAudioI2SDisableFIFOEmptyInterrupt();
    /**
     * @brief Unmasks the FIFO Data Available interrupt.
     */
    void DrvAudioI2SEnableFIFODataAvailableInterrupt();
    /**
     * @brief Masks the FIFO Data Available interrupt.
     */
    void DrvAudioI2SDisableFIFODataAvailableInterrupt();
    /**
     * @brief Push audio data to the FIFO
     * @param leftChannel left audio channel
     * @param rightChannel right audio channel
     */
    void DrvAudioI2SAddAudioData(int16_t leftChannel, int16_t rightChannel);
    /**
     * @brief Shift audio data from the FIFO
     */
    void DrvAudioI2SGetAudioData(int16_t* leftChannel, int16_t* rightChannel);
    /**
     * @brief Returns true if transmitter FIFO is empty; otherwise false.
     */
    bool DrvAudioI2SIsEmpty();
    /**
     * @brief Returns true if receiver FIFO has data available; otherwise false.
     */
    bool DrvAudioI2SIsDataAvailable();

#ifdef __cplusplus
}
#endif


#endif /* LEON_DRVAUDIOI2S_H_ */
