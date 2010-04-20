#include "cbase.h"
#include "OpenALGameSystem.h"
#include "OpenALSample.h"

/**
 * Returns true if the stream is ready to be used
 **/
bool IOpenALSample::Ready()
{
	return m_bReady;
}

/***
 * Play our stream
 ***/
bool IOpenALSample::Play()
{
	if (Playing())
		return true;

	for (int i=0; i < NUM_BUFFERS; ++i)
		if (!Stream(buffers[i])) return false;

	alSourceQueueBuffers(source, NUM_BUFFERS, buffers);
	alSourcePlay(source);

	return true;
}

bool IOpenALSample::Stop()
{
	if (Playing())
	{
		alSourceStop(source);
		ClearBuffers();

		return true;
	}

	return false;
}

void IOpenALSample::Pause()
{
	if (Playing())
		alSourcePause(source);
}

void IOpenALSample::BufferData(ALuint bid, ALenum format, const ALvoid* data, ALsizei size, ALsizei freq)
{
	alBufferData(bid, format, data, size, freq);
}

/***
 * Utility functions used to link this sample with OpenAL
 ***/
void IOpenALSample::OpenALInit()
{
	alGenBuffers(NUM_BUFFERS, buffers);
	OpenALCheck();

	alGenSources(1, &source);
	OpenALCheck();

		// Generate and initialize OpenAL buffers/source.
	UpdateEntity(-1);

	gOpenALGameSystem.AddSample(this);
}

void IOpenALSample::OpenALDestroy()
{
	Stop();
	ClearBuffers();

	alDeleteSources(1, &source);
	OpenALCheck();

	alDeleteBuffers(NUM_BUFFERS, buffers);
	OpenALCheck();

	gOpenALGameSystem.RemoveSample(this);
}

/***
 * Processes any updates torhat might need to be done regarding
 * the stream.
 ***/
bool IOpenALSample::OpenALUpdate(float frametime)
{
	int state, processed;
	bool active = false;

	if (Ready())
	{
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);

		while (processed--)
		{
			ALuint buffer;

			alSourceUnqueueBuffers(source, 1, &buffer);
			OpenALCheck();

			active = Stream(buffer);

			if (active)
			{
				alSourceQueueBuffers(source, 1, &buffer);
				OpenALCheck();
			}
		}

		// This is here to prevent underruns from completely stopping the sample
		if (active && state != AL_PLAYING && state != AL_PAUSED)
		{
			alSourcePlay(source);
		}

		return active;
	}

	return false;
}

/***
 * Clear and unqueue any of our source's pending buffers.
 ***/
void IOpenALSample::ClearBuffers()
{
	if (Playing())
	{
		Warning("OpenAL: Attempted to clear buffers while still playing. Stopping sound.\n");
		Stop();
	}

	alSourcei(source, AL_BUFFER, 0);
}

/***
 * Returns true if this stream is currently playing
 ***/
bool IOpenALSample::Playing()
{
	ALenum state;
	alGetSourcei(source, AL_SOURCE_STATE, &state);

	return (state == AL_PLAYING);
}

/***
 * Check for OpenAL errors.
 ***/
void IOpenALSample::OpenALCheck()
{
	if (alGetError() != AL_NO_ERROR)
		Warning("OpenAL: An error occured in OpenAL.\n");
}
/***
 * Generic functions for entity linkage possible
 ***/
void IOpenALSample::LinkEntity(CBaseEntity* entity)
{
	UnlinkEntity();

	m_pLinkedEntity = entity;
}

void IOpenALSample::UnlinkEntity(void)
{
	m_pLinkedEntity = NULL;
}

void IOpenALSample::UpdateEntity(float deltatime)
{
	float position[3];
	float orientation[6];
	Vector positionVector, fwd, right, up;

	if (m_pLinkedEntity)
	{
		positionVector = m_pLinkedEntity->GetAbsOrigin();
		position[0] = positionVector.x;
		position[1] = positionVector.y;
		position[2] = positionVector.z;

		AngleVectors(m_pLinkedEntity->GetAbsAngles(), &fwd, &right, &up);

		orientation[0] = fwd.x;
		orientation[1] = fwd.y;
		orientation[2] = fwd.z;
		orientation[3] = up.x;
		orientation[4] = right.y;
		orientation[5] = fwd.z;
	}
	else
	{
		position[0] = m_fPosition[0];
		position[1] = m_fPosition[1];
		position[2] = m_fPosition[2];

		orientation[0] = m_fOrientation[0];
		orientation[1] = m_fOrientation[1];
		orientation[2] = m_fOrientation[2];
		orientation[3] = m_fOrientation[3];
		orientation[4] = m_fOrientation[4];
		orientation[5] = m_fOrientation[5];
	}

	alSourcefv(source, AL_POSITION,  position);
	alSourcefv(source, AL_VELOCITY,  position);
	alSourcefv(source, AL_DIRECTION, position);
}

/***
 * Used to activate/deactivate looping
 ***/
void IOpenALSample::SetLoop(bool looping)
{
	m_bLooping = looping;
}

/***
 * Updating the position and orientation of our different sound sources. Note that
 * these values will be overwritten in an entity is linked to the sample.
 ***/
void IOpenALSample::SetPosition(float position[3])
{
	m_fPosition[0] = position[0];
	m_fPosition[1] = position[1];
	m_fPosition[2] = position[2];
}

void IOpenALSample::SetOrientation(float orientation[3])
{
	m_fOrientation[0] = orientation[0];	m_fOrientation[1] = orientation[1];
	m_fOrientation[2] = orientation[2];	m_fOrientation[3] = orientation[3];
	m_fOrientation[4] = orientation[4];	m_fOrientation[5] = orientation[5];
}

// Gets the full path of a specified sound file relative to the /sounds folder
void IOpenALSample::GetPath(const char* relativePath, char* buffer, size_t bufferSize)
{
  Q_snprintf(buffer, bufferSize, "%s/sounds/%s", engine->GetGameDirectory(), relativePath);
  for (; *buffer; ++buffer) {
    if (*buffer == '\\') *buffer = '/';
  }
}
