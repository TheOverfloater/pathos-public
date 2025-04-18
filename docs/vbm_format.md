# VBM format

# Description
The VBM format is Pathos's proprietary model format for storing geometric
information used for rendering. This format supports both vertex weights
and rudimentary facial animations.

I recommend using the Half-Life 2 SMD export/import tools for this purpose.
The models take VTAs(same as Half-Life 2) for facial animations. For an
example of such a model, check out "pathos/sources/modelsrc/npcs/harrison/"

Pathos supports a maximum of 4 vertex weights per vertex. You can compile
such models with the studiomdl too located under "pathos/sources/progs".
For ease of compilation, I recommend using the Pathos Model Viewer that
is located under "pathos/sources/progs/pmdlviewer". This tool has a built-in
tool that is meant to make compiling models far easier.

# Porting Half-Life 1 Models:
For porting GoldSrc models, your easiest option is to use the "mdlexport"
tool located under "pathos/sources/progs/". This command-line tool allows
you to generate PMF, VBM and texture files from the GoldSrc models.

I generally copy the models I need to a temporary folder and then set up
an output folder. You can convert a single model, or an entire folder of
models by specifying the folder containing the models, or just a single
MDL file for the tool. The second parameter needs to be the output folder.

Once the files are generated, you can copy the result into your mod folder,
and the models should now be in the Pathos VBM format and load properly.

Note: Models converted this way WILL NOT have vertex weights. For those,
you need to use your modelling program of choice, and add the vertex
weights as you need.

# Compiling VBM models:
Every VBM model is mostly the exact same as GoldSrc models, but you need
to set up the proper directories described in the "files.txt" file. You
will need to set up a sequence file called "reference_frame.smd" in your
model's source folder, which defines the bind pose. Every model needs this
file to tell it what the bind pose of the model is, as this is required for
vertex weights.

I generally just copy a reference smd file, and open it up. Then, I look
for the line starting with "triangles" and delete everything after that.
I recommend always taking a reference smd that stores the skinned parts,
if your reference SMDs do not have the same base pose.
