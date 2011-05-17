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

IOpenALSample* demoSample = NULL;

/***
 * Stops all demos that rely on the demoSample pointer.
 **/
void OpenALStopDemo()
{
    if (demoSample != NULL)
    {
		if (demoSample->IsPlaying())
			demoSample->Stop();

		delete demoSample;
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

ConCommand openal_ogg_demo("openal_ogg_demo", OpenALOggDemo, "Play a demo using OpenAL/ogg support.");
ConCommand openal_flac_demo("openal_flac_demo", OpenALFlacDemo, "Play a demo using OpenAL/flac support.");
ConCommand openal_mp3_demo("openal_mp3_demo", OpenALMP3Demo, "Play a demo using OpenAL/mp3 support.");
ConCommand openal_wav_demo("openal_wav_demo", OpenALWavDemo, "Play a demo using OpenAL/wav support.");

ConCommand openal_stop_demo("openal_stop_demo", OpenALStopDemo, "Stop the current OpenAL playback demo.");
