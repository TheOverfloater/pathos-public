# pathos-public
Pathos Engine - Public Release
Copyright (c) 2016 Andras Istvan "Overfloater" Lukacs/Andrew Stephen Lucas
Released under the MIT license

The Pathos Engine is a Quake-like engine developed as the basis for my own video game, which began life as a GoldSrc mod back in 2009. Over time though, due to engine limitations 
and Valve breaking old mods, I eventually decided to abandon GoldSrc and create this engine to suit my needs for the game I was making.

Pathos started development in 2016 with very little progress until 2019. Development picked up speed between 2019 and 2022, when the game was first fully ported onto the engine. Ever since then I have been fixing bugs and extending the engine with new features. Starting in early 2024, I decided that a release was not just possible, but it would be of great benefit to the entire hobbyist gamedev community, and the Half-Life modding community. While in the end the engine has not picked up much steam apart from one or perhaps two projects, I will keep supporting the GitHub release for as long as I can with bugfixes, small feature changes and I'll respond to any issues posted or questions asked.

I made a sort of developer diary while I was actively working on Pathos, which I shared on my YouTube channel. You can find the list at the link below:
https://www.youtube.com/playlist?list=PLQnXkjA1l7uamgh8_ZnrFPSQxm1Kr91oA

To run Pathos, you need the Visual Studio 2022 runtime libraries installed. These can be located under "Pathos Engine/vsruntime". While initially Pathos supported a 32-bit version, this for the time being is no longer supported, and I recommend only compiling the 64-bit version of the engine and game binaries. I don't plan on restoring the 32-bit version in the foreseeable future.

Pathos is a mainly single-player engine with limited to almost non-existent support for multiplayer/networking. It supports both BSP Version 30(Half-Life 1 BSP) and Pathos's own versions of it, the "Pathos BSP" Version 1 and Verison 2 formats, which themselves are based on version 30, but with larger datatypes for extended limits. Pathos uses it's own proprietary format for model geometry data in a separate file that is produced by Pathos's studiomdl application. This format is called VBM, short for "Vertex Buffered Model". VBM offers vertex weights, better floating point precision and none of the texture coordinate issues of MDL like UV shifting, etc. HL1 sprites are still used and supported by Pathos. They are the same format as they were in Half-Life and are practically unchanged.

You can find additional information in the files under "Pathos Engine/docs". This documentation is not a full comprehensive documentation of all the engine features, but it should be a good starting point. This documentation will keep being updated as time goes on. There is now also a comprehensive entity document called "entity_guide.md" located in the same "docs" folder as mentioned above. This guide gives a comprehensive explanation of all entities and their functions in the game.

Pathos has no dedicated level editor yet, due to mainly the fact that creating such a tool would take about as much effort as writing the engine took. Due to the fact that not many people are using Pathos, I held off on this indefinitely. While creating a dedicated editor would be useful, or adapting an already open-source tool like Trenchbroom would be a good idea, I'm holding off on this due to the lack of any real interest. There is an in-house, unofficial level editor that is used by me, based on the 2003 leak version of Hammer, but heavily edited. I cannot release this version however, as I would prefer not to attract the ire of Valve. I did try to get a license for VHE 3.5 a while back, but was unsuccessful at it.

Since most people will use their base Half-Life 1 models most likely, I recommend just using JACK and compiling your models without the -q and -s options, which strips texture and model data respectively. If using JACK and not VHE, include the -l option to unlock the 2048 vertex/normal limit. 

If you want to try the npc_security npc, then you'll want to go to your sound folder for barney under "Half-Life/valve/sound/barney/" and copy the contents to the following folder: 
"Pathos Engine/common/sound/security". 
Yeah, I was too lazy to revoice this NPC so far.

If you have any questions or concerns, please contact me at:
doommusic666@hotmail.com

Please check "credits.txt" for a more or less comprehensive list of whom I credit for the work put into Pathos, and the mod files it was shipped with. If someone finds something in Pathos's demo that belongs to them and are not credited, I apologize. A lot of the stuff used in the demo, like some textures and props, were picked up already second-hand and not documented as to where they came from or who made them. If this is the case, please write me and e-mail and I will update the documentation accordingly.

For a basic list of those I credit, whose work I referenced to write my own versions of functions, etc, the non-comprehensive list of the most important sources is as follows:
 - Id Software for Quake 1, which was my main source of reference when writing some of the engine code when it comes to server physics, the basic engine layout and 
 - Valve Software for the Half-Life SDK, whose functionality I referenced to reproduce entity behavior, as well as some of the studiomodel related code for rendering, collision with hitboxes, etc.
 - s1lentq AKA Dmitry Novikov for ReHLDS, which I referenced for some GoldSrc-specific feature implementations like rotating entity physics, NPC movement, miscellaneous engine functions, etc.
 - Iriy "BUzer" Sitnikov for his help in teaching me how to do graphics programming, and for the Paranoia mod and toolkit, which helped me a lot with writing the first renderer version back in 2008-2009.
 - Ryokeen for helping me back when I was learning how to do graphics programming with the first renderer version.

Thank you all!
