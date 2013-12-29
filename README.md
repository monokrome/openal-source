OpenAL-Source
=============

This is a project which aims to embed OpenAL into the Source Engine with a few other useful/related tools. This project pushes to take Source to accomplish new feats which it hasn't been able to do with it's in-engine audio. OpenAL for Source provides many new features to your Source games in comparison to the audio system that comes inside of the engine.

Another benefit gained from a custom audio solution is that you have direct access to the audio code in the SDK, which will provide you with more flexibility in your source games.

Features:
---------

- Audio moves at the speed of sound and respects the doppler effect
- OggVorbis, MP3, FLAC and WAV Playback
- Links audio sources to entities so that they are consistently in sync
- Audio can be local to the player or positional
  - Uses for "local" audio include media players, in-game radio, etc.
  - Positional audio can be used to add depth to an environment
- Samples can be grouped together based on group names
- Relinking standard Source entities to use OpenAL instead of Miles

Coming Soon:
------------
- More in-game syncronization possibly including:
  - Under water audio distortion
  - Occlusion culling (if Source allows)
  - Deafening effects
- Entities allowing mappers to modify the state of the system
- Fourier analysis system for beat detection and more

Notes:
======

All samples currently are not transient. They will not be removed on map change or if any other game-based event occurs - aside from quitting the game altogether. This is being worked on.

Remember that multi-channel audio can not be used as positional audio, so make sure that all positional sound effects are mono files.

