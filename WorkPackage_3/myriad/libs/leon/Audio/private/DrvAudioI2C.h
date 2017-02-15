/**
 * @file DrvAudioI2C.h
 *
 * @brief This file provides functions for setting up the audio chip
 */
#ifndef _DRV_AUDIO_I2C_H_
#define _DRV_AUDIO_I2C_H_

#include <stdbool.h>
#include <mv_types.h>

#ifdef __cplusplus
extern "C" {
#endif
    /**
     * @brief Set the audio chip to playback mode.
     * @param highSamplingRateMode if true the audio chip will be setup with an sampling rate of 48kHz; otherwise it will be setup with 44.1kHz
     */
    void DrvAudioI2CPlaybackMode( bool highSamplingRateMode );
    /**
     * @brief Set the audio chip to record mode.
     */
    void DrvAudioI2CRecordMode( void );
    /**
     * @brief Set audio volume
     * @param volume [0.0, 1.0] range
     */
    void DrvAudioI2CVolume(float volume);

#ifdef __cplusplus
}
#endif

#endif /* _DRV_AUDIO_I2C_H_ */
