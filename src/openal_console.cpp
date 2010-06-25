#include "cbase.h"
#include "convar.h"
#include "openal.h"
#include "openal_oggsample.h"
#include "c_basehlplayer.h"

COpenALOggSample oggSample;

void OpenALOggDemoPlay(void)
{
	Msg("Initializing demo of OpenAL/OggVorbis support.\n");

	if (oggSample.IsReady())
		oggSample.Close();

	oggSample.SetLooping(true);
	oggSample.SetPositional(false);
	
	oggSample.Open("music/binarpilot/defrag/01 Goof.ogg");

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

	oggSample.Open("demo/positional.ogg");

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

ConCommand openal_ogg_demo_play("openal_ogg_demo_play", OpenALOggDemoPlay, "Play the demo of OpenAL's ogg playback.");
ConCommand openal_ogg_demo_stop("openal_ogg_demo_stop", OpenALOggDemoStop, "Stop the demo of OpenAL's ogg playback.");
ConCommand openal_positional_demo_play("openal_positional_demo_play", OpenALPositionalDemoPlay, "Play a demo of using OpenAL for positional audio.");
ConCommand openal_positional_demo_stop("openal_positional_demo_stop", OpenALOggDemoStop, "Stop the demo of using OpenAL for positional audio.");
