# vbmcompiler usage

The VBM Compiler tool is basically Pathos's version of the studiomdl application that
GoldSrc uses. As such, it is also perfectly capable of producing HL1 MDL files for use
in GoldSrc, should you wish to use it as.

Pathos uses the same SMD and VTA data as Source 1 models, so you need thoe export
tools for your animation/modelling program to have these features. I recommend the tools
created by CannonFodder, as those are the ones I use personally.

This tool supports the dynamic declaration of render modes used by textures, the activity
map used by NPCs for animation lookups, as well as dynamically declaring hitgroups. All of
these scripts are declared in the same style:

>{
>	{ entry_1 entry_2 }
>}

You have two outer '{' '}' brackets surrounding a single line defined with a set of '{' '}'
closing brackets. Each entry in this inner list is separated by spaces. For this reason, if
you need to define a token which has spaces in it, make sure to enclose it in double quotes
so the tool parses it properly.

Theese scripts are located in the same folder as the vbmcompiler.exe, and are the following:

## activitymap.txt:
Defines the ACT_ types used in sequence declarations in the QC file. The first token is a
string, and is the name of the activity, always beginning with "ACT_". The second token is
the ID number for the activity. Make sure this list matches with what you have in your game's
activity list for full compatibility.

## rendermode_defs.txt:
This script allows you to specify your own rendermode types for textures. This isn't used by
Pathos, but you can use it in GoldSrc or Xash to define your own render modes. The first token
is the name of the rendermode, and the second one is the bitflag value your game expects. By
default this list contains the render modes used by GoldSrc, but you can expand it to include
new ones used by Sven Coop, Xash, etc.

## hitgroup_defs.txt:
This contains the custom hitgroups used. Hitgroups with ID -1 and 0 are reserved for the two
hitgroups "HITGROUP_NONE" and "HITGROUP_GENERIC" respectively, and cannot be used. Everything
else is free to be modified. The first token is the hitgroup ID, which is always an integer,
and the second is the hitgroup name, such as "HITGROUP_HELMET", etc.

## bone_hitgroup_automap.txt
A new feature this tool supports is the auto-mapping feature for bones, when creating hitboxes
from scratch(if none are defined in the QC file). This mapping file allows you to have the tool
auto-assign hitgroups based on either the name of the bone, or a matching part of the name. It
also allows for specific bones to be discarded from being used for hitboxes.
The first token in an entry is the name, or partial name of a bone, such as "Finger" or "L Hand",
"Head", etc. The second entry is the name of the hitgroup, which must match what's in the hitgroup
definitions file. If you however specify "discard" for the hitgroup, the bone will be discarded
from having hitboxes generated for it. If you want any bones containing the word "Finger" to be
discarded, then just put an entry like this in the script:

>	{ "Finger" discard }
   
# Launch arguments
This tool adds on several new features compared to studiomdl. These include the original ones
that came with studiomdl, and new ones. Among these are the following command launch 
arguments:
 - s - Strips geometry information from the HL1MDL output. I recommend this if you want
      to save on disk space consumed.
 - q - Strips texture data, and places only single blank white textures into the file
      in order to not make level editors like JACK or VHE error out.
 - l - Normally vbmcompiler will try to limit polygon counts in the HL1MDL data to the
      2048 vertex/normal limit. Useful if you still want to have model data in the
	  HL1MDL so you can map using the original Valve Hammer Editor. However if you
	  want to have the full model data, just set the -l argument, and the models will
	  be compiled with their full geometry information.
 - w - The vbmcompiler command line will wait for a key input before closing the window.
 - r - Tags bad normals with a special texture.
 - n - Tags corrupted normals with a special texture.
 - f - Flips every normal on every triangle.
 - a - Specify the angle at which normals will be merged into a single one.
 - h - The tool will dump all the hitboxes to the console and the log file.
 - d - Sets it so the application does not produce a VBM file.
 - t - Tell the compiler to rename a texture. First parameter is the original texture's
      name, the second is the new name.
 - e - Set maximum texture resolution(for Xash or other engines which support higher res
      model textures.
 - p - Set the amount of pixels that textures will be padded out with if they need to be
      resized to power of two resolutions.
 
# New QC options
In order to support facial animations, a new feature was added. When specifying a
submodel via "$body" or "studio", you can add "flex" after the smd filename, and
then specify the .VTA file to use.

Here is an example:
>$body "studio" "harrison_head_reference" flex "harrison_head_expressions" <br />

Flex controllers are defined in the QC as well, and these will be referenced in the
facial animation scripts later. Here is an example:
>$flexcontroller "mouth_open" 0.000000 1.000000 "sine" { "harrison_head_expressions" 0 } <br />

Here the name of the flex controller is specified, followed by the min/max value, then
the blending method used for calculating flex strength. Then finally, the VTA file, along
with the shapekey index.

You can also specify bones which will not be removed from the MDL file by optimization
functions. This is specified with the $protected command as shown below:
>$protected "Bip01 Eye Center" <br />

Engine shapekeys:
Some shapekeys are defined in the engine and used for basic facial animation behaviors
like opening the mouth, and blinking. These below are such shapekeys:
>$flexcontroller "mouth_open" 0.000000 1.000000 "sine" { "harrison_head_expressions" 0 } <br />
>$flexcontroller "blink" 0.000000 1.000000 "sine" { "harrison_head_expressions" 1 } <br />

The keys "blink" and "mouth_open" are used for random blinking behaviors, and for the
mouth movements of characters respectively. If you name them as anything else, then
your character will not blink at all, or move their mouths.

# Model LODs:
You can define LODs for each submodel, which will be applied based on what
type of LOOD you define. There's two types, namely "distance" and "shadow"
LODs. Shadow LODs are solely used for rendering shadows for models, while
the distance-based LODs are applied based on the distance to the viewer.

Here is an example of how a LOD is defined in the QC:
>$lod "marine_body_reference" "marine_body_lod1_reference" distance 300<br />

The first parameter after $lod is the submodel name, then the second is the
SMD for the LOD itself. The third parameter is the type of LOD, and the final
one is the distance at which this LOD is used. For the shadow type you do not
need this parameter.

## Planned features:
New features that will be added to this tool are the following:
 - Auto-splitting of submodels: For GoldSrc, this would basically entail splitting submodels into
   multiple parts to ensure that the max vertex/normal limit is not passed. This would only work
   if the model has no bodyparts with alternate submodels.
 - Vertex/triangle reduction: If the model has switchable bodyparts, then the tool would use an
   algorythm to automatically reduce the vertex/normal count by removing smaller triangles.
 - 24-bit TGA/BMP downsampling: If loading a texture that is not 8-bit, the tool will automatically
   reduce the color depth to a 256 color palette, not needing the modeller to perform this manually.
 - Collision meshes: This feature would allow the modeller to create a simplistic triangle mesh that
   acts to provide collisions against players, NPCs and bullets. This would remove the need for the
   level designer to create collision meshes for props used in the level.

