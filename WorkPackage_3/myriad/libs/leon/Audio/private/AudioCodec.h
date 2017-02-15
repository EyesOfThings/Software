/**
 * @file AudioCodec.h
 *
 * @brief This file provides functions for encoding and decoding ".wave" and ".opus" files
 */
#ifndef LEON_AUDIOCODEC_H_
#define LEON_AUDIOCODEC_H_

#ifdef __cplusplus
extern "C" {
#endif
    #include <SDCardIO.h>

    /**
     * This enum describes the codec type.
     */
    typedef enum __eAudioCodecType {
        /** Wave Codec */
        CodecTypeWave,
        /** Opus Codec */
        CodecTypeOpus,
        /** Unknown Codec */
        CodecTypeUnknown
    } AudioCodecType;
    /**
     * This enum describes the sample rate
     */
    typedef enum __eAudioCodecSampleRate {
        /** 44.1kHz */
        CodecSampleRate44100Hz,
        /** 48kHz */
        CodecSampleRate48000Hz,
        /** Unknown sample rate */
        CodecSampleRateUnknown
    } AudioCodecSampleRate;
    /**
     * This enum describes the number of channels
     */
    typedef enum __eAudioCodecChannel {
        /** 1 Channel */
        CodecChannelMono = 1,
        /** 2 Channel */
        CodecChannelStereo = 2,
        /** Unknown Channel */
        CodecChannelUnknown
    } AudioCodecChannel;
    /**
     * Defines a codec object.
     */
    typedef struct __sCodec {
        /** This property holds a file handler */
        SDCardFile* fileHandler;
        /** This property holds the codec type. */
        AudioCodecType type;
        /** This property holds the sampleRate */
        AudioCodecSampleRate sampleRate;
        /** This property holds the number of channels */
        AudioCodecChannel channels;
        /** The property flags if the codec encode or decode*/
        bool encode;
    } Codec;

    /**
     * @brief Create an audio decoder.
     * @param[in] filename represent the file with the given name (absolute path)
     * @return If the decoder is successfully created, the function returns a pointer to an object.
     * Otherwise, a null pointer is returned.
     */
    Codec* AudioCodecOpenDecoder( const char* filename );
    /**
     * @brief Create an audio encoder.
     * @param[in] filename represent the file with the given name (absolute path)
     * @param[in] type defines the codec type
     * @param[in] sampleRate defines the codec sample rate
     * @return If the encoder is successfully created, the function returns a pointer to an object.
     * Otherwise, a null pointer is returned.
     */
    Codec* AudioCodecOpenEncoder( const char* filename, AudioCodecType type, AudioCodecSampleRate sampleRate, AudioCodecChannel channels );
    /**
     * @brief Remove/Delete/Free the codec object.
     * @param[out] codec pointer to an audio codec object
     * @return If the buffer object is successfully freed, the function returns true.
     * Otherwise, false is returned.
     */
    bool AudioCodecClose( Codec** codec);
    /**
     * @brief Reads at most 'readCellSize' units from the audio resource into 'cells', and returns the number of cells read.
     * If an error occurs, such as when attempting to read from a file opened in WriteOnly mode, this function returns 0.
     * @param[in] codec pointer to an audio codec object
     * @param[out] cells a data array
     * @param[in] readCellSize number of cells
     */
    uint32_t AudioCodecReadPCM( Codec* codec, int16_t cells[], const uint32_t readCellSize );
    /**
     * @brief Writes at most 'writeCellSize' bytes of data from 'cells' to the file.
     * Returns the number of cells that were actually written, or 0 if an error occurred.
     * @param[in] codec pointer to an audio codec object
     * @param[in] cells a data array
     * @param[in] writeCellSize number of cells
     */
    uint32_t AudioCodecWritePCM( Codec* codec, const int16_t cells[], const uint32_t writeCellSize );
    /**
     * @brief Returns the total playback time in milliseconds.
     */
    float AudioCodecDuration( Codec* codec );
    /**
     * @brief Returns true if the current codec object is seekable; false otherwise.
     */
    bool AudioCodecIsSeekable( Codec* codec );
    /**
     * @brief Change the codec position, if the audio source is seekable.
     *
     * @param position in milliseconds.
     */
    void AudioCodecSetPosition( Codec* codec, uint32_t position );


#ifdef __cplusplus
}
#endif

#endif /* LEON_AUDIOCODEC_H_ */
