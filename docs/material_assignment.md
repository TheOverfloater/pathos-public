# Assigning material types 
Pathos allows you to more comfortably assign material types for each
of the textures in your level. Instead of manually editing your material
files, you can use console commands to help make it easier.

You have the ability to list all the textures in your level that use the
"default" material. This is usually concrete, which all newly generated
material files are set to.

Here at the commands listed below:
## r_list_default_materials
	This will generate a list of all materials in your map that are not assigned 
	a non-default material type. You can then use this list to select a texture 
	to display on-screen.
 
## r_show_list_material
	Use this with the index of the texture you want to display. The engine will 
	then diplay this material on-screen, so you know what texture you are working 
	with. Calling it with -1 will cause the texture to stop being diplayed on-screen.
 
## r_set_texture_material
	Call this with the material type name you want to assign to this texture. These 
	material types are the ones referenced in the footstep and materialdefs files in 
	your scripts folder. The command will then change the material type in the PMF 
	file, and write it to disk, and the texture will stop being displayed on-screen.
 
# Example
r_list_default_materials <br />
r_show_list_material 0 <br />
r_set_texture_material 0 computer <br />
... <br />
r_show_list_material 74 <br />
r_set_texture_material 74 concrete <br />
