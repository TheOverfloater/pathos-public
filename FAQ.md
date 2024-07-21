# Fequently asked questions
Pathos Engine - Public Release
Copyright (c) 2016 Andras Istvan "Overfloater" Lukacs/Andrew Stephen Lucas
Released under the MIT license

# Question 1)
> Can I run my Half-Life mod under Pathos?

Answer:
You cannot directly move your mod to Pathos from Half-Life 1, as the two engines,
while similar, do not support eachother's binaries. Also running the GoldSrc client
and server binaries would be a violation of the terms of the Half-Life SDK. Your
only legal option is to take the Pathos server and client binaries, and recode the
features you want in your mod.
You also cannot use any Half-Life 1 content legally in Pathos, so if you do need to
use any Valve-created content in your game, treat is as placeholder only. You can't
release a game on Pathos that uses any content created by Valve.

# Question 2)
> Where can I find documentation on Pathos's features?

Answer:
You can find documentation under the "docs" folder in the root directory.

# Question 3)
> Where can I find the source files for Pathos?

Answer:
The source code can be found under "pathos/sources/codesrc/". You will be able to
find the source files of the test levels under "pathos/sources/mapsrc", and the
models under "pathos/sources/modelsrc/".

# Question 4)
> Is Pathos truly fully legal?

Answer:
Pathos was fully written by me, using SDL2 as a base. All the code you will find in
the engine is completely custom code written by me. I referenced code from ReHLDS,
the Half-Life SDK and the Quake 1 engine while writing Pathos. Nothing was lifted or 
copied from the Half-Life SDK, ReHLDS or Quake 1's engine code.

# Question 5)
> What exactly does referencing mean? Why did you need to reference these codebases?

Answer:
By referencing, I mean that I examined the code in these codebases, and created my
own, unique versions of these for features I needed to replicate. The reason why I
referenced these is, that I had to replicate behavior similar enough that 13 years
of content developed for my Half-Life mod would behave the same under the new engine
code. I had to maintain compatibility to not have to throw work out or rework the
entire already existing campaign and/or models.
Realistically I could not afford to write a completely new engine, and then also to
modify and adjust over 83 levels that have been created over the years. Navigation
code, NPC behavior and navigation, and the behavior of entities that are also there
in Half-Life 1 had to be a close match. Eventual adjustments and changes still were
made in the end, and so quite a few entities deviate from the HL1 equivalents.

# Question 6)
> I've looked at the Pathos source code, and a lot of functions seem similar, and some
> of these functions even have similar, or the same names. Is this still legal?

Answer:
As previously stated, some of the code is similar, some functions might almost be
the same because I had to program in some behavior in the code that is almost the
same, or similar to what the mod version had.
However if you take a look at some of the code that is similar, mainly the node
graph code or some of the NPC code, you will still see that there are fundamental
differences in the code still. Even the node graph code is vastly different from
what the HLSDK has, with the only similarities being the function that removes in-
line nodes, and the code that computes the static routing tables. Still these are
different than what the HLSDK has. It's still custom code, but it behaves similarly
to what the HLSDK has.
Why? Same answer I can share above: I had to maintain similar, or the same behavior
in order to not have to rework NPC navigation on over 83 levels. Even then I still
spent a major amount of time fixing issues with NPCs when it came to this, which
caused the code to end up being still quite different.

# Question 7)
> That is all fine and dandy, but can you actually prove that Pathos is legal?

Answer:
Legality is not a concrete subject, all I can say on my art is that I did not lift
any code, I did not copy any code out of the mentioned sources that I referenced, I
merely wrote my own version of the functionality I had to reproduce.

However, I had the chance twice to gain feedback on this from Valve. One time back
in 2022 I managed to get in contact with a Valve employee, who explained to me that
as long as function names were only similar, and the code might be similar, but still
custom, Valve does not consider it to be a problem. I did not keep this exchange
however, as at the time opensourcing Pathos wasn't even an idea. My goal was to just
clear up the legality question, but also I tried to gain access to VHE's source code,
which I failed to do so.

The second feedback came when I managed to speak with a person who had some contact
with some people at Valve. From what I was told, they did take a look at the source
code and were concerned about whether it used HLSDK code, however they ended up
realizing that while the code was similar to some degree, it was not the same code,
and thus did not consider it to be a problem.

So in the end the legality of Pathos will never be cleared to 100%, but what I can
in fact tell you is that I did my best to avoid any legal issues, and that Valve, as
far as they are concerned, do not have any problems with Pathos.

# Question 8)
> I would like X and Y feature in Pathos. Can you please code these in?

Answer:
Pathos's set of features expands based on whether I have time to work on them or
not, and I cannot just take up any request for new features. Outside of Pathos I am
still developing my own video game based on Pathos, and I also have a full-time job
and other life priorities that I need to attend to. If you want a feature in Pathos,
you will most likely need to pick up the code yourself and add it.

# Question 9)
> I want to add features to Pathos but I do not have much C++/OpenGL/GLSL knowledge,
> can you help me?

The amount of time I have for Pathos is limited. While I can offer some advise as I
have time available, I cannot fully dedicate myself to helping you with the features
you want to add. You will need to learn more about OpenGL and C++ if you want to add
more features to Pathos.

# Question 10)
>My facially animated models look strange after the latest update, with their faces
>looking all weird and having the wrong vertices move. What is causing this?

The engine in the July update had the facial flex vertex limit raised, which breaks
compatibility with previous model compiles. You'll need to recompile the model with
the latest studiomdl to fix this.
