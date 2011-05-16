#include "cbase.h"
#include "convar.h"
#include "openal.h"
#include "openal_oggsample.h"
#include "openal_wavsample.h"
#include "openal_sample.h"
#include "openal_loader.h"
#include "openal_mp3sample.h"
//#include "c_basehlplayer.h"

#define OGG_DEMO_FILENAME "demo/demo.ogg"
#define POS_DEMO_FILENAME "demo/positional.ogg"

IOpenALSample* demoSample = NULL;

/***
 * Stops all demos that rely on the demoSample pointer.
 **/
void OpenALStopDemo()
{
    if (demoSample != NULL)
    {
        demoSample->Stop();
    }
}

/***
 * Plays a nice little looping demo local to the player position.
 **/
void OpenALPlayDemo(void)
{
    if (demoSample == NULL)
    {
        demoSample = g_OpenALLoader.Load("ogg");

        if (demoSample == NULL)
        {
            return;
        }
    }

    if (demoSample->IsPlaying())
    {
        demoSample->Stop();
    }

    demoSample->Open(OGG_DEMO_FILENAME);
    demoSample->Play();
}


/***
 * Plays a simple demo in 3D space. Puts the audio source where the player is standing.
 **/
void OpenALPlayPositionalDemo(void)
{
    Warning("Positional audio currently out of order!!!\n");
    return;

    // Todo: Re-implement this through the sample pool

    /*
	CBasePlayer* localPlayer = CBasePlayer::GetLocalPlayer();

	OpenALStopDemo();

	demoSample = g_OpenALLoader.Load("ogg");
	demoSample->SetLooping(true);

	if (localPlayer)
	{
		demoSample->SetPositional(true);

		// Does localPlayer need locked now? :/
		demoSample->SetPosition(localPlayer->GetAbsOrigin());
		demoSample->SetVelocity(localPlayer->GetAbsVelocity());
	}
	else
	{
		Msg("OpenAL: Demo can't find local player. Local playback enabled instead of positional.\n");
		demoSample->SetPositional(false);
	}

	demoSample->Open(POS_DEMO_FILENAME);

	if (demoSample->IsReady())
		demoSample->Play();
    */
}

ConCommand openal_play_demo("openal_play_demo", OpenALPlayDemo, "Play the demo of OpenAL's ogg playback.");
ConCommand openal_play_positional_demo("openal_play_positional_demo", OpenALPlayPositionalDemo, "Play a demo of using OpenAL for positional audio.");
ConCommand openal_stop_demo("openal_stop_demo", OpenALStopDemo, "Stop the current OpenAL playback demo.");

#define WAV_SAMPLE "demo/wave_playback.wav"

IOpenALSample *wavSample = NULL;

void OpenALWavStop()
{
    if (wavSample != NULL)
    {
        wavSample->Stop();
    }
}

/***
 * Plays an even nicer little demo local to the player position.
 **/
void OpenALWavStart()
{
    if (wavSample == NULL)
    {
        wavSample = g_OpenALLoader.Load("wav");

        if (wavSample == NULL)
        {
            return;
        }
    }

    if (wavSample->IsPlaying())
    {
        wavSample->Stop();
    }

    wavSample->Open(WAV_SAMPLE);
    wavSample->Play();
}

ConCommand openal_wav_demo_play("openal_wav_demo_play", OpenALWavStart, "Play the demo of OpenAL's wav playback.");
ConCommand openal_wav_demo_stop("openal_wav_demo_stop", OpenALWavStop, "Stop the demo of OpenAL's wav playback.");

#define FLAC_SAMPLE "demo/demo.flac"

IOpenALSample *flacSample = NULL;

void OpenALFLACStop()
{
    if (flacSample != NULL)
    {
        flacSample->Stop();
    }
}

void OpenALFLACStart()
{
    if (flacSample == NULL)
    {
        flacSample = g_OpenALLoader.Load("flac");

        if (flacSample == NULL)
        {
            return;
        }
    }

    if (flacSample->IsPlaying())
    {
        flacSample->Stop();
    }

    flacSample->Open(FLAC_SAMPLE);
    flacSample->Play();
}

ConCommand openal_flac_demo_play("openal_flac_demo_play", OpenALFLACStart, "Play the demo of OpenAL's FLAC playback.");
ConCommand openal_flac_demo_stop("openal_flac_demo_stop", OpenALFLACStop, "Stop the demo of OpenAL's FLAC playback.");


#define MP3_SAMPLE "demo/demo.mp3"

IOpenALSample *mp3Sample = NULL;

void OpenALMP3Stop()
{
    if (mp3Sample != NULL)
    {
        mp3Sample->Stop();
    }
}

void OpenALMP3Start()
{
    if (mp3Sample == NULL)
    {
        mp3Sample = g_OpenALLoader.Load("mp3");

        if (mp3Sample == NULL)
        {
            return;
        }
    }

    if (mp3Sample->IsPlaying())
    {
        mp3Sample->Stop();
    }

    mp3Sample->Open(MP3_SAMPLE);
    mp3Sample->Play();
}

ConCommand openal_mp3_demo_play("openal_mp3_demo_play", OpenALMP3Start, "Play the demo of OpenAL's FLAC playback.");
ConCommand openal_mp3_demo_stop("openal_mp3_demo_stop", OpenALMP3Stop, "Stop the demo of OpenAL's FLAC playback.");
