#include "cbase.h"
#include "convar.h"
#include "openal.h"
#include "openal_oggsample.h"
#include "openal_wavsample.h"
#include "openal_sample.h"
#include "openal_loader.h"
#include "openal_mp3sample.h"
#include "openal_sample_pool.h"
//#include "c_basehlplayer.h"

#define OGG_DEMO_FILENAME "demo/demo.ogg"
#define POS_DEMO_FILENAME "demo/positional.ogg"

IOpenALSample* demoSample;
SampleHandle_t oggHandle;

/***
 * Stops all demos that rely on the demoSample pointer.
 **/
void OpenALStopDemo()
{
	if (demoSample != NULL)
	{
		if (demoSample->IsReady())
			demoSample->Close();

		delete demoSample;
		demoSample = NULL;
	}
}

/***
 * Plays a nice little looping demo local to the player position.
 **/
void OpenALPlayDemo(void)
{
	OpenALStopDemo();

	demoSample = g_OpenALLoader.Load("ogg");

	demoSample->SetLooping(true);
	demoSample->SetPositional(false);

	demoSample->Open(OGG_DEMO_FILENAME);

	if (demoSample->IsReady())
		demoSample->Play();
    */

    oggHandle = g_OpenALSamplePool.CreateNewSample(OGG_DEMO_FILENAME);
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
    */
}

ConCommand openal_play_demo("openal_play_demo", OpenALPlayDemo, "Play the demo of OpenAL's ogg playback.");
ConCommand openal_play_positional_demo("openal_play_positional_demo", OpenALPlayPositionalDemo, "Play a demo of using OpenAL for positional audio.");
ConCommand openal_stop_demo("openal_stop_demo", OpenALStopDemo, "Stop the current OpenAL playback demo.");

#define WAV_SAMPLE "demo/wave_playback.wav"

COpenALWavSample wavSample;
SampleHandle_t wavHandle;

void OpenALWavStart()
{
    /*
    if ( wavSample.IsReady() )
    {
        wavSample.Close();
    }

    wavSample.SetLooping(true);
    wavSample.SetPositional(false);

    wavSample.Open(WAV_SAMPLE);

    if (wavSample.IsReady())
        wavSample.Play();
    */
}

void OpenALWavStop()
{
    /*
    if (wavSample.IsPlaying())
    {
        wavSample.Stop();
        wavSample.Close();
    }
    */
}

ConCommand openal_wav_demo_play("openal_wav_demo_play", OpenALWavStart, "Play the demo of OpenAL's wav playback.");
ConCommand openal_wav_demo_stop("openal_wav_demo_stop", OpenALWavStop, "Stop the demo of OpenAL's wav playback.");

#define MP3_SAMPLE "demo/mp3_playback.mp3"

COpenALMp3Sample mp3Sample;
SampleHandle_t mp3Handle = SAMPLE_HANDLE_INVALID;

void OpenALMp3Start()
{
    if (mp3Handle == SAMPLE_HANDLE_INVALID)
    {
        EmitSound_t ep;
        mp3Handle = g_OpenALSamplePool.CreateNewSample(MP3_SAMPLE, ep);
    }
}

void OpenALMp3Stop()
{
    if (mp3Handle != SAMPLE_HANDLE_INVALID)
    {
        g_OpenALSamplePool.Stop(mp3Handle);
        mp3Handle = SAMPLE_HANDLE_INVALID;
    }
}

ConCommand openal_mp3_demo_play("openal_mp3_demo_play", OpenALMp3Start, "Play the demo of OpenAL's mp3 playback.");
ConCommand openal_mp3_demo_stop("openal_mp3_demo_stop", OpenALMp3Stop, "Stop the demo of OpenAL's mp3 playback.");