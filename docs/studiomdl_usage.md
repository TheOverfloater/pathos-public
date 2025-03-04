# studiomdl usage

Currently Pathos relies on a modified version of GoldSrc's studiomdl, and for this
reason I cannot provide the source files with this release. Later on I will create
my own vbm compiler that will do away with this version, and it will be fully open-
sourced.

Pathos uses the same SMD and VTA data as Source 1 models, so you need thoe export
tools for your animation/modelling program to have these features.

# Launch arguments
This modified studiomdl adds on several new features. These include the original ones
that came with studiomdl, and new ones. Among these are the following command launch 
arguments:
 - s - Strips geometry information from the HL1MDL output. I recommend this if you want
      to save on disk space consumed.
 - q - Strips texture data, and places only a single blank white texture into the file
      in order to not make level editors like JACK or VHE error out.
 - l - Normally studiomdl will try to limit polygon counts in the HL1MDL data to the
      2048 vertex/normal limit. Useful if you still want to have model data in the
	  HL1MDL so you can map using the original Valve Hammer Editor. However if you
	  want to have the full model data, just set the -l argument, and the models will
	  be compiled with their full geometry information.
 - w - The studiomdl command line will wait for a key input before closing the window.
 
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
