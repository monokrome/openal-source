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

    bool CheckStream(ALuint buffer);

private:
    FileHandle_t wavFile;
    int m_iFrequency;
};

#endif // OPENAL_WAVSAMPLE_H