#include "cbase.h"
#include "openal.h"
//#include "c_basehlplayer.h" // For listener syncronization
#include "openal_oggsample.h"
#include "openal_sample_pool.h"

#include "AL/efx.h"

COpenALUpdateThread    g_OpenALUpdateThread;
COpenALGameSystem      g_OpenALGameSystem;

/**********
 * Methods for the OpenAL manager itself.
 **********/
COpenALGameSystem::COpenALGameSystem()
{
	m_grpGlobal = new openal_groupdata_t;
	m_grpGlobal->name = "global";

	// Create a default sample group that contains rendered samples
	m_AudioGroups.AddToTail(m_grpGlobal);
}

COpenALGameSystem::~COpenALGameSystem()
{
}

bool COpenALGameSystem::Add(IOpenALSample *sample)
{
	AUTO_LOCK_FM(m_vSamples);
	m_vSamples.AddToTail(sample);
	m_grpGlobal->samples.AddToTail(sample);

	return true;
}

bool COpenALGameSystem::Remove(IOpenALSample* sample)
{
    AUTO_LOCK_FM(m_vSamples);

    int index = m_vSamples.Find(sample);

    if (m_vSamples.IsValidIndex(index))
    {
        m_vSamples.Remove(index);
        m_grpGlobal->samples.FindAndRemove(sample);

        delete sample;
        sample = NULL;
    }

    return false;
}

bool COpenALGameSystem::Init()
{
	ALint ALhints[4];
	float gain = 0.0f;

	ALhints[0] = ALC_MAX_AUXILIARY_SENDS;
	ALhints[1] = requestedNumberOfAuxiliarySends;

	m_alDevice = alcOpenDevice(NULL);

	if (m_alDevice == NULL)
	{
		Warning("OpenAL: Device couldn't be properly opened. Initialization failed.\n");
		return false;
	}

	/**
	 * Check for effects extensions that we can use to provide digital signal processing
	 */
	if (alcIsExtensionPresent(m_alDevice, "ALC_EXT_EFX") == AL_FALSE)
	{
		m_bEffectsAvailable = false;
	}
	else
	{
		m_bEffectsAvailable = true;
		m_effectsType = OPENAL_EFFECTS_EFX;
	}

	m_alContext = alcCreateContext(m_alDevice, NULL);

	if (alGetError() != AL_NO_ERROR)
	{
		Warning("OpenAL: Couldn't create an OpenAL context. Initialization failed.\n");
		return false;
	}

	alcMakeContextCurrent(m_alContext);
	if (alGetError() != AL_NO_ERROR)
	{
		Warning("OpenAL: Couldn't make the OpenAL context current.\n");
		return false;
	}

	// Figure out how many auxiliary sends we've got access to
	if (m_bEffectsAvailable)
		alcGetIntegerv(m_alDevice, ALC_MAX_AUXILIARY_SENDS, 1, &m_iAuxiliarySendCount);

	// Initialize this to zero in order to prevent loud audio before we get the volume ConVar(s).
	alListenerfv(AL_GAIN, &gain);
	if (alGetError() != AL_NO_ERROR)
	{
		Warning("OpenAL: Couldn't change gain? This could get loud... Continuing without regard.\n");
	}

	// Set up the speed of sound. If this doesn't work right, you have old drivers.
	alSpeedOfSound(valveSpeedOfSound);
	if (alGetError() != AL_NO_ERROR)
	{
		Warning("OpenAL: You need to update your audio drivers or OpenAL for sound to work properly.\n");
		return false;
	}

	m_bInitialized = true;

	Update(-1);

	if (!g_OpenALUpdateThread.IsAlive())
		g_OpenALUpdateThread.Start();

	DevMsg("OpenAL: Init finished");

	if (m_bEffectsAvailable)
	{
		DevMsg(" with %d auxiliary sends per source.", m_iAuxiliarySendCount);
		alListenerf(AL_METERS_PER_UNIT, VALVEUNITS_TO_METERS(1));
	}

	DevMsg(".\nAL Renderer: %s\nAL Vendor: %s\nAL Version: %s\n", alGetString(AL_RENDERER), alGetString(AL_VENDOR), alGetString(AL_VERSION));

	return true;
}

#ifdef OPENAL_AUTOSTART_DEMO

void COpenALGameSystem::PostInit()
{
	engine->ClientCmd("openal_demo\n");
}

#endif

