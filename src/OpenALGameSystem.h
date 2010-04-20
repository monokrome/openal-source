#ifndef OPENAL_GAME_SYSTEM_H
#define OPENAL_GAME_SYSTEM_H

#include "igamesystem.h"
#include "utlvector.h"
#include "OpenALSample.h"
#include "al.h"
#include "alc.h"

/**
 * This is a game system that implements OpenAL into the Source Engine
 **/
class COpenALGameSystem : public CAutoGameSystemPerFrame
{
public:
	DECLARE_CLASS_GAMEROOT(COpenALGameSystem, CBaseGameSystemPerFrame);

	COpenALGameSystem();
	~COpenALGameSystem();

	bool Init();
	void Update(float frametime);
	void UpdateListener(float frametime);
	void Shutdown();

	bool Initialized();

	void AddSample(IOpenALSample* sample);
	void RemoveSample(IOpenALSample* sample);

	virtual char const *Name() { return "COpenALGameSystem"; }

private:
	ALCcontext *m_alContext;
	ALCdevice  *m_alDevice;
	bool        m_bHasEAX2;

	CUtlVector<IOpenALSample*> sampleVector;
};

extern COpenALGameSystem gOpenALGameSystem;

#endif
