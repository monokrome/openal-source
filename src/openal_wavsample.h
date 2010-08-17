#ifndef OPENAL_WAVSAMPLE_H
#define OPENAL_WAVSAMPLE_H

#include "Filesystem.h"
#include "openal_sample.h"

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
    char m_pszFileName[MAX_PATH_LENGTH]; // Debugging fun
    FileHandle_t wavFile;

    int m_iFrequency;   // Sample rate

    int m_iDataOffset;  // Where in the file is the 
    int m_iDataSize;    // Total size of PCM data
    int m_iReadDataSize;// How much data has been read this frame
};

#endif // OPENAL_WAVSAMPLE_H