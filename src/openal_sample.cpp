#include "cbase.h"
#include "openal.h"
#include "openal_sample.h"

IOpenALSample::IOpenALSample()
{
	m_bStreaming = false;
	m_bFinished = false;
	m_bLooping = false;
	m_bReady = false;

	m_fPosition[0] = 0.0f;
	m_fPosition[1] = 0.0f;
	m_fPosition[2] = 0.0f;
	m_fOrientation[0] = 0.0f;
	m_fOrientation[1] = 0.0f;
	m_fOrientation[2] = -1.0f;
	m_fOrientation[3] = 0.0f;
	m_fOrientation[4] = 0.0f;
	m_fOrientation[5] = 0.0f;
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
	int state, processed;
	bool active = false;

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
	 * Do any processing that needs to be done for positional audio before we start
	 * messing with the buffers.
	 ***/
	UpdatePositional(updateTime);

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
 * Methods for updating the source's position/orientation/etc
 ***/
inline void IOpenALSample::UpdatePositional(const float lastUpdate)
{
	if (!m_bPositional)
	{
		return;
	}

	alSourcefv(source, AL_POSITION,    &m_fPosition);
	alSourcefv(source, AL_ORIENTATION, &m_fOrientation);
}
