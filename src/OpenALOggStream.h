#ifndef OPENAL_OGG_STREAM_H
#define OPENAL_OGG_STREAM_H

#include <al/al.h>
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>
#include <vorbis/vorbisfile.h>
#include "Filesystem.h"
#include "OpenALSample.h"

/****
 * A stream based on the Ogg format
 ****/
class COpenALOggStream : public IOpenALSample
{
public:
	COpenALOggStream();
	~COpenALOggStream();

	bool Open(char* path);
	void Close();
	void DisplayInfo();
	bool Update(float frametime);
	bool Stream(ALuint buffer);

protected:
	void errorString(int code);

private:
	FileHandle_t    oggFile;
	OggVorbis_File  oggStream;
	vorbis_info*    vorbisInfo;
	vorbis_comment* vorbisComment;
};

#endif
