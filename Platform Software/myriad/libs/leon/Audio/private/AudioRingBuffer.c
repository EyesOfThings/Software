#include "AudioRingBuffer.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define AUDIO_FS 48000
//#define AUDIO_BUFFER_SIZE 96000 // Stereo * AUDIO_FS
#define AUDIO_BUFFER_SIZE 131072 // power of 2 -> efficiency reasons
#define AUDIO_BUFFER_SIZE_MASK 131071

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

Buffer* AudioRingBufferCreate() {
    Buffer* buffer = malloc( sizeof(Buffer) );
    buffer->readPosition = 0;
    buffer->writePosition = 0;
    buffer->fillCount = 0;
    buffer->locked = 0;
    buffer->buffer = malloc( AUDIO_BUFFER_SIZE * sizeof(int16_t) );

    return buffer;
}

bool AudioRingBufferFree(Buffer** buffer) {
    if( NULL == *buffer ) {
        return false;
    }

    // clean array (ring buffer)
    free( (*buffer)->buffer );
    (*buffer)->buffer = NULL;
    // clean buffer object
    free( *buffer );
    *buffer = NULL;

    return true;
}

uint32_t AudioRingBufferGetNumberOfReadableCells(Buffer* buffer) {
    if( NULL == buffer || buffer->locked ) {
        return 0;
    }
    return buffer->fillCount;
}

uint32_t AudioRingBufferGetNumberOfWritableCells(Buffer* buffer) {
    if( NULL == buffer || buffer->locked ) {
        return 0;
    }
    return AUDIO_BUFFER_SIZE - buffer->fillCount;
}

uint32_t AudioRingBufferRead(Buffer* buffer, int16_t data[], uint32_t length) {
    assert(NULL != buffer);
    if( buffer->locked ) {
        memset(data, 0, length*sizeof(int16_t));
        return 0;
    }

    const uint32_t countPossibleBufferReads = MIN(length, (buffer->fillCount & 0xFFFFFFFE) );
    const uint32_t leftover = length - countPossibleBufferReads;
    int16_t* buf = buffer->buffer;

    for(uint32_t i=0, index=buffer->readPosition; i<countPossibleBufferReads; ++i, ++index) {
        data[i] = buf[index & AUDIO_BUFFER_SIZE_MASK];
    }

    if( 0 != leftover ) {
        memset(&data[countPossibleBufferReads], 0, leftover*sizeof(int16_t));
    }

    buffer->readPosition = (buffer->readPosition+countPossibleBufferReads) & AUDIO_BUFFER_SIZE_MASK;
    buffer->fillCount -= countPossibleBufferReads;

    return countPossibleBufferReads;
}

uint32_t AudioRingBufferWrite(Buffer* buffer, int16_t data[], uint32_t length) {
    assert(NULL != buffer);
    if( buffer->locked ) {
        return 0;
    }

    const uint32_t countEmptyCells = AUDIO_BUFFER_SIZE - buffer->fillCount;
    const uint32_t countPossibleBufferWrites = MIN(length, countEmptyCells);
    int16_t* buf = buffer->buffer;

    for(uint32_t i=0, index=buffer->writePosition; i<countPossibleBufferWrites; ++i, ++index) {
        buf[index & AUDIO_BUFFER_SIZE_MASK] = data[i];
    }

    buffer->writePosition = (buffer->writePosition+countPossibleBufferWrites) & AUDIO_BUFFER_SIZE_MASK;
    buffer->fillCount += countPossibleBufferWrites;

    return countPossibleBufferWrites;
}

void AudioRingBufferReset(Buffer* buffer) {
    assert(NULL != buffer);
    buffer->locked = true;
    buffer->readPosition = 0;
    buffer->writePosition = 0;
    buffer->fillCount = 0;
    buffer->locked = false;
}
