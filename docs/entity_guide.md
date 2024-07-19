# Entity Guide

# Description
This guide features a comprehensive guide to all the entities you can find in Pathos.
Thanks to TWHL for their entity guide, which I referenced while writing this document.

# Shared keyvalues
Some entities share keyvalues that will be noted down here instead of featuring them multiple times in the
document:

## Render Mode
>Sets the render mode of the entity. Pathos supports both lit and unlit(legacy) transparent entities. The
>console variable r_legacytransparents enables or disables whether rendering is done using "legacy"
>fullbright transparent entities, or all transparent entities will have lightmapping applied to them.
 - Values:
   - "Normal": Normal, completely opaque entity.
   - "Color": Transparency with a color applied.
   - "Texture": Transparency with just the texture applied.
   - "Glow": Modulate sprite size and brightness based on distance to the viewer. Only usable by sprite 
   entities.
   - "Solid": Legacy/unused, was used in Half-Life for alpha tested textures.
   - "Additive": Brightness is determined by the texture color component's brightness, where black is 
   completely transparent, and white is completely opaque.
   - "Lit Color": If legacy transparency is set, this will be the same as "Color" but with lighting 
   and textures applied.
   - "Lit Texture": If legacy transparency is set, this will be the same as "Texture" but with 
   lighting applied.
   - "Lit Additive": If legacy transparency is set, this will be the same as "Additive" but with 
   lighting applied. The lighting will also affect transparency of the surfaces.
   - "Unlit Solid": Alpha tested surfaces but without lightmapping.   

## Render FX
>Can be used to set a special render effect on the entity, most of which affect the transparency of the
>entity in question. You need to set the render mode of the entity to "Texture" or "Additive" for this to
>have any effect.
 - Values:
    - "Normal": No render effect.
    - "Slow Pulse:" The transparency of the entity will be modulated as a slow pulse.
    - "Fast Pulse:" The transparency of the entity will be modulated as a fast pulse.
    - "Slow Wide Pulse": The transparency of the entity will be modulated as a slow pulse with a wider 
	alpha value range.
    - "Fast Wide Pulse": The transparency of the entity will be modulated as a fast pulse with a wider 
	alpha value range.
	- "Slow Strobe": A slow strobe effect is applied to the transparency.
	- "Fast Strobe": A fast strobe effect is applied to the transparency.
	- "Faster Strobe": A much faster strobe effect is applied to the transparency.
	- "Slow Flicker": The entity's transparency will slowly flicker.
	- "Fast Flicker": The entity's transparency will rapidly flicker.
	- "Slow Fade Away": Entity slowly fades out.
	- "Fast Fade Away": Entity fades out fast.
	- "Slow Become Solid": Turn from completely transparent to fully opaque slowly.
	- "Fast Become Solid": Turn from completely transparent to fully opaque rapidly.
	- "Constant Glow": Used by env_glow and env_sprite only, the glow will not fade out over distances.
	- "Distort": The entity will have a distortion effect applied. Only works with VBM models.
	- "Hologram (Distort + fade)": Same as the hologram effect used in Half-Life's training levels.
	- "Traced Glow": If glow occlusion is not enabled, neither glow traces, this will still force the 
	env_sprite or env_glow to perform tracelines against brushmodels also for glow occlusion tests.
	- "Skybox Object": Only useful for VBM models and brushmodels. Will set the entity to be visible in a 
	skybox. The entity will be only visible in the skybox, and is not rendered for the main renderpass.
	- "Skybox Object - In Z": Only useful for VBM models and brushmodels. Will set the entity to be visible 
	in a skybox, but will not have it's contributions to the depth buffer cleared after the skybox rendering 
	is done, and will occlude fragments blocked by it's geometry.
	- "Scaled Skybox VBM model": Only useful for VBM models. Will set the entity to be visible in a skybox, 
	and will also have the "Scale" value applied.
	- "Glow Aura": The entity will form a glow aura around it much like usable objects and items do. The 
	color of the aura can be overriden using the "Render Color" keyvalue.
	- "Mirror Only": The entity will only be rendered in a mirror and not be visible to the player otherwise.
	- "Monitor Only": The entity will only be rendered in a monitor, and not be visible to the player 
	outside of the monitor entity.
	- "Angular Sprite": If set, the env_sprite's angles will be used as a forward angle to calculate how 
	much it shines based on what angle the player is looking at it from.
	- "In Portal Entity": Marks the entity to be rendered for func_portal_surface entities. The entity 
	will not be visible in the main renderpass.
	- "Scaled VBM model": "Scale" is applied to this VBM model, allowing you to scale the model used.
	- "Scaled VBM model In Portal": Same as "In Portal Entity" and "Scaled VBM model" but combined.
	- "No VIS checks": The entity will have no VIS checks done for it, meaning it will be visible across 
	all the level.
	
## Light style/Appearance
>This is the same as it was in Half-Life, except in Pathos you need to use dynamic lights to have switchable
>and/or animated lighting in your level. Pathos does not support HLRAD-based switchable lighting due to
>issues with performance and half the surfaces not getting their lightmaps updated on more complex levels.

 - Values:
   - "Normal": No animated light effect is applied.
   - "Fluorescent flicker": Fast flickering in and out.
   - "Slow, strong pulse": A slow pulse from dark to fully bright.
   - "Slow pulse, noblack": A slow pulse without going completely black.
   - "Gentle pulse": A slow, gentler pulsing effect.
   - "Flicker A": Strong flickering with high contrast in flickers.
   - "Flicker B": Strong flickering with high contrast in flickers.
   - "Candle A": Very slight flickering pattern.
   - "Candle B": Very slight flickering pattern.
   - "Candle C": Very slight flickering pattern.
   - "Fast strobe": A fast strobing effect.
   - "Slow strobe": A slow strobe effect. 
   
## AI Trigger Condition
>The AI condition to check for when wanting to trigger an AI trigger target. The NPC will keep in checking if
>the AI condition is met, and will trigger it when that is the case, before clearing the AI trigger altogether.
 
 - Values:
     - "No Trigger": No AI trigger condition is set.
     - "See Player, Mad at Player": If the NPC sees the player and is hostile towards them.
     - "Take Damage": When the NPC takes any damage.
     - "50% Health Remaining": When the NPC's health drops below 50%
     - "Death": When the NPC dies.
     - "Hear World": If the NPC hears a world sound.
     - "Hear Player": If the NPC hears a sound emitted by the player.
     - "Hear Combat": If the NPC hears sounds of combat.
     - "See Player Unconditional": If the NPC sees the player in any AI state.
     - "See Player, Not In Combat": If the NPC sees the player while not in combat.
     - "See Any Enemy": If the NPC sees any type of enemy.   
   
# worldspawn
>The world entity.

 - Keyvalues:
   - "environment map (cl_skyname)": The skybox texture set to use for the sky.
   
# ai_flagtoggler
>This entity controls the spawnflags of an NPC. You can specify whether the flag removed or set, and which 
>flag to modify.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Trigger Mode": The trigger mode determines whether a flag is enabled or disabled.
     - Disable: This flag is removed from the NPC's settings.
	 - Enable: This flag will be set for the NPC.
   - "Flag"
     - WaitTillSeen: NPC will not perform any combat behaviors until seen by the player.
	 - Gag: The NPC will not play any idle voice lines.
	 - NPCClip: NPC will not be able to cross func_npcclip entities.
	 - Don't forget enemy player: If player is an enemy, then the NPC will not forget about them over time.
	 - Prisoner: This NPC will not be able to attack other NPCs or the player unless shot at first.
	 - WaitForScript: The NPC will remain in a frozen state until called by a scripted_sequence.
	 - Pre-Disaster: If a friendly NPC, the NPC will not follow the player if used, and will say pre-
	 disaster voice lines.
	 - Fade Corpse: If killed, the NPC's corpse will fade out over time.
	 - Immortal: This NPC cannot be killed.
	 - Don't fall: The NPC will not immediately drop to ground on spawn.
	 - Corpse disappears when not seen: The NPC's corpse will vanish when not in the player's view.
	 - Don't drop weapon when killed: The NPC will not drop any of their weapons upon death.
	 - No pushing by player or others: Other NPCs and the player cannot nudge this NPC to get out of the 
	 way.
	 
# ai_setenemy
>This entity can be used to specify an enemy NPC for another. If triggered, the NPC targeted will have the 
>target NPC set immediately as it's enemy.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": The NPC to set the enemy for.
   - "Enemy's Name": Targetname of the NPC to set as the enemy.
   
# ai_setfollowtarget
>This entity allows you to set the follow target of an NPC. Usually used to set the player as the follow
>target of a friendly NPC. If the "Target" field is blank, then the follow target will be cleared.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": Entity to set as the follow target.

# ai_settriggercondition
>Sets the AI trigger conditions on an NPC. This will override any AI trigger conditions you previously set
>on the NPC entity itself.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": Target entity's name.
   - "TriggerTarget": Target entity to trigger upon AI condition being met.
   - "Trigger Condition": The AI condition to check for, see "Shared keyvalues" for more info.
   
# ai_wandercontroller
>Enables, toggles or disables wandering NPC behavior. This works for friendly wandering NPCs only, like the
>security guard NPC.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": Target NPC to set wandering behavior for.
   - "Trigger Mode": Mode of setting behavior.
     - Toggle: Behavior is toggled depending on what it is set for in the NPC at the moment.
	 - Enable: Wandering behavior is explicitly enabled.
	 - Disable: Wandering behavior is explicitly disabled.
	
# ambient_oggstream
>This entity can be used to play back OGG music files. It works on multiple channels, and sounds can be
>called to fade in and/or fade out, or to loop.

 - Keyvalues:
   - "Name": Name of this entity.
   - "File Name": Path to the music file, relative to "\*modfolder\*/sound", eg "music/test_track.ogg".
   - "Fade In Time": Time it takes for the music track to fade in when triggered on.
   - "Fade out Time: Time it takes for the music to fade out when triggered off.
   - "Channel": Pathos supports a total of four channels to play music on, use this to specify the channel.
 - Spawn flags:
   - "Remove on fire": The entity is removed after being triggered.
   - "Loop music": The music track will play on a loop when triggered on.
   
# ambient_generic
>An entity very similar to the one from Half-Life 1, ambient_generic is your go-to entity for playing back
>audio files in your level. Can be used for global sounds, for local sounds. This entity only supports
>using .wav files for now. The sounds played by this entity will be restored on save loads with proper
>offsets applied based on timings.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Path/filename.wav of WAV": Path to the .wav file to play. 
   - "Volume (10 = loudest)": Volume of the ambient_generic.
   - "Start Volume": Initial volume when triggered on.
   - "Fade in time (0-100)": Time it takes for the sound to fade in when triggerd on.
   - "Fade out time (0-100)": Time it takes for the sound to fade out when triggered off.
   - "Pitch (> 100 = higher)": Pitch of the sound playback, can be anything from 50 to 250.
   - "Start Pitch": Initial pitch value when triggered on.
   - "Spin up time (0-100)": Time it takes for the pitch to reach maximum when triggered on.
   - "Spin down time (0-100)": Time it takes for the pitch to reach minimum when triggered off.
   - "Custom Radius": If set, this'll override the radius set in spawn flags, and use this value instead.
   - "Emitter Entity": Specify a point entity which emits the sound instead of the ambient_generic.
 - Spawn flags:
   - "Play Everywhere": If set, the sound will be played regardless of player position as a 2D sound.
   - "Small Radius": The sound will have a radius of 512 units.
   - "Medium Radius": The sound will have a radius of 768 units.
   - "Large Radius": The soudn will have a radius of 1024 units.
   - "Start Silent": The ambient_generic will start in an off state. For looped ones only.
   - "Not toggled": The ambient_generic is expected to be triggered to play a sound each time it is 
   triggered, instead of being toggled on and off.
   - "XXL Radius": The sound will have a radius of 4437 units.
   - "XL Radius:" The sound will have a radius of 3657 units.
   - "XS Radius": The sound will have a radius of 102 units.
   - "No reverb effects" - The ambient_generic will not be affected by room type settings.
   - "No occlusion dimming" - The sound will not be dimmed if blocked by a wall.
   - "Dim other sounds" - Other sounds will be dimmer while this sound plays.
   - "Mute all other sounds" - If triggered to play, all other sounds will be muted off.
   
# ammo_glock_clip
>Ammo for the Glock pistol.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": And entity to trigger when the player walks over/picks up the item.
   - "KillTarget": An entity that will be killed when this entity is picked up/walked over.
   - "Delay before trigger": How long until the target/killtarget is triggered.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, doesn't work on this type of entity.
   - "Visibility Distance": The distance at which the item will remain visible.
   
 - Spawn flags:
  - "Not in deathmatch": This entity will not be present in multiplayer.
  - "No Pickup Notifications": There will be no sounds, HUD notifications played when this item is picked
  up by the player. It will just be silently add to their inventory.
  - "Trigger Only on Pickup": The entity will only trigger it's target when the player actually picks up
  the item, otherwise the target is not triggered if the player walks over the item.

# env_beam
>Can be used to add beam effects to the game, much the same as it was in Half-Life 1. You can have multiple
>entities share the start/end entity name, in which case the beam will randomly pick one of each set of
>targets as it's origin.<br />
>If you only specify a start entity and no end entity, then in the specified radius, the env_beam will do a
>trace in a random direction and pick the impact point as the end position of the beam spawned.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Start Entity": Name of the beam start position marker entity.
   - "Ending Entity": Name of the beam end position marker entity.
   - "Brightness (1 - 255)": Controls the brightness of the beam.
   - "Beam Color (R G B)": Color of the beam.
   - "Radius": The radius in which the beam will find an arc position if no end entity is specified.
   - "Life": Specifies how long the beam will be present after spawn. A zero value makes the beam a 
   toggleabble effect.
   - "Width of beam": Specifies the width of the beam, from 0 to 255
   - "Amount of noise": The amount of noise in the beam from 0 to 255, where 0 is completely straight.
   - "Sprite Name": The sprite to use for the beam.
   - "Texture Scroll Rate": The scroll speed of the beam texture, from 0 to 100.
   - "Frames per 10 seconds": The framerate at which the sprite cycles between frames.
   - "Starting Frame": The first frame at which the beam starts.
   - "Strike again time (secs)": Delay between each individual beam strike. If "Random strike" is set, 
   then this will be the maximum delay between beam strikes.
   - "Damage / second": Damage dealt each second if an entity enters the beam's path.
   - "Tesla beam?": If set, the beam will spawn multiple arcs along it's length.
 - Spawn flags:
   - "Start On": The beam will start in the on state.
   - "Toggle: Triggering the beam toggles it on and off instead of creating a new beam with each trigger.
   - "Random Strike": If set, the delay between each strike is randomized with "Strike again time" being 
   the maximum delay, and 0 being the minimum.
   - "Ring": The entity will spawn a beam ring instead of a beam line.
   - "StartSparks": Sparks will randomly spawn at the start of the beam.
   - "EndSparks": Sparks will randomly spawn at the end of the beam.
   - "Shade Start": The beginning of the beam will fade in smoothly over several segments.
   - "Shade End": The end of the beam will smooth out smoothly over several segments.
   - "No start/end segment fade": Pathos by default applies a slight fade to the end/start segments of the
   beam, but setting this will disable that completely.
   
# env_beamfollow
>This entity allows you to attach a beam to another entity that will follow this entity, leaving a fading
>trail along it's path. The trails will fade out over the time specified in the entity one after the other,
>from back to the current position.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": Target entity to follow.
   - "Render Color": Color of the beam effect.
   - "Brightness": Brightness of the beam, where 0 is completely transparent and 255 is fully bright.
   - "Duration": The lifetime of each individual segment. Segments fade out completely after this time.
   - "Beam width": Width of the beam created.
   - "Sprite": Sprite to use for the beam trail effect.
   - "Noise": Amount of noise in the beam effect.
   - "Attachment": If the target entity is a VBM type entity, then if you specify a non-zero attachment, 
   the beam will follow that attachment on the entity.

# env_beamfx
>This entity allows you to use the beam disk, torus and cylinder effects as you wish to. The effect 
>specified will spawn from the entity's origin and expand over time.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Render Color": Color of the effect.
   - "Brightness": Brightness of the effect, where 0 is completely transparent and 255 is fully bright.
   - "Radius: The distance that the beam will expand to over the given time.
   - "Start frame": Starting frame to use for the sprite.
   - "Duration": Duration of the effect until it is removed.
   - "Beam width": Width of the beam itself.
   - "Sprite": Sprite to use for the beam.
   - "Noise": Amount of noise in the beam effect.
   - "Scroll speed": The speed at which the beam texture scrolls forward.
   - "Effect type"
     - "Disk": A flat disk.
	 - "Torus": An inward-curved torus.
	 - "Cylinder": A straight-walled cylinder.
   - "Minimum repeat delay": Minimum delay before the effect spawns again.
   - "Maximum repeat delay": Maximum delay before the effect spawns again.
 - Spawn flags:
   - "Start on": The entity will start in the on state.
   
# env_beverage
>Much like in Half-Life, this entity will spawn a soda can when triggered, and touching the soda can will
>heal the player by some amount.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Angles": Angles to set for the beverage entity on spawn.
   - "Capacity": Max number of times a soda can can be spawned.
   - "Beverage type": The type of beverage to spawn, it will be based on the skins in can.mdl.

# env_blood
>This entity can be used to spawn blood particles at it's origin. In Pathos this mostly just spawns the 
>blood particle effect using the particle engine.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Angles": Specifies the direction to shoot the blood particles in.
   
# env_blackhole
>Creates a black hole effect, complete with a shader effect that distorts the light around it like a real
>black hole would. It can also pull in objects, particles, client-side temporary entities, killing them. If
>the player is pulled in, they will be killed with a black screen.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Scale": Scales the size of the black hole effect.
   - "Pull strength": Strength of the pull effect. If zero, the black hole will only be a visual effect.
   - "Rotation speed": This controls the rotational velocity given to objects pulled in by the black hole.
   - "Growth time": Time in seconds until the black hole reaches full growth and force.
   - "Shrink time": Time in seconds until the black hole evaporates after being triggered off.
   - "Lifetime": Delay until black hole disappears. If set to -1, the black hole is toggleable.
   
 - Spawn flags:
   - "Start on": The black hole will be on by default.
   
# env_blur
>This entity controls the motion blur effect for the screen. It can be toggled on and off, and you can
>specify the blur fade value, which controls how fast the motion blur fades as the camera moves. This also
>enables a gaussian blur effect across the screen.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Blur fade": The fade time of the individual blur frames, set to a higher value to have a slower fade, 
   or to a low amount for a faster fade.
 
 - Spawn flags:
   - "Start on": The blur will be enabled by default.
   
# env_bubbles
>A brush-based entity, env_bubbles will spawn bubble trails within it's volume that will rise to the height
>of the brushmodel until disappearing.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Angles": Specifies the direction of the water current.
   - "Bubble density": The amount of bubbles spawned at once.
   - "Bubble frequency": The delay in seconds between spawning bubbles.
   - "Speed of Current": The water current speed.
 
 - Spawn flags:
   - "Start Off": If set, the entity will be off by default.
   
# env_cable
>A very simple entity for spawning cables between two points. This entity cannot be toggled on or off, and
>doesn't use a sprite at the moment, instead being just a solid black curve.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": Target entity name. Can be an info_target or another env_cable entity.
   - "Depth": The lowest point to which the bezier curve drops from the base.
   - "Width": Width of the cable.
   - "Segments": The number of segments in the cable, a higher value gives a smoother curve.
   
# env_cubemap
>This entity functions much like it's Source counterpart, in that it will create a cubemap image of the
>environment around it when you execute "r_buildcubemaps". This capture is daystage-specific, and for the
>nighttime a different set of images will be captured. In a future update I intend to make decals 
>toggleable as a new feature.

 - Keyvalues:
   - "Size": Choose from a different set of resolutions to use for the cubemap.
   
# env_decal
>The engine's version of infodecal, this entity allows you to spawn a specific decal, or a random one from
>a decal group defined in "scripts/decal_list.txt". Such decals can also be set to grow to full size over
>a given time.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Decal entry/group name": Name of the particular decal, or the decal group to use.
   - "Growth duration": The time it takes for the decal to expand to it's full size.
   
 - Spawnflags:
   - "Wait for Trigger": The decal will not spawn unless triggered to do so.
   - "Transfer at level change": This decal will be transferred to another level if in the transition 
   volume and PVS of the info_landmark and/or info_vismark entities tied to that level change.
   - "Don't decal prop models": This decal will not be applied onto prop VBM models like env_model.
   - "Normal permissive": The decal will be applied onto surfaces and VBM polies regardless of how closely 
   the target surface's/triangle's normal faces towards it.
   - "Random decal": The decal will pick a random decal out of a decal group specified in the entity.
   
# env_dlight
>A dynamic light entity that allows you to place a non-static light into the level that can have shadows
>that are either static(only rendered once, contains only static entities in the shadowmap), and then will
>use that for the remainder of being on, or will cast a dynamic shadowmap that is updated each frame. Use
>this entity for switchable and/or animated lights.

 - Keyvalues
   - "Name": Name of this entity. If no name is set, the dynamic light is on by default.
   - "Render Fx": Only the Render Fx values of "In Portal Entity, "Skybox Entity" apply to this entity, and
   can be used for dynamic lights that appear inside the skybox.
   - "Light style/Appearence": Check the "Shared keyvalues" section for more on this.
   - "Interpolate Custom Appearence": If set, the "Custom Appearance" will have it's values interpolated.
   - "Custom Appearance": Specify a custom appearence, from letter a to z, which specify the light
   strength at a given frame.
   - "Radius": Max area of effect in units.
   - "Render Color": Color of the dynamic light.
   - "Vertical Oscillation": Amount in units that specifies how much the light oscillates vertically.
   Oscillating lights will not work as static dynamic lights.
   - "Horizontal Oscillation": Amount in units that specifies how much the light oscillates on the X/Y axis.
   Oscillating lights will not work as static dynamic lights.
   - "Static": If set to static, the shadowmap is only rendered into once and will only contain static,
   non-moving entities in it. Use this setting to optimize on scenes where you don't mind not having
   shadows be cast from moving objects.
   - "Disable Shadowmaps": Disables shadowmaps entirely, creating a light that will shine through walls and
   other objects.
 
 - Spawn flags:
   - "Start On": If the entity has a targetname but you want it to be on by default, set this flag.
   
