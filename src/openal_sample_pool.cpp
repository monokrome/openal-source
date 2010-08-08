#include "cbase.h"

#include "openal_oggsample.h"
#include "openal_wavsample.h"

#include "openal_sample_pool.h"

CSamplePool      g_OpenALSamplePool;

CSamplePool::CSamplePool()
{
    m_SamplePool.Purge();
    m_iLastID = SAMPLE_HANDLE_INVALID;
}

CSamplePool::~CSamplePool()
{
    m_SamplePool.Purge();
}

bool CSamplePool::ShouldHandleThisSound(const char *filename)
{
    if (Q_stristr(filename, ".ogg") /* || Q_stristr(filename, ".wav") */ ) 
    {
        return true;
    }

    return false;
}

CodecType CSamplePool::GetCodecFromFileName( const char *filename )
{
    if (Q_stristr(filename, ".ogg") ) 
    {
        return CODEC_OGG;
    }

    if (Q_stristr(filename, ".wav") ) 
    {
        return CODEC_WAV;
    }
    
    return CODEC_NONE;
}

SampleHandle_t CSamplePool::GetNewHandle()
{
    m_iLastID += 1;
    return m_iLastID;
}

SampleHandle_t CSamplePool::CreateNewSample(const char *filename, bool shouldPlay /* = true */ )
{
    // NOTENOTE:    Consider paths to be of proper format
    //              when they arrive here

    // Remove any snd chars
    //const char *name = PSkipSoundChars(filename);
    IOpenALSample *newSample = NULL;

    // Figure out the proper codec
    CodecType codec = GetCodecFromFileName(filename);
    switch (codec)
    {
    case CODEC_OGG:
        newSample = new COpenALOggSample();
        break;
    case CODEC_WAV:
        newSample = new COpenALWavSample();
        break;
    //case CODEC_NONE:
    default:
        Warning("Unable to resolve codec for %s", filename);
        return SAMPLE_HANDLE_INVALID;
    }

    newSample->Open(filename);

    newSample->SetLooping(false);
    newSample->SetPositional(false);

    /*
    if (shouldPlay)
    {
        if (newSample->IsReady())
        {
            newSample->Play();
        }
    }
    */

    SampleData_t data;
    data.sample = newSample;
    data.codec = codec;
    data.handle = GetNewHandle();

    Msg("Sound %s played through OpenAL\n", filename);

    AUTO_LOCK_FM(m_SamplePool);

    m_SamplePool.AddToTail(data);
    return data.handle;
}

void CSamplePool::Stop( SampleHandle_t handle )
{
    AUTO_LOCK_FM(m_SamplePool);

    if (handle == SAMPLE_HANDLE_INVALID)
    {
        return;
    }

    int i = 0;

    while ( i < m_SamplePool.Count() )
    {
        if (!m_SamplePool.IsValidIndex(i))
        {
            // This blows...
            break;
        }

        SampleData_t *data = &m_SamplePool[i];
        if ( data->handle != handle )
        {
            i += 1;
            continue;;
        }

        // Is the sample invalid?
        if ( data->sample == NULL )
        {
            i += 1;
            continue;
        }

        data->wants_stop = true;

        break;
    }
}

void CSamplePool::Update()
{
    AUTO_LOCK_FM(m_SamplePool);

    if (!m_SamplePool.Count())
    {
        // We like optimizations
        return;
    }
    
    int i = 0;
    
    while ( i < m_SamplePool.Count() )
    {
        if (!m_SamplePool.IsValidIndex(i))
        {
            // This blows...
            break;
        }

        SampleData_t data = m_SamplePool[i];

        // Is the sample invalid?
        if ( data.sample == NULL )
        {
            i += 1;
            continue;
        }

        Assert( data.codec != CODEC_NONE);
        if (data.codec == CODEC_NONE)
        {
            i += 1;
            continue;
        }

        if ( data.sample->IsReady() )
        {
            data.sample->Update(gpGlobals->curtime);

            if ( !data.sample->IsPlaying() )
            {
                data.sample->Play();
            }
            else if (data.wants_stop)
            {
                data.sample->Stop();
            }
        }

        if ( data.sample->IsFinished() )
        {
            IOpenALSample *pSample = data.sample;

            pSample->Destroy();
            delete pSample;

            pSample = NULL;

            m_SamplePool.Remove(i);
        }
        else
            i += 1;
    }
}

void CSamplePool::Shutdown()
{
    AUTO_LOCK_FM(m_SamplePool);

    if (!m_SamplePool.Count())
    {
        // We like optimizations
        return;
    }

    int i = 0;

    while ( i < m_SamplePool.Count() )
    {
        if (!m_SamplePool.IsValidIndex(i))
        {
            // This blows...
            i += 1;
            continue;
        }

        IOpenALSample *pSample = m_SamplePool[i].sample;

        if (pSample == NULL)
        {
            i += 1;
            continue;
        }

        pSample->Destroy();
        delete pSample;

        pSample = NULL;

        m_SamplePool.Remove(i);

        /*
        else
            i += 1;
        */
    }
}
