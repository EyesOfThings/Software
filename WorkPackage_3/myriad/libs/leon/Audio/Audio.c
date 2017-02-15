#include "Audio.h"
#include "private/AudioRingBuffer.h"
#include "private/AudioCodec.h"
#include "private/DrvAudioI2C.h"
#include "private/DrvAudioI2S.h"

#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <assert.h>
#include <bsp.h>
#include <rtems.h>
#include <pthread.h>
#include <errno.h>
#include <sched.h>

#define AUDIO_DEFAULT_CHUNK 12
#define AUDIO_STEREO_CHUNK 24

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

static float _position = 0.0f;
static float _duration = 0.0f;
static int8_t _volume = 30;
static bool _muted = false;
static bool _seekable = false;
static bool _playbackMode = false;
static bool _recordMode = false;
static bool _paused = false;
static bool _stopped = true;
static bool _stereo = false;
static bool _highSamplingRateMode = true;
static bool _suppress = false;

static pthread_attr_t _audioThreadAttr = { 0 };
static pthread_t _audioThread = 0;
static pthread_mutex_t _audioMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t _audioConditionVariable = PTHREAD_COND_INITIALIZER;
static int _audioThreadReady = 0;

static Codec* _codecOut = NULL;
static Buffer* _bufferOut = NULL;
static int16_t* _irqReadData = NULL;

static Codec* _codecIn = NULL;
static Buffer* _bufferIn = NULL;
static int16_t* _irqWriteData = NULL;


static void freePlaybackData( void ) {
    if(_irqReadData) {
        free( _irqReadData );
        _irqReadData = NULL;
    }
    if(_codecOut) {
        AudioCodecClose(&_codecOut);
        _codecOut = NULL;
    }
    if(_bufferOut) {
        AudioRingBufferFree(&_bufferOut);
        _bufferOut = NULL;
    }
}
static void freeRecordData( void ) {
    if(_irqWriteData) {
        free( _irqWriteData );
        _irqWriteData = NULL;
    }
    if(_codecIn) {
        AudioCodecClose(&_codecIn);
        _codecIn = NULL;
    }
    if(_bufferIn) {
        AudioRingBufferFree(&_bufferIn);
        _bufferIn = NULL;
    }
}

static void I2S0IrqCallback(void* arg) {
    I2SIrqParam* irqParam = (I2SIrqParam*) arg;
    const int8_t CHUNK_SIZE = _stereo ? AUDIO_STEREO_CHUNK : AUDIO_DEFAULT_CHUNK;

    if( _playbackMode && DrvAudioI2SIsEmpty() ) {
        if(_paused) {
            for(int i=0; i<AUDIO_DEFAULT_CHUNK; ++i) {
                DrvAudioI2SAddAudioData(0, 0);
            }
        }
        else {
            uint32_t countFilledChunks = AudioRingBufferRead(_bufferOut, _irqReadData, CHUNK_SIZE);
            if(_muted) {
                memset(_irqReadData, 0, CHUNK_SIZE*sizeof(int16_t));
            }
            if(_stereo) {
                for(int i=0; i<CHUNK_SIZE ; i+=2) {
                    DrvAudioI2SAddAudioData(_irqReadData[i], _irqReadData[i+1]);
                }
            }
            else {
                // mono
                for(int i=0; i<CHUNK_SIZE ; ++i) {
                    DrvAudioI2SAddAudioData(_irqReadData[i], _irqReadData[i]);
                }
            }
            if(_highSamplingRateMode) {
                if(_stereo) {
                    _position += (1.0f/48000.0f)*(countFilledChunks/2)*1000;
                }
                else {
                    _position += (1.0f/48000.0f)*(countFilledChunks)*1000;
                }
            }
            else {
                if(_stereo) {
                    _position += (1.0f/44100.0f)*(countFilledChunks/2)*1000;
                }
                else {
                    _position += (1.0f/44100.0f)*countFilledChunks*1000;
                }
            }
        }
    }

    if( _recordMode && DrvAudioI2SIsDataAvailable() ) {
        if(_stereo) {
            for(int i=0; i<CHUNK_SIZE ; i+=2) {
                DrvAudioI2SGetAudioData(&(_irqWriteData[i]), &(_irqWriteData[i+1]));
            }
        }
        else {
            // mono
            for(int i=0; i<CHUNK_SIZE ; ++i) {
                DrvAudioI2SGetAudioData(&(_irqWriteData[i]), &(_irqWriteData[i]));
            }
        }
        if( _muted ) {
            memset(_irqWriteData, 0, CHUNK_SIZE*sizeof(int16_t));
        }
        if(!_paused) {
            AudioRingBufferWrite(_bufferIn, _irqWriteData, CHUNK_SIZE);
        }
    }

    BSP_Clear_interrupt(irqParam->irqVector);
}