# env_dlighttrain
>Similar to env_dlight, but this entity cannot be toggled on and off, and instead acts like much like a
>func_train entity, and will follow path_corners and be affected by trigger_movetrain.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Global Entity Name": If set, this entity will be transferred across a trigger_changelevel, and will
   try to find an entity with the same globalname on the other level, then will override it. Also note that
   env_dlighttrain and those entities like it will seek out a path_corner with the exact same name as they
   were travelling to when the levelchange occurred, in order to restore themselves. If they cannot find
   this path_corner, it will be treated as an error.
   - "First stop target": The path_corner the dynamic light will start at.
   - "Speed (units per second)": Speed of the entity when moving.
   - "Radius": Max area of effect in units.
   - "Render Color": Color of the dynamic light.

 - Spawn flags:
   - "No fire pass": This entity will not trigger "Fire on Pass" targets marked in path_corner entities when
   passing one.
   - "Start On": The entity will start moving on spawn.
   
# env_earthquake
>While it sounds similar to env_shake, this entity instead causes the view of the player to recieve random
>punching and kicks, and also causes the player to stumble around in random directions like he would during
>an earthquake. Best used in conjuction with an env_shake.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Minimum delay": Minimum delay between view punches and random jolts.
   - "Maximum delay": Maximum delay between view punches and random jolts.
   - "Minimum force": Minimum amount of force to apply during a kick/jolt.
   - "Maximum force": Maximum amount of force to apply during a kick/jolt.
   - "Duration": Specifies how long the effect lasts.
   - "Fade out time": If specified, the kicks/jolts will begin fading out for this duration before the
   entire duration of the effect ends.

# env_elight
>This entity procudes a light that will only affect VBM models and nothing else. This is used for setting
>the per-pixel lighting of models in a scene lit by static lightmap-based lights computed by HLRAD, so that
>model lighting quality is greatly improved.

 - Keyvalues
   - "Name": Name of this entity.
   - "Radius": The radius in which the light will affect objects. Due to legacy reasons this value will be
   multiplied by "9.5", the value set in the cvar "r_modellightfactor" by default. So if you want to specify
   a specific radius, divide your units by the value of the cvar.
   - "Render Color": Color of the light.

 - Spawn flags:
   - "Start On": If your entity light has a targetname, and you set this flag, the entity will spawn in the
   "On" state.
   
# env_elighttrain
>Very similar to env_dlighttrain, but instead this light entity, like env_elight, will only affect VBM
>models and not the world.

 - Keyvalues
   - "Name": Name of this entity.
   - "Global Entity Name": If set, this entity will be transferred across a trigger_changelevel, and will
   try to find an entity with the same globalname on the other level, then will override it. Also note that
   env_elighttrain and those entities like it will seek out a path_corner with the exact same name as they
   were travelling to when the levelchange occurred, in order to restore themselves. If they cannot find
   this path_corner, it will be treated as an error.
   - "First stop target": The path_corner the dynamic light will start at.
   - "Speed (units per second)": Speed of the entity when moving.
   - "Radius": The radius in which the light will affect objects. Due to legacy reasons this value will be
   multiplied by "9.5", the value set in the cvar "r_modellightfactor" by default. So if you want to specify
   a specific radius, divide your units by the value of the cvar.
   - "Render Color": Color of the dynamic light.
   - "Radius": The radius in which the light will affect objects. Due to legacy reasons this value will be
   multiplied by "9.5", the value set in the cvar "r_modellightfactor" by default. So if you want to specify
   a specific radius, divide your units by the value of the cvar.
   - "Render Color": Color of the light.
   
 - Spawn flags:
   - "No fire pass": This entity will not trigger "Fire on Pass" targets marked in path_corner entities when
   passing one.
   - "Start On": The entity will start moving on spawn.
   
# env_explosion
>Similar to the Half-Life 1 equivalent, this entity will cause an explosion when triggered, that, depending
>on the spawn flags set, will cause damage to nearby objects.

 - Keyvalues
   - "Name": Name of this entity.
   - "Magnitude": The force of the explosion.
  
 - Spawn flags:
   - "No Damage": The explosion will not damage anything nearby.
   - "Repeatable": This entity will not be removed after triggering, can be called multiple times to repeat 
   the explosion.
   - "No Fireball": No fireball will be produced.
   - "No Smoke": No smoke particles will be spawned.
   - "No Decal": The explosion will not spawn a scorch decal beneath it.
   - "No Sparks": The explosion will not spawn spark shower entities.
   
# env_fade
>This entity allows you to apply a fade effect to ths screen, either fading from or fading to. You can also
>stack multiple fades ontop of one another by specifying different layers for different env_fades.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Duration (seconds)": Time it takes to reach full alpha, or to fade from full alpha to completely
   transparent.
   - "Hold Fade (seconds)": Determines how long the fade stays at full alpha.
   - "Fade Alpha": Specifies the maximum opaqueness the fade will have.
   - "Fade Color (R G B)": Color of the fade effect.
   - "Fade layer": The layer on which the fade effect is applied. Fades stack ontop of eachother.
   
 - Spawn flags:
   - "Fade From": Setting this will make the fade stay for the amount of seconds set in "Hold Fade", before
   fading from max transparency to completely transparent.
   - "Modulate": Currently does nothing, I need to figure out what this did in HL1 exactly.
   - "Activator Only": Only apply this effect to the activator player.
   - "Permanent": The fade will remain active even across level transitions and will not be cleared.
   - "Stay Out": The fade will ignore the "Hold Fade" setting and will remain active until cleared by
   another env_fade on the same layer.
   - "Start On": The env_fade will start on at level spawn.
   
# env_fog
>This entity allows you to control a radial fog effect which is applied to the world, and optionally, the
>skybox. This entity can be set to blend with another env_fog entity upon being switched off, allowing you
>to have a smooth transition between different fog settings.<br />
>[!IMPORTANT]
>If you want to switch between two fog entities, both will need to have the "Blend" flag be set, and will 
>need to have a blend time specified. Blending doesn't work if you want to switch fog off entirely, it only 
>works when switching between two env_fog entities. Also note that if you have the "Blend" flag set and you
>want to switch all fog entities off, the fog will not be cleared.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Start Distance": Distance at which the fog effect starts to be applied.
   - "End Distance": Distance at which the fog reaches maximum saturation.
   - "Fog Color (R G B)": Color of the fog effect.
   - "Blend Time": Time it takes for this fog's properties to fully be applied on being switched on.
   - "Don't Fog Sky": Setting this to "No" will set it so the sky is fully covered by fog, while setting it
   to "Yes" will cause the sky to remain clear.
   
 - Spawn flags:
   - "Start On": The fog will start on by default.
   - "Blend": Set this flag if you want to blend between two fog entities.
   - "Clear Others on First": When this fog is switched on, it will set it so that any previous fog settings
   are cleared from memory. Useful if you have fog blending, but you want to switch another blended fog
   entity on and don't want it to blend when first turned on.
  
# env_funnel
>This entity is basically the same as env_funnel from Half-Life 1, but it allows you to set a custom sprite
>if you want. Funnel sprites can burst out from the origin, or travel towards the center based on the flag
>"Reverse" being set.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Sprite Name": Path to the sprite to use.
   
 - Spawn flags:
   - "Reverse": If set, particles will all be pulled into the center before disappearing.
   
# env_global
>An entity that allows you to set a global state with a name identifier that can be accessed by entities
>across levels. You can also set the state of this global variable, which will determine how entities like
>multisource and trigger_auto will react to it. This also allows you to set the global state of entities
>that use the "Global Entity Name" field.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Global State to Set": The name/identifier of the global state to store in memory.
   - "Trigger Mode": The state of the global that will be set when this entity is triggered.
     - "Off": The global will have an "Off" state set, meaning it won't trigger any trigger_auto and/or
	 multisource entities that read the state of this global.
	 - "On": The global being on means that any entities tracking the state of this global will trigger
	 their targets or will be enabled.
	 - "Dead": The global being dead will cause any entities that use the global name specified to be
	 removed from the game entirely upon level load. Useful for killing global entities across levels if
	 they have the "Global entity name" specified.
	 - "Toggle": Triggering the entity will cause the global variable's state to be toggled on or off
	 depending on it's current state.
	 - "Deleted": The global state will be entirely deleted from the stack as if it never existed. This
	 will not delete global entities across levels though, so keep that in mind.
   - "Initial State": The initial state to be set when the env_global spawns with the "Set Initial State"
   spawnflag enabled.
   
 - Spawn flags:
   - "Set Initial State": When this spawnflag is set, the env_global will set the state specified in the
   initial state keyvalue when it spawns.
   
# env_glow
>A static, non-switchable sprite entity used to set glows in the level. You need to specify the "Glow"
>rendermode for the glow effect to be applied.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied to the sprite.
   - "Sprite Name": Path to the sprite to use.
   - "Scale": Scale factor to be applied to the sprite.
   
# env_ladder
>This entity allows you to place a ladder into the game, that uses first-person animations to provide a more
>realistic ladder climbing experience. You need to place the origin of the ladder directly near the wall
>where you want the ladder to be, and the height of the ladder model needs to be aligned to the wall height.
>The bottom point of the ladder is determined by the marker position entity, which is usually an info_target
>entity placed beneath the origin point.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, doesn't work on this type of entity.
   - "Model": The VBM model to be used with this entity.
   - "Bottom marker": The entity which marks how deep the ladder reaches. Usually, this is an info_target
   entity.
   
 - Spawnflags:
   - "Top Access": Set this if you want this ladder to be accessed from the top.
   - "Start Disabled": The ladder will remain disabled until triggered on.
   
# env_laser
>An entity very similar to the env_beam entity, except it can have a sprite at the end, and deals damage
>to anything it hits.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Render FX": See "Shared keyvalues" for more information.
   - "Target of Laser": The entity the laser will be shooting at.
   - "Brightness (1 - 255)": Controls the brightness of the laser.
   - "Beam Color (R G B)": Color of the laser.
   - "Width of beam": Specifies the width of the laser, from 0 to 255
   - "Amount of noise": The amount of noise in the laser from 0 to 255, where 0 is completely straight.
   - "Sprite Name": The sprite to use for the laser.
   - "Texture Scroll Rate": The scroll speed of the laser texture, from 0 to 100.
   - "Frames per 10 seconds": The framerate at which the sprite cycles between frames.
   - "Starting Frame": The first frame at which the laser starts.
   - "Damage / second": Damage dealt each second if an entity enters the laser's path.
   
 - Spawn flags:
   - "Start On": The beam will start in the on state.
   - "StartSparks": Sparks will randomly spawn at the start of the beam.
   - "EndSparks": Sparks will randomly spawn at the end of the beam.
   - "Decal End": A decal will be applied where the beam hits anything.
 
# env_lensflare
>A lens flare effect that will be applied if the entity is in the player's view, complete with light haloes
>and a glow at the base.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Render Color": Color of the lens flare.
   - "Render amount": Transparency of the lens flare.
   - "Scale": Scale factor to be applied to the lens flare.
   
 - Spawn flags:
   - "Start on": If the entity has a targetname and this flag is set, the entity will be on by default.
 
# env_message
>Used to display a message on the screen. Messages need to be declared in titles.txt for this to work. It
>can also play an ambient sound if specified.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": Target entity to trigger when this entity gets triggered.
   - "Message Name": Name of the message declared in titles.txt to use.
   - "path/filename.wav of WAV": Path to .wav file to play when triggered.
   - "Volume 0-10": Volume of the sound to play, from 0 to 10.
   - "Sound Radius": Radius in which to play the sound.
     - "Small radius": The sound will have a radius of 512 units.
	 - "Medium radius": The sound will have a radius of 960 units.
	 - "Large radius": The sound will have a radius of 1024 units.
	 - "Play everywhere": The sound will be played globally as a 2D sound.
	 
 - Spawn flags:
   - "Play Once": If set, this env_message will be removed after being triggered.
   - "All Clients": Display the message for all players.
   
# env_model
>Your go-to for placing models into the game, this entity can be used to place animated prop models into
>your levels. env_model entities without targetnames are further optimized by not being handled by the
>engine, but rather on the client-side so that they do not take extra processing for game logic. These
>can also have decals applied by gunshots, blood, etc.

 - Keyvalues:
   - "Angles": Angles of the model.
   - "Name": Name of this entity.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, doesn't work on this type of entity.
   - "Model": VBM model to use.
   - "Body": The body value to use.
   - "Skin": Skin group to use.
   - "Sequence index": If the "Sequence name" is not set, this will specify what animation to use.
   - "Sequence name": Allows you tp specify the animation by name to use.
   - "Frame rate": Animation framerate multiplier value.
   - "Light Origin": The info_light_origin entity to use for getting lighting from the lightmap.
   - "Disable Shadows": If set, the entity will not cast any shadows.
   - "Scale": If the Render Fx "Scaled VBM model", "Scaled Skybox VBM model", or "Scaled VBM model In Portal"
   are applied, the VBM model will be scaled by this factor. Default value is 1.0.

 - Spawn flags:
   - "Triggering Changes Skin": When triggered, the entity will be set to use the next skin group.
   - "No elight tracelines": env_elight entities will not check occlusion with tracelines, meaning they will
   all light up the model regardless of being blocked by world geometry or not.
   - "No VIS checks": The entity will have VIS disabled and will be visible from everywhere.
   - "Start Invisible": The entity will be invisible until triggered on.
   
# env_model_breakable
>A breakable entity that uses a VBM model. It works exactly the same as func_breakable, in that you need to
>specify a material type and an object type to spawn. This entity will use the hitboxes within the VBM model
>to use for player collisions.

 - Keyvalues:
   - "Angles": Angles of the model.
   - "Name": Name of this entity.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, doesn't work on this type of entity.
   - "Global Entity Name": If set, this entity will be transferred across a trigger_changelevel, and will
   try to find an entity with the same globalname on the other level, then will override it.
   - "Target on Break": Entity to trigger when broken.
   - "Strength": The health points this entity will have.
   - "Material type": Type of material this breakable has.
   - "Gibs Direction": Direction gibs will take when broken.
     - "Random": Gib velocity is completely random.
	 - "Relative to Attack": The direction will depend on the attack vector.
   - "Delay before fire": Amount of time to wait before triggering the target.
   - "Gib Model": The VBM model to use for the gibs.
   - "Spawn On Break": The item type to spawn when broken.
   - "Explode Magnitude (0=none)": If set, the breakable will explode upon being broken, and deal damage to
   nearby entities.
   - "Spawn chance(1 == always)": The chance of spawning an object, based on a 1 out of n approach, where 
   "n" is the value specified in this field.
   - "Model": The VBM model to use for this entity.
   - "Body": The body value to use.
   - "Skin": The skin group value to use.
   
 - Spawn flags:
   - "Only Trigger": The entity cannot be broken by attacks, and will only break when triggered by another.
   - "Touch": Touching this entity will immediately cause it to break.
   - "Pressure": Applying pressure will cause this entity to break after a delay.
   - "Instant Melee": If this entity is hit with a Melee weapon(like the knife), it will break immediately
   regardless of the health points set.
   - "No gibs": The breakable will not spawn any gibs.
   - "No penetration damage": Damage from bullets that have penetrated through walls prior to hitting this
   entity will not deal any damage.
   - "Smart ammo spawn": If set, the "Spawn on Break" setting is ignored, and instead the breakable will
   spawn ammo for the weapon the player has the lowest amount of ammo for.
   
# env_modeltrain
>Similar to env_dlighttrain and env_elighttrain, except this entity will spawn a moving VBM model that
>follows path_corner entities.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Global Entity Name": If set, this entity will be transferred across a trigger_changelevel, and will
   try to find an entity with the same globalname on the other level, then will override it.
   - "Angles": Angles of the model.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, doesn't work on this type of entity.
   - "First stop target": The path_corner entity the model train will start at.
   - "Model": The VBM model to use for this entity.
   - "Speed (units per second)": Movement speed of the entity.
   - "Angular Veocity (y z x)": Rotational velocity on specific axes.
   
 - Spawn flags:
   - "No fire pass": This entity will not trigger "Fire on Pass" targets marked in path_corner entities when
   passing one.
   - "Start On": The entity will start moving on spawn.
   
# env_particle_system
>The particle system entity allows you to spawn particle effects defined by a system or cluster script. The
>type of script needs to be specified properly, otherwise you will encounter an error. Particle systems can
>also be attached to other entities, and to follow a specific attachment on that entity.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": The target entity that this particle system will follow.
   - "Angles": The angles specifying the direction in which particles will be emitted.
   - "Definition File": The script that defines the behaviors of this particle entity.
   - "Script Type": The type of script to use, can be a system script, or a cluster script.
   - "Attachment on target": The attachment point to use, if attached to an entity that uses a VBM model.
   - "Attachment mode": Defines the way the particle system will attach to it's target entity.
     - "Entity origin": Follow entity origin while shooting particles in the direction set in "Angles".
	 - "Entity attachment": Follow attachment specified by index on target entity while shooting particles 
	 in the direction set in "Angles".
	 - "Entity attachment": Follow first attachment specified by index on target entity, while shooting the
	 particles in the direction specified by the attachment, and the attachment by the index right after it.

 - Spawn flags:
   - "Start On": If this entity has a targetname, setting this will enable the entity on spawn.
   - "Remove On Fire": The particle system will be deleted after it is triggered on.
   - "Not Toggled": If set, this particle system will not clear it's particles when triggered on, and will
   instead just spawn again.
   
# env_particleeffect
>This is an entity I created moreso for my nostalgia reasons than any actual use. It will spawn Quake 1
>particles based on the type specified. Some effects will need you to specify a Quake 1 palette range,
>and/or a target entity. The effect can be set to repeat on a loop.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": The target entity that this particle effect will accelerate towards. Needed by certain types
   of effects. Usually an info_target will suffice.
   - "Angles": The angles specifying the direction in which particles will be emitted.
   - "Effect type": Type of particle effect to use.
     - "Particle Explosion 1": Quake 1 explosion effect.
	 - "Particle Explosion 2": Same as "Particle Explosion 1", but you can specify a color range in the Q1
	 color palette.
	 - "Blob Explosion": Quake 1 "Tarbaby" explosion effect.
	 - "Rocket Explosion": Rocket projectile explosion effect.
	 - "Particle Effect": A particle effect that travels in a direction. Needs "Target" set to work.
	 - "Lava Splash": Lava splash effect in a rectangular area.
	 - "Teleport Splash": Teleport splash effect from Quake 1.
	 - "Rocket Trail": Rocket trail type, needs "Target" to be set.
   - "Start color index": Beginning of color index range in the Quake 1 palette.
   - "End color index": End of color index range in the Quake 1 palette.
   - "Particle count": Number of particles to spawn, for "Particle Effect" only.
   - "Rocket trail type": Type of rocket trail to spawn.
   - "Minimum repeat delay": Minimum delay before the effect occurs again.
   - "Maximum repeat delay": Maximum delay before the effect occurs again.   
   
 - Spawn flags:
   - "Start On": If set, this entity will spawn on the "On" state.
   
# env_render
>Use this entity to set the render properties of another entity. It can be used to change the render mode,
>transparency, Render Fx and render color of another entity. It's spawnflags allow you to specify properties
>should be set and what should be left as-is.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": The entity to set the render properties of.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied to the target entity.
   
 - Spawn flags:
   - "No Renderfx": The Render Fx of the target entity will not be modified.
   - "No Renderamt": The Render Amount of the target entity will not be modified.
   - "No Rendermode": The Render Mode of the target entity will not be modified.
   - "No Rendercolor": The Render color of the target entity will not be modified.
   
# env_roomtype
>This entity will set the environmental audio effect to a specified preset regardless of where the player is
>in the level. The applications of this entity are niche at best, and it is advised to use trigger_sound
>instead of this entity.

 - Keyvalues:
   - "Room Type": The preset to use for environmental audio effects.
   
# env_rot_light
>A VBM model entity that will spawn a rotating emergency light that uses two spotlights, with or without 
>shadowing enabled. I recommend disabling shadowing to improve performance if needed.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Angles": The angles specifying the direction in which particles will be emitted.
   - "Framerate(0-1)": Framerate multiplier for the animation played.
   - "Render Color": Color of the spotlights, the default color is red.

 - Spawn flags:
   - "No shadowmaps": The lights will not cast any shadows.
   - "Start on": If the entity has a targetname and this is set, it will start in the "On" state.
   
# env_shake
>An entity that will apply a screenshake effect based on the settings specified.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Amplitude 0-16": The amplitude of the shake.
   - "Effect radius": If not a global shake, this will be the radius in which the shake can take effect.
   - "Duration (seconds)": Duration of the shake.
   - "0.1 = jerk, 255.0 = rumble": The force of the shake.
   
 - Spawn flags:
   - "GlobalShake": The shake will ignore the "Effect radius" and will work across the level.
   - "In air also": The player will be shaken even if they are not on ground.
   
# env_setangles
>Use this entity to set the angles of another entity.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": The entity to set the angles of.
   - "Angles": The angles the target entity will be set to use.
   
# env_setbody
>Set the body value of a target entity. Only works on VBM models.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": The entity to set the body value of.
   - "Body to Set": The body value to set for the target entity.
   
# env_setbodygroup
>A more sophisticated version of env_setbody, this allows you to set the body value based on bodgroup and
>submodel names. This should ensure that independent of changes in the model, as long as the bodygroup and
>submodel names match, it will always set the correct body value. Only works on VBM models.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": The entity to set the bodygroup of.
   - "Bodygroup": Name of the bodygroup to look up.
   - "Submodel": Name of the submodel to seek in the named bodrgroup.
   
# env_setsequence
>This entity will set a sequence for another animated entity by name. Best used with env_model entities.
>Only works on VBM models.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": The entity to set the bodygroup of.
   - "Sequence Name": The name of the sequence/animation to set.
   
# env_setskin
>Sets the skin value of another entity. Only works with VBM models.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": The entity to set the bodygroup of.
   - "Skin to Set": The skin group index to set.
   
# env_setskytexture
>Changes the skybox texture used to the one specified in this entity. If you want to switch between multiple
>env_setskytexture entities, it is better to set the "Do not send off message".

 - Keyvalues:
   - "Name": Name of this entity.
   - "Sky texture": The base name of the sky texture set to use.

 - Spawn flags:
   - "Start On": The skybox is set on level spawn.
   - "Do not send off message": If turned off, the skybox will not clear itself in the renderer. Use this if
   you want to switch from one env_setskytexture to another without switching back to the default specified
   in the worldspawn entity.
   
