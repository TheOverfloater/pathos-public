Copyright (c) 2021 Andrew Stephen "Overfloater" Lucas

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

I want to thank the following parties for their part with developing this project:
 - Valve Software for the code referenced from the Half-Life SDK while developing 
   this project.
 - Id Software for the code referenced from the Quake 1 engine while developing 
   this project.
 - dreamstalker for the code referenced from ReHLDS while developing this project.
 - Magic Nipples for helping me with the motorbike's surface alignment code.
 - Yriy "BUzer" Sitnikov for the mod Paranoia, and the help he gave me while 
   developing the renderer code under the original mod version.
 - Ryokeen for his help with graphics programming back in the early days.
 - Various other sources whose snippets I used to solve some problems.
 - K.V. "Enko" Stepchenko for supplying the beam renderer code I referenced to write 
   my own version of it

Clarifications on what "referencing" means in this context:
Since a lot of people seem confused on what the context of "referencing x code" means, 
I will offer a concise explanation. Pathos was originally developed to replace GoldSrc 
as the basis for my own video game, that was the entire premise of it's existence and 
how it came to be.

This meant that I had to reproduce an engine that worked with more or less the same 
data and content developed over 15 years. Why? Simply because I could not realistically 
deviate much from the original mod version's functionality without forcing myself to 
rework an insurmountable amount of content. My first and foremost goal was to reach a 
state of compatibility with the content I already had.
So when I say I referenced code from ReHLDS, the Half-Life SDK or Quake, I mean that 
I examined the original code, functions, and wrote my own version of it without lifting 
any code whatsoever from the references. This means that the code in Pathos is all fully 
original code written by me that is meant to perform similar, close enough functionality 
as it did in the original engine.

Some code, some function names will be similar to what you could find in the referenced 
code, both in how they process data and what they do in general, but other pieces of code 
will be totally different. This always depended on how much I could deviate from the 
original functionality without breaking support for the content that already had existed. 
Whenever I had the chance to come up with an original implementation that did a better job 
than what I referenced, I went for that, like the save-restore system of Pathos that is 
completely original, as well as the server-client messaging system, the UI code, etc. 
Wherever Pathos code closely resembles the HLSDK code, like say the NPC navigation and node 
graph code, it does so because I could not afford breaking support for the navigation system 
I had in HL. Still, if you take a close look at this code and compare it to what you find in 
the HLSDK, you'll see that only a few functions closely resemble their HLSDK counterparts, 
like the routing table, which I admit is a bit over my head in how it is computed exactly. 
Other parts like the actual route building code is almost completely an original design and 
does not suffer from the HLSDK version's really poor version.

As far as actual confirmation on Valve being okay with Pathos, I have these two points to 
share:
- I actually had the chance to have a very brief communication about this with an actual Valve 
employee, who explained to me that as long as function names and the code was only  similar 
and not directly lifted, Valve has no problems with the code in question. I did not save this 
e-mail exchange because at the time a public release of Pathos was not even a consideration, 
and the legality of the code was not my primary concern, but rather I was hoping to have 
access to VHE 3.5 at the time.
- I was also informed that Valve themselves took a look at the public release later and 
concluded that while the code was similar, it was not directly lifted HLSDK code, and that 
Valve themselves are not concerned about Pathos and are okay with it.

Note:
If you don't find your name in the credits list, and you find code that seems
like it belongs to you, please contact me, and I will add your name to the
credits list if it checks out that I used your code.

Parts of the code were written ages ago, some are over ten years old, and
because of that a lot has been lost to time, and to my lackluster bookkeeping.
