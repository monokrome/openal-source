Instructions for properly installing/building these files follow. The library
currently comes with Debug versions of the required library files which you
can use if you prefer not to immediately compile them.

Currently, you will need to compile and link against them yourself until
I compile Release variants and get the rest working. Make sure that you
have an OpenAL runtime installed before attempting to run your game with
OpenAL support.

If you are using the libraries provided in this package, you can safely
skip step 1 until you decide to build your project for release.

1) Download and install the following libraries:

   OpenAL (Not needed if you have hardware AL support):
     http://openal-soft-1.12.854-bin.zip

   libogg:
     http://downloads.xiph.org/releases/ogg/libogg-1.2.0.zip

   libvorbis:
     http://downloads.xiph.org/releases/vorbis/libvorbis-1.3.1.zip

2) Add the include directories from those libraries to your project. Make
   sure that your OpenAL headers are in an AL folder, your ogg headers are
   in an ogg folder, and your vorbis headers are in a vorbis folder. The
   include statements look like this:

   #include "AL/al.h"
   #include "ogg/ogg.h"
   #include "vorbis/vorbisfile.h"

3) Link the client DLL to the libraries that you've downloaded. OpenAL's
   statically linked libraries will then load the runtime that you installed
   earlier so that different machines can use different drivers.

   If you want to force users to use a specific driver, you can move it's DL
   so that it is sitting in the same directory as hl2.exe. However, I strongly
   recommend that you do not do this unless you have an insanely good reason
   to do so. Most likely, you don't - even if you think that you do.

4) Build your project

5) Copy the "sounds" directory from this project into your mod's GAMEDIR

6) To test OpenAL, start up your mod. You should immediately hear sound
   playing as the menu loads. This is because the demo is activated for
   autoload by default in this code. You can now safely open up openal.cpp
   and comment out this line near the top of the file:

   #define OPENAL_AUTOSTART_DEMO

G) You can run this console command to stop audio playback:
openal_ogg_demo_stop


A Note Regarding the Law

The audio distributed within this project has been created by the author
of the project to avoid any type of copyright infringement. There is
no violations of Copyright or other illegal distribution going on here.

The License

The code in this document is shared under the MIT license. The MIT license
grants the ability for this code to be used in any type of project whether
it be a commercial or hobbyist project. The only restrictions are that you
can't claim that you did the work that I did, and any issues that may arise
due to the use of this project are done "at your own risk". With that said,
I don't forsee there being any issues. If you have any other question
regarding the license of this project, please read LICENSE.txt or learn
more about the license at http://www.opensource.org/licenses/mit-license.php