# env_shooter
>An entity that will shoot small models with collisions, which will bounce around the level.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Angles": The angles specify the direction the gibs will be shot towards.
   - "Number of Gibs": Number of gibs to emit before stopping.
   - "Delay between shots": The amount of delay between each gib being shot out.
   - "Gib Velocity": Speed of gibs when they spawn.
   - "Course Variance": Variance in direction of velocity.
   - "Gib Life": The lifetime of each gib spawned.
   - "Model or Sprite name": The sprite or VBM model to use for the gibs.
   - "Material Sound": The type of material sound the gibs will play when colliding.
   - "Gib Sprite Scale": Scale of the sprite used.
   - "Gib Skin": Skin group to use for the gibs.
   
 - Spawn flags:
   - "Repeatable": This entity can be called to shoot gibs multiple times.
   
# env_spark
>A simple entity that will spawn a spark particle effect, either based on a delay or when triggered.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Angles": The angles specify the direction the sparks will be shot towards.
   - "Max Delay": Maximum delay in seconds between sparks.

 - Spawn flags:
   - "Toggle": If set, the sparks will be emitted until the entity is triggered off.
   - "Start ON": If set, the entity will spawn in the "On" state by default even if a targetname is set.
   
# env_specialfog
>This entity is an engine feature used by my game only, it is used to set a vertical fog effect that starts
>away from the center by 400 or so units. This was used in a deep crevice with a bridge inbetween.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Trigger Mode": Specify whether the effect is enabled or disabled when this entity is triggered.
   
# env_spotlight
>The spotlight entity allows you to set up a projective light source. Useful for things like well,
>spotlights or projectors even. These entities are much cheaper on performance than env_dlight when it
>comes to shadowing, and I recommend using these instead wherever possible.

 - Keyvalues
   - "Name": Name of this entity. If no name is set, the dynamic light is on by default.
   - "Angles": The angles specify the direction the spotlight will be shining towards.
   - "Render Fx": Only the Render Fx values of "In Portal Entity, "Skybox Entity" apply to this entity, and
   can be used for dynamic lights that appear inside the skybox.
   - "Light style/Appearence": Check the "Shared keyvalues" section for more on this.
   - "Interpolate Custom Appearence": If set, the "Custom Appearance" will have it's values interpolated.
   - "Custom Appearance": Specify a custom appearence, from letter a to z, which specify the light
   strength at a given frame.
   - "Radius": Max area of effect in units.
   - "Render Color": Color of the dynamic light.
   - "Field of View": The field of view of the spotlight.
   - "Texture Index": Texture index into the list specified in the "projective_textures.txt" file located
   in "textures\general\".
   - "Static": If set to static, the shadowmap is only rendered into once and will only contain static,
   non-moving entities in it. Use this setting to optimize on scenes where you don't mind not having
   shadows be cast from moving objects.
   - "Disable Shadowmaps": Disables shadowmaps entirely, creating a light that will shine through walls and
   other objects.
 
 - Spawn flags:
   - "Start On": If the entity has a targetname but you want it to be on by default, set this flag.
   
# env_sprite
>A toggleable sprite entity that can play animations, and can be toggled on and off, or set to play it's
>animations only once before turning off.

 - Keyvalues:
   - "Angles": Angles of the sprite.
   - "Name": Name of this entity.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied to the sprite.
   - "Framerate": Framerate of the sprite animation to be played.
   - "Sprite Name": Sprite model to use.
   - "Scale": Scale factor to be applied, by default it is 1.0.
   - "Angular FOV": Field of view to use for sprites with the "Angular Sprite" Render FX set.
   - "Attachment Entity": Name of the entity to follow if set.
   - "Attachment number(0 = none)": Index of attachment on target, if target if a VBM entity.
 
 - Spawn flags:
   - "Start on": If the entity has a targetname and this is set, the sprite will spawn in the "On" state.
   - "Play Once": The sprite will play it's animations once when triggered, and then will turn off.
   
# env_spritetrain
>Much like env_sprite, except this entity functions like a func_train, and will follow path_corners like
>one. Cannot be turned on or off.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Global Entity Name": If set, this entity will be transferred across a trigger_changelevel, and will
   try to find an entity with the same globalname on the other level, then will override it.
   - "Angles": Angles of the sprite.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied to the sprite.
   - "First stop target": The path_corner entity to start at.
   - "Sprite": Sprite model to use.
   - "Speed (units per second)": Speed of movement.
   - "Alpha ( 1-255 )": Alpha value of the sprite rendered.
   - "Sprite Scale": Scale applied to the sprite, default is 1.0.

 - Spawn flags:
   - "No fire pass": This entity will not trigger "Fire on Pass" targets marked in path_corner entities when
   passing one.
   - "Start On": The entity will start moving on spawn.
   
# env_sun
>Similar to he env_lensflare entity, except this will set a sun lens flare in the sky that is independent
>of the player's position in the world. Useful for a sun effect, but you can have multiple of these in a
>level for multiple suns.
>[!NOTE] 
>For determining the Pitch and Roll values of the sun flare, spawn an env_sun in your map, compile
>it and then load it into the game. Use the "sun_debug_pitch" and "sun_debug_roll" cvars to determine the
>ideal values for the entity.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Render Color": Color of the lens flare effect.
   - "Scale": The scale of the lens flare sprites.
   - "Pitch": Pitch value of the sun direction.
   - "Roll": Roll value of the sun direction.

 - Spawn flags:
   - "Start On": If the entity has a targetname and this flag is set, the entity will spawn in the "On"
   state.
   - "Portal Sun": If set, the sun effect will only be visible if emitted from a func_portal_surface.
   
# env_syncanimation
>Force two entities to sync their animations completely. This is useful for ensuring that some scripted
>scenes between an NPC and an object they're handling remainds completely in-sync.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": The entity to sync up.
   - "Sync target": The entity to sync the entity specified in "Target" to.
   
# envpos_sky
>This entity marks the position of the 3D sky's skybox dome, where your skybox objects are. This entity
>can also function like a train entity, and move around in the skybox dome. It is to be used in conjuction
>with an envpos_world entity.

 - Keyvalues:
   - "Name": Name of this entity.
   - "First stop target": The path_corner entity to start at.
   - "Speed (units per second)": Speed of movement.
   
 - Spawn flags:
   - "No fire pass": This entity will not trigger "Fire on Pass" targets marked in path_corner entities when
   passing one.
   - "Start On": The entity will start moving on spawn.
   
# envpos_portal
>This entity marks the origin point of a portal surface in the portal's own space. To be used in conjuction
>with envpos_portal_world and func_portal_surface entities.

 - Keyvalues:
   - "Name": Name of this entity.
   - "World marker entity": Name of the envpos_portal_world entity that this will use as the world position
   marker.
   - "Scale": The relative scale of the portal area compared to the world, where a value of "1" means the
   portal area is of the same size as the world.
   - "Sky texture": Sky texture to use for the portal.
   
# envpos_portal_world
>This entity specifies the local-space origin of the portal system.

 - Keyvalues:
   - "Name": Name of this entity.
   
# envpos_world
>This entity marks the local world-origin of the skybox, and is to be use in conjuction with an envpos_sky
>entity. This entity also allows you to specify fogging of the skybox, and also a custom sky texture. This
>entity can be toggled on or off to disable the skybox, or to switch to another envpos_world with another
>set of settings for fog, size, skybox texture, etc.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Size": Relative size of the skybox, where "1" is the same size as local space.
   - "Fog Start Distance": Distance at which skybox fog starts to take effect.
   - "Fog End Distance": Distance at which skybox fog reaches full saturation.
   - "Fog Color (R G B)": Color of the fog effect applied.
   - "Don't Fog Sky": Setting this to "No" will set it so the sky is fully covered by fog, while setting it
   to "Yes" will cause the sky to remain clear.
   - "Sky texture": Sky texture to use for the the skybox.

 - Spawn flags:
   - "Start On": If this entity has a targetname and this flag is set, the entity will start in the "On"
   state.
   - "Send off message": If set, the skybox will clear any skybox settings set after it is turned off. Use
   this if you want to switch the skybox off and don't plan to replace it with another envpos_world entity.
   
# func_bike_block
> An invisible brush-based entity that is triggered to be solid when the player tis on an item_motorbike
> entity. Useful for preventing the player from entering certain spaces while on a motorbike.

# func_breakable
>A brush-based entity that can be broken by being triggered, or by being hit with bullets or a melee 
>weapon. It can spawn gibs or items/weapons if set.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Global Entity Name": If set, this entity will be transferred across a trigger_changelevel, and will
   try to find an entity with the same globalname on the other level, then will override it.   
   - "Target on Break": Entity to trigger when broken.
   - "Strength": The health points this entity will have.
   - "Material type": Type of material this breakable has.
   - "Gibs Direction": Direction gibs will take when broken.
     - "Random": Gib velocity is completely random.
	 - "Relative to Attack": The direction will depend on the attack vector.
   - "Delay before fire": Amount of time to wait before triggering the target.
   - "Gib Model": The VBM model to use for the gibs.
   - "Spawn On Break": The item type to spawn when broken.
   - "Explode Magnitude (0=none)": If set, the breakable will explode upon being broken, and deal damage to
   nearby entities.
   - "Spawn chance(1 == always)": The chance of spawning an object, based on a 1 out of n approach, where 
   "n" is the value specified in this field.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, only works with "Color" rendermode.
   - "Angles": Angles of the model.
   - "Noclip": Specify whether this entity should produce clipnodes, which determine if it collides with
   other entities and/or the player or not. Setting this to "Yes" can also saved on clipnode counts.
   - "ZHLT Lightflags": Special lighting flags to set, like "Opaque" or "Embedded fix".
   - "Light Origin Target": The entity will move by it's center to this position when lighting is 
   calculated for it in HLRAD.
   - "Minimum light level": If set, the entity cannot have light values darker than this amount.
   
 - Spawn flags:
   - "Only Trigger": The entity cannot be broken by attacks, and will only break when triggered by another.
   - "Touch": Touching this entity will immediately cause it to break.
   - "Pressure": Applying pressure will cause this entity to break after a delay.
   - "Instant Melee": If this entity is hit with a Melee weapon(like the knife), it will break immediately
   regardless of the health points set.
   - "No gibs": The breakable will not spawn any gibs.
   - "No penetration damage": Damage from bullets that have penetrated through walls prior to hitting this
   entity will not deal any damage.
   - "Smart ammo spawn": If set, the "Spawn on Break" setting is ignored, and instead the breakable will
   spawn ammo for the weapon the player has the lowest amount of ammo for.
   
# func_button
>A brush-based entity that can be used to trigger other entities when used by the player. It can be enabled
>or disabled by a multisource entity, and has a set ot use sounds for each state.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Global Entity Name": If set, this entity will be transferred across a trigger_changelevel, and will
   try to find an entity with the same globalname on the other level, then will override it.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, only works with "Color" rendermode.
   - "Angles": Defines the movement direction of the func_button if it is set to move.
   - "ZHLT Lightflags": Special lighting flags to set, like "Opaque" or "Embedded fix".
   - "Light Origin Target": The entity will move by it's center to this position when lighting is 
   calculated for it in HLRAD.
   - "Noclip": Specify whether this entity should produce clipnodes, which determine if it collides with
   other entities and/or the player or not. Setting this to "Yes" can also saved on clipnode counts.
   - "Speed": Movement speed of the button if "Move" flag is set.
   - "Targetted object": Target entity to trigger.
   - "Trigger when used and locked": If the button is locked by a multisource and the player presses it,
   it will trigger the entity specified in this field.
   - "Paired button(s) name": Paired buttons are meant to be buttons that target the same entity, but on
   the other side of a wall, etc. When specified here, the paired button(s) will also be set to wait before
   being usable again.
   - "Lip": Buttons move their own length on an axis when triggered and the "Move" flag is set, and the lip
   will be subtracted from that amount, so they don't move the full length. A negative value specified here
   will cause the button to move farther away than it normally would.
   - "Master": A multisource that can be used to enable or disable this entity.
   - "Sounds": The sound the button will play when used.
   - "Custom Sound": A user-specifiable sound used by the button. Will override "Sounds" setting.
   - "Delay before reset": Delay before the button can be pressed again and/or returns to it's base 
   position. A value of -1 means the button remains pressed.
   - "Delay before trigger": Delay in seconds before the target gets triggered.
   - "Locked Sound": Sound to play when the button is locked by a multisource.
   - "Custom Locked Sound": User-specifiable locked sound. Will override "Locked Sound" setting.
   - "Unlocked Sound": The sound the button will play when it's pressed after being unlocked.
   - "Custom Unlocked Sound": User-specifiable unlocked sound. Will override "Unlocked Sound" setting.
   - "Locked Sentence": The sentence the button will play when locked.
   - "Unlocked Sentence": The sentence the button will play when pressed after being unlocked.
   - "Minimum light level": If set, the entity cannot have light values darker than this amount.
   
 - Spawn flags:
   - "Toggle": Instead of returning to it's base position, the button will toggle between it's two
   positions when pressed.
   - "Sparks": The button will emit sparks when pressed.
   - "Touch Activates": Touching the button activates it.
   - "Move": If set, the button will move when pressed.
   - "Nodraw": The button will remain invisible, but can still be pressed.
   - "Hide reticle unless enabled": The use reticle will not be drawn around the button unless it's 
   multisource enabled, and it has a valid target.
   
# func_clipeconomy
>This entity is recognized by HLCSG and will have all of it's clipnodes removed. This allows you to save a
>lot on clipnode counts, which will also improve performance. Bullets, weapons, and melee weapons will 
>still collide with these objects, only NPC/player collisions will be missing. The best use of these is for
>more complex brush geometry/brush props in the level, which you then wrap clip brushes around to provide
>simpler, less expensive collisions.<br />
>If you give this entity an origin brush, then you can have it be rotated around if you set the spawn flag
>"Take Angles". This is useful for having rotated complex brushmodels without suffering from snap-to-grid
>issues.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Global Entity Name": If set, this entity will be transferred across a trigger_changelevel, and will
   try to find an entity with the same globalname on the other level, then will override it.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, only works with "Color" rendermode.
   - "ZHLT Lightflags": Special lighting flags to set, like "Opaque" or "Embedded fix".
   - "Light Origin Target": The entity will move by it's center to this position when lighting is 
   calculated for it in HLRAD.
   - "Angles": If the spawnflag "Take Angles" is set an you give this entity an origin brush, then you can
   assign this entity to be at a certain angle.
   - "Parent": If set, this brushmodel will have it's collisions disabled, and will follow the parent
   entity's origin. It will pick up the same origin as the parent, and any offset from that origin is not
   supported right now.
   - "Noclip": Defaults to 1, but HLCSG will ignore this setting regardless for this entity.
   - "Minimum light level": If set, the entity cannot have light values darker than this amount.
   - "Ignore night mode": If set, the "Minimum light level" will not be ignored even if compiling RAD in 
   night mode. Otherwise, if compiling in night mode, the minimum light limit is ignored.
   
 - Spawn flags:
   - "Take Angles": The entity will not have it's angles cleared on spawn. Best used with an origin brush
   to define the center of rotation.
   - "Not in deathmatch": this entity will not spawn in multiplayer mode.
   
# func_conveyor
>An entity that will push the player and/or other moving entities if they stand on it. It will also cause
>all textures with "scroll" in the beginning of their name to scroll at the specified speed forward on the
>S coordinate.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Global Entity Name": If set, this entity will be transferred across a trigger_changelevel, and will
   try to find an entity with the same globalname on the other level, then will override it.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, only works with "Color" rendermode.
   - "Angles": This determines the vector at which the func_conveyor will push the entities on it.
   - "ZHLT Lightflags": Special lighting flags to set, like "Opaque" or "Embedded fix".
   - "Light Origin Target": The entity will move by it's center to this position when lighting is 
   calculated for it in HLRAD.
   - "Conveyor Speed": Speed at which the conveyor scrolls and pushes the entities on it.
   - "Minimum light level": If set, the entity cannot have light values darker than this amount.
   
 - Spawn flags:
   - "No Push": Don't push entities on the func_conveyor.
   - "Not Solid": The conveyor will be pass-through for both players/npcs and bullets, grenades, etc.
   - "Start Invisible": The conveyor will be invisible until triggered on.
   
# func_door
>A basic door entity that can move in one direction. It can use the legacy sound set like Half-Life 1 doors
>did, or it can be specified to use custom sounds. func_door entities without targetnames will by default
>open on touch, while doors with targetnames need to be triggered to open, however this can be overridden
>with the "Touch opens" spawn flag.

 - Keyvalues:
   - "Name": Name of this entity.
   - "ZHLT Lightflags": Special lighting flags to set, like "Opaque" or "Embedded fix".
   - "Light Origin Target": The entity will move by it's center to this position when lighting is 
   calculated for it in HLRAD.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, only works with "Color" rendermode.
   - "Global Entity Name": If set, this entity will be transferred across a trigger_changelevel, and will
   try to find an entity with the same globalname on the other level, then will override it.
   - "Angles": Defines the movement direction of the door if it is set to move.
   - "KillTarget": The door will kill this entity when it is done moving.
   - "Speed": Amount of distance the door moves per second.
   - "Master": Name of a multisource entity controlling whether the door is enabled or disabled.
   - "Move Sound": Movement sound for this door.
   - "Custom Move Sound": Custom move sound for the door. Will override the "Move sound" setting.
   - "Stop Sound": Sound to play when the door stops moving.
   - "Custom Stop Sound": User-specifiable stop sound. Will override "Stop Sound" setting.
   - "Delay before close": Time until door closes again, a value of -1 will cause it to stay open.
   - "Lip": The door usually moves it's own length before stopping, but the lip value will cut off n units
   from this distance. A negative value will cause the door to move farther out.
   - "Damage inflicted when blocked": Damage inflicted upon any entity that blocks the door from moving.
   - "Target": Entity to trigger when done moving.
   - "Delay before fire": Delay before the target entity is triggered.
   - "Fire on Close": Target to trigger when the door closes.
   - "Locked Sound": Sound to play when the door is touched, and is locked by a multisource.
   - "Custom Locked Sound": User-specifiable locked sound. Will override "Locked Sound" setting.
   - "Unlocked Sound": Sound to play when opened for the first time after being unlocked.
   - "Custom Unlocked Sound": User-specifiable unlocked sound. Will override "Unlocked Sound" setting.
   - "Locked Sentence": Sentence to play when touched while locked.
   - "Unlocked Sentence": Sentence to play when opened for the first time after being unlocked.
   - "Minimum light level": If set, the entity cannot have light values darker than this amount.
   
 - Spawn flags:
   - "Starts open": The door will start in the open position until touched/triggered.
   - "Don't link": Don't link movement with other doors that have the same name.
   - "Passable": The door will be non-solid.
   - "Toggle": If set, the door expects to be triggered to open or close, and will not auto-close.
   - "Use Only": Only triggering, or using with the use key will open this door.
   - "NPCs Can't": NPCs cannot open this door to get by.
   - "Touch Opens": Overrides default behavior where doors with targetnames cannot be opened by touching,
   and will allow the player to open this door via touch.
   - "No Proximity Checks": The door will not perform proximity checks for NPCs/players and will close
   regardless.
   - "Nodraw": The door will be invisible.
   - "Not in deathmatch": This entity will not be present in multiplayer. 
  
# func_door_rotating
>Mostly the same entity as func_door, but instead of moving, it'll rotate around an origin point when
>touched or triggered. It can use the legacy sound set like Half-Life 1 doors did, or it can be specified 
>to use custom sounds. func_door_rotating entities without targetnames will by default open on touch, while
>doors with targetnames need to be triggered to open, however this can be overridden with the "Touch opens"
>spawn flag.

 - Keyvalues:
   - "Name": Name of this entity.
   - "ZHLT Lightflags": Special lighting flags to set, like "Opaque" or "Embedded fix".
   - "Light Origin Target": The entity will move by it's center to this position when lighting is 
   calculated for it in HLRAD.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, only works with "Color" rendermode.
   - "Global Entity Name": If set, this entity will be transferred across a trigger_changelevel, and will
   try to find an entity with the same globalname on the other level, then will override it.
   - "Angles": The initial angles of the door on spawn.
   - "KillTarget": The door will kill this entity when it is done moving.
   - "Speed": Amount of distance the door moves per second.
   - "Master": Name of a multisource entity controlling whether the door is enabled or disabled.
   - "Move Sound": Movement sound for this door.
   - "Custom Move Sound": Custom move sound for the door. Will override the "Move sound" setting.
   - "Stop Sound": Sound to play when the door stops moving.
   - "Custom Stop Sound": User-specifiable stop sound. Will override "Stop Sound" setting.
   - "Delay before close": Time until door closes again, a value of -1 will cause it to stay open.
   - "Lip": The door usually moves it's own length before stopping, but the lip value will cut off n units
   from this distance. A negative value will cause the door to move farther out.
   - "Damage inflicted when blocked": Damage inflicted upon any entity that blocks the door from moving.
   - "Target": Entity to trigger when done moving.
   - "Delay before fire": Delay before the target entity is triggered.
   - "Fire on Close": Target to trigger when the door closes.
   - "Locked Sound": Sound to play when the door is touched, and is locked by a multisource.
   - "Custom Locked Sound": User-specifiable locked sound. Will override "Locked Sound" setting.
   - "Unlocked Sound": Sound to play when opened for the first time after being unlocked.
   - "Custom Unlocked Sound": User-specifiable unlocked sound. Will override "Unlocked Sound" setting.
   - "Locked Sentence": Sentence to play when touched while locked.
   - "Unlocked Sentence": Sentence to play when opened for the first time after being unlocked.
   - "Minimum light level": If set, the entity cannot have light values darker than this amount.
   - "Distance (deg)": The amount to rotate, in degrees.
   - "Pitch Yaw Roll (Y Z X)": Set an angle for the door.
   
 - Spawn flags:
   - "Reverse Dir": The door will move counter-clockwise on it's axis.
   - "One-way": The door will not move based on which side it's triggered/touched from, and will instead
   move in a single direction only specified based on "Reverse dir" and the "Distance" setting being
   negative or positive.
   - "X Axis": Set this to make the door move on the X axis.
   - "Y Axis": Set this to make the door move on the Y axis.
   - "Starts open": The door will start in the open position until touched/triggered.
   - "Don't link": Don't link movement with other doors that have the same name.
   - "Passable": The door will be non-solid.
   - "Toggle": If set, the door expects to be triggered to open or close, and will not auto-close.
   - "Use Only": Only triggering, or using with the use key will open this door.
   - "NPCs Can't": NPCs cannot open this door to get by.
   - "Touch Opens": Overrides default behavior where doors with targetnames cannot be opened by touching,
   and will allow the player to open this door via touch.
   - "No Proximity Checks": The door will not perform proximity checks for NPCs/players and will close
   regardless.
   - "Nodraw": The door will be invisible.
   - "Not in deathmatch": This entity will not be present in multiplayer. 
   
