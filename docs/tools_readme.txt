All the tools described here have more detailed descriptions under Pathos Engine/docs, so
please consult those files for more information.

 - mdlexport
This tool will take either a single HL1 MDL file, or an entire folder, and output the
result into another folder specified via the launch params. It will generate both the VBM
files and the PMF files, as well as the TGA files from the models.

An example of using it for an entire folder:
mdlexport D:\MDLExport\input D:\MDLExport\output

For a single model file:
mdlexport D:\MDLExport\input\awning.mdl D:\MDLExport\output

The generated textures are resized to power of two resolution. The PMF files will contain
all the necessary flags set, based on the ones that were already set inside the MDL file
for the individual textures.

 - studiomdl
This tool is used for compiling your .qc files into Pathos-compatible .mdl and .vbm files.
This can be used through the command prompt, but I would recommend using the included Pathos
Model Viewer for the purpose of compiling models, as it has a versatile set of utility
functions that will make life much easier for you. See "studiomdl_usage.txt" for more.

 - mdldec
This is Krastito's mdl decompiler from 2003. It will not decompile the VBM components, as
there's currently no tool available for that purpose. 

 - worlcraft
Due to the dubious legal nature of distributing a custom Hammer based on the 2003 leak, I
have as of yet decided not to include this in the release until later on. I still have to
decide what I will do in terms of a level editor, but the most likely bet is that I'll fork
TrenchBroom and add Pathos support.

 - pmdlviewer
Pathos's Model Viewer is based on the original Half-Life Model Viewer by Mike Ciragan, and
offers numerous useful features like a Compile Model tab, with the ability to copy the
resulting VBM and MDL files to a set of destination folders. See "model_viewer.txt" for
more information.