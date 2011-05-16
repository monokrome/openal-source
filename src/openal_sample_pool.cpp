#include "cbase.h"

#include "openal_oggsample.h"
#include "openal_wavsample.h"
#include "openal_mp3sample.h"

#include "openal_sample_pool.h"

//CSamplePool      g_OpenALSamplePool;

// Stolen from IEngineSound.h
#define SOUND_FROM_LOCAL_PLAYER		-1
#define SOUND_FROM_WORLD			0

CSamplePool::CSamplePool()
{
    m_SamplePool.Purge();
    m_iLastID = INVALID_SAMPLE_HANDLE;
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

    if (Q_stristr(filename, ".mp3"))
    {
        return CODEC_MP3;
    }
    
    return CODEC_NONE;
}

SampleHandle_t CSamplePool::GetNewHandle()
{
    m_iLastID += 1;
    return m_iLastID;
}

SampleHandle_t CSamplePool::CreateNewSample(const char *filename, const EmitSound_t &ep, bool shouldPlay /* = true */ )
{
    SampleHandle_t handle = CreateNewSample(filename, shouldPlay);
    SetParameters(handle, ep);

    return handle;
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
    case CODEC_MP3:
        newSample = new COpenALMp3Sample();
        break;
    //case CODEC_NONE:
    default:
        Warning("Unable to resolve codec for %s", filename);
        return SAMPLE_HANDLE_INVALID;
    }

    newSample->Open(filename);

    newSample->SetLooping(false);
    newSample->SetPositional(false);

    SampleData_t data;
    data.sample = newSample;
    data.codec = codec;
    data.handle = GetNewHandle();

    Msg("Sound %s played through OpenAL\n", filename);

    AUTO_LOCK_FM(m_SamplePool);

    m_SamplePool.AddToTail(data);
    return data.handle;
}

SampleData_t* CSamplePool::AquireSampleFromHandle( SampleHandle_t handle ) 
{
    if (handle == SAMPLE_HANDLE_INVALID)
    {
        return NULL;
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

        return data;
    }

    return NULL;
}

void CSamplePool::SetParameters( SampleHandle_t handle, const EmitSound_t &ep )
{
    SampleData_t *data = AquireSampleFromHandle(handle);

    if (!data)
        return;

    data->sample->SetGain(ep.m_flVolume);

    if (ep.m_nSpeakerEntity != SOUND_FROM_LOCAL_PLAYER)
    {
        CBaseEntity *pEnt = CBaseEntity::Instance( ep.m_nSpeakerEntity );
        if (pEnt!= NULL)
        {
            data->sample->LinkEntity( pEnt );
        }

        if (ep.m_pOrigin)
        {
            data->sample->SetPosition(ep.m_pOrigin->x, ep.m_pOrigin->y, ep.m_pOrigin->z );
        }
    }
}

void CSamplePool::Stop( SampleHandle_t handle )
{
    AUTO_LOCK_FM(m_SamplePool);

    SampleData_t *data = AquireSampleFromHandle(handle);

    if (!data)
    {
        return;
    }

    data->wants_stop = true;
}

void CSamplePool::PreFrame()
{
    AUTO_LOCK_FM(m_SamplePool);

    if (!m_SamplePool.Count())
    {
        return;
    }

    int i = 0;

    while ( true )
    {
        if (!m_SamplePool.IsValidIndex(i))
        {
            break;
        }

        SampleData_t data = m_SamplePool[i];

        // Is the sample invalid?
        if ( data.sample == NULL )
        {
            IOpenALSample *pSample = data.sample;

            pSample->Destroy();
            delete pSample;

            pSample = NULL;

            m_SamplePool.Remove(i);

            i += 1;
            continue;
        }

        if (data.codec == CODEC_NONE)
        {
            IOpenALSample *pSample = data.sample;

            pSample->Destroy();
            delete pSample;

            pSample = NULL;

            m_SamplePool.Remove(i);

            i += 1;
            continue;
        } 

        if ( data.sample->IsFinished() && !data.sample->IsPlaying() )
        {
            IOpenALSample *pSample = data.sample;

            pSample->Destroy();
            delete pSample;

            pSample = NULL;

            m_SamplePool.Remove(i);
        }

        i += 1;
    }
}

void CSamplePool::Frame()
{
    AUTO_LOCK_FM(m_SamplePool);

    if (!m_SamplePool.Count())
    {
        return;
    }

    int i = 0;

    while ( true )
    {
        if (!m_SamplePool.IsValidIndex(i))
        {
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

        if ( data.sample->IsFinished() && !data.sample->IsPlaying() )
        {
            IOpenALSample *pSample = data.sample;

            pSample->Destroy();
            delete pSample;

            pSample = NULL;

            m_SamplePool.Remove(i);
            continue;
        }

        if ( data.sample->IsReady() )
        {
            data.sample->Update(gpGlobals->curtime);

            if ( !data.sample->IsPlaying() && !data.sample->IsFinished() )
            {
                data.sample->Play();
            }
            else if (data.wants_stop)
            {
                data.sample->Stop();
            }
        }
        
        i += 1;
    }

}

void CSamplePool::PostFrame()
{

}

void CSamplePool::Update()
{
}

void CSamplePool::Shutdown()
{
    PurgeAll();
}

void CSamplePool::PurgeAll()
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

/*
CON_COMMAND(openal_purge_samples, "Purges all samples in the pool")
{
    g_OpenALSamplePool.PurgeAll();
}
*/
        delete pSample;

        pSample = NULL;

        m_SamplePool.Remove(i);
    }    
}


CON_COMMAND(openal_purge_samples, "Purges all samples in the pool")
{
    g_OpenALSamplePool.PurgeAll();
}