void* audioBufferRoutine(void* arg) {
    while(true) {
        if( !_suppress && _playbackMode ) {
            if( _paused ) {
                continue;
            }
            pthread_mutex_lock( &_audioMutex );

            const uint32_t numberOfCells = AudioRingBufferGetNumberOfWritableCells(_bufferOut);
            if( numberOfCells ) {
                int16_t* cells = malloc(numberOfCells * sizeof(int16_t));
                const uint32_t numberOfReadCells = AudioCodecReadPCM( _codecOut, cells, numberOfCells );
                AudioRingBufferWrite(_bufferOut, cells, numberOfReadCells);
                free(cells);
                if( (0 == numberOfReadCells && 0 == AudioRingBufferGetNumberOfReadableCells(_bufferOut)) ) {
                    _suppress = true;
                    //AudioStop();
                }
            }

            pthread_mutex_unlock( &_audioMutex );
        }
        if( !_suppress && _recordMode ) {
            if( _paused ) {
                continue;
            }
            pthread_mutex_lock( &_audioMutex );

            const uint32_t numberOfCells = AudioRingBufferGetNumberOfReadableCells(_bufferIn);
            if( numberOfCells ) {
                int16_t* cells = malloc(numberOfCells * sizeof(int16_t));
                uint32_t numberOfReadCells = AudioRingBufferRead(_bufferIn, cells, numberOfCells);
                AudioCodecWritePCM(_codecIn, cells, numberOfReadCells);
                free(cells);
                if(_highSamplingRateMode) {
                    if(_stereo) {
                        _position += (1.0f/48000.0f)*(numberOfReadCells/2)*1000;
                    }
                    else {
                        _position += (1.0f/48000.0f)*(numberOfReadCells)*1000;
                    }
                }
                else {
                    if(_stereo) {
                        _position += (1.0f/44100.0f)*(numberOfReadCells/2)*1000;
                    }
                    else {
                        _position += (1.0f/44100.0f)*numberOfReadCells*1000;
                    }
                }
            }

            pthread_mutex_unlock( &_audioMutex );
        }

        if( _suppress ) {
            pthread_mutex_lock( &_audioMutex );
            if( _suppress ) {
                AudioStop();
                _audioThreadReady = 0;
                while(!_audioThreadReady) {
                    int condition_error = pthread_cond_wait(&_audioConditionVariable, &_audioMutex);
                    assert(0 == condition_error); // Does RTEMS has enough "condition variable" resources available?
                    // please, check the CONFIGURE_MAXIMUM_POSIX_CONDITION_VARIABLES define in rtems_config.h
                }
            }
            pthread_mutex_unlock( &_audioMutex );
        }

    }
}

float AudioGetPosition( void ) {
    return _position;
}
void AudioSetPosition( float position ) {
    if(!_seekable) {
        return;
    }
    DrvAudioI2SDisableFIFOEmptyInterrupt();
    AudioCodecSetPosition(_codecOut, _position);
    AudioRingBufferReset(_bufferOut);
    _position = position;
    DrvAudioI2SEnableFIFOEmptyInterrupt();
}
float AudioGetDuration( void ) {
    return _duration;
}

