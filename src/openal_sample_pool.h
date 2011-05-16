#ifndef OPENAL_SAMPLE_POOL_H
#define OPENAL_SAMPLE_POOL_H

#include "openal_sample.h"

// Purpose:
// This file provides an abstraction for samples. Ideally, the sample datatypes
// should never be visible outside this file.

enum CodecType
{
    CODEC_NONE,
    CODEC_WAV,
    CODEC_OGG,
};

#define CODEC_LAST = CODEC_OGG

// This is used to reference samples in the pool, 
typedef int SampleHandle_t;
#define INVALID_SAMPLE_HANDLE -1

struct SampleData_t 
{
    SampleData_t()
    {
        Reset();
    }

    void Reset()
    {
        handle = INVALID_SAMPLE_HANDLE;
        sample = NULL;
        codec = NULL;
        wants_stop = false;
    }
    
    int handle;
    IOpenALSample *sample;
    const char *codec;
    bool wants_stop;
    //EmitSound_t data;
};

class CSamplePool
{
public:
    CSamplePool();
    ~CSamplePool();

    // Init();
    void Shutdown();

    void Update();

    void PreFrame();
    void Frame();
    void PostFrame();

    inline bool ShouldHandleThisSound(const char *filename); // Use to check if this sound should be sent througn OpenAL or not
        
    // We'll have to update this func when more data is required to pass
    // through to the samples
    SampleHandle_t CreateNewSample(const char *filename, bool shouldPlay = true);
    SampleHandle_t CreateNewSample(const char *filename, const EmitSound_t &ep, bool shouldPlay = true);

    void SetParameters( SampleHandle_t handle, const EmitSound_t &ep ); // Parses the EmitSound_t struct

    void Stop( SampleHandle_t handle );

    SampleData_t* AquireSampleFromHandle( SampleHandle_t handle );

    void PurgeAll();

private:

    SampleHandle_t  GetNewHandle();
    int     m_iLastID;

    const char* GetCodecFromFileName( const char *filename );

    CUtlVectorMT<CUtlVector<SampleData_t>>   m_SamplePool;
};

//extern CSamplePool g_OpenALSamplePool;

#endif // OPENAL_SAMPLE_POOL_H
