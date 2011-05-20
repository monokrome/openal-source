#include "cbase.h"
#include "openal.h"
#include "openal_sample.h"

IOpenALSample::IOpenALSample()
{
	m_bStreaming = false;
	m_bFinished = false;
	m_bLooping = false;
	m_bReady = false;
	m_bRequiresSync = true;
	m_bPositional = false;
    m_bPersistent = false;

	m_fGain = 1.0;
	m_fFadeScalar = 1.0;

	m_fPosition[0] = 0.0f;
	m_fPosition[1] = 0.0f;
	m_fPosition[2] = 0.0f;
	m_fVelocity[0] = 0.0f;
	m_fVelocity[1] = 0.0f;
	m_fVelocity[2] = 0.0f;

	metadata = new KeyValues(NULL);

    m_pLinkedEntity = NULL;
}

IOpenALSample::~IOpenALSample()
{
	Destroy(); // It never hurts to verify!
}

void IOpenALSample::Init()
{
	alGenBuffers(NUM_BUFFERS, buffers);
    ALenum error = alGetError();
	if (error != AL_NO_ERROR)
	{
		Warning("OpenAL: Error generating a sample's buffers. Sample will not play.\n");
        OPENAL_ERROR(error);
		return;
	}

	alGenSources(1, &source);
    error = alGetError();

	if (error != AL_NO_ERROR)
	{
		Warning("OpenAL: Error generating a sample's source. Sample will not play.\n");
        OPENAL_ERROR(error);
		return;
	}

	alSourcef(source, AL_REFERENCE_DISTANCE, valveUnitsPerMeter);
	if (alGetError() != AL_NO_ERROR)
	{
		Warning("OpenAL: You need to update your audio drivers or OpenAL for sound to work properly.\n");
	}

	m_bReady = InitFormat();
	g_OpenALGameSystem.Add(this);
}

void IOpenALSample::Destroy()
{
	m_bFinished = true; // Mark this for deleting and to be ignored by the thread.

	Stop();
	DestroyFormat();

	alDeleteSources(1, &source);
    ALenum error = alGetError();
	if ( error != AL_NO_ERROR)
	{
		Warning("OpenAL: Error deleting a sound source. Destroying anyway.\n");
        OPENAL_ERROR(error);
	}

	alDeleteBuffers(NUM_BUFFERS, buffers);
    if ( error != AL_NO_ERROR)
    {
		Warning("OpenAL: Error deleting buffers. Destroying anyway.\n");
        OPENAL_ERROR(error);
	}
}

void IOpenALSample::Update(const float updateTime)
{
	if (m_bFinished)
	{
		Destroy();
		return;
	}

	if (!IsReady())
	{
		Warning("OpenAL: Sample update requested while not ready. Skipping Update.\n");
		return;
	}

	/***
	 * Let's do any processing that needs to be done for syncronizing with the game engine
	 * prior to working on the buffers.
	 ***/
	UpdatePositional(updateTime);
	UpdateBuffers(updateTime);

    SubUpdate();

	m_bRequiresSync = false;
}

inline void IOpenALSample::UpdateBuffers(const float updateTime)
{
	int state, processed;
	bool active = false;

	alGetSourcei(source, AL_SOURCE_STATE, &state);
	alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);

	while (processed--)
	{
		ALuint buffer;
        ALenum error;

		alSourceUnqueueBuffers(source, 1, &buffer);
        error = alGetError();
		if (error != AL_NO_ERROR)
		{
			Warning("OpenAL: There was an error unqueuing a buffer. Issues may arise.\n");
            OPENAL_ERROR(error);
		}

		active = CheckStream(buffer);

		// I know that this block seems odd, but it's buffer overrun protection. Keep it here.
		if (active)
		{
			alSourceQueueBuffers(source, 1, &buffer);
            error = alGetError();
            if (error != AL_NO_ERROR)
            {
				Warning("OpenAL: There was an error queueing a buffer. Expect some turbulence.\n");
                OPENAL_ERROR(error);
			}

			if (state != AL_PLAYING && state != AL_PAUSED)
			{
				alSourcePlay(source);
			}
		}
	}
}

