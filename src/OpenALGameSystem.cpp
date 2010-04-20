#include "cbase.h"
#include "c_basehlplayer.h"
#include "OpenALGameSystem.h"

/**
 * I'm not sure if this works properly on the server DLL. Mostly because I can't forsee
 * a use-case for OpenAL on the server.
 **/
#ifdef CLIENT_DLL
#    define __PLAYER_CLASS C_BasePlayer
#else
#    define __PLAYER_CLASS CBasePlayer
#endif

COpenALGameSystem gOpenALGameSystem;

COpenALGameSystem::COpenALGameSystem()
{
	m_bHasEAX2 = false;

	m_alDevice = NULL;
	m_alContext = NULL;
}

COpenALGameSystem::~COpenALGameSystem()
{
	Shutdown();
}

/**
 * Called while the Source client starts up.
 **/
bool COpenALGameSystem::Init()
{
	if (Initialized())
		return true;

	// TODO: What should I use here? I hate DirectX.
	m_alDevice = alcOpenDevice((ALCchar*) "Generic Hardware");

	if (m_alDevice == NULL)
	{
		Warning("OpenAL: Device couldn't be properly opened.\n");
		return false;
	}

	m_alContext = alcCreateContext(m_alDevice, NULL);
	if (alGetError() != AL_NO_ERROR)
	{
		Warning("OpenAL: Context could not be properly created.\n");
		return false;
	}

	alcMakeContextCurrent(m_alContext);
	if (alGetError() != AL_NO_ERROR)
	{
		Warning("OpenAL: Couldn't make context current.\n");
		return false;
	}

	if (alIsExtensionPresent("EAX2.0"))
		m_bHasEAX2 = true;
	else
		m_bHasEAX2 = false;

	UpdateListener(-1);

	return true;
}

/***
 * Called when the system is getting ready to shut down.
 ***/
void COpenALGameSystem::Shutdown()
{
	if (m_alDevice != NULL)
	{
		if (m_alContext != NULL)
		{
			alcMakeContextCurrent(NULL);
			alcDestroyContext(m_alContext);
			m_alContext = NULL;
		}

		alcCloseDevice(m_alDevice);
		m_alDevice = NULL;
	}

	for (int i=0; i < sampleVector.Count(); ++i)
	{
		if (!sampleVector[i]) continue;
		delete sampleVector[i];
	}

	sampleVector.RemoveAll();
}

/***
 * This is what manages updating the state of our sound system
 *
 * NOTE:
 * Depending on performance, we might need to use frametime within the system's
 * update methods to fine-tune anything. This is why it is passed around so much,
 * even where it possibly seems a bit redundant and/or useless.
 ***/
void COpenALGameSystem::Update(float frametime)
{
	if (Initialized())
	{
		UpdateListener(frametime);

		for (int i=0; i < sampleVector.Count(); ++i)
		{
			if (!sampleVector[i]) continue;
			sampleVector[i]->Update(frametime);
		}
	}
}

	// This updates the listener to be in-sync with the local player
void COpenALGameSystem::UpdateListener(float frametime)
{
	float position[3], orientation[6], gain;
	Vector earPosition, fwd, right, up;
	__PLAYER_CLASS *localPlayer;
	ConVar *pVolume;

	gain    = 0.0f; // HACK: Keeps the listener muted until we have the volume CVar
	pVolume = cvar->FindVar("volume");
	localPlayer = CBasePlayer::GetLocalPlayer();
	if (pVolume)
		gain = pVolume->GetFloat();

	/***
	 * I could be wrong, but I think that this will be NULL during map loading, and
	 * possibly a few other points where Update() will be called.
	 ***/
	if (localPlayer)
	{
		/**
		 * This just method aliases to EyePosition. Works great for human characters...
		 **/
		earPosition = localPlayer->EarPosition();
		position[0] = earPosition.x;
		position[1] = earPosition.y;
		position[2] = earPosition.z;

		AngleVectors(localPlayer->EyeAngles(), &fwd, &right, &up);

		orientation[0] = fwd.x;
		orientation[1] = fwd.y;
		orientation[2] = fwd.z;
		orientation[3] = up.x;
		orientation[4] = up.y;
		orientation[5] = up.z;
	}

	alListenerfv(AL_POSITION,    position);
	alListenerfv(AL_ORIENTATION, orientation);
	alListenerfv(AL_GAIN,        &gain);
}

/***
 * Returns true if the program has properly initialized, or false otherwise.
 ***/
bool COpenALGameSystem::Initialized()
{
	return (m_alContext != NULL && m_alDevice != NULL);
}

/***
 * Adds a sample to the system
 ***/
void COpenALGameSystem::AddSample(IOpenALSample *sample)
{
	sampleVector.AddToTail(sample);
}

/***
 * Removes the provided sample from the CUtlVector, and deletes it.
 ***/
void COpenALGameSystem::RemoveSample(IOpenALSample* sample)
{
	int result = sampleVector.Find(sample);
	if (result >= 0) sampleVector.Remove(result);
}
