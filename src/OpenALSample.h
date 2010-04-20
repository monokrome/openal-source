#ifndef OPENAL_SAMPLE_H
#define OPENAL_SAMPLE_H

#include "cbase.h"
#include <al/al.h>

#define NUM_BUFFERS 4
#define BUFFER_SIZE 65536 // 65536 bytes = 64KB

class IOpenALSample
{
public:
	void OpenALInit();
	void OpenALDestroy();

	void SetLoop(bool looping);
	void SetPosition(float position[3]);
	void SetOrientation(float orientation[6]);

	bool Play();
	bool Stop();
	void Pause();
	bool Playing();
	bool Ready();

	void LinkEntity(CBaseEntity* entity);
	void UnlinkEntity();

	void GetPath(const char* relativePath, char* buffer, size_t bufferSize);

	virtual bool Open(char* path)=0;
	virtual void Close()=0;
	virtual void DisplayInfo()=0;
	virtual bool Stream(ALuint buffer)=0;
	virtual bool Update(float frametime)=0;

	void BufferData(ALuint bid, ALenum format, const ALvoid* data, ALsizei size, ALsizei freq);

	bool OpenALUpdate(float frametime);
	void UpdateEntity(float frametime);

	void ClearBuffers();
	void OpenALCheck();

protected:
	bool   m_bLooping;
	bool   m_bReady;

	float m_fPosition[3];
	float m_fOrientation[6];

	ALuint buffers[NUM_BUFFERS];
	ALuint source;
	ALenum format;

	CBaseEntity* m_pLinkedEntity;
};

#endif