# func_fade
>This is a brushmodel that fades in or out over a given amount of time when triggered. The initial "Render
>Amount" setting and the one set in "Target opacity" will determine in which direction the fade occurs.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Global Entity Name": If set, this entity will be transferred across a trigger_changelevel, and will
   try to find an entity with the same globalname on the other level, then will override it.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, only works with "Color" rendermode.
   - "ZHLT Lightflags": Special lighting flags to set, like "Opaque" or "Embedded fix".
   - "Light Origin Target": The entity will move by it's center to this position when lighting is 
   calculated for it in HLRAD.
   
 - Spawn flags:
  - "Not in deathmatch": This entity will not be present in multiplayer.  
   
# func_friction
>This entity modifies surface friction when any player is touching it, causing the player to slide around
>and skid. The degree of friction lost depends on the setting in the entity.

 - Keyvalues:
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, only works with "Color" rendermode.
   - "Percentage of standard (0 - 100)": Determines the degree of friction removed.
   
 - Spawn flags:
  - "Not in deathmatch": This entity will not be present in multiplayer. 
   
# func_illusionary
>A brush entity that does not collide with entities or bullets, and will not block the line of sight of
>NPCs. Useful for things like haloes around lights, cobwebs hanging from the ceiling, etc.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Angles": If the spawnflag "Take Angles" is set an you give this entity an origin brush, then you can
   assign this entity to be at a certain angle.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, only works with "Color" rendermode.
   - "ZHLT Lightflags": Special lighting flags to set, like "Opaque" or "Embedded fix".
   - "Light Origin Target": The entity will move by it's center to this position when lighting is 
   calculated for it in HLRAD.
   - "Minimum light level": If set, the entity cannot have light values darker than this amount.
   - "Ignore night mode": If set, the "Minimum light level" will not be ignored even if compiling RAD in 
   night mode. Otherwise, if compiling in night mode, the minimum light limit is ignored.
   
 - Spawn flags:
   - "Take Angles": The entity will not have it's angles cleared on spawn. Best used with an origin brush
   to define the center of rotation.
   - "Not in deathmatch": This entity will not be present in multiplayer.
   
# func_ladder
>This brush entity defines a volume in which the player can climb on a surface. Best paired with a ladder
>brush entity with the func_ladder right ontop of it's climbable side.

# func_mirror
>An entity that defines a surface which mirrors the area around it on a single specified surface. All the
>faces of the brush defining the mirror need to be textured with NULL. The single reflective surface itself
>is best set at a high texture scale like 15.00 or so to ensure that only a single surface is divided, as
>having multiple surfaces will make the mirror invalid.

 - Keyvalues:
   - "Name": Name of this entity.

 - Spawn flags:
  - "Not in deathmatch": This entity will not be present in multiplayer.
  
# func_null
>This brush entity will be removed on level spawn. Useful for things like brush based lights that need to
>be removed after compiling.

# func_monitor
>An entity that can display another part of the level viewed from an "info_monitorcamera" entity. You can
>have multiple monitor brushes tied into one entity, however ensure that you are using the "monitor"
>texture on all parts you want to display the image. Also, it's preferable to "Fit" the texture onto the
>sides visible, so that the camera image doesn't get tiled.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Resolution": A list of resolutions to pick from for rendering the scene.
   - "Effects": You can enable Grayscale or just render with color.
   
 - Spawn flags:
   - "Start Off": The entity will start invisible until triggered on.
   
# func_npcclip
>Very similar to Half-Life's func_monsterclip, this entity will block any NPCs from moving through it which
>have the "NPC Clip" flag set in their spawnflags. This entity can be turned on orr off by triggering it.

 - Keyvalues:
   - "Name": Name of this entity.

 - Spawn flags:
   - "Start Off": The entity will start invisible until triggered on.
   
# func_pendulum
>A solid entity that will swing around on an origin point in a direction, before returning back to it's
>base angles. This is useful for things like light fixtures swinging around, or crates suspended from
>cranes that swing back and forth. This entity by default rotates on it's Z axis.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Global Entity Name": If set, this entity will be transferred across a trigger_changelevel, and will
   try to find an entity with the same globalname on the other level, then will override it.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, only works with "Color" rendermode.
   - "Angles": The initial angles of the pendulum on spawn.
   - "ZHLT Lightflags": Special lighting flags to set, like "Opaque" or "Embedded fix".
   - "Light Origin Target": The entity will move by it's center to this position when lighting is 
   calculated for it in HLRAD.
   - "Speed": The speed of the rotation.
   - "Distance (deg)": Distance in degrees to rotate before returning.
   - "Damping (0-1000): Amount of speed damping.
   - "Damage inflicted when blocked": If an NPC or the player is blocking the pendulum, this amount of
   damage will be inflicted.
   - "Minimum light level": If set, the entity cannot have light values darker than this amount.
   
 - Spawn flags:
  - "Not in deathmatch": This entity will not be present in multiplayer.
  - "Start ON": The pendulum will start on regardless of having a name.
  - "Passable": The entity will be non-solid to everything.
  - "Auto-return": Automatically teturn to the base position without needing to be triggered.
  - "X Axis": Rotate on the X axis.
  - "Y Axis": Rotate on the Y axis.
  
# func_plat
>A brush-blased moving platform that will move the amount of distance specified in "Travel altitude", which
>can be both a positive value(for upwards movement), or a negative value for downwards movement. The plat
>can be left without a name, in which case it'll automatically begin moving when the player enters the
>volume of movement, or can be set to only move when triggered by another entity.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Global Entity Name": If set, this entity will be transferred across a trigger_changelevel, and will
   try to find an entity with the same globalname on the other level, then will override it.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, only works with "Color" rendermode.
   - "Move Sound": Set of move sounds to choose from.
   - "Custom Move Sound": User-specifiable custom sound file to use for movement.
   - "Stop Sound": Set of stop sounds to choose from.
   - "Custom Stop Sound": User-specifiable custom sound file to use for stopping.
   - "Sound Volume 0.0 - 1.0": Volume to use for the sounds.
   - "ZHLT Lightflags": Special lighting flags to set, like "Opaque" or "Embedded fix".
   - "Light Origin Target": The entity will move by it's center to this position when lighting is 
   calculated for it in HLRAD.
   - "Speed": Speed of movement in units per second.
   - "Travel altitude (can be negative)": The distance to travel upwards, or downwards if negative.
   - "Minimum light level": If set, the entity cannot have light values darker than this amount.
   
 - Spawn flags:
   - "Toggle": The plat will not automatically return to it's base position, and will only move when 
   triggered.
   
# func_platrot
>A brush-blased moving platform that will move the amount of distance specified in "Travel altitude", which
>can be both a positive value(for upwards movement), or a negative value for downwards movement, while also
>rotating for the amount specified in "Spin amount". The plat can be left without a name, in which case 
>it'll automatically begin moving when the player enters the volume of movement, or can be set to only move
>when triggered by another entity.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Global Entity Name": If set, this entity will be transferred across a trigger_changelevel, and will
   try to find an entity with the same globalname on the other level, then will override it.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, only works with "Color" rendermode.
   - "Move Sound": Set of move sounds to choose from.
   - "Custom Move Sound": User-specifiable custom sound file to use for movement.
   - "Stop Sound": Set of stop sounds to choose from.
   - "Custom Stop Sound": User-specifiable custom sound file to use for stopping.
   - "Sound Volume 0.0 - 1.0": Volume to use for the sounds.
   - "Angles": The initial angles of the plat on spawn.
   - "ZHLT Lightflags": Special lighting flags to set, like "Opaque" or "Embedded fix".
   - "Light Origin Target": The entity will move by it's center to this position when lighting is 
   calculated for it in HLRAD.
   - "Speed": Speed of movement in units per second.
   - "Travel altitude (can be negative)": The distance to travel upwards, or downwards if negative.
   - "Rotation": The amount to rotate the plat, in degrees.
   - "Minimum light level": If set, the entity cannot have light values darker than this amount.
   
 - Spawn flags:
   - "Toggle": The plat will not automatically return to it's base position, and will only move when 
   triggered.
  - "X Axis": Rotate on the X axis.
  - "Y Axis": Rotate on the Y axis.

# func_portal_surface
>This brush-based entity acts as a "portal", in that it allows you to see into another part of the level on
>it's surface, used in conjuction with the "envpos_portal" and "envpos_portal_world" entities. Any surfaces
>meant to display this must be textured with the "portal" texture, otherwise they will just be invisible. 
>This entity is useful cases where you want to display another part of the level. It can also be used much 
>like a skybox, in case you might want to display multiple skybox-like portals.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Portal marker entity": The envpos_portal entity that marks the position in portal space.
   - "Noclip": Specify whether this entity should produce clipnodes, which determine if it collides with
   other entities and/or the player or not. Setting this to "Yes" can also saved on clipnode counts.

 - Spawn flags:
   - "Start off": The entity will be invisible until triggered on by another entity.
   - "Not solid": This entity will not collide with NPCs, players or bullets.
   
# func_pushable
>A brush entity that can be pushed around by the player, and will have physics applied. If the "Breakable" 
>spawn flag is set, then it can also be broken by damage. The bouyancy will determine if the object will
> float in water or sink.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Global Entity Name": If set, this entity will be transferred across a trigger_changelevel, and will
   try to find an entity with the same globalname on the other level, then will override it.   
   - "Target on Break": Entity to trigger when broken.
   - "Strength": The health points this entity will have.
   - "Material type": Type of material this breakable has.
   - "Gibs Direction": Direction gibs will take when broken.
     - "Random": Gib velocity is completely random.
	 - "Relative to Attack": The direction will depend on the attack vector.
   - "Delay before fire": Amount of time to wait before triggering the target.
   - "Gib Model": The VBM model to use for the gibs.
   - "Spawn On Break": The item type to spawn when broken.
   - "Explode Magnitude (0=none)": If set, the breakable will explode upon being broken, and deal damage to
   nearby entities.
   - "Spawn chance(1 == always)": The chance of spawning an object, based on a 1 out of n approach, where 
   "n" is the value specified in this field.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, only works with "Color" rendermode.
   - "Angles": Angles of the model.
   - "ZHLT Lightflags": Special lighting flags to set, like "Opaque" or "Embedded fix".
   - "Light Origin Target": The entity will move by it's center to this position when lighting is 
   calculated for it in HLRAD.
   - "Hull Size": Specify the clipping hull this entity will use for collisions.
   - "Friction (0-400)": The amount of friction against the floor the entity will have when being pushed 
   around.
   - "Buoyancy": How bouyant the object will be when in water.
   - "Minimum light level": If set, the entity cannot have light values darker than this amount.
   
 - Spawn flags:
   - "Only Trigger": The entity cannot be broken by attacks, and will only break when triggered by another.
   - "Touch": Touching this entity will immediately cause it to break.
   - "Pressure": Applying pressure will cause this entity to break after a delay.
   - "Breakable": If set, this pushable will also function as a breakable, and can be broken by attacks.
   - "Instant Melee": If this entity is hit with a Melee weapon(like the knife), it will break immediately
   regardless of the health points set.
   - "No gibs": The breakable will not spawn any gibs.
   - "No penetration damage": Damage from bullets that have penetrated through walls prior to hitting this
   entity will not deal any damage.
   - "Smart ammo spawn": If set, the "Spawn on Break" setting is ignored, and instead the breakable will
   spawn ammo for the weapon the player has the lowest amount of ammo for. 
   
# func_rot_button
>A button entity that will rotate along it's axis before triggering it's target. If set, the button will
>then return to it's base angles after triggering. This entity can be locked by a multisource. This entity
>by default rotates on it's Z axis, specified by an origin brush.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Global Entity Name": If set, this entity will be transferred across a trigger_changelevel, and will
   try to find an entity with the same globalname on the other level, then will override it.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, only works with "Color" rendermode.
   - "Angles": Initial angles of the button.
   - "ZHLT Lightflags": Special lighting flags to set, like "Opaque" or "Embedded fix".
   - "Light Origin Target": The entity will move by it's center to this position when lighting is 
   calculated for it in HLRAD.
   - "Targetted object": Entity to trigger when done rotating.
   - "Trigger when used and locked": Entity to trigger while locked by a multisource.
   - "Master": The name of the multisource entity controlling this button.
   - "Speed": Speed of rotation in degrees per second.
   - "Sounds": The list of available sounds for the button to play when used.
   - "Custom Sound": User-specifiable sound to play when used, will override "Sounds" setting.
   - "Delay before reset": Delay until the button starts returning to it's original angles. A value of -1
   will disable this and the button will remain rotated.
   - "Delay before trigger": Delay before the button's target is triggered after it is done rotating.
   - "Distance (deg)": The amount of distance to rotate, in degrees.
   - "Minimum light level": If set, the entity cannot have light values darker than this amount.
   
 - Spawn flags:
   - "Not solid": The entity will not collide with NPCs, players or bullets.
   - "Reverse Dir": The button rotate in the opposite direction to it's "Distance" setting.
   - "Toggle": If set, the button will not return to it's original angles, but instead pressing it to
   rotate each time will cause it to trigger it's target.
   - "X Axis": The button will rotate on it's X axis.
   - "Y Axis": The button will rotate on it's Y axis.
   
# func_rotating
>This entity will rotate on it's axis continously if on, and will accelerate/decelerate based on the fan
>friction set within the entity properties. Great to use in conjuction with an env_spotlight for a nice
>lighting and shadowing effect. The entity can accelerate/decelerate when toggled, which is specified by
>the "Fan Friction" value, or if this is left as zero, it'll turn off immediately.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Global Entity Name": If set, this entity will be transferred across a trigger_changelevel, and will
   try to find an entity with the same globalname on the other level, then will override it.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, only works with "Color" rendermode.
   - "Angles": Initial angles of the entity.
   - "ZHLT Lightflags": Special lighting flags to set, like "Opaque" or "Embedded fix".
   - "Light Origin Target": The entity will move by it's center to this position when lighting is 
   calculated for it in HLRAD.
   - "Rotation Speed": Speed of rotation, in degrees per second.
   - "Volume (10 = loudest)": The volume of the sound the fan makes.
   - "Friction (0 - 100%)": The friction value for acceleration/deceleration, the value here specifies the
   gain or loss per second as the fan accelerates/decelerates.
   - "Fan Sounds": A set of sounds to specify for the fan.
   - "Path/filename.wav of WAV": Allows you to specify a custom fan movement sound.
   - "Minimum light level": If set, the entity cannot have light values darker than this amount.
   - "X Y Z - Move here after lighting": Using this, the func_rotating will have it's lighting calculated
   by HLRAD, but then will move it's origin to the coordinates specified in this field for actual runtime
   execution.
   - "Damage inflicted when blocked": If an entity is blocking this fan, this much damage will be dealt to
   the blocking entity.
   
 - Spawn flags:
   - "Start On": The entity, if it has a targetname, will start in the "On" state.
   - "Reverse Direction": The fan will rotate counter-clockwise on it's axis.
   - "X Axis": The fan will rotate on it's X axis.
   - "Y Axis": The fan will rotate on it's Y axis.
   - "Acc/Dcc": If set, the fan will accelerate/decelerate when toggled.
   - "Fan Pain": If set, the fan will inflict damage when blocked.
   - "Not Solid": The fan will not collide with NPCs, players or bullets.
   - "Small Radius": The fan sound will have a radius of 512 units.
   - "Medium Radius": The fan sound will have a radius of 768 units.
   - "Large Radius": The fan sound will have a radius of 1024 units.
   
# func_slippery
>This entity changes how the player interacts with sloped surfaces when he is touching this entity. 
>Essentially, this can be used to make sloped surfaces more slippery, meaning the player cannot walk up 
>them as he normally would be able to, and slides back down.

 - Keyvalues:
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, only works with "Color" rendermode.
   - "Angles": Angles of the model.
   - "ZHLT Lightflags": Special lighting flags to set, like "Opaque" or "Embedded fix".
   - "Light Origin Target": The entity will move by it's center to this position when lighting is 
   calculated for it in HLRAD.
   - "Slipperiness factor(0-10)": The degree of slipperiness, with a value between 0 and 10.
   
 - Spawn flags:
  - "Not in deathmatch": This entity will not be present in multiplayer.
  
# func_trackautochange
>This entity is like a lift, but for tracktrains instead, and will immediately begin moving to it's target
>destination when a func_tracktrain arrives at the path_track that this entity also uses as it's current
>stop target. The train will then be re-aligned onto the bottom track. If the altitude is not set, this
>will simply rotate the func_tracktrain to it's next route. When done moving, it'll automatically activate
>the train on it.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, only works with "Color" rendermode.
   - "Global Entity Name": If set, this entity will be transferred across a trigger_changelevel, and will
   try to find an entity with the same globalname on the other level, then will override it.
   - "Move Sound": Set of move sounds to choose from.
   - "Custom Move Sound": User-specifiable custom sound file to use for movement.
   - "Stop Sound": Set of stop sounds to choose from.
   - "Custom Stop Sound": User-specifiable custom sound file to use for stopping.
   - "Sound Volume 0.0 - 1.0": Volume to use for the sounds.
   - "Travel altitude": The distance to travel vertically.
   - "Spin amount": How much to spin on the Z axis while travelling.
   - "Train to switch": The name of the func_tracktrain that we expect to use this trackchange.
   - "Top track": The top path_track entity that the trackchange and the func_tracktrain use. This needs to
   mark the end/beginning of the topmost track of the func_tracktrain's route.
   - "Bottom track": The bottom path_track entity that the trackchange and the func_tracktrain use. This 
   needs to mark the end/beginning of the bottom track of the func_tracktrain's route.
   - "Move/Rotate speed": The movement/rotation speed.
   - "ZHLT Lightflags": Special lighting flags to set, like "Opaque" or "Embedded fix".
   - "Light Origin Target": The entity will move by it's center to this position when lighting is 
   calculated for it in HLRAD.
   - "Minimum light level": If set, the entity cannot have light values darker than this amount.
   
 - Spawn flags:
   - "Start at Bottom": If set, the trackchange will start at the bottom path_track.
   - "Rotate Only": If set, altitude is ignore and the trackchange will only rotate.
   - "X Axis": Rotate on the X axis.
   - "Y Axis": Rotate on the Y axis.
   
# func_trackautochange_rc
>This entity is like a lift, but for tracktrains instead, but will not immediately begin moving to it's 
>target destination when a func_tracktrain arrives at the path_track that this entity also uses as it's
>current stop target. Instead, this entity needs to be triggered first to move. The train will then be 
>re-aligned onto the bottom track. If the altitude is not set, this will simply rotate the func_tracktrain
>to it's next route. When done moving, it'll automatically activate the train on it.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, only works with "Color" rendermode.
   - "Global Entity Name": If set, this entity will be transferred across a trigger_changelevel, and will
   try to find an entity with the same globalname on the other level, then will override it.
   - "Move Sound": Set of move sounds to choose from.
   - "Custom Move Sound": User-specifiable custom sound file to use for movement.
   - "Stop Sound": Set of stop sounds to choose from.
   - "Custom Stop Sound": User-specifiable custom sound file to use for stopping.
   - "Sound Volume 0.0 - 1.0": Volume to use for the sounds.
   - "Travel altitude": The distance to travel vertically.
   - "Spin amount": How much to spin on the Z axis while travelling.
   - "Train to switch": The name of the func_tracktrain that we expect to use this trackchange.
   - "Top track": The top path_track entity that the trackchange and the func_tracktrain use. This needs to
   mark the end/beginning of the topmost track of the func_tracktrain's route.
   - "Bottom track": The bottom path_track entity that the trackchange and the func_tracktrain use. This 
   needs to mark the end/beginning of the bottom track of the func_tracktrain's route.
   - "Move/Rotate speed": The movement/rotation speed.
   - "ZHLT Lightflags": Special lighting flags to set, like "Opaque" or "Embedded fix".
   - "Light Origin Target": The entity will move by it's center to this position when lighting is 
   calculated for it in HLRAD.
   - "Minimum light level": If set, the entity cannot have light values darker than this amount.
   
 - Spawn flags:
   - "Start at Bottom": If set, the trackchange will start at the bottom path_track.
   - "Rotate Only": If set, altitude is ignore and the trackchange will only rotate.
   - "X Axis": Rotate on the X axis.
   - "Y Axis": Rotate on the Y axis.
   
# func_trackchange
>This entity is like a lift, but for tracktrains instead, but will not immediately begin moving to it's 
>target destination when a func_tracktrain arrives at the path_track that this entity also uses as it's
>current stop target. Instead, this entity needs to be triggered first to move. The train will then be 
>re-aligned onto the bottom track. If the altitude is not set and "Rotate Only" is set, this will simply
>rotate the func_tracktrain to it's next route. When done moving, it will not automatically activate the
>train on it to move unlike the trackautochange entities.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, only works with "Color" rendermode.
   - "Global Entity Name": If set, this entity will be transferred across a trigger_changelevel, and will
   try to find an entity with the same globalname on the other level, then will override it.
   - "Move Sound": Set of move sounds to choose from.
   - "Custom Move Sound": User-specifiable custom sound file to use for movement.
   - "Stop Sound": Set of stop sounds to choose from.
   - "Custom Stop Sound": User-specifiable custom sound file to use for stopping.
   - "Sound Volume 0.0 - 1.0": Volume to use for the sounds.
   - "Travel altitude": The distance to travel vertically.
   - "Spin amount": How much to spin on the Z axis while travelling.
   - "Train to switch": The name of the func_tracktrain that we expect to use this trackchange.
   - "Top track": The top path_track entity that the trackchange and the func_tracktrain use. This needs to
   mark the end/beginning of the topmost track of the func_tracktrain's route.
   - "Bottom track": The bottom path_track entity that the trackchange and the func_tracktrain use. This 
   needs to mark the end/beginning of the bottom track of the func_tracktrain's route.
   - "Move/Rotate speed": The movement/rotation speed.
   - "ZHLT Lightflags": Special lighting flags to set, like "Opaque" or "Embedded fix".
   - "Light Origin Target": The entity will move by it's center to this position when lighting is 
   calculated for it in HLRAD.
   - "Minimum light level": If set, the entity cannot have light values darker than this amount.
   
 - Spawn flags:
   - "Start at Bottom": If set, the trackchange will start at the bottom path_track.
   - "Rotate Only": If set, altitude is ignore and the trackchange will only rotate.
   - "X Axis": Rotate on the X axis.
   - "Y Axis": Rotate on the Y axis.
   
