#include "cbase.h"
#include "convar.h"
#include "openal.h"
#include "openal_oggsample.h"
#include "openal_wavsample.h"
#include "openal_sample.h"
#include "openal_loader.h"
//#include "c_basehlplayer.h"

#define OGG_DEMO_FILENAME "demo/demo.ogg"
#define POS_DEMO_FILENAME "demo/positional.ogg"

COpenALOggSample oggSample;
IOpenALSample* loaderSample;

void OpenALOggDemoPlay(void)
{
	if (oggSample.IsReady())
		oggSample.Close();

	oggSample.SetLooping(true);
	oggSample.SetPositional(false);

	oggSample.Open(OGG_DEMO_FILENAME);

	if (oggSample.IsReady())
		oggSample.Play();
}


void OpenALPositionalDemoPlay(void)
{
	CBasePlayer* localPlayer = CBasePlayer::GetLocalPlayer();

	Msg("Initializing demo of positional OpenAL support.\n");

	if (oggSample.IsReady())
		oggSample.Close();

	oggSample.SetLooping(true);

	if (localPlayer)
	{
		oggSample.SetPositional(true);

		// Does localPlayer need locked now? :/
		oggSample.SetPosition(localPlayer->GetAbsOrigin());
		oggSample.SetVelocity(localPlayer->GetAbsVelocity());
	}
	else
	{
		Msg("OpenAL: Demo can't find local player. Global enabled.\n");
		oggSample.SetPositional(false);
	}

	oggSample.Open(POS_DEMO_FILENAME);

	if (oggSample.IsReady())
		oggSample.Play();
}

void OpenALOggDemoStop(void)
{
	if (oggSample.IsPlaying())
	{
		oggSample.Stop();
		oggSample.Close();
	}
}

void OpenALLoaderDemoStart(void)
{
	if (loaderSample)
	{
		delete loaderSample;
		loaderSample = NULL;
	}

	loaderSample = g_OpenALLoader.Load("ogg");
	loaderSample->Open(POS_DEMO_FILENAME);

	if (loaderSample->IsReady())
		loaderSample->Play();
}

ConCommand openal_ogg_demo_play("openal_ogg_demo_play", OpenALOggDemoPlay, "Play the demo of OpenAL's ogg playback.");
ConCommand openal_ogg_demo_stop("openal_ogg_demo_stop", OpenALOggDemoStop, "Stop the demo of OpenAL's ogg playback.");
ConCommand openal_positional_demo_play("openal_positional_demo_play", OpenALPositionalDemoPlay, "Play a demo of using OpenAL for positional audio.");
ConCommand openal_positional_demo_stop("openal_positional_demo_stop", OpenALOggDemoStop, "Stop the demo of using OpenAL for positional audio.");

ConCommand openal_loader_demo_start("openal_loader_demo_start", OpenALLoaderDemoStart, "Loads a file with the loader instead of direct sample instantiation.");

#define WAV_SAMPLE "demo/wave_playback.wav"

COpenALWavSample wavSample;

void OpenALWavStart()
{
    if ( wavSample.IsReady() )
    {
        wavSample.Close();
    }

    wavSample.SetLooping(true);
    wavSample.SetPositional(false);

    wavSample.Open(WAV_SAMPLE);

    if (wavSample.IsReady())
        wavSample.Play();
}

void OpenALWavStop()
{
    if (wavSample.IsPlaying())
    {
        wavSample.Stop();
        wavSample.Close();
    }
}

ConCommand openal_wav_demo_play("openal_wav_demo_play", OpenALWavStart, "Play the demo of OpenAL's wav playback.");
ConCommand openal_wav_demo_stop("openal_wav_demo_stop", OpenALWavStop, "Stop the demo of OpenAL's wav playback.");