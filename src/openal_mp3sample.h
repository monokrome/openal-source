#ifndef __OPENAL_MP3SAMPLE_H
#define __OPENAL_MP3SAMPLE_H

#include "Filesystem.h"
#include "openal_sample.h"
#include "openal_loader.h"

#include <libmad/mad.h>

#define MAX_PATH_LENGTH 1024
#define INPUT_BUFFER_SIZE (40000)

struct input {
    unsigned char *data;
    unsigned long length;
};

struct audio_stats {
    unsigned long clipped_samples;
    mad_fixed_t peak_clipping;
    mad_fixed_t peak_sample;
};

struct audio_dither {
    mad_fixed_t error[3];
    mad_fixed_t random;
};

class COpenALMp3Sample : public IOpenALSample
{
public:
    COpenALMp3Sample();
    ~COpenALMp3Sample();

    virtual void Open(const char* filename);
    virtual void Close();

    virtual bool InitFormat();
    virtual void DestroyFormat();

    bool CheckStream(ALuint buffer);

    //void UpdateMetadata(); // If we ever care to implement some ID3 reading...

    FileHandle_t mp3File;

    struct mad_stream	Stream;
    struct mad_frame	Frame;
    struct mad_synth	Synth;
    mad_timer_t			Timer;
    struct input        Input;
    struct audio_stats  Stats;

    int FrameCount;
    int freq;
    bool hitEOF;
    //int ReadSize;
};

class COpenALMp3LoaderExt : public IOpenALLoaderExt
{
public:
	virtual bool Init();
	virtual void Shutdown();

	virtual IOpenALSample* Get();
};

#endif
