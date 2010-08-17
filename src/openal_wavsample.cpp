//=============================================================================//
//
// Purpose: Provides a Wav sampler for use with OpenAL
//
// Author: Maestro
// Date: 06 Aug. 2010
//
// See https://ccrma.stanford.edu/courses/422/projects/WaveFormat/
// for an excellent explanation of the wav format
//
//=============================================================================//

#include "cbase.h"
#include "openal.h"
#include "openal_wavsample.h"
#include "openal_loader.h"

#include "memdbgon.h"

// Utility functions for format verification
static unsigned short readByte16(const unsigned char buffer[2]) 
{
    //return (buffer[0] << 8) + buffer[1];
    return (buffer[1] << 8) + buffer[0];
}

static unsigned long readByte32(const unsigned char buffer[4]) 
{
    //return (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
    return (buffer[3] << 24) + (buffer[2] << 16) + (buffer[1] << 8) + buffer[0];
}


COpenALWavSample::COpenALWavSample()
{
    m_iDataSize = 0;
    m_iReadDataSize = 0;
    m_iFrequency = 0;
    m_iDataOffset = 0;
    wavFile = FILESYSTEM_INVALID_HANDLE;
}

COpenALWavSample::~COpenALWavSample()
{

}

void COpenALWavSample::Open(const char* filename)
{
    // Todo:
    // Make a more sturdy version of this?

    char path[MAX_PATH_LENGTH];
    char magic[5];
    magic[4] = '\0';

    unsigned char buffer32[4];
    unsigned char buffer16[2];

    m_iDataOffset = 44;

    g_OpenALGameSystem.GetSoundPath(filename, path, sizeof(path) );

    wavFile = filesystem->Open(path, "rb");

    if (wavFile == FILESYSTEM_INVALID_HANDLE)
    {
        Q_snprintf(path, sizeof(path), "sound/%s", filename);

        wavFile = filesystem->Open(path, "rb");

        if (wavFile == FILESYSTEM_INVALID_HANDLE)
        {
            Warning("Unable to locate %s\n", path);
            return;
        }
    }

    Q_strncpy(m_pszFileName, path, sizeof(m_pszFileName) );

    // Magic check
    if ( !filesystem->Read(magic, 4, wavFile) )
    {
        Warning("Unable to read wav file: %s \n", filename);
        return;
    }

    if (!FStrEq(magic, "RIFF"))
    {
        Warning("Invalid file format in wave file %s (no RIFF magic)\n", filename);
        return;
    }

    // Skip file size
    filesystem->Seek(wavFile, 4, FILESYSTEM_SEEK_CURRENT);

    // WAVE check
    if ( !filesystem->Read(magic, 4, wavFile) )
    {
        Warning("Unable to read wav file: %s\n", filename);
        return;
    }

    if (!FStrEq(magic, "WAVE"))
    {
        Warning("Invalid file format in wave file %s (no WAVE format)\n", filename);
        return;
    }

    // fmt check
    if ( !filesystem->Read(magic, 4, wavFile) )
    {
        Warning("Unable to read wav file: %s\n", filename);
        return;
    }

    if (!FStrEq(magic, "fmt "))
    {
        Warning("Invalid file format in wave file %s (no fmt subchunk)\n", filename);
        return;
    }

    // Check size of (1)
    if ( !filesystem->Read(buffer32, 4, wavFile) )
    {
        Warning("Unable to read wav file: %s\n", filename);
        return;
    }
    
    unsigned long subChunk1Size = readByte32(buffer32);
    if (subChunk1Size < 16)
    {
        Warning("Invalid file format in wave file %s (fmt chunk too small, truncated?)\n", filename);
        return;
    }

    // Check PCM format
    if ( !filesystem->Read(buffer16, 2, wavFile) )
    {
        Warning("Unable to read wav file: %s\n", filename);
        return;
    }

    unsigned short audioFormat = readByte16(buffer16);

    if (audioFormat != 1)
    {
        Warning("Invalid file format in wave file %s (audio format is not PCM)\n", filename);
        return;
    }

    // read channels
    if ( !filesystem->Read( buffer16, 2, wavFile) ) 
    {
        Warning("Unable to read wav file: %s\n", filename);
        return;
    }

    unsigned short channels = readByte16(buffer16);

    // read frequency
    if ( !filesystem->Read(buffer32, 4, wavFile) )
    {
        Warning("Unable to read wav file: %s\n", filename);
        return;
    }

    unsigned long frequency = readByte32(buffer32);
    m_iFrequency = frequency;

    filesystem->Seek( wavFile, 6, FILESYSTEM_SEEK_CURRENT);

    // read bps
    if ( !filesystem->Read( buffer16, 2, wavFile) ) 
    {
        Warning("Unable to read wav file: %s\n", filename);
        return;
    }

    unsigned short bps = readByte16(buffer16);

    if (channels == 1)
    {
        format = (bps == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
    }
    else
    {
        format = (bps == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
    }
    
    // read data sub-chunk
    if ( !filesystem->Read( magic, 4, wavFile) ) 
    {
        Warning("Unable to read wav file: %s\n", filename);
        return;
    }

    if ( FStrEq(magic, "fact" ) )
    {
        // Skip fact chunk if present
        filesystem->Seek(wavFile, 8, FILESYSTEM_SEEK_CURRENT);

        if ( !filesystem->Read( magic, 4, wavFile) ) 
        {
            Warning("Unable to read wav file: %s\n", filename);
            return;
        }

        m_iDataOffset += 12; // 8+4
    }

    if ( !FStrEq(magic, "data") )
    {
        Warning("Invalid file format in wave file %s (no data sub-chunk)\n", filename);
        return;
    }

    if ( !filesystem->Read( buffer32, 4, wavFile) ) 
    {
        Warning("Unable to read wav file: %s\n", filename);
        return;
    }

    unsigned long subChunk2Size = readByte32(buffer32);
    m_iDataSize = subChunk2Size;

    m_bFinished = false;

    Init();
}

void COpenALWavSample::Close()
{
    filesystem->Close(wavFile);
    m_bReady = false;
    ClearBuffers();
}


void COpenALWavSample::SubUpdate()
{
}

bool COpenALWavSample::CheckStream(ALuint buffer)
{
    if (!IsReady()) return false;

    char data[OPENAL_BUFFER_SIZE];
    int result, size=0;

    while ( size < OPENAL_BUFFER_SIZE )
    {
        result = filesystem->Read( data, OPENAL_BUFFER_SIZE, wavFile );

        if (result > 0) // More data is waiting to be read
        {
            size += result;
        }
        else
        {
            if (result < 0) // There was an error reading.
            {
                Warning("Wav: An error occured while attempting to read the wav file.");
                return false;
            }
            else
            {
                break;
            }
        }
    }

    if (size == 0)
    {
        m_iReadDataSize = 0;

        if (!m_bLooping)
        {
            m_bFinished = true;
            Stop();
            return false;
        }

        // Seek to the beginning of the audio data, skipping the chunks before it
        filesystem->Seek( wavFile, m_iDataOffset, FILESYSTEM_SEEK_HEAD );

        // Buffer the new stuff
        result = filesystem->Read( data, OPENAL_BUFFER_SIZE, wavFile );

        if (result > 0) // More data is waiting to be read
        {
            size += result;
        }
    }

    // This prevents us from buffering more than the file itself allows us to.
    // If we have read more than the actual data chunk length specifies, the 
    // rest is truncated.
    m_iReadDataSize += size;
    if (m_iReadDataSize > m_iDataSize)
    {
        size = size - (m_iReadDataSize - m_iDataSize);
    }

    BufferData(buffer, format, data, size, m_iFrequency);

    return true;
}

class COpenALWavLoaderExt : public IOpenALLoaderExt
{
public:
    bool Init()
    {
        g_OpenALLoader.Register(this, "wav");
        return true;
    }

    ~COpenALWavLoaderExt()
    {
        g_OpenALLoader.Deregister(this, "wav");
    }

    IOpenALSample* Get()
    {
        return new COpenALWavSample();
    }
};

COpenALWavLoaderExt wavLoader;