# func_tracktrain
>A brush-based entity that move between a set of tracks, based on it's origin brush. It is a bit more
>sophisticated than a func_train, allowing for things like banking and rotation based on the path to the
>next tracktrain. When creating this entity, make sure to align it's front towards the 0 Z angle, so that
>it will face the proper direction ingame once compiled. This entity currently is not player-controlled,
>and needs to be triggered on and off to move, and needs an origin brush, which defines how it aligns to
>path_track entities.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Global Entity Name": If set, this entity will be transferred across a trigger_changelevel, and will
   try to find an entity with the same globalname on the other level, then will override it.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, only works with "Color" rendermode.
   - "Angles": Initial angles of the entity.
   - "ZHLT Lightflags": Special lighting flags to set, like "Opaque" or "Embedded fix".
   - "Light Origin Target": The entity will move by it's center to this position when lighting is 
   calculated for it in HLRAD.
   - "First stop target": The path_track entity that this train will spawn at.
   - "Sound": A set of movement sounds to choose from.
   - "Custom Sound": A user-specifiable movement sound to use, will override "Sound".
   - "Custom Start Sound": A user-specifiable train start sound to use.
   - "Custom Stop Sound": A user-specifiable stop sound to use.
   - "Distance between the wheels": The distance between the wheels of the tracktrain, with the origin
   brush expected to be in the middle of these. This is used to determine banking on the angle of movement
   as the tracktrain moves around.
   - "Height above track": Sets how many units above the path marked by the path_tracks the tracktrain will
   sit.
   - "Initial speed": The base speed value.
   - "Speed (units per second)": Max movement speed.
   - "Damage on crush": Damage to inflict on any entity that is blocking this tracktrain.
   - "Volume (10 = loudest)": The volume of the sounds this entity makes.
   - "Bank angle on turns": The maximum angle the train will roll on it's side when turning.
   - "Minimum light level": If set, the entity cannot have light values darker than this amount.
   
 - Spawn flags:
   - "No Pitch (X-rot)": Do not modify pitch value on turns.
   - "No User Control": The train will take speed from the path_track's "New Speed" value when passed.
   - "Passable": This train is non-solid, and will not block NPCs, players or bullets.

# func_train
>A brush based train entity that travels between path_corners, but does not rotate on turns or perform
>banking maneuvers. It moves with a constant velocity unless changed by a path_corner it passes. Useful
>for elevators, simple trains, etc. By default func_trains position the center of their bounding box at
>the path_corner entities, but you can override this behavior to use the origin brush instead, if you
>set the "Origin brush" spawnflag.

 - Keyvalues:
   - "Angles": Initial angles of the entity. Set an origin brush for this to work properly.
   - "Name": Name of this entity.
   - "Global Entity Name": If set, this entity will be transferred across a trigger_changelevel, and will
   try to find an entity with the same globalname on the other level, then will override it.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, only works with "Color" rendermode.
   - "ZHLT Lightflags": Special lighting flags to set, like "Opaque" or "Embedded fix".
   - "Light Origin Target": The entity will move by it's center to this position when lighting is 
   calculated for it in HLRAD.
   - "Noclip": Specify whether this entity should produce clipnodes, which determine if it collides with
   other entities and/or the player or not. Setting this to "Yes" can also saved on clipnode counts.
   - "First stop target": The name of the path_corner the func_train will start at.
   - "Move Sound": A set of movement sounds to choose from.
   - "Custom Move Sound": A user-specifiable movement sound to use. Will override "Move Sound" setting.
   - "Stop Sound": A list of stop sounds to play when the train stops.
   - "Custom Stop Sound": A user-specifiable stopping sound to use. Will override "Stop Sound" setting.
   - "Speed (units per second)": Movement speed of the train entity.
   - "Angular Veocity (y z x)": When moving, the train will rotate on it's axes based on the velocities set
   in this field. This can be changed by path_corner entities.
   - "Damage on crush": Damage to inflict on a blocking entity.
   - "Contents": Apply any contents flags here for special contents like slime, water, etc.
   - "Sound Volume 0.0 - 1.0": Movement/stop sound volume.
   - "Minimum light level": If set, the entity cannot have light values darker than this amount.

 - Spawn flags:
   - "Not solid": The train will not block NPCs, players or bullets.
   - "No fire pass": If set, this train will not fire "Fire on Pass" targets of path_corners it passes.
   - "No NPC nudge": Do not perform nudges on NPCs that use this as a ground entity. Nudging is used to
   ensure that entities standing ontop of a train entity do not get stuck for any reason.
   - "Sound at origin": Play the sound at the func_train's origin brush instead of it's center.
   - "Don't block nodes": This func_train will not block info_node linking logic, and NPCs will be able to
   check if the train is still blocking the path.
   - "Start On": If the train has a targetname, and this flag is set, the train will start moving on spawn.
   - "Origin brush": The func_train will use the origin brush for positioning itself on path_corners,
   instead of using it's center as is the normal behaviour.
   - "Always take avelocity": Take Angular Velocity values from path_corners even if the func_train has
   no avelocity currently.
   
# func_train_copy
>A brush based train entity that travels between path_corners, but does not rotate on turns or perform
>banking maneuvers. It moves with a constant velocity unless changed by a path_corner it passes. Useful
>for elevators, simple trains, etc. By default func_trains position the center of their bounding box at
>the path_corner entities, but you can override this behavior to use the origin brush instead, if you
>set the "Origin brush" spawnflag.
>Although this is a point entity in the FGD/Hammer, the purpose of this entity is to act like a func_train,
>but it doesn't have it's own brush model. Instead, it can be set to copy the brushmodel of another brush-
>based entity and use that as it's own. This is useful to save on level resources when you would need to
>copy the same brush geometry multiple times. The copied entity can be of any brushmodel type, such as a
>func_wall, func_clipeconomy, etc.

 - Keyvalues:
   - "Angles": Initial angles of the entity. Set an origin brush for this to work properly.
   - "Name": Name of this entity.
   - "Global Entity Name": If set, this entity will be transferred across a trigger_changelevel, and will
   try to find an entity with the same globalname on the other level, then will override it.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, only works with "Color" rendermode.
   - "ZHLT Lightflags": Special lighting flags to set, like "Opaque" or "Embedded fix".
   - "Light Origin Target": The entity will move by it's center to this position when lighting is 
   calculated for it in HLRAD.
   - "Noclip": Specify whether this entity should produce clipnodes, which determine if it collides with
   other entities and/or the player or not. Setting this to "Yes" can also saved on clipnode counts.
   - "Copy target": Name of the brush entity to use as the brushmodel of this entity.
   - "First stop target": The name of the path_corner the func_train will start at.
   - "Move Sound": A set of movement sounds to choose from.
   - "Custom Move Sound": A user-specifiable movement sound to use. Will override "Move Sound" setting.
   - "Stop Sound": A list of stop sounds to play when the train stops.
   - "Custom Stop Sound": A user-specifiable stopping sound to use. Will override "Stop Sound" setting.
   - "Speed (units per second)": Movement speed of the train entity.
   - "Angular Veocity (y z x)": When moving, the train will rotate on it's axes based on the velocities set
   in this field. This can be changed by path_corner entities.
   - "Damage on crush": Damage to inflict on a blocking entity.
   - "Contents": Apply any contents flags here for special contents like slime, water, etc.
   - "Sound Volume 0.0 - 1.0": Movement/stop sound volume.
   - "Minimum light level": If set, the entity cannot have light values darker than this amount.

 - Spawn flags:
   - "Not solid": The train will not block NPCs, players or bullets.
   - "No fire pass": If set, this train will not fire "Fire on Pass" targets of path_corners it passes.
   - "No NPC nudge": Do not perform nudges on NPCs that use this as a ground entity. Nudging is used to
   ensure that entities standing ontop of a train entity do not get stuck for any reason.
   - "Sound at origin": Play the sound at the func_train's origin brush instead of it's center.
   - "Don't block nodes": This func_train will not block info_node linking logic, and NPCs will be able to
   check if the train is still blocking the path.
   - "Start On": If the train has a targetname, and this flag is set, the train will start moving on spawn.
   - "Origin brush": The func_train will use the origin brush for positioning itself on path_corners,
   instead of using it's center as is the normal behaviour.
   - "Always take avelocity": Take Angular Velocity values from path_corners even if the func_train has
   no avelocity currently.
   
# func_wall
>A basic brush-based entity that functions as a wall, but can be removed(via Kill Target), or can have a
>Render Mode or Render FX set. It can also take angles if it has an origin brush, allowing you to rotate
>brush the geometry of the model without suffering from snap-to-grid issues. For this purpose you need to
>set the spawn flag "Take Angles".

 - Keyvalues:
   - "Name": Name of this entity.
   - "Angles": Angles of the entity. Set an origin brush for this to work properly.
   - "Global Entity Name": If set, this entity will be transferred across a trigger_changelevel, and will
   try to find an entity with the same globalname on the other level, then will override it.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, only works with "Color" rendermode.
   - "ZHLT Lightflags": Special lighting flags to set, like "Opaque" or "Embedded fix".
   - "Light Origin Target": The entity will move by it's center to this position when lighting is 
   calculated for it in HLRAD.
   - "Noclip": Specify whether this entity should produce clipnodes, which determine if it collides with
   other entities and/or the player or not. Setting this to "Yes" can also saved on clipnode counts.
   - "Parent": If set, this brushmodel will have it's collisions disabled, and will follow the parent
   entity's origin. It will pick up the same origin as the parent, and any offset from that origin is not
   supported right now.
   - "Minimum light level": If set, the entity cannot have light values darker than this amount.
   - "Ignore night mode": If set, the "Minimum light level" will not be ignored even if compiling RAD in 
   night mode. Otherwise, if compiling in night mode, the minimum light limit is ignored.
   
 - Spawn flags:
  - "Not in deathmatch": This entity will not be present in multiplayer.
  - "Take Angles": If coupled with an origin brush, the func_wall will be rotated based on it's "Angles"
  settings.
  
# func_wall_toggle
>This entity is a basic brush entity that can also be toggled on and off, which will make it completely
>invisible and non-solid. Like the func_wall entity, it can take Render Modes and Render FX settings.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Angles": Angles of the entity. Set an origin brush for this to work properly.
   - "Global Entity Name": If set, this entity will be transferred across a trigger_changelevel, and will
   try to find an entity with the same globalname on the other level, then will override it.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, only works with "Color" rendermode.
   - "ZHLT Lightflags": Special lighting flags to set, like "Opaque" or "Embedded fix".
   - "Light Origin Target": The entity will move by it's center to this position when lighting is 
   calculated for it in HLRAD.
   - "Noclip": Specify whether this entity should produce clipnodes, which determine if it collides with
   other entities and/or the player or not. Setting this to "Yes" can also saved on clipnode counts.
   - "Parent": If set, this brushmodel will have it's collisions disabled, and will follow the parent
   entity's origin. It will pick up the same origin as the parent, and any offset from that origin is not
   supported right now.
   - "Minimum light level": If set, the entity cannot have light values darker than this amount.
   - "Ignore night mode": If set, the "Minimum light level" will not be ignored even if compiling RAD in 
   night mode. Otherwise, if compiling in night mode, the minimum light limit is ignored.
   
 - Spawn flags:
  - "Not in deathmatch": This entity will not be present in multiplayer.
  - "Starts Invisible": The entity will start invisible and non-solid.
  - "Not Solid": The entity will be visible, but will act like a func_illusionary otherwise and will not
  collide with any other entities or players.
  - "Don't block nodes": This entity will not block node graph linking, so that when it is turned off, NPCs
  can move around the nodes it would block when enabled.
  - "Take Angles": If coupled with an origin brush, the func_wall will be rotated based on it's "Angles"
  settings.
  - "Nodraw": The entity will not be visible at all, but will still block other entities, players, bullets,
  etc.
  
# func_water
>The go-to water entity for shader water, this entity can also move in any direction like a door entity, it
>can have movement sounds, and all the other features of doors. Anything entering it will behave as if in
>water. Water entities are also tied to scripts based on the index specified in the water entity, and the
>level name.

>[!NOTE]
>Scripts for water entities are located inside "\*modfolder\*/scripts/water", and will be named with the
>following schema:<br />
>water\_\*mapname\*\_\*index\*.txt<br />
>So for entitytest2, the script will be named as "water_entitytest2_0.txt". If the daystage is set to the
>night mode, then a "_n" is appended to the end of the script name, like "water_entitytest2_0_n.txt". The
>contents of the script are defined using the fields described in "docs/water_scripts.md".
>Otherwise, the default values are taken from "scripts/water/water_default.txt".

 - Keyvalues:
   - "Name": Name of this entity.
   - "ZHLT Lightflags": Special lighting flags to set, like "Opaque" or "Embedded fix".
   - "Light Origin Target": The entity will move by it's center to this position when lighting is 
   calculated for it in HLRAD.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, only works with "Color" rendermode.
   - "Global Entity Name": If set, this entity will be transferred across a trigger_changelevel, and will
   try to find an entity with the same globalname on the other level, then will override it.
   - "Angles": Defines the movement direction of the water if it is set to move.
   - "KillTarget": The water will kill this entity when it is done moving.
   - "Speed": Amount of distance the water moves per second.
   - "Master": Name of a multisource entity controlling whether the water is enabled or disabled.
   - "Move Sound": Movement sound for this water.
   - "Custom Move Sound": Custom move sound for the water. Will override the "Move sound" setting.
   - "Stop Sound": Sound to play when the water stops moving.
   - "Custom Stop Sound": User-specifiable stop sound. Will override "Stop Sound" setting.
   - "Delay before close": Time until water closes again, a value of -1 will cause it to stay open.
   - "Lip": The water usually moves it's own length before stopping, but the lip value will cut off n units
   from this distance. A negative value will cause the water to move farther out.
   - "Damage inflicted when blocked": Damage inflicted upon any entity that blocks the water from moving.
   - "Target": Entity to trigger when done moving.
   - "Delay before fire": Delay before the target entity is triggered.
   - "Fire on Close": Target to trigger when the water closes.
   - "Locked Sound": Sound to play when the water is touched, and is locked by a multisource.
   - "Custom Locked Sound": User-specifiable locked sound. Will override "Locked Sound" setting.
   - "Unlocked Sound": Sound to play when opened for the first time after being unlocked.
   - "Custom Unlocked Sound": User-specifiable unlocked sound. Will override "Unlocked Sound" setting.
   - "Locked Sentence": Sentence to play when touched while locked.
   - "Unlocked Sentence": Sentence to play when opened for the first time after being unlocked.
   - "Contents": Specify a content type.
   - "Script Index": The index of the script to use for this water entity.
   
 - Spawn flags:
   - "Starts open": The door will start in the open position until touched/triggered.
   - "Passable": The door will be non-solid.
   - "Use Only": Only triggering, or using with the use key will open this door.
   
# game_dialouge
>This entity is useful for dialouge spoken by the player character. Sounds played by this entity will then
>lower the volume of other sounds, and the sound will also be save-restored on reload. This entity can also
>function as a "look at" entity, playing it's dialouge and triggering it's target when played if the player 
>looks at it. If set to start off, the Look At behavior will only begin working after this entity has first
>been triggered on by another entity.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": Target entity to trigger when triggered.
   - "WAV Name (e.g. vox/c.wav)": The wav file to use for the dialouge.
   - "Radius": If "Look At" is set, then the entity will only play if the player is within this radius.

 - Spawn flags:
   - "Look At": The entity will trigger it's dialouge automatically if looked at by the player, and if the
   "Radius" is set, the player will need to be within the radius *and* looking at the dialouge for it to be
   triggered.
   - "Start Off": The entity's "Look at" behavior will be disabled until triggered on by another entity.
   - "Repeatable": The entity will not remove itself after being triggered, and can be re-triggered to play
   any number of times.
   
# game_objective
>An entity that is used to specify the objectives that appear in the Objectives Window, a Game UI window
>the player can call to be displayed. A maximum of six active objectives are supported, due to limitations
>in the Game UI code. This entity can be used to add, or to remove an objective. Normally the entity sends
>a notification to the player when triggered, but this can be disabled.

>[!NOTE]
>Objectives are defined using scripts, located under "\*modfolder\*/scripts/gameui/objectives", where the
>format is the following:
```
$title "Objective name"<br />
{<br />
Body of text explaining the objective<br /><br />
}<br />
```
 - Keyvalues:
   - "Name": Name of this entity.
   - "Objective Identifier": Identifier of the objective, this needs to be the same as the name of the
   script file for the objective.
   - "Trigger Mode": Defines the behavior when triggered. Can either add a new objective, or remove it from
   the list of objectives.

 - Spawn flags:
   - "No Notifications": The entity will not notify the player of the changes it makes.
   
# game_radio
>Displays a radio message on the screen and plays the sound. A label will pop up on the right side of the
>screen displaying a radio icon, and the caller's name. Sounds played by this entity will also lower the
>volume of other sounds playing in the game. It can also be set to 

 - Keyvalues:
   - "Name": Name of this entity.
   - "WAV Name (e.g. vox/c.wav)": Path to the sound file to use.
   - "Talker's name": Name of the caller displayed on the screen.
   - "Hold time": How long to display the label.
   - "Label color (R G B)": Background color of the label.
   - "Transparency": Defines the transparency of the label, from 0 to 255.
   
# game_setdaystage
>Changes the day-stage of the game, where this will cause the game to instantly switch between lightmaps
>used for the game, as well as light_environment settings. Depending on the stage specified, either it will
>load the default lightmap, or it will load one of the two alternate lightmaps that can be stored in an ALD
>file generated by HLRAD. The change is instantenous, so I recommend using this during level changes, like
>returning to a chapter/level after night falls, daylight returns, etc.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Day stage": The stage of day to set.
     - "Normal": Loads the default lightmap and ignores the ALD file.
	 - "Night": Loads the nighttime lightmap, compiled by HLRAD using the "-nightmode" launch argument.
	 - "Daylight Return": Loads the "daylight return" lightmap, compiled by HLRAD using the 
	 "-daylightreturn" launch argument.
	
# game_text
>This entity can display a custom message on the screen without needing to define anything in titles.txt. I
>recommend using this for very short messages that don't have return carriages in them. The duration of the
>message being displayed depends on the "Text Effect" field, where, when using "Scan Out", the message will
>scan the letters out one by one depending on the "Fade in Time" setting, then it will remain until "Hold
>Time" until fading out.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": Entity to trigger after being triggered.
   - "Message Text": The text to display on the screen.
   - "X (0 - 1.0 = left to right) (-1 centers)": The starting X coordinate of the message. Setting this to 
   -1 will cause the message to be centered in the middle.
   - "Y (0 - 1.0 = top to bottom) (-1 centers)": The starting Y coordinate of the message. Setting this to 
   -1 will cause the message to be centered in the middle.
   - "Text Effect": The type of effect to use for the text.
     - "Fade In/Out": The entire message will fade in at once, then fade out at once.
	 - "Credits": The message will flicker between the two colors specified.
	 - "Scan Out": The message will be displayed by scanning out the characters one by one, first starting
	 with the secondary color, then fading to the primary color.
   - "Color1": The primary color of the message text.
   - "Color2": The secondary color of the message text, used by the "Credits" effect, and as the starting
   color when "Scan out" is set as the characters fade to the primary color.
   - "Fade in Time (or character scan time)": The fade in time for the whole message, or if the "Scan Out"
   "Text Effect" is set, the time it takes for an individual character to fully appear.
   - "Fade Out Time": The time it takes for the entire message to fade out at the end. Setting this to zero
   will just cause the message to disappear.
   - "Hold Time": How long the message will remain displayed at full transparency after it is done fading/
   scanning in.
   - "Scan time (credits effect only)": Defines how fast the two colors alternate with the "Credits" effect
   set for the message.
   - "Text Channel": The channel to use for this text message. Only a single message can occupy a channel,
   and any previous messages still active/displayed there will be removed instantly.
   
# game_timer
>Sets a timer on the HUD, in the top middle, which will display a message and count down the number of
>milliseconds, seconds, minutes and hours until the timer reaches zero. Useful for situations where you
>want the player to be aware of a time limit imposed on them.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Mode": Defines whether the counter is set to start when the entity is triggered, or whether
   triggering this entity will remove the timer from the HUD.
   - "Duration": The amount of time in seconds until the timer runs out.
   - "Counter title": The text to display in the timer tab.
   
# gibshooter
>Similar to the env_shooter entity but a bit more basic, this entity will shoot human gibs in the specified
>direction when triggered. The type of gib and model cannot be changed.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Angles": The angles specify the direction the gibs will be shot towards.
   - "Number of Gibs": Number of gibs to emit before stopping.
   - "Delay between shots": The amount of delay between each gib being shot out.
   - "Gib Velocity": Speed of gibs when they spawn.
   - "Course Variance": Variance in direction of velocity.
   - "Gib Life": The lifetime of each gib spawned.
   
 - Spawn flags:
   - "Repeatable": This entity can be called to shoot gibs multiple times.
   
# info_landmark
>This entity is used to mark the position of level changes. When changing a level, this position is used to
>collect entities to transfer using the PVS of the info_landmark entity. This entity must have the same
>position in the destination level relative to the level geometry, otherwise players/NPCs and any other
>entities transferred will be shifted around. You can also use a trigger_transition entity by giving it the
>same name as the info_landmark, which will only allow entities inside the bounding box of the transition
>entity to be transferred to the next level.
>If you are having issues with some entities not making it through the level change due to them not being
>in the PVS of the info_landmark, check out the info_vismark entity, which can be used to provide further
>sample spots for PVS data.

 - Keyvalues:
   - "Name": Name of this entity.
   