bool AudioIsMuted( void ) {
    return _muted;
}
void AudioSetMute(bool enable) {
    _muted = enable;
}
bool AudioIsSeekable( void ) {
    return _seekable;
}
int8_t AudioGetVolume( void ) {
    return _volume;
}
void AudioSetVolume( int8_t volume ) {
    _volume = volume;
    if(volume < 0) {
        _volume = 0;
    }
    if(volume > 100) {
        _volume = 100;
    }
    if(_recordMode) {
        return;
    }
    DrvAudioI2CVolume( volume/100.0f );
}
bool AudioPlay(const char* filepath ) {
    int mutex_lock_error = pthread_mutex_lock( &_audioMutex );
    assert(0 == mutex_lock_error); // Does RTEMS has enough mutex resources available?
    // please, check the CONFIGURE_MAXIMUM_POSIX_MUTEXES define in rtems_config.h

    // create Thread if it not already exists
    if( 0 == _audioThread ) {
        int thread_error = 0;
        thread_error = pthread_attr_init(&_audioThreadAttr);
        assert(0 == thread_error);

        thread_error = pthread_attr_setinheritsched(&_audioThreadAttr, PTHREAD_EXPLICIT_SCHED);
        assert(0 == thread_error);

        thread_error = pthread_attr_setschedpolicy(&_audioThreadAttr, SCHED_RR);
        assert(0 == thread_error);

        thread_error = pthread_create(&_audioThread, &_audioThreadAttr, audioBufferRoutine, NULL);
        assert(0 == thread_error); // Does RTEMS has enough thread resources available?
        // please, check the CONFIGURE_MAXIMUM_POSIX_THREADS define in rtems_config.h

        pthread_detach(_audioThread);
    }
    if(NULL == filepath || 0 == strnlen(filepath, NAME_MAX)) {
        pthread_mutex_unlock( &_audioMutex );
        return false;
    }

    // stop audio file
    AudioStop();
    freePlaybackData();
    freeRecordData();

    // create audio ring buffer for playback mode
    _bufferOut = AudioRingBufferCreate();
    // create audio decoder
    _codecOut = AudioCodecOpenDecoder(filepath);

    if( NULL == _codecOut ) {
        AudioStop();
        pthread_mutex_unlock( &_audioMutex );
        return false;
    }

    // allocate memory for interrupt
    if(_codecOut->channels == CodecChannelMono) {
        _irqReadData = malloc(AUDIO_DEFAULT_CHUNK*sizeof(int16_t));
        _stereo = false;
    }
    else {
        _irqReadData = malloc(AUDIO_STEREO_CHUNK*sizeof(int16_t));
        _stereo = true;
    }

    _duration = AudioCodecDuration(_codecOut);
    _seekable = AudioCodecIsSeekable(_codecOut);
    _position = 0.0f;
    if(_seekable) {
        AudioCodecSetPosition(_codecOut, _position);
    }
    AudioRingBufferReset(_bufferOut);

    // setup audio chip
    if(_codecOut->sampleRate == CodecSampleRate48000Hz) {
        _highSamplingRateMode = true;
    }
    else {
       _highSamplingRateMode = false;
    }
    DrvAudioI2CPlaybackMode(_highSamplingRateMode);
    AudioSetVolume(_volume);
    DrvAudioI2SInit( I2S0IrqCallback );

    _muted = false;
    _playbackMode = true;
    _recordMode = false;
    _paused = false;
    _stopped = false;
    _suppress = false;

    DrvAudioI2SEnableFIFOEmptyInterrupt();

    _audioThreadReady = 1;
    int condition_error = pthread_cond_signal( &_audioConditionVariable );
    assert(0 == condition_error); // Does RTEMS has enough "condition variable" resources available?
    // please, check the CONFIGURE_MAXIMUM_POSIX_CONDITION_VARIABLES define in rtems_config.h

    pthread_mutex_unlock( &_audioMutex );
    return true;
}
void AudioPause( void ) {
    if(true == _paused) {
        return;
    }
    _paused = true;
}
void AudioStop( void ) {
    if(true == _stopped) {
        return;
    }
    DrvAudioI2SDisableFIFOEmptyInterrupt();
    DrvAudioI2SDisableFIFODataAvailableInterrupt();

    _recordMode = false;
    _playbackMode = false;
    _suppress = true;

    _paused = false;
    _muted = false;
    _highSamplingRateMode = true;
    _position = 0.0f;
    _seekable = false;
    _duration = 0.0f;
    _stereo = true;

    _stopped = true;
    if( pthread_equal(_audioThread, pthread_self()) ) {
        freePlaybackData();
        freeRecordData();
    }
}

