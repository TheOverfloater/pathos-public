# Basic files used by Pathos

# Models:
Models are expected to be under the pathos/models folder, but subfolders here are
also supported. You need both a .mdl and .vbm file here, only HL1MDL will not work
and the engine will not load your model. See "tools_readme.txt" mentioned above on
how you can convert your HL1MDL files to Pathos's formats.
 
# Textures:
Pathos supports TGA, DDS and WAD textures. The current level editor still requires
WAD3 to be used, so textures used by levels need to be in .WAD files. However, this
does not mean you can't use your own external, hi-res file and have Pathos not load
the texture from the WAD file.

World textures will be searched under the following path:
"pathos/textures/world/*wad filename*

Model textures will always be searched under this path:
"pathos/textures/models/*model filename*
 
# Sprites:
Sprites will be located under "pathos/sprites", and are expected to be the same
format as Half-Life 1 sprites.
 
# PMF files:
PMF(Pathos Material File) files will be located in the specified folers. These
script define the basic properties of textures loaded by the engine. They can
specify multiple textures. All world/model resources look for PMF files that
specify the path to the individual images that need to be loaded.
 
# Level formats:
Pathos supports the Half-Life 1 BSP format(Version 30), as well as the Pathos
BSP V1 format. The vluzacn compilers shipped with Pathos will compile the levels
with the Pathos format.
 
# Shaders:
Pathos's shader code takes BSS(Branching Shader Script) scripts, which are located
under "pathos/scripts/shaders". These are the shaders on which the engine components
rely for rendering.
 
# Decals:
Decal textures are stored under "textures/decals". These decals require the color
that marks transperency to be an RGB value of 128, 128, 128. Alpha values will be
alpha tested by the engine.
 
# Sky textures:
Sky textures are located under "textures/sky". These follow the same alignment as
they did in Half-Life. Texture sizes are not limited, and you can use any size
skybox you want.
 
# Detail textures:
Not a predefined thing or constrained to a single folder, but you'll be able to find
an assortment of detail textures under "textures/detail".
 
# Caustics effects:
Water caustics textures are defined as a list of textures in the file "caustics_textures.txt"
under "textures/general". You can add your own textures in any number in this file.
 
# Flashlight/projective light textures:
The textures used for projective lights are defined in the "projective_textures.txt"
file located in "textures/general". The env_spotlight entity can then take an index
into this list to use as it's projective texture.

# UI:
The Pathos menu UI is defined in scripts located under "pathos/scripts/ui". You can
use these scripts to modify the appearence of the menu UI, but overall if you want
any modifications that go beyond stylistic changes, you'll need to edit the code.
