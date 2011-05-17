#include "cbase.h"
#include "convar.h"
#include "openal.h"
#include "openal_oggsample.h"
#include "openal_wavsample.h"
#include "openal_sample.h"
#include "openal_loader.h"
#include "openal_mp3sample.h"
//#include "c_basehlplayer.h"

// Defines differt strings that are used for OpenAL's console commands. %s = file extension
#define OPENAL_DEMO_FILENAME "demo/demo.%s"
#define OPENAL_DEFAULT_DEMO_FORMAT "ogg"
#define OPENAL_NUMBER_OF_FORMATS 4

IOpenALSample* demoSample = NULL;

char *formats[OPENAL_NUMBER_OF_FORMATS] = { "ogg", "flac", "mp3", "wav" };

/***
 * Stops all demos that rely on the demoSample pointer.
 **/
void OpenALStopDemo()
{
    if (demoSample != NULL)
    {
		if (demoSample->IsPlaying())
			demoSample->Stop();

		//delete demoSample;
		//demoSample = NULL;
        g_OpenALGameSystem.Remove(demoSample);
    }
}

/***
 * Receives a file extension, and attempts to load and play demo/demo.<fileExtension>.
 */
void OpenALStartDemo(char *fileExtension)
{
	OpenALStopDemo();

	demoSample = g_OpenALLoader.Load(fileExtension);

	if (demoSample == NULL)
	{
		return;
	}

	demoSample->Open(VarArgs(OPENAL_DEMO_FILENAME, fileExtension));
	demoSample->SetLooping(true);
    demoSample->Persist();
	demoSample->Play();
}

void OpenALOggDemo(void)
{
	OpenALStartDemo("ogg");
}

void OpenALFlacDemo(void)
{
	OpenALStartDemo("flac");
}

void OpenALMP3Demo(void)
{
	OpenALStartDemo("mp3");
}

void OpenALWavDemo(void)
{
	OpenALStartDemo("wav");
}

/**
 * Provides a more generic command that simply takes a format and plays a demo for that format. If
 * no format is specified, reverts to the first format in the `formats` array.
 */
void OpenALDemo(const CCommand &args)
{
	char *format;

	if (args.ArgC() > 1)
		format = (char *) args[1];
	else
		format = formats[0];

	OpenALStartDemo(format);
}

// Provides autocompletion for the openal_demo command in the console.
static int OpenALDemo_AutoComplete(char const *partial,
								   char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
{
	int number_results = 0;

	for (int i=0; i < OPENAL_NUMBER_OF_FORMATS; ++i)
	{
		char *format_command = VarArgs("openal_demo %s", formats[i]);

		if (strncmp(format_command, partial, strlen(partial)) == 0)
		{
			strcpy(commands[number_results], format_command);
			++number_results;
		}
	}

	return number_results;
}


ConCommand openal_demo("openal_demo", OpenALDemo, "Play a demo using OpenAL and the specified format.", 0, OpenALDemo_AutoComplete);

ConCommand openal_ogg_demo("openal_ogg_demo", OpenALOggDemo, "Play a demo using OpenAL/ogg support.");
ConCommand openal_flac_demo("openal_flac_demo", OpenALFlacDemo, "Play a demo using OpenAL/flac support.");
ConCommand openal_mp3_demo("openal_mp3_demo", OpenALMP3Demo, "Play a demo using OpenAL/mp3 support.");
ConCommand openal_wav_demo("openal_wav_demo", OpenALWavDemo, "Play a demo using OpenAL/wav support.");

ConCommand openal_stop_demo("openal_stop_demo", OpenALStopDemo, "Stop the current OpenAL playback demo.");

CON_COMMAND( openal_play, "Play an arbitrary file using the OpenAL system" )
{
    if ( args.ArgC() != 2 )
        return;

    char path[MAX_PATH];
    V_strcpy(path, args[1]);

    IOpenALSample *pSample = g_OpenALLoader.Load(path);

    if (pSample != NULL && pSample->IsReady())
    {
        pSample->Play();
    }
}