void AudioResume( void ) {
    if(false == _paused) {
        return;
    }
    AudioSetPosition(_position);
    _paused = false;
}
bool AudioRecord(const char* filepath ) {
    int mutex_lock_error = pthread_mutex_lock( &_audioMutex );
    assert(0 == mutex_lock_error); // Does RTEMS has enough mutex resources available?
    // please, check the CONFIGURE_MAXIMUM_POSIX_MUTEXES define in rtems_config.h

    // create Thread if it not already exists
    if( 0 == _audioThread ) {
        int thread_error = 0;
        thread_error = pthread_attr_init(&_audioThreadAttr);
        assert(0 == thread_error);

        thread_error = pthread_attr_setinheritsched(&_audioThreadAttr, PTHREAD_EXPLICIT_SCHED);
        assert(0 == thread_error);

        thread_error = pthread_attr_setschedpolicy(&_audioThreadAttr, SCHED_RR);
        assert(0 == thread_error);

        thread_error = pthread_create(&_audioThread, &_audioThreadAttr, audioBufferRoutine, NULL);
        assert(0 == thread_error); // Does RTEMS has enough thread resources available?
        // please, check the CONFIGURE_MAXIMUM_POSIX_THREADS define in rtems_config.h

        pthread_detach(_audioThread);
    }
    if(NULL == filepath || 0 == strnlen(filepath, NAME_MAX)) {
        pthread_mutex_unlock( &_audioMutex );
        return false;
    }

    // stop audio file
    AudioStop();
    freePlaybackData();
    freeRecordData();

    // create audio ring buffer for recording mode
    _bufferIn = AudioRingBufferCreate();
    // create audio encoder
    _codecIn = AudioCodecOpenEncoder(filepath, CodecTypeWave, CodecSampleRate48000Hz, CodecChannelStereo);

    if( NULL == _codecIn ) {
        AudioStop();
        pthread_mutex_unlock( &_audioMutex );
        return false;
    }

    // allocate memory for interrupt
    _irqWriteData = malloc(AUDIO_STEREO_CHUNK*sizeof(int16_t));
    _stereo = true;

    _duration = 0.0f;
    _seekable = AudioCodecIsSeekable(_codecIn);
    _position = 0.0f;
    if(_seekable) {
        AudioCodecSetPosition(_codecIn, _position);
    }
    AudioRingBufferReset(_bufferIn);

    // setup audio chip
    _highSamplingRateMode = true;
    DrvAudioI2CRecordMode();
    DrvAudioI2SInit( I2S0IrqCallback );

    _muted = false;
    _playbackMode = false;
    _recordMode = true;
    _paused = false;
    _stopped = false;
    _suppress = false;

    DrvAudioI2SEnableFIFODataAvailableInterrupt();

    _audioThreadReady = 1;
    int condition_error = pthread_cond_signal( &_audioConditionVariable );
    assert(0 == condition_error); // Does RTEMS has enough "condition variable" resources available?
    // please, check the CONFIGURE_MAXIMUM_POSIX_CONDITION_VARIABLES define in rtems_config.h

    pthread_mutex_unlock( &_audioMutex );
    return true;
}
bool AudioIsInPlaybackMode( void ) {
    return _playbackMode;
}
bool AudioIsPaused( void ) {
    return _paused;
}
bool AudioIsStopped( void ) {
    return _stopped;
}
bool AudioIsInRecordMode( void ) {
    return _recordMode;
}