/***
* Generic playback controls
***/
void IOpenALSample::Play()
{
    int buffersToQueue = 0;

    if (IsPlaying())
        return; // Well, that was easy!

    for (int i=0; i < NUM_BUFFERS; ++i)
    {
        if (CheckStream(buffers[i]))
        {
            ++buffersToQueue;
        }
    }

    if (buffersToQueue == 0)
    {
        Warning("OpenAL: Couldn't play a stream.\n");
        return;
    }

    ALenum error;

    alSourceQueueBuffers(source, buffersToQueue, buffers);
    error = alGetError();
    if (error != AL_NO_ERROR)
    {
        Warning("OpenAL: There was an error queueing buffers. This will probably fix itself, but it's still not ideal.\n");
        OPENAL_ERROR(error);
    }

    alSourcePlay(source);
    
    error = alGetError();
    if (error != AL_NO_ERROR)
    {
        Warning("OpenAL: Playing an audio sample failed horribly.\n");
        OPENAL_ERROR(error);
    }
}

void IOpenALSample::Stop()
{
	if (!IsPlaying())
		return; // Whachootockinaboutwillis?

	alSourceRewind(source);
	if (alGetError() != AL_NO_ERROR)
	{
		Warning("OpenAL: Error stopping a sound. This is less than good news.\n");
        OPENAL_ERROR(alGetError());
	}

	ClearBuffers();
}

void IOpenALSample::Pause()
{
	Warning("OpenAL: Pausing hasn't been implemented yet?! That's ridiculous...\n");
}

/***
 * Checks whether or not this sample is currently ready to be played.
 ***/
bool IOpenALSample::IsReady()
{
	return m_bReady && !m_bFinished;
}

/***
 * Is this a positional sound?
 ***/
bool IOpenALSample::IsPositional()
{
	return m_bPositional;
}

/***
 * This basically marks the sample for safe deletion so that threading
 * doesn't run into issues by trying to access a dead sample.
 ***/
bool IOpenALSample::IsFinished()
{
    if (m_bFinished)
    {
        float seconds_played;
        alGetSourcef(source, AL_SEC_OFFSET, &seconds_played);

        if (alGetError() == AL_NO_ERROR)
        {
            // HACKHACK: Samples that are done streaming the same frame as they 
            // start playing, will be deleted instantly
            return seconds_played > 0.1f;
        }

        return true;
    }

    return false;
}

/***
 * Activate the loop process.
 ***/
void IOpenALSample::SetLooping(bool shouldLoop = true)
{
	m_bLooping = shouldLoop;
}

/***
 * Checks whether or not this sample is currently playing.
 ***/
bool IOpenALSample::IsPlaying()
{
	ALenum state;
	alGetSourcei(source, AL_SOURCE_STATE, &state);

    ALenum error = alGetError();
    if (error != AL_NO_ERROR)
    {
        OPENAL_ERROR(error); // Spy's sappin' mah error buffer!
    }

	return (state == AL_PLAYING);
}

/***
 * Clears any unneeded buffered data.
 ***/
void IOpenALSample::ClearBuffers()
{
	if (IsPlaying())
	{
		DevMsg("OpenAL: ClearBuffers() called while playing. Sample will stop now.\n");
		Stop();
	}

	alSourcei(source, AL_BUFFER, 0);

	if (alGetError() != AL_NO_ERROR)
    {
		Warning("OpenAL: An error occured while attempting to clear a source's buffers.\n");
        OPENAL_ERROR(alGetError());
    }
}

/***
 * Keep those buffers flowing.
 ***/
void IOpenALSample::BufferData(ALuint bufferID, ALenum format, const ALvoid* data, ALsizei size, ALsizei freq)
{
	alBufferData(bufferID, format, data, size, freq);
    ALenum error = alGetError();
	if (error != AL_NO_ERROR)
	{
		Warning("OpenAL: There was an error buffering audio data. Releasing deadly neurotoxin in 3... 2.. 1..\n");
        OPENAL_ERROR(alGetError());
	}
}

/***
 * Methods for updating the source's position/velocity/etc
 ***/
void IOpenALSample::SetPositional(bool positional=false)
{
    int already_positional = 0;
    alGetSourcei(source, AL_SOURCE_RELATIVE, &already_positional);

    // We don't need to set positional if we've already done so!
    if (already_positional)
    {
        return;
    }

	m_bPositional = positional;
	
	if (m_bPositional)
	{
		m_bRequiresSync = true;
		alSourcei(source, AL_SOURCE_RELATIVE, AL_FALSE);
		alSourcef(source, AL_ROLLOFF_FACTOR, baseRolloffFactor*200);
        
        ALenum error = alGetError();
        if (error != AL_NO_ERROR)
        {
			Warning("OpenAL: Couldn't update rolloff factor to enable positional audio.\n");
            OPENAL_ERROR(error);
		}
	}
	else
	{
		alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
		alSourcef(source, AL_ROLLOFF_FACTOR, 0.0f);

        ALenum error = alGetError();
		if (error != AL_NO_ERROR)
		{
			Warning("OpenAL: Couldn't update rolloff factor to disable positional audio.\n");
            OPENAL_ERROR(error);
		}
	}

	m_bRequiresSync = true;
}

