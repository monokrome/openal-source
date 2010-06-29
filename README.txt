OpenAL-Source, Version 0.2.5
Embeds OpenAL into the Source Engine with a few other useful/related tools.

Features:
- Audio moves at the speed of sound and respects the doppler effect
- OggVorbis Playback
- Links audio sources to entities so that they are consistently in sync
- Audio can be local to the player or positional
  - Uses for "local" audio include a media players, in-game radio, etc.
  - Positional audio can be used to add depth to an environment
- Samples can be grouped together based on group names

Coming:
- More in-game syncronization possibly including:
  - Under water audio distortion
  - Occlusion culling (if Source allows)
  - Deafening effects
- More advanced file loading facilities
- Support for WAV, Flac, and possibly MP3
- Entities allowing mappers to modify the state of the system
- Relinking standard Source entities to use OpenAL instead of Miles

All samples currently are not transient. They will not be removed on map change
or if any other game-based event occurs, aside from quitting the game
altogether. This will be changed in a later release.

Remember that multi-channel audio can not be used as positional audio, so
make sure that all positional sound effects are mono files.

