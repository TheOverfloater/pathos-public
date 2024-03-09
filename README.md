# pathos-public
Pathos Engine - Public Release
Copyright (c) 2016 Andras Istvan "Overfloater" Lukacs/Andrew Stephen Lucas
Released under the MIT license

The Pathos Engine is a Quake-like engine developed as the basis for my own video 
game, which began life as a GoldSrc mod back in 2009. Ovet time though, due to engine 
limitations and Valve breaking old mods, I eventually decided to abandon GoldSrc 
and create this engine for myself.

Pathos started development in 2016 with very little progress until 2019. Development 
picked up speed up between 2019 and 2022, when the game was first fully ported onto 
the engine. Ever since then I have been fixing bugs and extending the engine with new 
features. Starting in early 2024, I decided that a release was not just possible, but 
it would be of great benefit to the entire hobbyist gamedev community, and the Half-Life 
modding community.

The development progress can be seen here:
https://www.youtube.com/playlist?list=PLQnXkjA1l7uamgh8_ZnrFPSQxm1Kr91oA

To run Pathos, you need the Visual Studio 2010 runtime libraries installed. These can
be located under "Pathos Engine/vsruntime". Although Pathos supports 32-bit, I don't
recommend using it, as it's antiquated and slow.

Pathos is a mainly single-player engine with limited to almost non-existent support
for multiplayer/networking. It supports both BSP Version 30(Half-Life 1 BSP) and
Pathos's own version of it, the "Pathos BSP Version 1" format, which itself is based 
on version 30, but with larger datatypes for extended limits.
Pathos uses it's own proprietary format for model geometry data in a separate file
that is produced by Pathos's studiomdl application. This format is called VBM, short
for "Vertex Buffered Model". VBM offers vertex weights, better floating point precision
and none of the texture coordinate issues of MDL like shifting, etc.
HL1 sprites are still used and supported by Pathos. They are the same format as they
were in Half-Life and are practically unchanged.

You can find additional information in the files under "Pathos Engine/docs". This 
documentation is not a full comprehensive documentation of all the engine features,
but it should be a good starting point.

I still have to write a file describing all the custom entities, but all of these are 
available in examples under "pathos/sources/mapsrc". The full list will still take some 
time, and I didn't want to delay opensourcing Pathos any longer. So here you go with
the first public release, which will keep being extended as time goes on.

You might notice I haven't provided a level editor yet. Since most people will use their
base Half-Life 1 models most likely, I recommend just using JACK and compiling your models
without the -q and -s options, which strip texture and model data respectively. If using
JACK and not VHE, include the -l option to unlock the 2048 vertex/normal limit. 

If you want to try the npc_security npc, then you'll want to go to your sound folder for
barney under "Half-Life/valve/sound/barney/" and copy th contents to the following folder:
"Pathos Engine/common/sound/security". Yeah, I was too lazy to revoice this NPC so far.

If you have any questions or concerns, please contact me at:
doommusic666@hotmail.com

Please check "credits.txt" for a more or less comprehensive list of whom I credit for the
work put into Pathos, and the mod files it was shipped with. If someone finds something
in Pathos's demo that belongs to them and are not credited, I apologize. A lot of the stuff
used in the demo, like some textures and props, were picked up already second-hand and not
documented as to where they came from or who made them. If this is the case, please write
me and e-mail and I will update the documentation accordingly.
