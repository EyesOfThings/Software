/**
 * @file Audio.h
 *
 * @brief This file provides functions for playing and recording audio
 */
#ifndef _AUDIO_H_
#define _AUDIO_H_

#ifndef __RTEMS__
#error "Audio depends on RTEMS"
#else

#include <stdbool.h>
#include <mv_types.h>

/*
 * Features:
 *      ".wave"
 *      (".opus") not yet ready
 *      48kHz
 *      44.1kHz
 *      Stereo
 *      Mono
 *      Playback mode
 *      Record mode
 * Important:
 *      - consumes one posix thread resource
 *      - consumes one posix mutex resource
 *      - consumes one posix condition variable
 *
 */
#ifdef __cplusplus
extern "C" {
#endif
    /**
     * @brief Returns the current position in milliseconds.
     *
     * To change this position, use the AudioSetPosition(u32) method.
     */
    float AudioGetPosition( void );
    /**
     * @brief Change the playback position, if the audio source is seekable.
     *
     * @param position in milliseconds.
     */
    void AudioSetPosition( float position );
    /**
     * @brief Returns the total playback time in milliseconds.
     */
    float AudioGetDuration( void );
    /**
     * @brief Returns true if the playback is muted; otherwise false.
     */
    bool AudioIsMuted( void );
    /**
     * @brief Enable or disable the mute mode.
     */
    void AudioSetMute( bool enable );
    /**
     * @brief Returns true if the playback is seekable; false otherwise.
     */
    bool AudioIsSeekable( void );
    /**
     * @brief Returns the playback volume.
     */
    int8_t AudioGetVolume( void );
    /**
     * @brief Set the playback volume.
     *
     * @param volume the range is from 0(silent) to 100(maximum), values outside this range will be clamped.
     */
    void AudioSetVolume( int8_t volume );
    /**
     * @brief Start playing the current source.
     *
     * @param filepath path to the audio source.
     */
    bool AudioPlay( const char* filepath );
    /**
     * @brief Pause playing the current source.
     */
    void AudioPause( void );
    /**
     * @brief Stop playing, and reset the play position to the beginning.
     */
    void AudioStop( void );
    /**
     * @brief Resume playing/recording audio.
     */
    void AudioResume( void );
    /**
     * @brief Start recording.
     * @param filepath save the audio data to file destination.
     */
    bool AudioRecord( const char* filepath );
    /**
     * @brief Returns true if playback is active; otherwise false.
     */
    bool AudioIsInPlaybackMode( void );
    /**
     * @brief Returns true if playback/record is paused; otherwise false.
     */
    bool AudioIsPaused( void );
    /**
     * @brief Returns true if playback/record is stopped; otherwise false.
     */
    bool AudioIsStopped( void );
    /**
     * @brief Returns true if record is active; otherwise false.
     */
    bool AudioIsInRecordMode( void );

#ifdef __cplusplus
}
#endif

#endif

#endif /* _AUDIO_H_ */