# info_light_origin
>This entity can be used to mark a position on the map where env_model entities will take their lighting
>from.

 - Keyvalues:
   - "Name": Name of this entity.
   
# info_monitorcamera
>This point entity defines the position where a monitor will render it's scene from, using the angles and
>field of view of this entity. You can limit the render distance of the monitor camera using the "Radius"
>setting.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Angles": The view angles used to render the camera's scene.
   - "Field of view": Defines the field of view for the camera.
   - "Radius": If set, the monitor will only render scene components within the bounding box defined by
   this value. This is useful for saving on performance with monitors.
   
# info_node
>An entity used to define the navigation paths in a level. These entities will look up other info_node
>entities and test the links between them during node graph generation. Special entities like doors,
>toggleable walls, npc clip entities will be taken into account when generating this graph. It is best
>practive to lay nodes by the concept of you can walk between the two without being blocked by a non-
>moving entity like a func_wall or by static world geometry, clip brushes.

 - Keyvalues:
   - "Angles": This is used by certain hint types to define where an NPC should turn before performing
   their hint-based activity, like looking out a window, sitting down on a couch, etc.
   - "Linking Range": You can set a maximum range in which this entity will seek out other info_nodes to
   link up with. Can be useful for preventing unwanted linking between distances that are too big.
   - "Region name": This can be used as an identifier by certain NPCs that patrol areas, or just wander
   around in them, to limit them to a particular region of nodes defined by a region name.
   - "Hint": The activity hint for the node, currently only used by Wandering NPCs like the security guard.
     - "No Hint": The node has no hint type set.
	 - "Sitting Spot": A Wandering NPC can seek this node out when the fatigue level is high, and will turn
	 to the angles set in the node entity, before sitting down.
	 - "Window Spot": A Wandering NPC will come to this node to look out a window, turning to the angles
	 set in the node entity.
	
# info_node_air
>A node entity much like "info_node", but used to define node paths for entities that either fly or swim.
>Currently, there is no AI code implemented to actually make use of this type of entity.

#info_null
>A point entity that will be removed before it can spawn. Mainly used to shut up error reports in Hammer
>for targets that are not actually used.

 - Keyvalues:
   - "Name": Name of this entity.

# info_player_deathmatch
>Marks a spawn point in multiplayer mode. Usable, but due to Pathos having almost no multiplayer support,
>it doesn't have much actual use.

 - Keyvalues:
   - "Angles": The angles the player spawning on this spot will start with.
   - "Target": Triggered when a player spawns at this spot.
   - "Master": A multisource controlling whether this spawn point is enabled.
   
 - Spawn flags:
   - "Not in deathmatch": this entity will not spawn in multiplayer mode.

# info_player_start
> Defines the spot where the player will spawn when the level is loaded.

 - Keyvalues:
   - "Angles": The angles the player spawning on this spot will start with.
   
 - Spawn flags:
   - "Not in deathmatch": this entity will not spawn in multiplayer mode.

# info_target
>A very basic point entity that other entities like beams, etc can use as a target.

 - Keyvalues:
   - "Name": Name of this entity.
   
# info_teleport_destination
>Marks a destination for trigger_teleport entities, or it can act as a landmark for them if they are set to
>act as relative teleports.

# info_vismark
>This entity is designed to act as additional VIS sampling points for level changes. You need to name your
>vismark entities with the same name as your info_landmark entity to work. This entity is useful if you
>need to ensure that entities otherwise not in the PVS of the info_landmark will also be carried over, by
>placing one of these entities in the area they occupy.

 - Keyvalues:
   - "Name": Name of this entity.
   
# item_diary
>This entity is an odd one, it is essentially a combination of an item and a trigger_cameramodel entity. It
>will play back an audio file while the player is holding a diary item. It is used in my game for story
>purposes. It could be used for playing back the player's thoughts while he is reading a journal or some
>other setting.

 - Keyvalues:
   - "Angles": The angles used by the item.
   - "Name": Name of this entity.
   - "Target": Triggered when the first-person cutscene is done playing.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, only works with "Color" rendermode.
   - "Trigger on Entry": Target to trigger when the player enters the animation.
   - "Duration": The duration for which the player will be holding the diary in front of their face.
   - "Sound to Play": The audio file to play back while the player is holding the diary.
   - "Player Dialouge": Player dialouge file to play at the very end. This will be cleared after the first
   use and will not play after the item_diary is used again.
   - "Type": Defines the diary model used.
     - "Benefactor": Will use the model located at "models/props/diary_benefactor.mdl".
     - "Emanations": Will use the model located at "models/props/diary_emanations.mdl".
	 
 - Spawn flags:
   - "Start Invisible": This diary will remain invisible and unusable until triggered by another entity.
   - "Stay Disabled": The diary can only be used once, and will remain disabled afterwards.
   - "Cannot be skipped": The player cannot skip the playback by pressing the "Use" key.
   
# item_glock_flashlight
>An underbarrel flashlight pickup for the Glock 17.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": And entity to trigger when the player walks over/picks up the item.
   - "KillTarget": An entity that will be killed when this entity is picked up/walked over.
   - "Delay before trigger": How long until the target/killtarget is triggered.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, doesn't work on this type of entity.
   - "Visibility Distance": The distance at which the item will remain visible.
   
 - Spawn flags:
  - "Not in deathmatch": This entity will not be present in multiplayer.
  - "No Pickup Notifications": There will be no sounds, HUD notifications played when this item is picked
  up by the player. It will just be silently add to their inventory.
  - "Trigger Only on Pickup": The entity will only trigger it's target when the player actually picks up
  the item, otherwise the target is not triggered if the player walks over the item.     
   
# item_glock_silencer
>A silencer pickup for the Glock 17.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": And entity to trigger when the player walks over/picks up the item.
   - "KillTarget": An entity that will be killed when this entity is picked up/walked over.
   - "Delay before trigger": How long until the target/killtarget is triggered.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, doesn't work on this type of entity.
   - "Visibility Distance": The distance at which the item will remain visible.
   
 - Spawn flags:
  - "Not in deathmatch": This entity will not be present in multiplayer.
  - "No Pickup Notifications": There will be no sounds, HUD notifications played when this item is picked
  up by the player. It will just be silently add to their inventory.
  - "Trigger Only on Pickup": The entity will only trigger it's target when the player actually picks up
  the item, otherwise the target is not triggered if the player walks over the item.   
   
# item_kevlar
>An item that adds armor points to the player, depending on the skill values in skill.cfg.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Angles": The angles used by the item.
  - "Target": And entity to trigger when the player walks over/picks up the item.
   - "KillTarget": An entity that will be killed when this entity is picked up/walked over.
   - "Delay before trigger": How long until the target/killtarget is triggered.
   
 - Spawn flags:
  - "Not in deathmatch": This entity will not be present in multiplayer.
  - "No Pickup Notifications": There will be no sounds, HUD notifications played when this item is picked
  up by the player. It will just be silently add to their armor value.
  - "Trigger Only on Pickup": The entity will only trigger it's target when the player actually picks up
  the kevlar, otherwise the target is not triggered if the player walks over the item.
  
# item_healthkit
>An item the player carries with them, and needs to use in order to heal wounds.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Angles": The angles used by the item.
   - "Target": And entity to trigger when the player walks over/picks up the item.
   - "KillTarget": An entity that will be killed when this entity is picked up/walked over.
   - "Delay before trigger": How long until the target/killtarget is triggered.
   
 - Spawn flags:
  - "Not in deathmatch": This entity will not be present in multiplayer.
  - "No Pickup Notifications": There will be no sounds, HUD notifications played when this item is picked
  up by the player. It will just be silently added to their inventory.
  - "Trigger Only on Pickup": The entity will only trigger it's target when the player actually picks up
  the kevlar, otherwise the target is not triggered if the player walks over the item.
  
# item_motorbike
>This entity functions as a motorbike vehicle, the player can mount it and use it to move around the level
>as if he was riding a motorbike. The entity is still not completely finished, so expect changes in future
>Pathos updates.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Angles": The angles used by the item.
  
# item_security
>A generic item that will use the security card model from "w_objects.mdl", and will trigger it's target
>and remove itself when the player touches it's bounding box.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Angles": The angles used by the item.
   - "Target": And entity to trigger when the player walks over/picks up the item.
   - "KillTarget": An entity that will be killed when this entity is picked up/walked over.
   - "Delay before trigger": How long until the target/killtarget is triggered.
   
 - Spawn flags:
  - "Not in deathmatch": This entity will not be present in multiplayer.
  - "No Pickup Notifications": There will be no sounds, HUD notifications played when this item is picked
  up by the player. It will just be silently add to their armor value.
  - "Trigger Only on Pickup": The entity will only trigger it's target when the player actually picks up
  the kevlar, otherwise the target is not triggered if the player walks over the item.
  
# item_shoulderlight
>This item will add a shoulder-mounted flashlight to the player's inventory, allowing them to have a weapon
>-independent flashlight on-hand.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Angles": The angles used by the item.
   - "Target": And entity to trigger when the player walks over/picks up the item.
   - "KillTarget": An entity that will be killed when this entity is picked up/walked over.
   - "Delay before trigger": How long until the target/killtarget is triggered.
   
 - Spawn flags:
  - "Not in deathmatch": This entity will not be present in multiplayer.
  - "No Pickup Notifications": There will be no sounds, HUD notifications played when this item is picked
  up by the player. It will just be silently add to their armor value.
  - "Trigger Only on Pickup": The entity will only trigger it's target when the player actually picks up
  the kevlar, otherwise the target is not triggered if the player walks over the item.
  
# item_taperecorder
>The tape recorder entity uses an animated tape recorder to play back a sound file. This sound file will
>play both at the entity's location, and will appear on the player's HUD much like a radio message, and
>the sound will also play for the player independently. When done playing back, the tape recorder turns
>itself off.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Angles": The angles used by the item.
   - "Sound to play": Path to the .wav file to play back.
   - "Label title": The title of the label displayed on the HUD.
   - "Duration": The duration of playback.
   - "Label color (R G B)": The background color of the label.
   - "Transparency": The transparency of the label, from 0 to 255.
   
 - Spawn flags:
   - "Start Invisible": The tape recorder will remain invisible and unusable until triggered on by another
   entity.
   
# light
>A simple point light source used by HLRAD to calculate lighting. This entity is removed on spawn, and can
>only contribute to static lighting, meaning it cannot be switched on and off. For switchable/animated 
>light sources, please refer to the env_dlight entity.

 - Keyvalues:
   - "Brightness": Defines the color of the light and the brightness. First three elements are the color,
   with values from 0 to 255. The fourth element is the intensity of the light source.
   - "ZHLT Fade": Multiplies the light value without affecting distances at which the light shines.
   - "ZHLT Falloff": Set the light intensity falloff model used.
     - "Default": Linear light falloff.
	 - "Inverse Linear"
	 - "Inverse Square"
	 
# light_spot
>A spotlight light source that will shine at it's target or at the specified angles, acting as a spotlight
>that will only illuminate what is in it's field of view. This entity statically contributes to lighting in
>the level, and cannot be switched or be animated. For switchable/animated light sources, please refer to
>the env_spotlight entity.

 - Keyvalues:
   - "Target": The entity to aim the spotlight at if specified. This will override "Angles".
   - "Angles": Defines the direction in which the spotlight will shine.
   - "ZHLT Fade": Multiplies the light value without affecting distances at which the light shines.
   - "ZHLT Falloff": Set the light intensity falloff model used.
     - "Default": Linear light falloff.
	 - "Inverse Linear"
	 - "Inverse Square"
   - "Inner (bright) angle": Defines the angle beyond which lighting achieves full intensity.
   - "Outer (fading) angle": Between the inner angle and this angle, light values will fade out the closer
   the lit area is to the outer angle of the spotlight.
   - "Pitch": This overrides the pitch value in Angles. A pitch value of -90 is straight down, a value of
   90 is straight up.
   - "Brightness": Defines the color of the light and the brightness. First three elements are the color,
   with values from 0 to 255. The fourth element is the intensity of the light source.
   - "Is Sky": This will cause the spotlight to act like a light_environment, with lighting projected from
   the sky brushes instead of projecting any light itself.

# night_light
>A simple point light source used by HLRAD to calculate lighting, but this only works with "-nightmode" set
>in the HLRAD launch arguments, and will only be visible in night mode. For more info, check the
>game_daystage entity. This entity is removed on spawn,  and can only contribute to static lighting, 
>meaning it cannot be switched on and off. For switchable/ >animated light sources, please refer to the 
>env_dlight entity.

 - Keyvalues:
   - "Brightness": Defines the color of the light and the brightness. First three elements are the color,
   with values from 0 to 255. The fourth element is the intensity of the light source.
   - "ZHLT Fade": Multiplies the light value without affecting distances at which the light shines.
   - "ZHLT Falloff": Set the light intensity falloff model used.
     - "Default": Linear light falloff.
	 - "Inverse Linear"
	 - "Inverse Square"
	 
# night_light_spot
>A spotlight light source that will shine at it's target or at the specified angles, acting as a spotlight
>that will only illuminate what is in it's field of view. This version of the spotlight only contributes to
>lighting if "-nightmode" is set in the HLRAD launch arguments, and will only be visible if the day stage
>is set to night mode, which is set using the "game_daystage" entity. This entity statically contributes to
>lighting in the level, and cannot be switched or be animated. For switchable/animated light sources, 
>please refer to the env_spotlight entity.

 - Keyvalues:
   - "Target": The entity to aim the spotlight at if specified. This will override "Angles".
   - "Angles": Defines the direction in which the spotlight will shine.
   - "ZHLT Fade": Multiplies the light value without affecting distances at which the light shines.
   - "ZHLT Falloff": Set the light intensity falloff model used.
     - "Default": Linear light falloff.
	 - "Inverse Linear"
	 - "Inverse Square"
   - "Inner (bright) angle": Defines the angle beyond which lighting achieves full intensity.
   - "Outer (fading) angle": Between the inner angle and this angle, light values will fade out the closer
   the lit area is to the outer angle of the spotlight.
   - "Pitch": This overrides the pitch value in Angles. A pitch value of -90 is straight down, a value of
   90 is straight up.
   - "Brightness": Defines the color of the light and the brightness. First three elements are the color,
   with values from 0 to 255. The fourth element is the intensity of the light source.
   - "Is Sky": This will cause the spotlight to act like a light_environment, with lighting projected from
   the sky brushes instead of projecting any light itself.
   
# light_environment
>This entity defines global lighting coming from the sun. This entity cannot be switched on or off, and is
>projected from sky brushes across the level. Whether this entity's lighting effects are applied can also
>depend on the day stage set.

 - Keyvalues:
   - "Angles": Defines the direction in which the spotlight will shine.
   - "ZHLT Fade": Multiplies the light value without affecting distances at which the light shines.
   - "ZHLT Falloff": Set the light intensity falloff model used.
     - "Default": Linear light falloff.
	 - "Inverse Linear"
	 - "Inverse Square"
   - "Pitch": The pitch value, where -90 is stragiht down, and 90 is straight up. Setting this to anything
   but zero will override the pitch in Angles.
   - "Brightness": Defines the color of the light and the brightness. First three elements are the color,
   with values from 0 to 255. The fourth element is the intensity of the light source.
   - "Daylight return only": This light will only be applied when "-daylightreturn" is set in HLRAD and the
   day stage is set to daylight return.
   - "Daylight return only": This light will only be applied when "-nightmode" is set in HLRAD and the day 
   stage is set to night time.
   
# npc_clone_soldier
>A squad-based hostile NPC that can also patrol a given area. They can carry three types of weapons, like
>the Sig 552, M249 and the Shotgun. They will attack the player or security guards on sight.

 - Keyvalues:
   - "Target": If set to a path_corner, the NPC will navigate the path given by the path_corner entities
   until reaching the end of the path.
   - "Name": Name of this entity.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, doesn't work on this type of entity.
   - "Angles": Used to set the orientation of the NPC on spawn.
   - "TriggerTarget 1": Target entity to trigger upon AI condition 1 being met.
   - "Trigger Condition 1": The AI condition to check for, see "Shared keyvalues" for more info.
   - "TriggerTarget 2": Target entity to trigger upon AI condition 2 being met.
   - "Trigger Condition 2": The AI condition to check for, see "Shared keyvalues" for more info.
   - "Don't drop weapons when killed": Controls whether any weapons will be dropped upon death.
   - "Force Skill Setting": Can be used to force the NPC's damage stats, health, etc to conform to a skill
   setting other than what is set in sv_skill.
     - "None": Skill settings will be applied like default.
	 - "Easy": Only use "Easy" skill values.
	 - "Normal": Only use "Normal/Medium" skill values.
	 - "Hard": Only use "Hard" skill values.
	 - "Less than Hard": Depending on the skill setting of the game, apply either "Easy" or "Normal" skill
	 values, but never "Hard".
   - "Patrol": Control whether this NPC will be patrolling around or not.
   - "Node region": The name of the info_nodes this NPC will patrol around.
   - "Patrol Radius": Max distance at which the NPC will seek out info_nodes to patrol from it's current
   location.
   - "Squad Name": Name of the squad this NPC belongs to.
   - "Weapons": Weapon to use.
   
 - Spawn flags:
   - "WaitTillSeen": The NPC will wait to be in the player's field of view before doing anything.
   - "Gag": The NPC will not do any idle banter.
   - "NPC Clip": func_npcclip brush entities will block this entity's movement.
   - "Don't forget player": If the player is an enemy, the NPC's awareness of the player will not time out
   after 30 seconds of not having spotted the player. This will cause the NPC to keep trying to find the
   player.
   - "Prisoner": The NPC will not perform any AI combat until being shot at.
   - "SquadLeader": If set, this NPC will be the leader of the squad it is in.
   - "No pushing by player or others": Neither the player nor other NPCs can push/nudge this NPC to move
   out of their way if the NPC is blocking their navigation.
   - "WaitForScript": The NPC will be in a waiting AI state until used by a scripted_sequence entity.
   - "Pre-Disaster": The NPC will use idle banter that is marked "Pre-Disaster", and will also not follow
   the player around when used.
   - "Fade Corpse": The corpse of the NPC will fade after dying.
   - "Immortal": The NPC can bleed and have damage decals applied when shot, and moan in pain, but will not
   actually recieve any damage.
   - "Don't Fall": When spawning, the NPC will not automatically be position on the ground below.
   
# npc_generic
>A generic NPC that cannot attack, will not be attacked by other NPCs, and does not talk or follow the
>player if used. Best reserved for passive characters in the scene, or for scripted_sequences.

 - Keyvalues:
   - "Target": If set to a path_corner, the NPC will navigate the path given by the path_corner entities
   until reaching the end of the path.
   - "Name": Name of this entity.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, doesn't work on this type of entity.
   - "Angles": Used to set the orientation of the NPC on spawn.
   - "TriggerTarget 1": Target entity to trigger upon AI condition 1 being met.
   - "Trigger Condition 1": The AI condition to check for, see "Shared keyvalues" for more info.
   - "TriggerTarget 2": Target entity to trigger upon AI condition 2 being met.
   - "Trigger Condition 2": The AI condition to check for, see "Shared keyvalues" for more info.
   - "Don't drop weapons when killed": Controls whether any weapons will be dropped upon death.
   - "Force Skill Setting": Can be used to force the NPC's damage stats, health, etc to conform to a skill
   setting other than what is set in sv_skill.
     - "None": Skill settings will be applied like default.
	 - "Easy": Only use "Easy" skill values.
	 - "Normal": Only use "Normal/Medium" skill values.
	 - "Hard": Only use "Hard" skill values.
	 - "Less than Hard": Depending on the skill setting of the game, apply either "Easy" or "Normal" skill
	 values, but never "Hard".
   - "Model": The model to use for this NPC.
   - "Body": The body value to use for the NPC.
   - "Not Solid": If set, this NPC will not collide with other NPCs or entities.
 
 - Spawn flags:
   - "WaitTillSeen": The NPC will wait to be in the player's field of view before doing anything.
   - "Gag": The NPC will not do any idle banter.
   - "NPC Clip": func_npcclip brush entities will block this entity's movement.
   - "Prisoner": The NPC will not perform any AI combat until being shot at.
   - "SquadLeader": If set, this NPC will be the leader of the squad it is in.
   - "No pushing by player or others": Neither the player nor other NPCs can push/nudge this NPC to move
   out of their way if the NPC is blocking their navigation.
   - "WaitForScript": The NPC will be in a waiting AI state until used by a scripted_sequence entity.
   - "Pre-Disaster": The NPC will use idle banter that is marked "Pre-Disaster", and will also not follow
   the player around when used.
   - "Fade Corpse": The corpse of the NPC will fade after dying.
   - "Immortal": The NPC can bleed and have damage decals applied when shot, and moan in pain, but will not
   actually recieve any damage.
   - "Don't Fall": When spawning, the NPC will not automatically be position on the ground below.
   
