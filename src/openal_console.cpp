#include "cbase.h"
#include "convar.h"
#include "openal.h"
#include "openal_oggsample.h"

COpenALOggSample oggSample;

void OpenALOggDemoPlay(void)
{
	Msg("Initializing demo of OpenAL/OggVorbis support.\n");

	oggSample.SetLooping(true); // Lets loop this sample.
	oggSample.Open("demo/demo.ogg");

	if (oggSample.IsReady())
		oggSample.Play();
}

void OpenALOggDemoStop(void)
{
	oggSample.Stop();
}

ConCommand openal_ogg_demo_play("openal_ogg_demo_play", OpenALOggDemoPlay, "Play the demo of OpenAL's ogg playback.");
ConCommand openal_ogg_demo_stop("openal_ogg_demo_stop", OpenALOggDemoStop, "Stop the demo of OpenAL's ogg playback.");
