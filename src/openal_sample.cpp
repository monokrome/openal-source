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
	m_bLinkedToEntity = false;
	m_bPositional = false;

	m_fPosition[0] = 0.0f;
	m_fPosition[1] = 0.0f;
	m_fPosition[2] = 0.0f;
	m_fDirection[0] = 0.0f;
	m_fDirection[1] = 0.0f;
	m_fDirection[2] = 1.0f;
}

IOpenALSample::~IOpenALSample()
{
	Destroy(); // It never hurts to verify!
}

void IOpenALSample::Init()
{
	alGenBuffers(NUM_BUFFERS, buffers);

	if (alGetError() != AL_NO_ERROR)
	{
		Warning("OpenAL: Error generating a sample's buffers. Sample will not play.\n");
		return;
	}

	alGenSources(1, &source);

	if (alGetError() != AL_NO_ERROR)
	{
		Warning("OpenAL: Error generating a sample's source. Sample will not play.\n");
		return;
	}

	m_bReady = InitFormat();

	g_OpenALGameSystem.Add(this);
}

void IOpenALSample::Destroy()
{
	m_bFinished = true;

	Stop();

	DestroyFormat();

	if (alGetError() != AL_NO_ERROR)
	{
		Warning("OpenAL: Error Stopping a sound. Destroying anyway.\n");
	}

	alDeleteSources(1, &source);

	if (alGetError() != AL_NO_ERROR)
	{
		Warning("OpenAL: Error deleting a sound souce. Destroying anyway.\n");
	}

	alDeleteBuffers(NUM_BUFFERS, buffers);

	if (alGetError() != AL_NO_ERROR)
	{
		Warning("OpenAL: Error deleting buffers. Destroying anyway.\n");
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

	m_bRequiresSync = false;
}

inline void IOpenALSample::UpdateBuffers(const float updateTime)
{
	int state, processed;
	bool active = false;

	alGetSourcei(source, AL_SOURCE_STATE, &state);
	alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);

	UpdateFormat(updateTime);

	while (processed--)
	{
		ALuint buffer;

		alSourceUnqueueBuffers(source, 1, &buffer);

		if (alGetError() != AL_NO_ERROR)
		{
			Warning("OpenAL: There was an error unqueuing a buffer. Issues may arise.\n");
		}

		active = CheckStream(buffer);

		if (active)
		{
			alSourceQueueBuffers(source, 1, &buffer);

			if (alGetError() != AL_NO_ERROR)
			{
				Warning("OpenAL: There was an error queueing a buffer. Expect some turbulence.\n");
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
	if (IsPlaying())
		return; // Well, that was easy!

	for (int i=0; i < NUM_BUFFERS; ++i)
	{
		if (!CheckStream(buffers[i]))
		{
			Warning("OpenAL: Couldn't play a stream. A buffer wasn't a valid audio stream.\n");
			return;
		}
	}

	alSourceQueueBuffers(source, NUM_BUFFERS, buffers);
	alSourcePlay(source);
}

void IOpenALSample::Stop()
{
	if (!IsPlaying())
	{
		return; // Why was this called? Useless.
	}

	alSourceStop(source);
	ClearBuffers();
}

void IOpenALSample::Pause()
{
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
	return m_bFinished;
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

	return (state == AL_PLAYING);
}

/***
 * Clears any unneeded buffered data.
 ***/
void IOpenALSample::ClearBuffers()
{
	if (IsPlaying())
	{
		Warning("OpenAL: ClearBuffers() called while playing. Sample will stop now.\n");
		Stop();
	}

	alSourcei(source, AL_BUFFER, 0);
}

/***
 * Keep those buffers flowing.
 ***/
void IOpenALSample::BufferData(ALuint bid, ALenum format, const ALvoid* data, ALsizei size, ALsizei freq)
{
	alBufferData(bid, format, data, size, freq);

	if (alGetError() != AL_NO_ERROR)
	{
		Warning("OpenAL: There was an error buffering audio data. Releasing deadly neurotoxin in 3... 2.. 1..\n");
	}
}

/***
 * Methods for updating the source's position/direction/etc
 ***/
void IOpenALSample::SetPositional(bool positional=false)
{
	m_bPositional = positional;
	
	if (m_bPositional)
	{
		m_bRequiresSync = true;
		alSourcei(source, AL_ROLLOFF_FACTOR, BASE_ROLLOFF_FACTOR);
	}
	else
	{
		alSourcei(source, AL_ROLLOFF_FACTOR, 0);
	}
}

inline void IOpenALSample::UpdatePositional(const float lastUpdate)
{
	if (!m_bRequiresSync && !m_bLinkedToEntity) return;

	float position[3];
	float direction[3];

	if (m_bPositional)
	{
		position[0] = m_fPosition[0];
		position[1] = m_fPosition[1];
		position[2] = m_fPosition[2];

		direction[0] = m_fDirection[0];
		direction[1] = m_fDirection[1];
		direction[2] = m_fDirection[2];
	}
	else
	{
		position[0] = 0.0f;
		position[1] = 0.0f;
		position[2] = 0.0f;

		direction[0] = 0.0f;
		direction[1] = 0.0f;
		direction[2] = -1.0f;
	}

	alSourcefv(source, AL_POSITION,    position);
	if (alGetError() != AL_NO_ERROR)
		Warning("OpenAL: Couldn't update a source's position.\n");

	alSourcefv(source, AL_DIRECTION, direction);
	if (alGetError() != AL_NO_ERROR)
		Warning("OpenAL: Couldn't update a source's direction.\n");
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

void IOpenALSample::SetDirection(const Vector direction)
{
	m_fDirection[0] = direction.x;
	m_fDirection[1] = direction.x;
	m_fDirection[2] = direction.x;

	m_bRequiresSync = true;
}

void IOpenALSample::SetDirection(const float direction[3])
{
	m_fDirection[0] = direction[0];
	m_fDirection[1] = direction[1];
	m_fDirection[2] = direction[2];

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

	m_bLinkedToEntity = true;
	m_pLinkedEntity = ent;
}

void IOpenALSample::UnlinkEntity()
{
	m_bLinkedToEntity = false;
	m_pLinkedEntity = NULL;
}
