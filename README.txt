Embeds OpenAL into the Source Engine with a few other useful/related tools.

Features:
- OggVorbis Playback

Coming:
- Positional audio
- Linking audio sources to entities so that they are in sync

Currently, this library plays multichannel audio without any positional data. Al
samples are currently transient. They will not be removed on map change or if any
other game-based event occurs, aside from quitting the game altogether. This will
be changed in a later release.

