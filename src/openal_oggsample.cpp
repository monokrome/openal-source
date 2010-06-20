#include "cbase.h"
#include "openal.h"
#include "openal_oggsample.h"

// We are declaring some utility functions in order to help ogg know how to use
// the Source engine's filesystem functionality instead of standard I/O methods.
size_t vorbis_read_func(void *ptr, size_t size, size_t nmemb, void *datasource);
int vorbis_close_func(void *datasource);
int vorbis_seek_func(void *datasource, ogg_int64_t offset, int whence);
long vorbis_tell_func(void *datasource);

COpenALOggSample::COpenALOggSample()
{
}

COpenALOggSample::~COpenALOggSample()
{
}

void COpenALOggSample::Open(const char* filename)
{
	char abspath[MAX_PATH_LENGTH];

	// This will create our callbacks for reading files using Source
	ov_callbacks valveCallbacks;
	valveCallbacks.close_func = vorbis_close_func;
	valveCallbacks.read_func = vorbis_read_func;
	valveCallbacks.seek_func = vorbis_seek_func;
	valveCallbacks.tell_func = vorbis_tell_func;

	// Gets an absolute path to the provided filename
	g_OpenALGameSystem.GetSoundPath(filename, abspath, sizeof(abspath));

	oggFile = filesystem->Open(abspath, "rb");

	if (!oggFile)
	{
		Warning("OggVorbis: Could not open ogg file: %s\n. Aborting.", filename);
		return;
	}

	if (ov_open_callbacks(oggFile, &oggSample, NULL, 0, valveCallbacks) < 0)
	{
		filesystem->Close(oggFile);
		Warning("OggVorbis: Could not properly open ogg stream for %s\n. Aborting.", abspath);
		return;
	}

		// Get our metadata from ogg
	vorbisInfo = ov_info(&oggSample, -1);
	vorbisComment = ov_comment(&oggSample, -1);

		// And now we can figure out what format this sample is
	if (vorbisInfo->channels == 1)
		format = AL_FORMAT_MONO16;
	else
		format = AL_FORMAT_STEREO16;

	Init();
}

void COpenALOggSample::Close()
{
	m_bFinished = true;
	m_bReady = false;

	ov_clear(&oggSample);

	Destroy();
}

bool COpenALOggSample::CheckStream(ALuint buffer)
{
	char data[BUFFER_SIZE];
	int result, section=0, size=0;

	while (size < BUFFER_SIZE)
	{
		result = ov_read(&oggSample, data+size, BUFFER_SIZE-size, 0, 2, 1, &section);

		if (result > 0) // More data is waiting to be read
		{
			size += result;
		}
		else
		{
			if (result < 0) // There was an error reading.
			{
				Warning("Ogg: An error occured while attempting to read the ogg file.");
				return false;
			}
			else
			{
				break;
			}
		}
	}

	if (size == 0)
	{
		if (!m_bLooping)
		{
			m_bFinished = true;
			return false;
		}

		result = ov_time_seek(&oggSample, 0);

		if (result < 0)
		{
			Warning("Ogg: An error occured while attempting to perform a seek on a sample.\n");
		}
		else
		{
			DevMsg("Ogg: Successfully looped.\n");
		}
	}

	BufferData(buffer, format, data, size, vorbisInfo->rate);

	return true;
}

/***************
 * The following code is not relative to the class. It implements the functions needed to
 * allow the vorbis library to properly work with Valve's filesystem methods instead of.
 * using system standard I/O libraries.
 ***************/
size_t vorbis_read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	FileHandle_t* fileHandle = (FileHandle_t*) datasource;
	return filesystem->Read(ptr, size*nmemb, fileHandle)/size;
}

int vorbis_close_func(void *datasource)
{
	filesystem->Close((FileHandle_t*) datasource);
	return 0; // This never gets checked anyway.
}

int vorbis_seek_func(void *datasource, ogg_int64_t offset, int whence)
{
	FileSystemSeek_t seekPoint;
	FileHandle_t* fileDesc = (FileHandle_t*) datasource;
	ogg_int64_t originalPos = filesystem->Tell(fileDesc);

	switch (whence)
	{
		case SEEK_CUR:
			seekPoint = FILESYSTEM_SEEK_CURRENT;
			break;

		case SEEK_END:
			if (originalPos == filesystem->Size(fileDesc)-offset) return originalPos;

			seekPoint = FILESYSTEM_SEEK_TAIL;
			break;

		case SEEK_SET:
		default:
			seekPoint = FILESYSTEM_SEEK_HEAD;
			if (originalPos == offset) return originalPos;
	}

	filesystem->Seek(fileDesc, offset, seekPoint);

		// If the position hasn't changed, we had an issue.
	if (originalPos == filesystem->Tell(fileDesc))
		return -1;

	return filesystem->Tell(fileDesc);
}

long vorbis_tell_func(void *datasource)
{
	return filesystem->Tell((FileHandle_t*) datasource);
}