void COpenALGameSystem::Shutdown()
{
	if (g_OpenALUpdateThread.IsAlive())
		g_OpenALUpdateThread.CallWorker(COpenALUpdateThread::EXIT);

	AUTO_LOCK_FM(m_vSamples);
	for (int i=0; i < m_vSamples.Count(); i++)
	{
		delete m_vSamples[i];
	}
    
	m_vSamples.RemoveAll();
    
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

	FOR_EACH_LL(m_AudioGroups, i)
	{
		openal_groupdata_t* groupData = m_AudioGroups[i];
		delete groupData;
	}

	m_AudioGroups.RemoveAll();
}


/***
 * This is the standard IGameSystem inherited Update section which we will be using
 * in order to syncronize OpenAL with other various game systems. Basically, anything
 * put here doesn't have to worry about thread locking - but this method shouldn't ever
 * touch the sample vector. If it absolutely has to, don't forget to properly lock it the
 * same way that UpdateSamples() does.
 ***/
void COpenALGameSystem::Update(float frametime)
{
	UpdateListener(frametime);
}

/***
 * Updates listener information. This is inline because it's only separated for
 * organization purposes. It really doesn't need to be separated in the stack.
 ***/
inline void COpenALGameSystem::UpdateListener(const float frametime)
{
	float position[3], orientation[6], gain=0.0f;
	Vector earPosition, fwd, right, up;
	CBasePlayer *localPlayer;
	ConVar *pVolume;

	pVolume = cvar->FindVar("volume");
	if (pVolume) gain = pVolume->GetFloat();

	localPlayer = CBasePlayer::GetLocalPlayer();

	if (localPlayer)
	{
		earPosition = localPlayer->EarPosition();
		AngleVectors(localPlayer->EyeAngles(), &fwd, &right, &up);

		position[0] = earPosition.x;
		position[1] = earPosition.y;
		position[2] = earPosition.z;

		orientation[0] = fwd.x;
		orientation[1] = fwd.y;
		orientation[2] = fwd.z;
		orientation[3] = up.x;
		orientation[4] = up.y;
		orientation[5] = up.z;
	}
	else
	{
		position[0] = 0.0f;
		position[1] = 0.0f;
		position[2] = 0.0f;

		orientation[0] = 0.0f;
		orientation[1] = 0.0f;
		orientation[2] = -1.0f; // Should this be positive or negative?
		orientation[3] = 0.0f;
		orientation[4] = 0.0f;
		orientation[5] = 0.0f;
	}

	alListenerfv(AL_POSITION,    position);
	if (alGetError() != AL_NO_ERROR)
		Warning("OpenAL: Couldn't update the listener's position.\n");

	alListenerfv(AL_ORIENTATION, orientation);
	if (alGetError() != AL_NO_ERROR)
		Warning("OpenAL: Couldn't update the listener's orientation.\n");

	alListenerfv(AL_GAIN,        &gain);
	if (alGetError() != AL_NO_ERROR)
		Warning("OpenAL: Couldn't properly set the listener's gain.\n");
}

/***
 * This is where streams are actually buffered, played, etc. This is called repeatedly
 * by the thread process, and therefore need not be called from Update().
 ***/
void COpenALGameSystem::UpdateSamples(const float updateTime)
{
	RemoveEmptyGroups();

	/**
	 * For safe thread execution, we need to lock the Vector before accessing it.
	 * This macro declares a class with inline constructors/destructors allowing
	 * automatic closing of the mutex once the vector leaves the stack.
	 **/
	AUTO_LOCK_FM(m_vSamples);

	// Update our samples.
	for (int i=0; i < m_vSamples.Count(); ++i)
	{
        IOpenALSample *pSample = m_vSamples[i];

		if (pSample != NULL)
		{
			if (pSample->IsReady())
				pSample->Update(updateTime);

			if (pSample->IsFinished() && !pSample->IsPersistent())
            {
                // This automatically calls destroy on the sample
                delete pSample; 
                pSample = NULL;

				m_vSamples.Remove(i);
            }
		}
		else
		{
			m_vSamples.Remove(i);
		}
	}
}

// Gets the full path of a specified sound file relative to the /sound folder
void COpenALGameSystem::GetSoundPath(const char* relativePath, char* buffer, size_t bufferSize)
{
  Q_snprintf(buffer, bufferSize, "%s/sound/%s", engine->GetGameDirectory(), relativePath);

  for (; *buffer; ++buffer) {
    if (*buffer == '\\') *buffer = '/';
  }
}

/***
 * Methods for managing samples
 ***/
IOpenALSample* COpenALGameSystem::GetSample(char* filename)
{
/*
	IOpenALSample* theSample;

	theSample = new COpenALOggSample();
	theSample->Open(filename);

	if (!theSample->IsReady())
		delete theSample;

	return theSample;
*/
	Warning("OpenAL: GetSample() is not yet implemented.\n");
	return NULL;
}

