#include "cbase.h"

#include "openal_oggsample.h"
#include "openal_wavsample.h"


#include "openal_sample_pool.h"

CSamplePool      g_OpenALSamplePool;

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

const char* CSamplePool::GetCodecFromFileName( const char *filename )
{
    // Delete me
    return NULL;
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
    char extension[8];
    V_ExtractFileExtension(filename, extension, sizeof(extension) );
    char *codec = extension;

    if (codec == NULL)
    {
        Warning("Sample Pool: Unable to resolve sample for filename %s\n", filename);
        return INVALID_SAMPLE_HANDLE;
    }
    
    newSample = g_OpenALLoader.Load(codec);

    if (newSample == NULL)
    {
        return INVALID_SAMPLE_HANDLE;
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
    if (handle == INVALID_SAMPLE_HANDLE)
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

        if (data.codec == NULL)
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

        Assert( data.codec != NULL);
        if (data.codec == NULL)
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

        if (pSample == NULL)
        {
            i += 1;
            continue;
        }

        pSample->Destroy();
        delete pSample;

        pSample = NULL;

        m_SamplePool.Remove(i);
    }    
}


CON_COMMAND(openal_purge_samples, "Purges all samples in the pool")
{
    g_OpenALSamplePool.PurgeAll();
}