# npc_security
>A security guard NPC that is not hostile to the player unless set to be so. They also have Wandering AI
>behavior and will wander around the level using info_nodes and their hint types.

 - Keyvalues:
   - "Target": If set to a path_corner, the NPC will navigate the path given by the path_corner entities
   until reaching the end of the path.
   - "Name": Name of this entity.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, doesn't work on this type of entity.
   - "Angles": Used to set the orientation of the NPC on spawn.
   - "TriggerTarget 1": Target entity to trigger upon AI condition 1 being met.
   - "Trigger Condition 1": The AI condition to check for, see "Shared keyvalues" for more info.
   - "TriggerTarget 2": Target entity to trigger upon AI condition 2 being met.
   - "Trigger Condition 2": The AI condition to check for, see "Shared keyvalues" for more info.
   - "Don't drop weapons when killed": Controls whether any weapons will be dropped upon death.
   - "Force Skill Setting": Can be used to force the NPC's damage stats, health, etc to conform to a skill
   setting other than what is set in sv_skill.
     - "None": Skill settings will be applied like default.
	 - "Easy": Only use "Easy" skill values.
	 - "Normal": Only use "Normal/Medium" skill values.
	 - "Hard": Only use "Hard" skill values.
	 - "Less than Hard": Depending on the skill setting of the game, apply either "Easy" or "Normal" skill
	 values, but never "Hard".
   - "Use Sentence": Sentence to play when used and wants to follow the player.
   - "Un-Use Sentence": Sentence to play when used and will no longer follow the player.
   - "Node region": The node region to wander around in when wandering behavior is set.
   - "Wander": Enable/disable wandering AI behaviors.
   - "Hostile?": If set, the NPC will be hostile to the player and other friendly NPCs.
   - "Head": The head to use for the NPC.
   - "Weapons": Weapon the NPC will have equipped.
   
 - Spawn flags:
   - "WaitTillSeen": The NPC will wait to be in the player's field of view before doing anything.
   - "Gag": The NPC will not do any idle banter.
   - "NPC Clip": func_npcclip brush entities will block this entity's movement.
   - "Prisoner": The NPC will not perform any AI combat until being shot at.
   - "SquadLeader": If set, this NPC will be the leader of the squad it is in.
   - "No pushing by player or others": Neither the player nor other NPCs can push/nudge this NPC to move
   out of their way if the NPC is blocking their navigation.
   - "WaitForScript": The NPC will be in a waiting AI state until used by a scripted_sequence entity.
   - "Pre-Disaster": The NPC will use idle banter that is marked "Pre-Disaster", and will also not follow
   the player around when used.
   - "Fade Corpse": The corpse of the NPC will fade after dying.
   - "Immortal": The NPC can bleed and have damage decals applied when shot, and moan in pain, but will not
   actually recieve any damage.
   - "Don't Fall": When spawning, the NPC will not automatically be position on the ground below.
   
# npc_security_dead
>A dead npc_security entity that can be shot at and gibbed.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, doesn't work on this type of entity.
   - "Angles": Used to set the orientation of the NPC on spawn.
   - "Pose": The pose to use for the corpse
   - "Sequence": specify the sequence displayed in Hammer, by sequence index.
   
# npcmaker
>This entity can be used to spawn NPCs in the level, but it can also spawn non-NPC entities like weapon_
>and ammo_ entities, or item_ entities. 

> [!NOTE]
>This entity allows you to have the "Weapon" and "Head" settings be classname dependent. Say you want to
>specify the weapons for npc_security, then you'd edit your FGD to contain the following two fields:
>>weapons_npc_security
>>heads_npc_security
>And for the values these fields can have, you just want to copy your FGD entries from the npc_security
>entity, ending up with something like this:
```
heads_npc_security(choices) : "Head(Security Guard)" : 0 =<br />
[<br />
	0 : "Random"<br />
	1 : "Old White"<br />
	2 : "Young White"<br />
]<br />
weapons_npc_security(Choices) : "Weapons(Security Guard)" : 1 =<br />
[<br /><br />
1 : "Glock"<br />
2 : "Desert Eagle"<br />
4 : "TRG42"<br />
]<br />
```

 - Keyvalues:
   - "Name": Name of this entity.
   - "Angles": Used to set the orientation of the NPC on spawn.
   - "Target On Release": The entity that'll be triggered whenever a new NPC is spawned.
   - "Target On Death": The entity that'll be triggered when an NPC created by this entity dies.
   - "NPC Type": The classname of the entity, ie "npc_security".
   - "Childrens' Name": The targetname of the created entity.
   - "Number of NPCs": The max number of NPCs to spawn. A value of -1 will make this an infinite spawner.
   - "Frequency": The delay between NPC spawns. A value of -1 will set it so that a new NPC will only be
   spawned when the previous one dies.
   - "Max live children": Max NPCs alive at a given time before new ones are spawned.
   - "(Legacy)Weapon": This is a legacy feature to set weapons, use custom weapon settings as described
   above in "Notes".
   - "(Legacy)(Marine)Head": This is a legacy feature to set heads, use custom head settings as described 
   above in "Notes".   
   - "(Clone Soldier)Weapons": Weapon to set for the "npc_clone_soldier" entity.
   -"(Security)Weapons": Weapon to set for the "npc_security" entity.
   
# multi_manager
>This entity, when triggered, will fire the targets specified in it's keyvalues, based on name and a delay
>in seconds. To edit the trigger targets, you need to turn SmartEdit off in the Object Properties window,
>and then specify targets as keyvalue-value pairs. For example if you want to trigger an entity called
>"doofus" after 2 seconds, you'll set "Key" to "doofus" and "Value" to "2".

 - Keyvalues:
   - "Name": Name of this entity.
   
 - Spawn flags:
   - "multithreaded": The multi_manager is expected to be triggered multiple times at once, and setting
   this flag will ensure that no collisions with timings occur.
   
# multisource
>This entity can control the disabled/enabled state of other entities like triggers, doors, buttons, etc.
>Unlike in Half-Life, this entity will not need for every entity targeting it to trigger it before it is
>enabled, and only needs to be triggered once to toggle it's state. Further down the line, a legacy mode
>feature is planned to restore the original functionality.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Global State Master": If set, the multisource will only be enabled if the global state specified in
   this field is also enabled. These global states can be set by "env_global" entities.
 
 - Spawn flags:
   - "Start Enabled": If set and not in legacy mode, the multisource will start enabled.
   
# path_corner
>An entity that acts as a route point for func_train, func_train_copy, env_dlighttrain, env_modeltrain,
>env_elighttrain and env_spritetrain entities. This entity can modify the speed, or the angular velocity
>of trains that pass it, or the train can trigger the target specified in the "Fire On Pass" field.
>NPCs can also target a path_corner entity, and will then walk the path laid out by the path_corner entity.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Angles": Angles of this entity.
   - "Next stop target": The next path_corner in the route.
   - "Fire On Pass": Entity to fire when a train passes this path_corner. Note that each train entity that
   passes this path_corner will fire this target. If you only want one train to fire this, set the spawn
   flag "No fire pass" on the other train entities.
   - "Wait here (secs)": The train will stop on this path_corner and wait for n seconds before continuing
   on forward.
   - "New Train Speed": The train passing this path_corner will have this value set as it's speed.
   - "Angular Veocity (y z x)": The train will have these angular velocity values assigned to it when it
   passes this path_corner. Note that trains with avelocity set as "0 0 0" will not take avelocity values, 
   so if you want them to take these as well, tick the "Always take avelocity" flag on the train entity.
   
 - Spawn flags:
   - "Wait for retrigger": If set, any train arriving at this path_corner will stop and wait to to be
   triggered once again before moving.
   - "Teleport": The train will, instead of travelling to this path_corner, immediately teleport to it.
   - "Fire once": The "Fire On Pass" target entity will only be triggered once by the first train that
   passes this entity, and then the field will be cleared.
   - "Set Zero Avelocity": Set angular velocity on train entities even if the path corner has it as all
   zeroes. Otherwise, zero avelocities on path_corners will be ignored completely.
   
# path_track
>This type of path entity is used by func_tracktrain and func_trackchange type entities, and offers more
>specific features compared to path_corner. These paths can be enabled or disabled by triggering them.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Next stop target": The next path_track entity in the route.
   - "Fire On Pass": Entity to fire when a train passes this path_corner.
   - "Branch Path": Alternate path to use if the next stop target path_track is disabled.
   - "Fire on dead end": Fire target if the train passes this entity and this is a dead end. To achieve
   this, the path_track needs to be disabled, and the branch path needs to be empty.
   - "New Train Speed": The new speed to set for the func_tracktrain entity.
  
 - Spawn flags:
   - "Disabled": Starts disabled by default.
   - "Fire once": The "Fire On Pass" target entity will only be triggered once by the first train that
   passes this entity, and then the field will be cleared.
   - "Branch Reverse": The branch path will only be reachable when going backwards.
   - "Disable train": Disable controls on the train if it passes this path_track entity.
   
# player_loadsaved
>An entity that will fade the screen out, then optionally display a message before reloading the last saved
>game file.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Fade Duration (seconds)": The time it takes for the fade to reach full alpha intensity.
   - "Hold Fade (seconds)": How long the fade is held after reaching max alpha intensity.
   - "Fade Alpha": The maximum transparency for the fade, from 0 to 255.
   - "Fade Color (R G B)": The color of the fade.
   - "Show Message delay": Delay before the message is displayed.
   - "Message To Display": The message to display, from titles.txt.
   - "Reload delay": The time from being triggered it takes for the save to be loaded.
   
# player_weaponstrip
>This entity will remove all of the player's weapons, ammo and medkits when triggered.

 - Keyvalues:
   - "Name": Name of this entity.
   
# scripted_sentence
>An entity that can be used to force an NPC to play a sentence while looking at another entity or NPC. The
>sentence field can be left blank if you just want an NPC to stare at another one for a given amount of
>time after this entity is triggered. The sounds played by this entity will be restored on save loads with 
>proper offsets applied based on timings.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Sentence Name": The name of the sentence to play. Normally this will be a sentence group, but if you
   want a specific sentence entry, place a "!" before the name of the sentence group.
   - "Speaker Entity": The entity that will speak this sentence.
   - "Sentence Time": The amount of time the NPC will spend staring at the speak target.
   - "Radius": If the spawn flag "Subtitles only in Radius" is set, subtitles will only be visible when the
   player is within this distance of the NPC that speaks.
   - "Delay Before Refire": The delay between attempts made by the scripted_sentence to find an acceptable
   speaker entity.
   - "Listener": The NPC that we're talking to/staring at. Set to "player" for the player to be the target,
   or to the classname of the NPC you want to use, if you want any available NPC of a given type to speak 
   this sentence, ie "npc_security".
   - "Volume 0-10": The volume of the sentence spoken.
   - "Sound Radius": Radius in which to play the sound.
     - "Small radius": The sound will have a radius of 512 units.
	 - "Medium radius": The sound will have a radius of 960 units.
	 - "Large radius": The sound will have a radius of 1024 units.
	 - "Play everywhere": The sound will be played globally as a 2D sound.
	 - "X-Large": The sound will have a radius of 4864 units.
	
 - Spawn flags:
   - "Fire Once": This entity can only be used once, and will remove itself after use.
   - "Followers Only": Only NPCs following thee player can be picked.
   - "Interrupt Speech": Play sentence even if the target NPC is speaking at the moment.
   - "Concurrent": The sentence can play even if other NPCs are speaking at the moment.
   - "Subtitles only in Radius": Subtitles will only be displayed if the player is within the given
   radius of the speaker entity.
   
# scripted_sequence
>An entity that can be used to make an NPC perform a scripted animation or movement. The NPC can either be
>called to move to this entity by either running or walking, or to teleport to it immediately. The NPC can
>be also made to be stuck in an idle loop before being triggered to play it's "Play" animation. This can be
>followed by entering into a loop sequence until being triggered to cease it, at which point it can also
>perform the "Exit" animation.
>In order, the animations are played like this if all are set:
>"Idle Animation"->"Action Animation"->"Loop Animation"->"Loop Exit Anim"
>Loop animations keep playing until the scripted_sequence is triggered to stop.

>[!NOTE]
>If you use "Instantaneous" as the "Move to Position" setting, you will need to trigger the script again
>after the entity has moved, say 0.1s after triggering the script, to make the NPC perform the scripted
>sequence.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": And entity to trigger when the scripted_sequence is done playing.
   - "KillTarget": An entity that will be killed when the scripted_sequence is done playing.
   - "Delay before trigger": How long until the target/killtarget is triggered.
   - "Angles": The NPC will turn to these angles when performing the scripted sequence.
   - "Target NPC": The NPC that will be used by this scripted entity. Can either be it's targetname, or the
   classname of the NPC, ie "npc_security".
   - "Action Animation": Animation to perform when triggered to do so.
   - "Idle Animation": Optional idle loop animation to perform before being triggered. Normally the script
   entity will look for the target within distance defined in the "Search Radius" field when the "Only in
   Radius" field is set, or it'll search globally. If the spawn flag "Trigger to Idle" is set, the script
   will begin idling once it is triggered to do so.
   - "Loop Animation": The loop animation to play after the "Action Animation" is done playing.
   - "Loop Exit Anim": Exit animation to play when triggered to stop looping.
   - "Search Radius": The distance in which the entity will look for suitable target(s) when the target is
   specified as a classname. Needs the flag "Only in Radius" to be set if you want to use a targetname 
   instead of a classname.
   - "Repeat Rate ms": The delay between attempts to possess the target NPC.
   - "Move to Position": The way the NPC will take up the position of the script entity.
     - "No": The NPC will remain at it's original spot and just play the script.
	 - "Walk": The NPC will walk to the scripted_sequence, then turn to the angles set in the entity.
	 - "Run": The NPC will run to the scripted_sequence, then turn to the angles set in the entity.
	 - "Instantaneous": The NPC will immediately be teleported to the script position, and take up a 
	 waiting script state. The script will need to be triggered again for it to perform the script.
	 - "No - Turn to Face": THe NPC will not move to the script entity, but instead just take up the angles
	 of the scripted_sequence.
	 - "Walk - No Turn": Walk to the position of the script entity, but do not take up it's angles.
	 - "Run - No Turn": Run to the position of the script entity, but do not take up it's angles.
	 - "No - Turn to Player": The NPC won't move, but instead will turn to face the player.
   - "Combat sounds interupt": If the spawn flag "Sounds can interrupt" is set and you set this to true,
   then any combat/weapon firing sounds can interrupt the script.
   - "World sounds interupt": If the spawn flag "Sounds can interrupt" is set and you set this to true,
   then any world/mundane sounds can interrupt the script. 
   - "Player sounds interupt": If the spawn flag "Sounds can interrupt" is set and you set this to true,
   then any sounds made by the player can interrupt the script. 
   - "Danger sounds interupt": If the spawn flag "Sounds can interrupt" is set and you set this to true,
   then any dangerous sounds(Grenades tumbling, etc) can interrupt the script. 
   - "Trigger on play sequence start": This field is used for very precise timing, exactly when the "Play"
   animation begins playing. Usually reserved for env_syncanimation entities.
   - "Trigger on loop sequence start": This field is used for very precise timing, exactly when the "Loop"
   animation begins playing. Usually reserved for env_syncanimation entities.
   - "Trigger on loop exit sequence start": This field is used for very precise timing, exactly when the 
   "Loop Exit" animation begins playing. Usually reserved for env_syncanimation entities.
   
 - Spawn flags:
   - "Repeatable": The entity will not remove itself after it is done playing a sequence, meaning it can be
   set to repeat the script multiple times.
   - "Leave Corpse": If the NPC is killed while performing the sequence with a death event, the script will
   not fade the corpse out after it is done playing.
   - "No Interruptions": The script playback cannot be interrupted by any means, which means that any kind
   of interrupts, sounds, being hit, used, etc will not cancel the script.
   - "Override AI": The NPC will be forcefully possessed regardless of their AI state.
   - "No Script Movement": Offsets in the animation cannot change the position of the entity on exit.
   - "Trigger to Idle": If set, the target entity will not begin looping the Idle animation unless this
   entity is triggered first.
   - "Only in Radius": Only seek out the entity specified by a targetname when it is in the radius of this
   scripted entity.
   - "Sounds can interrupt": Hearing specific sounds can interrupt this script.
   - "Enemies can interrupt": Seeing any kind of enemy will immediately cancel the script.
   
# trigger_auto
>An entity that will automatically trigger it's target when the level starts or gets reloaded. It can also 
>be controlled by a global state, so that it only triggers it's target on level load/spawn when the global
>state is in the "On" state. These global states can be set with the "env_global" entity. Note that if you
>do not have this entity removed after triggering, it'll keep triggering it's target with each load of the
>level.

 - Keyvalues:
   - "Target": And entity to trigger when the level starts/loads.
   - "KillTarget": An entity that will be killed when the scripted_sequence is done playing.
   - "Delay before trigger": How long until the target/killtarget is triggered.
   - "Global State to Read": A global state that controls whether this entity is enabled or disabled.
   - "Trigger State": The triggering mode used, which can be "On", "Off" or "Toggle".
   - "Priority L1 Target": Priority level 1 target, will be overridden by higher priority levels, and is
   cleared after triggering.
   - "Priority L1 Global State": The global state that controls the priority level 1 trigger behavior.
   - "Priority L2 Target": Priority level 2 target, will be overridden by higher priority levels, and is
   cleared after triggering, and also clears priority level targets beneath it without letting them trigger
   their own targets.
   - "Priority L2 Global State": The global state that controls the priority level 2 trigger behavior.
   - "Priority L3 Target": Priority level 3 target, which will be triggered if the global for this level
   is enabled. It'll also clear targets beneath this level, and prevent them from triggering their own
   targets.
   - "Priority L3 Global State": The global state that controls the priority level 3 trigger behavior.

 - Spawn flags:
   - "Remove On fire": This entity is removed after firing it's target(s).
   
# trigger_autosave
>A brush-based trigger entity that will cause an autosave to occur. Can be controlled by a multisource, and
>can be set to remain in-game and autosave during other day stages. Otherwise, it removes itself after it
>has been touched by a player. It can also be triggered by another entity to perform the autosave.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Master": Name of the multisource controlling this trigger.

 - Spawn flags:
   - "All Day Stages": Instead of removing itself when touched, the entity will wait for another day stage
   to be set before it'll autosave again when triggered.
   
# trigger_camera
>A camera entity that will take over the view, and display the world from it's position. It can be set to
>look at a specific entity after activation, or can be given an angle to face. It can also follow 
>path_corner entities and move about. This entity will also be save-restored when the game is reloaded.

 - Keyvalues:
   - "Target": And entity to trigger when the level starts/loads.
   - "KillTarget": An entity that will be killed when the scripted_sequence is done playing.
   - "Name": Name of this entity.
   - "Angles": Specifies the direction to look towards unless set to look at a target.
   - "Hold time": The duration for which this entity will be active.
   - "Path Corner": The first path_corner entity to start at.
   - "Initial Speed": The initial speed of the entity.
   - "Acceleration units/sec^2": How much the camera will accelerate towards it's destination.
   - "Stop Deceleration units/sec^2": Amount of deceleration before reaching the destination.
   - "Angular speed": The speed at which the camera will turn towards it's look target.
   
# trigger_cameramodel
>This entity acts as a script entity for first-person cutscenes. It uses an animated model to take control
>of the player's view much like trigger_camera, but instead the view will follow the attachments inside the
>model specified as it animates. The first attachment(0 index) defines the view origin, while the second
>attachment(index 1) is the forward vector, while the third(at index 2) defines the left vector. Using this
>a view angle is derived from the animated entity.
>This entity can also be set to snap to the player's current position upon activation, which makes it very
>useful for cases where you want it to take over regardless of the player's position or angles.

>[!NOTE]
>The sequences used by this entity are played in the following order if not set to loop:
>"Resting Sequence"->"Sequence to Play"
>Or if a loop sequence is set:
>"Resting Sequence"->"Loop Entry Sequence"->"Loop Sequence"->"Sequence to Play"

>[!NOTE]
>If you use a loop but don't specify a "Sequence to Play", you will still need to trigger the entity twice
>to make it exit.

 - Keyvalues:
   - "Angles": The angles to use for the animation.
   - "Name": Name of this entity.
   - "Target": Entity to trigger when the cameramodel is done playing.
   - "Parent": If set, this brushmodel will have it's collisions disabled, and will follow the parent
   entity's origin. It will pick up the same origin as the parent, and any offset from that origin is not
   supported right now.
   - "Model": The VBM model to use for the first-person cutscene.
   - "Resting Sequence": When triggered on, the entity will play this sequence on a loop until triggered
   to play the next animtion. If the same value is specified as for 
   - "Loop Entry Sequence": If set to loop, this'll be played after the entity is triggered to exit the
   resting sequence.
   - "Lerp time": Time it takes for the view to be linearly interpolated into the position it'll be playing
   at. Needs the flag "Lerp view" to be set for this to work.
   - "Trigger on Loop": Entity to trigger when the cameramodel enters into the "Loop Sequence" playback.
   - "Field of View": The field of view to use for this entity when it plays.
   - "Body": The body value, use to set a specific submodel to use.
   
 - Spawn flags:
   - "Follow Player": Immediately snap to the player's position when triggered to play.
   - "Leave on Trigger": If set, the cameramodel will not release the player when it is done playing, and
   will wait to be triggered to exit completely.
   - "Lerp view": If set, the view of the player will interpolate into the position defined by the
   animation playing and the attachments.
   - "Keep VModel": The view model will not holster when this entity is activated, and will remain active
   during the animation playback.
   -  "Keep Angles": If you set this with "Follow Player" set, the player's angles and view angles will be 
   taken from this entity's angles instead of the players.
   - "Interpolation": Allow animation interpolation between sequence change when the entity is triggered.
   
# trigger_changelevel
>If this entity is touched by the player, or is triggered by another entity, it will initiate a level
>transition to the next level specified. Note that level changes are only valid between two levels if they
>all point to the other level, eg if on entitytest1 you are changing to entitytest2, on entitytest2 you
>need the changelevel on the other end to refer to entitytest1.
>This entity also uses an info_landmark entity for changing levels, which acts as the local space marker,
>and should occupy the same position relative to the world on both ends. Additionally, a trigger_transition
>entity with the same name as the landmark can function to exclude any transitioning entities not inside
>bounding box of the transition trigger.
>If you put info_vismark entities into the level with the same name as the info_landmark, these entities
>will be used to sample additional VIS information to extend the range where entities are picked to be
>included in the transition to the next level.

 - Keyvalues:
   - "Name": The name of this entity.
   - "New map name": The name of the next level to load, eg "entitytest2".
   - "Landmark name": The name of the landmark, and optionally trigger_transition and info_vismark entities
   tied to this level change.
   
 - Spawn flags:
   - "USE Only": Only change levels when triggered by another entity, will disable trigger by touch.
   
# trigger_chance
>An entity with a chance of triggering it's target after the entity itself is triggered.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": Entity to trigger when the cameramodel is done playing.
   - "Chance Percentage": The percentage in which the probability it tested. If set to 90, then there's a
   90% chance the entity will trigger what it is targeting.
   
# trigger_changetarget
>An entity used to change the "Target" field of another entity to the named entity in this entity. It can
>be used with any entity that triggers a target, except for multi_managers.

 - Keyvalues:
   - "Name": The name of this entity.
   - "Target": The entity to change the target of.
   - "KillTarget": An entity that will be killed when this entity is triggered.
   - "Delay before trigger": How long until the killtarget is removed.
   - "New Target": The name of the entity to change the trigger target to.
   