/***
 * Group management.
 ***/
openal_groupdata_t* COpenALGameSystem::FindGroup(char* name)
{
	openal_groupdata_t* theGroup;

	FOR_EACH_LL(m_AudioGroups, i)
	{
		if (m_AudioGroups[i]->name == name)
		{
			return m_AudioGroups[i];
		}
	}

	theGroup = new openal_groupdata_t;

	theGroup->name = name;
	return theGroup;
}

void COpenALGameSystem::RemoveSampleGroup(char* name)
{
	openal_groupdata_t* theGroup = FindGroup(name);

	m_AudioGroups.FindAndRemove(theGroup);
}

void COpenALGameSystem::AddSampleToGroup(char* groupName, IOpenALSample *sample)
{
	openal_groupdata_t* theGroup = FindGroup(groupName);

	AUTO_LOCK_FM(theGroup->samples);
	theGroup->samples.AddToTail(sample);
}

void COpenALGameSystem::RemoveSampleFromGroup(char* groupName, IOpenALSample *sample)
{
	openal_groupdata_t* theGroup = FindGroup(groupName);

	AUTO_LOCK_FM(theGroup->samples);
	theGroup->samples.FindAndRemove(sample);
}

int COpenALGameSystem::RemoveEmptyGroups()
{
	int removalCount = 0;

	FOR_EACH_LL(m_AudioGroups, i)
	{
		if (m_AudioGroups[i]->samples.Count() < 1 && m_AudioGroups[i] != m_grpGlobal)
		{
			m_AudioGroups[i]->samples.RemoveAll();
			++removalCount;	
		}
	}

	return removalCount;
}


/*********
 * Methods for the update thread.
 *********/
COpenALUpdateThread::COpenALUpdateThread()
{
	SetName("OpenALUpdateThread");
}

COpenALUpdateThread::~COpenALUpdateThread()
{
}

bool COpenALUpdateThread::Init()
{
	return true;
}

void COpenALUpdateThread::OnExit()
{
}

// This is our main loop for the OpenAL update thread.
int COpenALUpdateThread::Run()
{
	unsigned nCall;

	while (IsAlive())
	{
		// Make sure this thread isn't ready to exit yet.
		if (PeekCall(&nCall))
		{
			// If this thread has been asked to safely stop processing...
			if (nCall == EXIT)
			{
				Reply(1);
				break;
			}
		}

		// Otherwise, let's keep those speakers pumpin'
        g_OpenALGameSystem.UpdateSamples(gpGlobals->frametime);
	}

	return 0;
}

void PrintALError(ALenum error, const char *file, int line)
{
    switch(error)
    {
    case AL_NO_ERROR:
        /*
        Warning("OpenAL returned error:\n");
        Msg("-File: %s (%i)\n"
        "-Error Code: %s\n"
        "--------------------------------------\n",
        file, line, "AL_NO_ERROR" );
        */
        break;

    case AL_INVALID_NAME:
        Warning("OpenAL returned error:\n");
        Msg("-File: %s (%i)\n"
            "-Error Code: %s\n"
            "--------------------------------------\n",
            file, line, "AL_INVALID_NAME" );
        break;

    case AL_INVALID_ENUM:
        Warning("OpenAL returned error:\n");
        Msg("-File: %s (%i)\n"
            "-Error Code: %s\n"
            "--------------------------------------\n",
            file, line, "AL_INVALID_ENUM" );
        break;

    case AL_INVALID_VALUE:
        Warning("OpenAL returned error:\n");
        Msg("-File: %s (%i)\n"
            "-Error Code: %s\n"
            "--------------------------------------\n",
            file, line, "AL_INVALID_VALUE" );
        break;

    case AL_INVALID_OPERATION:
        Warning("OpenAL returned error:\n");
        Msg("-File: %s (%i)\n"
            "-Error Code: %s\n"
            "--------------------------------------\n",
            file, line, "AL_INVALID_OPERATION" );
        break;

    case AL_OUT_OF_MEMORY:
        Warning("CRITICAL!!!\n"); 
        Warning("OpenAL returned error:\n");
        Msg("-File: %s (%i)\n"
            "-Error Code: %s\n"
            "--------------------------------------\n",
            file, line, "AL_OUT_OF_MEMORY" );
        break;
    default:
        Warning("OpenAL returned error:\n");
        Msg("-File: %s (%i)\n"
            "-Unknown error code: %i\n"
            "--------------------------------------\n",
            file, line, error);
    };
}
