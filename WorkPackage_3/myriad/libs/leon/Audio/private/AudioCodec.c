#include "AudioCodec.h"

#include <stdlib.h>
#include <string.h>

typedef struct __sRIFFHeader {
    char chunkID[4];
    u32 chunkSize;
    char format[4];
    char subChunk1ID[4];
    u32 subChunk1Size;
    u16 audioFormat;
    u16 numChannels;
    u32 sampleRate;
    u32 byteRate;
    u16 blockSize;
    u16 bitsPerSample;
    char subChunk2ID[4];
    u32 subChunk2Size;
} RIFFHeader;

static void findCodec(Codec* codec) {
    RIFFHeader waveHeader;
    SDCardFileSetPosition(codec->fileHandler, 0);
    SDCardFileRead(codec->fileHandler, (uint8_t*)&waveHeader, sizeof(RIFFHeader));
    if(
        0 == memcmp(waveHeader.chunkID, "RIFF", 4) &&
        0 == memcmp(waveHeader.format, "WAVE", 4) &&
        1 == waveHeader.audioFormat &&
        ( 1 == waveHeader.numChannels || 2 == waveHeader.numChannels ) &&
        ( 44100 == waveHeader.sampleRate || 48000 == waveHeader.sampleRate ) &&
        16 == waveHeader.bitsPerSample
    ) {
        // valid wave file
        codec->type = CodecTypeWave;
        codec->sampleRate = (44100 == waveHeader.sampleRate) ? CodecSampleRate44100Hz : CodecSampleRate48000Hz;
        codec->channels = ( 1 == waveHeader.numChannels ) ? CodecChannelMono : CodecChannelStereo;
    }
    // TODO: OPUS
}
static void updateWaveHeader(Codec* codec) {
    if( NULL == codec || false == codec->encode ) {
        return;
    }
    const uint32_t fileSize = SDCardFileGetSize(codec->fileHandler);
    RIFFHeader waveHeader;
    memcpy(waveHeader.chunkID, "RIFF", 4);
    waveHeader.chunkSize = fileSize - 8;
    memcpy(waveHeader.format, "WAVE", 4);
    waveHeader.subChunk1ID[0] = 0x66;
    waveHeader.subChunk1ID[1] = 0x6d;
    waveHeader.subChunk1ID[2] = 0x74;
    waveHeader.subChunk1ID[3] = 0x20;
    waveHeader.subChunk1Size = 16;
    waveHeader.audioFormat = 1;
    waveHeader.numChannels = codec->channels == CodecChannelMono ? 1 : 2;
    waveHeader.sampleRate = codec->sampleRate == CodecSampleRate44100Hz ? 44100 : 48000;
    waveHeader.byteRate = waveHeader.sampleRate * waveHeader.numChannels * 2;
    waveHeader.blockSize = waveHeader.numChannels * 2;
    waveHeader.bitsPerSample = 16;
    memcpy(waveHeader.subChunk2ID, "data", 4);
    waveHeader.subChunk2Size = fileSize - sizeof(RIFFHeader);
    if( SDCardFileSetPosition(codec->fileHandler, 0) ) {
        SDCardFileWrite(codec->fileHandler, (uint8_t*)&waveHeader, sizeof(RIFFHeader));
    }
}

Codec* AudioCodecOpenDecoder( const char* filename ) {
    Codec* codec = malloc( sizeof(Codec) );
    codec->fileHandler = SDCardFileOpen(filename, "rb", false);
    if( NULL == codec->fileHandler ) {
        free( codec );
        return NULL;
    }
    codec->encode = false;
    codec->type = CodecTypeUnknown;
    codec->sampleRate = CodecSampleRateUnknown;
    codec->channels = CodecChannelUnknown;
    findCodec(codec);
    if( CodecTypeUnknown == codec->type) {
        SDCardFileClose(&(codec->fileHandler));
        free( codec );
        return NULL;
    }
    return codec;
}

Codec* AudioCodecOpenEncoder( const char* filename, AudioCodecType type, AudioCodecSampleRate sampleRate, AudioCodecChannel channel ) {
    Codec* codec = malloc( sizeof(Codec) );
    codec->fileHandler = SDCardFileOpen(filename, "w+b", false);
    if( NULL == codec->fileHandler ) {
        free( codec );
        return NULL;
    }
    codec->encode = true;
    codec->type = type;
    codec->sampleRate = sampleRate;
    codec->channels = channel;
    if(type == CodecTypeWave) {
        // write RIFF HEADER
        RIFFHeader waveHeader;
        memset(&waveHeader, 0, sizeof(RIFFHeader));
        SDCardFileWrite(codec->fileHandler, (uint8_t*)&waveHeader, sizeof(RIFFHeader));
    }
    // TODO: OPUS

    return codec;
}

bool AudioCodecClose(Codec** codec) {
    if( NULL == *codec ) {
        return false;
    }

    if((*codec)->encode) {
        updateWaveHeader(*codec);
    }
    // TODO: OPUS

    SDCardFileClose(&((*codec)->fileHandler));
    // clean buffer object
    free( *codec );
    *codec = NULL;

    return true;
}

uint32_t AudioCodecReadPCM(Codec* codec, int16_t cells[], const uint32_t readCellSize ) {
    if( NULL == codec || true == codec->encode ) {
        return 0;
    }
    if( codec->type == CodecTypeWave ) {
        const uint32_t numberOfReadBytes = SDCardFileRead(codec->fileHandler, (u8*)cells, readCellSize*sizeof(int16_t));
        return numberOfReadBytes / sizeof(int16_t);
    }
    // TODO: OPUS
    return 0;
}
uint32_t AudioCodecWritePCM( Codec* codec, const int16_t cells[], const uint32_t writeCellSize ) {
    if( NULL == codec || false == codec->encode ) {
        return 0;
    }
    if( codec->type == CodecTypeWave ) {
        const uint32_t numberOfReadBytes = SDCardFileWrite(codec->fileHandler, (u8*)cells, writeCellSize*sizeof(int16_t));
        return numberOfReadBytes / sizeof(int16_t);
    }
    // TODO: OPUS
    return 0;
}

float AudioCodecDuration( Codec* codec ) {
    if( NULL == codec || codec->encode ) {
        return 0;
    }
    if( codec->type == CodecTypeWave ) {
        uint32_t bytes = SDCardFileGetSize(codec->fileHandler) - sizeof(RIFFHeader);
        uint8_t numberOfChannels = codec->channels == CodecChannelMono ? 1 : 2;
        uint32_t freq = codec->sampleRate == CodecSampleRate48000Hz ? 48000 : 44100;
        return ( bytes/((float)(numberOfChannels*sizeof(int16_t)*freq)) )*1000.0f;
    }
    // TODO: OPUS
    return 0;
}

bool AudioCodecIsSeekable( Codec* codec ) {
    if( NULL == codec || codec->encode ) {
        return false;
    }
    // TODO: OPUS
    return codec->type == CodecTypeWave;
}

void AudioCodecSetPosition( Codec* codec, uint32_t position ) {
    if( NULL == codec || codec->encode ) {
        return;
    }
    if( codec->type == CodecTypeWave ) {
        uint8_t numberOfChannels = codec->channels == CodecChannelMono ? 1 : 2;
        uint32_t freq = codec->sampleRate == CodecSampleRate48000Hz ? 48000 : 44100;
        uint32_t positionInBytes = (position / 1000) * (numberOfChannels*sizeof(int16_t)*freq) + sizeof(RIFFHeader);
        SDCardFileSetPosition(codec->fileHandler, positionInBytes);
    }
    // TODO: OPUS
}