# trigger_coderegister
>An entity that lets you manually register an access code/passwcode in the game. This passcode will then
>be read by trigger_login and trigger_keypad entities.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Code ID": The Id of this code.
   - "Code": The code itself. If "Generate Code" is set, then the code generated is appended after what is
   present here, ie. a value of "bossnasss-" here and "Generate Code" set will result in "bossnass-666" as
   the generated value. Please not that adding anything but numbers here will make it invalid when used
   with trigger_keypad entities.

 - Spawn flags:
   - "Generate code": If set, the entity will automatically generate a random code between 2 and 6 digits.
   
# trigger_counter
>A brush-based counter entity that will increment it's counted value whenever touched or triggered. When it
>reaches the "Count before activation" value, it'll trigger it's target.

 - Keyvalues:
   - "Target": The entity this trigger will fire after being touched/triggered.
   - "Name": Name of this entity.
   - "Filter by name": If set, only an entity, like an NPC or pushable that has this targetname will be
   able to activate this trigger entity.
   - "Kill target": Entity to kill/remove when activated.
   - "Master": Multisource controlling whether this entity is enabled or disabled.
   - "Delay before trigger": Delay before the target entity is triggered.
   - "Message": Message to display when triggered on.
   - "Count before activation": The amount of times this entity needs to be triggered before it fires it's
   own target.
   
 - Spawn flags:
   - "NPCs": NPCs can touch this trigger to activate it.
   - "No Clients": Players cannot touch this trigger.
   - "Pushables": Pushables can activate this trigger.

# trigger_counter_p
>Similar to trigger_counter, but this is instead a point entity that does not function as a trigger volume.
>It will trigger it's target after being triggered the amount of times specified in "Count before trigger".

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": The entity this trigger will fire after being triggered.
   - "Count before trigger": The amount of times this entity needs to be triggered before it fires it's own
   target.
   
# trigger_endgame
>Triggering this entity will cause the game to end and return to the menu.

 - Keyvalues:
   - "Name": Name of this entity.
   
# trigger_forceclose
>An entity, that when triggered, will close any door entities it targets.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": The door this entity will force to close when triggered.
   
# trigger_forceholster
>This can be used to set whether the player is holding their weapon, or will be forced to holster it. It
>can also set other special flags that determine movement speed, etc.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Mode": The mode by which this entity will change the "Force Holstering" state of the player.

 - Spawn flags:
   - "Dream sequence": A special use case, it will make the player immune to drowning, and also prevent the
   playback of any swimming/water sounds while underwater.
   - "No slow walking": Do not modify the movement speed of the player.
   
# trigger_globaldelayed
>An entity that will set a delayed trigger that will work across levels. The targetted entity needs to be
>present across all levels where this is expected to occur.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": The entity this trigger will fire after the time runs out.
   - "Duration": The time in seconds before the target is triggered.
   
# trigger_gravity
>An entity that changes the gravity of the entity touching it. Gravity is modified by a multiplier between 
>the 0-1 range.

 - Keyvalues:
   - "Target": The entity this trigger will fire after being touched/triggered.
   - "Name": Name of this entity.
   - "Filter by name": If set, only an entity, like an NPC or pushable that has this targetname will be
   able to activate this trigger entity.
   - "Kill target": Entity to kill/remove when activated.
   - "Master": Multisource controlling whether this entity is enabled or disabled.
   - "Delay before trigger": Delay before the target entity is triggered.
   - "Message": Message to display when triggered on.
   - "Gravity (0-1)": The multiplier for the gravity value.
   
 - Spawn flags:
   - "NPCs": NPCs can touch this trigger to activate it.
   - "No Clients": Players cannot touch this trigger.
   - "Pushables": Pushables can activate this trigger.
   
# trigger_hurt
>A brush entity that will hurt any entity that touches it. The type of damage can be specified, as well as
>the delay between each time damage is dealt.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": The entity this will trigger when touched.
   - "Master": The multisource controlling this trigger.
   - "Damage": The amount of damage to deal.
   - "Delay before trigger": The delay before the target of this entity is triggered.
   - "Delay before next hurt": Delay between inflicting damage points.
   - "Damage Type": The type of damage to deal to the entity touching this trigger.
 
 - Spawn flags:
   - "Target Once": If set, the target of this trigger will only be triggered once.
   - "Start Off": Start disabled until triggered on by another entity.
   - "No clients": Clients touching this entity will not be hurt, and they cannot cause it to trigger it's
   target entity.
   - "Fire Client Only": Only players can cause this entity to trigger it's target.
   - "Touch Client Only": This trigger will only interact with players.

# trigger_keypad
>An entity that will spawn a keypad window, with the "Code Id" specified being displayed on the bottom if
>that code has been registered in the list of codes read. Only purely numeric codes will work with this
>entity. For codes that contain text, check "trigger_login".

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": The entity this will trigger when the proper code is inputted.
   - "Passcode": The code that the player has to input to make this entity work. Use this if you do not
   want to use registered keycodes. The info box with the code will not appear in this case.
   - "Code ID": The global ID of the code to use with this keypad window. Will override "Passcode".

 - Spawn flags:
   - "Stay Till Next": The keypad window will stay until another window, ie a text window will spawn.
   
# trigger_killplayer
>An entity that kills the player when it is triggered.

 - Keyvalues:
   - "Name": Name of this entity.
   
# trigger_login
>Very similar to trigger_keypad, but instead it expects a username to be specified, as well as a password
>that can be auto-generated with a trigger_textwindow or trigger_coderegister, or be specified in this
>entity itself.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": The entity this will trigger when the proper username and password is inputted.
   - "Password": Password to use for this login to work. 
   - "Code ID": The global ID of the code to use with this login window. Will override "Password".
   
 - Spawn flags:
   - "Stay Till Next": The login window will stay until another window, ie a text window will spawn.
   
# trigger_lookat
>An entity that will trigger it's target when the player is looking at it, with the option to set a minimum
>distance and an amount of time the player has to be looking at it. This entity will remove itself once it
>has triggered it's target.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": The entity this will trigger when the player looks at it long and close enough.
   - "Radius": You can specify a radius the player has to be in to look at this entity and make it trigger
   it's target. If set to zero, the distance to the viewer is ignored.
   - "Look At Time": The amount of time the player has to spend continously looking at the entity for it to
   trigger it's target.
   
 - Spawn flags:
   - "Start Off": The entity needs to be triggered on before it will be able to trigger anything.
   - "Ignore Glass Occlusion": Any transparent entities between the viewer and this entity will be ignored.

# trigger_move
>This entity allows you to move an entity to another position, with the option to specify a landmark that
>turns this into a relative moving procedure.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": The entity that we will be moving to another position.
   - "Destination": The destination, can either be the position to move to, or it'll specify a landmark
   entity. This landmark entity can be an info_teleport_destination, etc.
   - "Landmark Entity": The landmark which defines the position everything will be relative to when the
   entity is moved. This should align with the "Destination" landmark.
   - "Ground Entity": If you are moving an NPC or player onto a moving entity like a train, you want to
   specify the name of that entity here so that it is immediately set as the ground entity.

 - Spawn flags:
   -  "Take angles of target": If this is set, the angles of the moved entity will be set to those of the
   entity named in "Destination".
   
# trigger_movetrain
>This entity allows you to move a func_train, func_train_copy, env_dlighttrain, env_elighttrain,
>env_spritetrain or env_modeltrain to another path_corner route. 

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": The entity that we will be moving to another path_corner route.
   - "Destination Node": The path_corner we'll be moving this train to.
   - "Speed": The new speed of the train.
   
 - Spawn flags:
   - "Redirect": If set, the train, instead of being instantenously moved, will instead reroute itself to
   move towards the target path_corner specified.
   
# trigger_multiple
>A trigger that will keep firing it's target as long as a valid entity is touching it. You can specify the
>delay betwen each touch registered. When used with a Master or a trigger_changetarget, this can be useful
>for toggling certain entity setups.

 - Keyvalues:
   - "Target": The entity this trigger will fire after being touched/triggered.
   - "Name": Name of this entity.
   - "Filter by name": If set, only an entity, like an NPC or pushable that has this targetname will be
   able to activate this trigger entity.
   - "Kill target": Entity to kill/remove when activated.
   - "Master": Multisource controlling whether this entity is enabled or disabled.
   - "Delay before trigger": Delay before the target entity is triggered.
   - "Message": Message to display when activated.
   - "Delay before reset": The delay before this entity can be trigger again.
   
 - Spawn flags:
   - "NPCs": NPCs can touch this trigger to activate it.
   - "No Clients": Players cannot touch this trigger.
   - "Pushables": Pushables can activate this trigger.

# trigger_npcpull
>An entity that will begin pulling NPCs towards a position marked in the entity, akin to a vacuum pulling 
>objects towards itself. This entity will first try to pull objects towards itself, but if the entity that
>is marked in "Final Position Enitity" can pull them, it will instead begin pulling them towards itself.
>You need to trigger this entity for it to start working, and only specific NPCs will have the required
>pull animations to actually act like they are being pulled off their feet.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": An brush entity like a func_illusionary, which defines the bounding box of the pull in which
   NPCs will be pulled to the final position marker.
   - "Final Position Enitity": The entity that marks the final position to pull NPCs towards once this
   position marker has free line of sight to the NPC.
   - "Pull velocity": The force the NPCs will be pulled with.
   - "Full force time": The time it takes for the pull to reach full force after being triggered on.
   - "Fade out time": The time it takes for the effect to fade out after being triggered off.
   
# trigger_once
>This brush-based trigger will fire it's target when touched by a valid entity, and then will remove itself
>from the game entirely.

 - Keyvalues:
   - "Target": The entity this trigger will fire after being touched/triggered.
   - "Name": Name of this entity.
   - "Filter by name": If set, only an entity, like an NPC or pushable that has this targetname will be
   able to activate this trigger entity.
   - "Kill target": Entity to kill/remove when activated.
   - "Master": Multisource controlling whether this entity is enabled or disabled.
   - "Delay before trigger": Delay before the target entity is triggered.
   - "Message": Message to display when activated.
   
 - Spawn flags:
   - "NPCs": NPCs can touch this trigger to activate it.
   - "No Clients": Players cannot touch this trigger.
   - "Pushables": Pushables can activate this trigger.
   
# trigger_paralyzeplayer
>This entity will prevent any input from the player from affecting the position, movement and view angles
>of the player.

 - Keyvalues:
   - "Name": Name of this entity.
   
# trigger_push
>This brush entity will push any objects that touch it. It can be triggered on and off, and the angles can
>be used to set the direction of the push.

 - Keyvalues:
   - "Target": The entity this trigger will fire after being touched/triggered.
   - "Name": Name of this entity.
   - "Filter by name": If set, only an entity, like an NPC or pushable that has this targetname will be
   able to activate this trigger entity.
   - "Kill target": Entity to kill/remove when activated.
   - "Master": Multisource controlling whether this entity is enabled or disabled.
   - "Delay before trigger": Delay before the target entity is triggered.
   - "Message": Message to display when activated.
   - "Angles": The angles define in which direction entities will be pushed.
   - "Speed of push": The speed by which the object wille be pushed.
   
 - Spawn flags:
   - "NPCs": NPCs can touch this trigger to activate it.
   - "No Clients": Players cannot touch this trigger.
   - "Pushables": Pushables can activate this trigger.
 
# trigger_relay
>This point entity can be used as a relay, as to trigger a target entity on or off, ot toggle it's current
>state. This depends on the setting in "Trigger State". It can also be used to remove entities from the
>game using the "Kill target" field.

 - Keyvalues:
   - "Name": The name of this entity.
   - "Target": The entity to trigger.
   - "KillTarget": An entity that will be killed when this entity is triggered.
   - "Delay before trigger": How long until the killtarget is removed.
   - "Trigger State": Defines what state to trigger the target with. "Off" and "On" will turn any entities 
   off that support this(like env_ entities), or using "Toggle" will toggle their current state depending
   on what it is at the moment of triggering.
   
 - Spawn flags:
   - "Remove On fire": When triggered, this entity will be removed from the game.
   
# trigger_relay_binary
>This is a relay entity that tracks it's own enabled/disabled state, and will trigger a trigger target
>depending on what state it is in, with a unique target or the "On" and the "Off" state. The initial state
>can be toggled by triggering it.

 - Keyvalues:
   - "Name": The name of this entity.
   - "Target on turn on": When triggered on, the entity will fire this target.
   - "Trigger on mode": When triggered on, the entity will use this mode to trigger it's target. "Off" and 
   "On" will turn any entities off that support this(like env_ entities), or using "Toggle" will toggle 
   their current state depending on what it is at the moment of triggering.
   - "Target on turn off": When triggered on, the entity will fire this target.
   - "Trigger onoffmode": When triggered on, the entity will use this mode to trigger it's target. "Off" 
   and "On" will turn any entities off that support this(like env_ entities), or using "Toggle" will toggle
   their current state depending on what it is at the moment of triggering.
   - "Delay before trigger": The delay before either target is triggered.
   - "Initial state": Set the initial state of this binary trigger.

# trigger_repeat
>Once triggered, this point entity will keep triggering it's target until it exhausts the number of repeats
>specified.

 - Keyvalues:
   - "Name": The name of this entity.
   - "Target": The entity to trigger.
   - "Delay between triggers": The delay in seconds between triggers.
   - "Number of repeats": The number of times to trigger. A value of -1 will keep triggering indefinitely.
   
# trigger_saveweapons
>This entity will save the weapons, their clip and ammo counts, the number of healthkits, the health and
>the armor values of the player, and restore all of them when triggered again.

 - Keyvalues:
   - "Name": The name of this entity.
   
# trigger_setsavetitle
>Set the title used for save files that will be displayed in the Save/Load window. Useful for setting the
>title of a chapter to appear on your save game file.

 - Keyvalues:
   - "Name": The name of this entity.
   - "Savegame Title": The title to display for the save file.

 - Spawn flags:
   - "Set on Spawn": The entity will set the save title the moment it spawns.
   
# trigger_slowmove
>This entity will set the movement speed of the player when triggered. How it modifies this setting will
>depend on the "Mode" specified.

 - Keyvalues:
   - "Name": The name of this entity.
   - "Mode": The mode of setting the slow movement on the player.
   
# trigger_speedup
>Modifies the speed of an entity by linearly increasing it from a starting velocity to a final value, with
>the amount of time specified for this to occur. Most commonly used for modifying the speed of func_train
>entities.

 - Keyvalues:
   - "Name": The name of this entity.
   - "Target": The entity to modify the speed of.
   - "Start Velocity": The initial speed to start speeding up from.
   - "End Velocity": The final velocity value to set at the end.
   - "Duration": The time it takes for the target to reach full velocity.
   
# trigger_sound
>When the player is touching this brush trigger, the entity will set the environmental audio effect
>specified in "Room Type".

 - Keyvalues:
   - "Name": The name of this entity.
   - "Master": Multisource controlling whether this entity is enabled or disabled.
   - "Room Type": The room effect to use.
   
# trigger_subway_controller
>An entity used for a subway system, this takes the input from a Game UI window it spawns, then it will
>trigger the target depending on the player's choice. The choices avaiable for the player are determined
>by the flags set using a trigger_subway_flagger entity. Currently this entity only supports one subway
>line, but changes are planned to include mode.

 - Keyvalues:
   - "Name": The name of this entity.
   - "Suburb Target": The target to trigger if the "Suburbs" option is enabled and clicked on.
   - "Ghetto Target": The target to trigger if the "Ghetto" option is enabled and clicked on.
   - "Outskirts Target": The target to trigger if the "Outskirts" option is enabled and clicked on.
   - "Centre Target": The target to trigger if the "Centre" option is enabled and clicked on.
   - "Line": The subway line this is used for. Currently, only "Bergen-Eckhart" is supported.

 - Spawn flags:
   - "Disable Options": All the options will be set to the "Disabled by Administrator" state when the
   player spawns the Game UI window.
   
# trigger_subway_flagger
>This entity is used to set one of the subway controller destinations as "Enabled". Currently only supports
>the "Bergen-Eckhart" subway line.

 - Keyvalues:
   - "Name": The name of this entity.
   - "Enable Location": The subway destination to enable.
   
# trigger_teleport
>This entity will teleport any entities that are touching it. This works mainly with players, and NPCs will
>only be teleported if they are moving as well. To reliably teleport NPCs, please look at "trigger_move".
>The teleport can directly teleport the entity touching it to a specific spot, or can be set as a relative
>teleport, and move all entities touching it relative to the "Landmarker" and destination marker entities.

 - Keyvalues:
   - "Target": The entity that marks the destination.
   - "Name": Name of this entity.
   - "Filter by name": If set, only an entity, like an NPC or pushable that has this targetname will be
   able to activate this trigger entity.
   - "Kill target": Entity to kill/remove when activated.
   - "Master": Multisource controlling whether this entity is enabled or disabled.
   - "Delay before trigger": Delay before the target entity is triggered.
   - "Message": Message to display when activated.
   - "Teleport Sound": sound to make when teleporting an object.
   - "Landmarker": If set, the landmark at the source of the teleportation will be used to calculate the
   origin offsets applied at the destination marker.
   
 - Spawn flags:
   - "NPCs": NPCs can touch this trigger to activate it.
   - "No Clients": Players cannot touch this trigger.
   - "Pushables": Pushables can activate this trigger.
   - "Relative": The teleport will do a relative teleportation of the entities touching it, depending on
   the "Landmarker" entity set at the source.
   
# trigger_textwindow
>Triggering this point entity will make it display a text file viewer Game UI window, located under 
>"\*modfolder\*/text". If a "Code ID" is specified, then the code set in/generated by this entity will be
>saved to the globally shared set of codes.

>[!NOTE]
>The expected format of the text files is the following:
```
$title "Title of the Window"<br />
{<br />
Body of Text<br />
Code: %passcode%<br />
}<br />
```
Where the token %passcode% will be replaced with the code geenrated.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Text file to load": The path to the text file to load for the window.
   - "Code ID": The Id the code read is saved with.
   - "Code": The code to save for the code id. If "Generate code" is set, then the generated code will be
   appended to this code.
   
 - Spawn flags:
   - "Generate code": The text window will generate a value between 2 and 6 digits, then append it to the
   value in "Code" if any.
   
# trigger_toggletarget
>This entity can be used to set the visibility and solidity of a brush entity. Can be used to make doors,
>walls, etc disappear or re-appear.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": The entity/entities that this will change the visibility/solidity of.
   - "Trigger Mode": Defines whether solidity and visibility are disabled or enabled.
   
# trigger_transition
>Used to mark the bounding box of a level transition, and needs to be given the same name as the landmark
>entity for the changelevel. Any objects not within the bounding box of this brush entity will be excluded
>from the level transition.

 - Keyvalues:
   - "Name": Name of this entity, needs to have the same name as the info_landmark.
   
# trigger_vacuum
>This entity, when triggered on, will begin pulling the player towards itself with increasing force. It
>uses the AI navigation graph to determine a path for pulling on the player, so make sure to place your
>info_nodes properly around the area where you want to use this entity.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Pull velocity": The force with which the player will be pulled at maximum force.
   - "Full force time": The time it takes for the vacuum to reach full force.
   - "Fade out time": How long the pull will fade out over once triggered off.
   
# trigger_zoom
>This entity will modify the player's field of view value, blending it between two values depending in the
>settings and the initial state of the entity.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Start FOV": The initial FOV value, set automatically if "Start Zoomed" is set.
   - "End FOV": The final FOV value to set after the entity is done zooming in. 
   - "Duration": The time it takes for the FOV to reach the "End FOV" value.
   
 - Spawn flags:
  - "Start Zoomed": The FOV will start on spawn, and set the player's view to be the "Start FOV" value.
  
# weapon_glock
>A Glock 17 pistol pickup for the player.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": And entity to trigger when the player walks over/picks up the item.
   - "KillTarget": An entity that will be killed when this entity is picked up/walked over.
   - "Delay before trigger": How long until the target/killtarget is triggered.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, doesn't work on this type of entity.
   - "Visibility Distance": The distance at which the item will remain visible.
   
 - Spawn flags:
  - "Not in deathmatch": This entity will not be present in multiplayer.
  - "No Pickup Notifications": There will be no sounds, HUD notifications played when this item is picked
  up by the player. It will just be silently add to their inventory.
  - "Trigger Only on Pickup": The entity will only trigger it's target when the player actually picks up
  the item, otherwise the target is not triggered if the player walks over the item.

# weapon_handgrenade
>A hand grenade pickup for the player.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": And entity to trigger when the player walks over/picks up the item.
   - "KillTarget": An entity that will be killed when this entity is picked up/walked over.
   - "Delay before trigger": How long until the target/killtarget is triggered.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, doesn't work on this type of entity.
   - "Visibility Distance": The distance at which the item will remain visible.
   - "Silenced": If set, the Glock will spawn with a silencer on.
   - "Flashlight": If set, the Glock will spawn with a flashlight underbarrel attachment.
   
 - Spawn flags:
  - "Not in deathmatch": This entity will not be present in multiplayer.
  - "No Pickup Notifications": There will be no sounds, HUD notifications played when this item is picked
  up by the player. It will just be silently add to their inventory.
  - "Trigger Only on Pickup": The entity will only trigger it's target when the player actually picks up
  the item, otherwise the target is not triggered if the player walks over the item.
  
# weapon_knife
>A knife pickup for the player.

 - Keyvalues:
   - "Name": Name of this entity.
   - "Target": And entity to trigger when the player walks over/picks up the item.
   - "KillTarget": An entity that will be killed when this entity is picked up/walked over.
   - "Delay before trigger": How long until the target/killtarget is triggered.
   - "Render Mode": See "Shared keyvalues" for more information.
   - "Render FX": See "Shared keyvalues" for more information.
   - "FX Amount (1 - 255)": Degree of transparency for blended rendermodes.
   - "FX Color (R G B)": The color to be applied, doesn't work on this type of entity.
   - "Visibility Distance": The distance at which the item will remain visible.
   
 - Spawn flags:
  - "Not in deathmatch": This entity will not be present in multiplayer.
  - "No Pickup Notifications": There will be no sounds, HUD notifications played when this item is picked
  up by the player. It will just be silently add to their inventory.
  - "Trigger Only on Pickup": The entity will only trigger it's target when the player actually picks up
  the item, otherwise the target is not triggered if the player walks over the item.