inline void IOpenALSample::UpdatePositional(const float lastUpdate)
{
	if (!m_bRequiresSync && !m_pLinkedEntity) return;

	float position[3];
	float velocity[3];

	if (m_pLinkedEntity )
	{
        /*
		// TODO: Provide methods for better control of this position
		position[0] = m_pLinkedEntity->GetAbsOrigin().x;
		position[1] = m_pLinkedEntity->GetAbsOrigin().y;
		position[2] = m_pLinkedEntity->GetAbsOrigin().y;

		velocity[0] = m_pLinkedEntity->GetAbsVelocity().x;
		velocity[1] = m_pLinkedEntity->GetAbsVelocity().y;
		velocity[2] = m_pLinkedEntity->GetAbsVelocity().z;
        */
        
        position[0] = m_pLinkedEntity->GetLocalOrigin().x;
        position[1] = m_pLinkedEntity->GetLocalOrigin().y;
        position[2] = m_pLinkedEntity->GetLocalOrigin().y;

        velocity[0] = m_pLinkedEntity->GetLocalVelocity().x;
        velocity[1] = m_pLinkedEntity->GetLocalVelocity().y;
        velocity[2] = m_pLinkedEntity->GetLocalVelocity().z;
	}
	else
	{
		if (m_bPositional)
		{
			position[0] = m_fPosition[0];
			position[1] = m_fPosition[1];
			position[2] = m_fPosition[2];

			velocity[0] = m_fVelocity[0];
			velocity[1] = m_fVelocity[1];
			velocity[2] = m_fVelocity[2];
		}
		else
		{
			position[0] = 0.0f;
			position[1] = 0.0f;
			position[2] = 0.0f;

			velocity[0] = 0.0f;
			velocity[1] = 0.0f;
			velocity[2] = 0.0f;
		}
	}

	// alSource3f(source, AL_POSITION, VALVEUNITS_TO_METERS(position[0]), VALVEUNITS_TO_METERS(position[1]), VALVEUNITS_TO_METERS(position[2]));
	alSourcefv(source, AL_POSITION,    position);
	if (alGetError() != AL_NO_ERROR)
		Warning("OpenAL: Couldn't update a source's position.\n");

	// alSource3f(source, AL_VELOCITY, VALVEUNITS_TO_METERS(velocity[0]), VALVEUNITS_TO_METERS(velocity[1]), VALVEUNITS_TO_METERS(velocity[2]));
	alSourcefv(source, AL_VELOCITY, velocity);
	if (alGetError() != AL_NO_ERROR)
		Warning("OpenAL: Couldn't update a source's velocity.\n");

	alSourcef(source, AL_GAIN, m_fGain * m_fFadeScalar);
}

void IOpenALSample::SetPosition(float x, float y, float z)
{
	m_fPosition[0] = x;
	m_fPosition[1] = y;
	m_fPosition[2] = z;

	m_bRequiresSync = true;
}

void IOpenALSample::SetPosition(Vector position)
{
	m_fPosition[0] = position.x;
	m_fPosition[1] = position.y;
	m_fPosition[2] = position.z;

	m_bRequiresSync = true;
}

void IOpenALSample::SetPosition(const float position[3])
{
	m_fPosition[0] = position[0];
	m_fPosition[1] = position[1];
	m_fPosition[2] = position[2];

	m_bRequiresSync = true;
}

void IOpenALSample::SetVelocity(const Vector velocity)
{
	m_fVelocity[0] = velocity.x;
	m_fVelocity[1] = velocity.x;
	m_fVelocity[2] = velocity.x;

	m_bRequiresSync = true;
}

void IOpenALSample::SetVelocity(const float velocity[3])
{
	m_fVelocity[0] = velocity[0];
	m_fVelocity[1] = velocity[1];
	m_fVelocity[2] = velocity[2];

	m_bRequiresSync = true;
}

/***
 * This provides a special case where you can simply link an entity to the sample, and
 * the sample's update methods will go ahead and take care of the rest.
 ***/
void IOpenALSample::LinkEntity(CBaseEntity *ent)
{
	if (!ent)
		Warning("OpenAL: Couldn't properly link an entity to a source. Ignoring request.\n");

	m_pLinkedEntity = ent;
}

void IOpenALSample::UnlinkEntity()
{
	m_pLinkedEntity = NULL;
}