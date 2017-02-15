/**
 * @file AudioRingBuffer.h
 *
 * @brief This file provides functions for an audio ring buffer
 */
#ifndef LEON_AUDIORINGBUFFER_H_
#define LEON_AUDIORINGBUFFER_H_

#include <stdbool.h>
#include <mv_types.h>

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * Defines an audio ring buffer object.
     */
    typedef struct __sBuffer {
        /** This property holds the current read position. */
        uint32_t readPosition;
        /** This property holds the current write position. */
        uint32_t writePosition;
        /** This property holds the current number of filled buffer cells */
        uint32_t fillCount;
        /** This property holds the buffer data. */
        int16_t* buffer;
        /** This property is a lock flag, if true reading and writing is not possible */
        bool locked;
    } Buffer;

    /**
     * @brief Create an audio ring buffer object.
     * @return If the ring buffer is successfully created, the function returns a pointer to an object.
     * Otherwise, a null pointer is returned.
     */
    Buffer* AudioRingBufferCreate();
    /**
     * @brief Remove/Delete/Free the ring buffer object.
     * @param[out] buffer pointer to an audio ring buffer object
     * @return If the buffer object is successfully freed, the function returns true.
     * Otherwise, false is returned.
     */
    bool AudioRingBufferFree(Buffer** buffer);
    /**
     * @brief Returns the number of available data cells.
     * @param[in] buffer pointer to an audio ring buffer object
     * @return number of available buffer cells.
     */
    uint32_t AudioRingBufferGetNumberOfReadableCells(Buffer* buffer);
    /**
     * @brief Returns the number of free data cells.
     * @param[in] buffer pointer to an audio ring buffer object
     * @return number of free buffer cells.
     */
    uint32_t AudioRingBufferGetNumberOfWritableCells(Buffer* buffer);
    /**
     * @brief Read from an audio ring buffer object.
     * @param[in] buffer pointer to an audio ring buffer object
     * @param[out] data pointer to an array
     * @param[in] reads at most 'length' cells from the buffer
     * @return the number of cells read
     */
    uint32_t AudioRingBufferRead(Buffer* buffer, int16_t data[], uint32_t length);
    /**
     * @brief Write to an audio ring buffer object.
     * @param[in] buffer pointer to an audio ring buffer object
     * @param[out] data pointer to an array
     * @param[in] length number of cells
     * @return the number of cells
     */
    uint32_t AudioRingBufferWrite(Buffer* buffer, int16_t data[], uint32_t length);
    /**
     * @brief Reset an audio ring buffer object.
     * @param[in] buffer pointer to an audio ring buffer object
     */
    void AudioRingBufferReset(Buffer* buffer);

#ifdef __cplusplus
}
#endif

#endif /* LEON_AUDIORINGBUFFER_H_ */
