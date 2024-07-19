# Water shader

Pathos uses scripts to define the parameters of water shader entities. This is done to save on the amount of
data being sent between the client and server, as well as to make it easier to specify the properties of water
in a level.

Water scripts are organized by level, the script's index defined in the water entity with "Script Index", and
whether it the day stage is nighttime or not. This means a water entity will seek a script under the following
file path:
"\*modfolder\*/scripts/water/water_\*levelname\*_\*scriptindex\*.txt
If the nighttime daystage is set, a "_n" is appended at the very end, like:
"\*modfolder\*/scripts/water/water_\*levelname\*_\*scriptindex\*_n.txt

For example, one of the water scripts used in the entity test levels is named as such:
water_entitytest2c_0_n.txt.

If a script cannot be found for a specific level and script index, then the script file "water_default.txt"
will be used. If this script is also missing, the engine will use default values defined in the code. If a
nighttime script cannot be found, it'll try to load the regular, non-night script file without the "_n" post-
fix.

Water entities employ optimizations, like combining water surfaces that share the same z coordinate for the
surface of the water into only rendering a single reflection for all of them. You can add more optimizations
using tokens in the scripts.

# The script file definitions

The scripts use a set of tokens and flags to specify the properties of water. These include the ability to set
the underwater fog, the color of the water, the degree of reflectivity, as well as several optimizations that
cut back on fidelity to improve performance.

The tokens and their effects/parameters can be seen below. Some parameters take values, others are just to be
put in the script by themselves and act as flags.

 - fresnel: Defines the reflectivity, where higher values reduce reflectivity, and lower ones increase the
 water surface's reflectiveness. This also depends on the height above the water surface.
 - colr: The underwater fog and water surface's red color component.
 - colg: The underwater fog and water surface's green color component.
 - colb: The underwater fog and water surface's blue color component.
 - fogend: The underwater fog end distance, where the fog reaches maximum saturation.
 - fogstart: The distance at which the underwater fog begins to take effect.
 - causticscale: The scaler for the water caustics texture, where smaller values will reduce fresnel density,
 while lower values increase the density of fresnel effects.
 - causticstrength: A value from 0 to 1, which determines the degree of caustics brightness.
 - causticstimescale: This determines the animation speed of water caustics effects.
 - lightstrength: The degree by which lightmap lighting is visible on a water surface.
 - specularstrength: The strength of specular right reflections on the water surface.
 - phongexponent: The phong exponent value, where larger values reduce the area of light reflectivity, while
 lower values increase it.
 - wavefresnelstrength: This sets how much the water ripples affect surface reflectivity, where a value of
 1.0 will cause very strong reflection breaks by the water waves, and a value of 0 will make the surface
 very smoothly reflective, almost mirror-like.
 - scrollu: Water surface texture scroll speed on the X axis of the texture.
 - scrollv: Water surface texture scroll speed on the Y axis of the texture.
 - strength: Sets the strength of the water distortion effect caused by the waves.
 - timescale: The scaler for the water texture waviness, where lower values will cause slower water movement,
 and higher values will make the waviness very rapid.
 - texturescale: Can be used to apply scaling on the water surface's texture, where higher values will make
 the texture scale smaller, and lower values will make the scaling of the texture bigger. This also depends
 on the scales the level designed set for the water surface texture.
 - refractonly: The water will only render refractions, and no water reflections. This will result in water
 that looks a lot like the "Mechmod water" effect sometimes used in GoldSrc. Best used for optimizing areas
 that don't need reflections or are too slow with them enabled.
 - cheaprefraction: Refractions will not be rendered in a separate pass, and instead will use current 
 contents of the screen to create a refracted underwater image. Water fog, and caustics cannot be used with
 this type of water. Best used for optimizing certain areas that don't need complex water.
