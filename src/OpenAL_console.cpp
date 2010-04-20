#include "cbase.h"
#include "convar.h"
#include "OpenALGameSystem.h"
#include "OpenALOggStream.h"

COpenALOggStream OpenALOggDemoStream;

/***
 * Functions for testing basic use of the OpenAL API with Ogg files
 ***/
void OpenALOggDemoPlay(void)
{
	if (OpenALOggDemoStream.Open("demos/demo.ogg"))
	{
		OpenALOggDemoStream.SetLoop(true);
		OpenALOggDemoStream.Play();
	}
}

void OpenALOggDemoStop(void)
{
	if (OpenALOggDemoStream.Playing())
		OpenALOggDemoStream.Stop();
}

ConCommand openal_ogg_demo_play("openal_ogg_demo_play", OpenALOggDemoPlay, "Play the demo of OpenAL's ogg playback.");
ConCommand openal_ogg_demo_stop("openal_ogg_demo_stop", OpenALOggDemoStop, "Stop the demo of OpenAL's ogg playback.");
