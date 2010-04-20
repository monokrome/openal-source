#include "cbase.h"
#include "OpenALGameSystem.h"
#include "OpenALOggStream.h"

#define MAX_PATH_LENGTH 1024

// Declare our later-defined callback functions
size_t vorbis_read_func(void *ptr, size_t size, size_t nmemb, void *datasource);
int vorbis_close_func(void *datasource);
int vorbis_seek_func(void *datasource, ogg_int64_t offset, int whence);
long vorbis_tell_func(void *datasource);

COpenALOggStream::COpenALOggStream()
{
	m_bLooping = false;
	m_bReady = false;
}

COpenALOggStream::~COpenALOggStream()
{
	Close();
}

/***
 * Open a new sample
 ***/
bool COpenALOggStream::Open(char* path)
{
	char abspath[MAX_PATH_LENGTH];

		// Set up callbacks so that we can use Valve's filesystem tools with Vorbis
	ov_callbacks valveCallbacks;
	valveCallbacks.read_func = vorbis_read_func;
	valveCallbacks.close_func = vorbis_close_func;
	valveCallbacks.seek_func = vorbis_seek_func;
	valveCallbacks.tell_func = vorbis_tell_func;

		// Get the proper path to our file
	GetPath(path, abspath, sizeof abspath);

		// And open it up!
	oggFile = filesystem->Open(abspath, "rb"); //fopen(path, "rb");
	if (!oggFile)
	{
		Warning("Could not open ogg file: %s\n", path);
		return false;
	}

	if (ov_open_callbacks(oggFile, &oggStream, NULL, 0, valveCallbacks) < 0)
	{
		filesystem->Close(oggFile);
		Warning("Could not properly open ogg stream for %s\n", path);
		return false;
	}

	vorbisInfo = ov_info(&oggStream, -1);
	vorbisComment = ov_comment(&oggStream, -1);

		// Grab our sample's format
	if (vorbisInfo->channels == 1)
		format = AL_FORMAT_MONO16;
	else
		format = AL_FORMAT_STEREO16;

	OpenALInit();

	m_bReady = true;
	return true;
}

/***
 * Cleans up our ogg stream
 ***/
void COpenALOggStream::Close()
{
	OpenALDestroy();

	ov_clear(&oggStream);

	m_bReady = false;
}


/***
 * Processes any updates torhat might need to be done regarding
 * the stream.
 ***/
bool COpenALOggStream::Update(float frametime)
{
	UpdateEntity(frametime);
	OpenALUpdate(frametime);
	return true;
}

/***
 * Checks whether or not our stream has finished playing
 ***/
bool COpenALOggStream::Stream(ALuint buffer)
{
	char data[BUFFER_SIZE];
	int  result, section=0, size=0;
	while (size < BUFFER_SIZE)
	{
		result = ov_read(&oggStream, data+size, BUFFER_SIZE-size, 0, 2, 1, &section);

		if (result > 0) // More data coming up
		{
			size += result;
		}
		else
		{
			if (result < 0) // An error occured
			{
				errorString(result);
				return false;
			}
			else // File is finished
			{
				break;
			}
		}
	}

	if (size == 0)
	{
		if (!m_bLooping)
			return false;

		// This effectively loops the file.
		result = ov_time_seek(&oggStream , 0);

		if (result < 0)
			errorString(result);
			return false;
	}

	BufferData(buffer, format, data, size, vorbisInfo->rate);
	OpenALCheck();

	return true;
}


/***
 * Check for ogg errors.
 ***/
void COpenALOggStream::errorString(int code)
{
	switch (code)
	{
		case OV_EREAD:
			Warning("Ogg: Read error from media.\n");
			return;

		case OV_ENOTVORBIS:
			Warning("Ogg: The provided data is not Vorbis.\n");
			return;

		case OV_EVERSION:
			Warning("Ogg: Vorbis version mismatch.\n");
			return;

		case OV_EBADHEADER:
			Warning("Ogg: File contains an invalid vorbis header.\n");
			return;

		case OV_EFAULT:
			Warning("Ogg: Internal logic fault. There was a bug or corruption.\n");
			return;

		default:
			Warning("Ogg: An unexplainable phenomenon has occured.\n");
			return;
	}
}

/***
 * Displays information about our stream on the HUD
 ***/
void COpenALOggStream::DisplayInfo()
{
	// TODO: This.
	// vorbisInfo->version
	// vorbisInfo->channels
	// vorbisInfo->rate
	// vorbisInfo->bitrate_upper
	// vorbisInfo->bitrate_nominal
	// vorbisInfo->bitrate_lower
	// vorbisInfo->bitrate_window
	// vorbisComment->vendor

	/*
		for (int i=0; i < vorbisComment->comments; i++)
		{
			Msg(vorbisComment->user_comments[i]);
		}
	*/
}

/***************
 * The following code is not relative to the class. It implements the functions needed to
 * allow the vorbis library to properly work with Valve's filesystem methods.
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
	size_t originalPos = filesystem->Tell(fileDesc);

	switch (whence)
	{
		case SEEK_SET:
			seekPoint = FILESYSTEM_SEEK_HEAD;
			if (originalPos == offset) return originalPos;
			break;
		case SEEK_CUR:
			seekPoint = FILESYSTEM_SEEK_CURRENT;
			break;
		case SEEK_END:
			seekPoint = FILESYSTEM_SEEK_TAIL;
			if (originalPos == filesystem->Size(fileDesc)-offset) return originalPos;
			break;
		default: // Not very useful, but it prevents warnings in MSVC.
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
