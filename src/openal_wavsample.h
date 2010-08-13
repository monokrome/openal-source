#ifndef OPENAL_WAVSAMPLE_H
#define OPENAL_WAVSAMPLE_H

#include "Filesystem.h"
#include "openal_sample.h"
#include "openal_loader.h"

#define MAX_PATH_LENGTH 1024

class COpenALWavSample : public IOpenALSample
{
public:
    COpenALWavSample();
    ~COpenALWavSample();

    virtual void Open(const char* filename);
    virtual void Close();

    virtual void SubUpdate();

    bool CheckStream(ALuint buffer);

private:
    const char *m_pszFileName;
    FileHandle_t wavFile;

    int m_iFrequency;   // Sample rate

    int m_iDataOffset;  // Where in the file is the 
    int m_iDataSize;    // Total size of PCM data
    int m_iReadDataSize;// How much data has been read this frame
    int m_iDataOffset; // Where in the file is the 
};

class COpenALWavLoaderExt : public IOpenALLoaderExt
{
public:
	virtual bool Init();
	~COpenALWavLoaderExt();

	virtual IOpenALSample* Get();
};

#endif // OPENAL_WAVSAMPLE_H