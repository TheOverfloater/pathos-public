/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.

===============================================
*/

//
// mdlexport.c: exports the textures of a .mdl file and creates .pmf entries
// models/<scriptname>.mdl.
//

#pragma warning( disable : 4244 )
#pragma warning( disable : 4237 )
#pragma warning( disable : 4305 )

#include <SDL.h>
#ifdef WIN32
#include <Windows.h>
#endif

#include "includes.h"
#include "common.h"
#include "main.h"
#include "constants.h"

//===============================================
// 
//
//===============================================
bool DirectoryExists( const Char* dirPath )
{
	DWORD ftyp = GetFileAttributesA(dirPath);
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;   // this is a directory!

	return false;    // this is not a directory!
}

//===============================================
// 
//
//===============================================
bool FileExists( const Char* filepath )
{
	DWORD ftyp = GetFileAttributesA(filepath);
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;

	return true;
}

//===============================================
// 
//
//===============================================
bool CreateDirectory( const Char* dirPath )
{
	DWORD type = GetFileAttributesA(dirPath);
	if(type == INVALID_FILE_ATTRIBUTES)
		return CreateDirectoryA(dirPath, NULL) ? true : false;

	// Check if it's a directory
	if(type & FILE_ATTRIBUTE_DIRECTORY)
		return true;

	return false;
}

//===============================================
// 
//
//===============================================
bool ProcessClusterScript( const Char* pstrScriptFile, const Char* pData, const Char* pstrOutputPath )
{
	Char szLine[4096];

	CString f;
	f << "$clusterscript" << NEWLINE;
	f << "{" << NEWLINE;

	const Char* pstr = pData;
	while(pstr)
	{
		pstr = Common::ReadLine(pstr, szLine);
		if(!qstrlen(szLine))
			continue;

		f << "\t$script " << szLine << NEWLINE;
	}
	f << "}" << NEWLINE;

	CString basename;
	Common::Basename(pstrScriptFile, basename);

	CString finalPath;
	finalPath << pstrOutputPath << PATH_SLASH_CHAR << basename << ".txt";

	FILE* pf = fopen(finalPath.c_str(), "wb");
	if(!pf)
	{
		printf("Failed to open '%s' for writing.\n", finalPath.c_str());
		return false;
	}

	Int32 num = fwrite(f.c_str(), sizeof(Char), f.length(), pf);
	fclose(pf);

	if(num != f.length())
	{
		printf("Failed to write %d bytes for '%s', wrote %d instead.\n", f.length(), finalPath.c_str(), num);
		return false;
	}

	return true;
}

//===============================================
// 
//
//===============================================
bool ProcessParticleScript( const Char* pstrScriptFile, const Char* pData, const Char* pstrOutputPath )
{
	static Char field[MAX_PARSE_LENGTH];
	static Char value[MAX_PARSE_LENGTH];

	system_definition_t def;
	def.mainalpha = 1.0;

	const Char *pToken = pData;
	while(1)
	{
		if(!pToken)
			break;

		pToken = Common::Parse(pToken, field);

		if(!pToken)
			break;

		pToken = Common::Parse(pToken, value);

		if(!qstrcmp(field, "systemshape"))				def.shapetype = SDL_atoi(value);
		else if(!qstrcmp(field, "minvel"))				def.minvel = SDL_atof(value);
		else if(!qstrcmp(field, "maxvel"))				def.maxvel = SDL_atof(value);
		else if(!qstrcmp(field, "maxofs"))				def.maxofs = SDL_atof(value);
		else if(!qstrcmp(field, "fadein"))				def.fadeintime = SDL_atof(value);
		else if(!qstrcmp(field, "fadedelay"))			def.fadeoutdelay = SDL_atof(value);
		else if(!qstrcmp(field, "mainalpha"))			def.mainalpha = SDL_atof(value);
		else if(!qstrcmp(field, "veldamp"))				def.velocitydamp = SDL_atof(value);
		else if(!qstrcmp(field, "veldampdelay"))		def.veldampdelay = SDL_atof(value);
		else if(!qstrcmp(field, "life"))				def.maxlife = SDL_atof(value);
		else if(!qstrcmp(field, "lifevar"))				def.maxlifevar = SDL_atof(value);
		else if(!qstrcmp(field, "pcolr"))				def.primarycolor.x = SDL_atoi(value);
		else if(!qstrcmp(field, "pcolg"))				def.primarycolor.y = SDL_atoi(value);
		else if(!qstrcmp(field, "pcolb"))				def.primarycolor.z = SDL_atoi(value);
		else if(!qstrcmp(field, "scolr"))				def.secondarycolor.x = SDL_atoi(value);
		else if(!qstrcmp(field, "scolg"))				def.secondarycolor.y = SDL_atoi(value);
		else if(!qstrcmp(field, "scolb"))				def.secondarycolor.z = SDL_atoi(value);
		else if(!qstrcmp(field, "ctransd"))				def.transitiondelay = SDL_atof(value);
		else if(!qstrcmp(field, "ctranst"))				def.transitiontime = SDL_atof(value);
		else if(!qstrcmp(field, "ctransv"))				def.transitionvar = SDL_atof(value);
		else if(!qstrcmp(field, "scale"))				def.scale = SDL_atof(value);
		else if(!qstrcmp(field, "scalevar"))			def.scalevar = SDL_atof(value);
		else if(!qstrcmp(field, "scaledampdelay"))		def.scaledampdelay = SDL_atof(value);
		else if(!qstrcmp(field, "scaledampfactor"))		def.scaledampfactor = SDL_atof(value);
		else if(!qstrcmp(field, "gravity"))				def.gravity = SDL_atof(value);
		else if(!qstrcmp(field, "systemsize"))			def.systemsize = SDL_atoi(value);
		else if(!qstrcmp(field, "maxparticles"))		def.maxparticles = SDL_atoi(value);
		else if(!qstrcmp(field, "intensity"))			def.particlefreq = SDL_atof(value);
		else if(!qstrcmp(field, "startparticles"))		def.startparticles = SDL_atoi(value);
		else if(!qstrcmp(field, "maxparticlevar"))		def.maxparticlevar = SDL_atoi(value);
		else if(!qstrcmp(field, "lightmaps"))			def.lightcheck = SDL_atoi(value);
		else if(!qstrcmp(field, "collision"))			def.collision = SDL_atoi(value);
		else if(!qstrcmp(field, "colwater"))			def.colwater = SDL_atoi(value);
		else if(!qstrcmp(field, "rendermode"))			def.rendermode = SDL_atoi(value);
		else if(!qstrcmp(field, "display"))				def.displaytype = SDL_atoi(value);
		else if(!qstrcmp(field, "impactdamp"))			def.impactdamp = SDL_atof(value);
		else if(!qstrcmp(field, "rotationvar"))			def.rotationvar = SDL_atof(value);
		else if(!qstrcmp(field, "rotationvel"))			def.rotationvel = SDL_atof(value);
		else if(!qstrcmp(field, "rotationdamp"))		def.rotationdamp = SDL_atof(value);
		else if(!qstrcmp(field, "rotationdampdelay"))	def.rotationdampdelay = SDL_atof(value);
		else if(!qstrcmp(field, "rotxvar"))				def.rotxvar = SDL_atof(value);
		else if(!qstrcmp(field, "rotxvel"))				def.rotxvel = SDL_atof(value);
		else if(!qstrcmp(field, "rotxdamp"))			def.rotxdamp = SDL_atof(value);
		else if(!qstrcmp(field, "rotxdampdelay"))		def.rotxdampdelay = SDL_atof(value);
		else if(!qstrcmp(field, "rotyvar"))				def.rotyvar = SDL_atof(value);
		else if(!qstrcmp(field, "rotyvel"))				def.rotyvel = SDL_atof(value);
		else if(!qstrcmp(field, "rotydamp"))			def.rotydamp = SDL_atof(value);
		else if(!qstrcmp(field, "rotydampdelay"))		def.rotydampdelay = SDL_atof(value);
		else if(!qstrcmp(field, "randomdir"))			def.randomdir = SDL_atoi(value);
		else if(!qstrcmp(field, "overbright"))			def.overbright = SDL_atoi(value);
		else if(!qstrcmp(field, "create"))				def.create = value;
		else if(!qstrcmp(field, "deathcreate"))			def.deathcreate = value;
		else if(!qstrcmp(field, "watercreate"))			def.watercreate = value;
		else if(!qstrcmp(field, "windx"))				def.windx = SDL_atof(value);
		else if(!qstrcmp(field, "windy"))				def.windy = SDL_atof(value);
		else if(!qstrcmp(field, "windvar"))				def.windvar = SDL_atof(value);
		else if(!qstrcmp(field, "windtype"))			def.windtype = SDL_atoi(value);
		else if(!qstrcmp(field, "windmult"))			def.windmult = SDL_atof(value);
		else if(!qstrcmp(field, "minwindmult"))			def.minwindmult = SDL_atof(value);
		else if(!qstrcmp(field, "windmultvar"))			def.windmultvar = SDL_atof(value);
		else if(!qstrcmp(field, "stuckdie"))			def.stuckdie = SDL_atof(value);
		else if(!qstrcmp(field, "maxheight"))			def.maxheight = SDL_atof(value);
		else if(!qstrcmp(field, "tracerdist"))			def.tracerdist = SDL_atof(value);
		else if(!qstrcmp(field, "fadedistnear"))		def.fadedistnear = SDL_atoi(value);
		else if(!qstrcmp(field, "fadedistfar"))			def.fadedistfar = SDL_atoi(value);
		else if(!qstrcmp(field, "numframes"))			def.numframes = SDL_atoi(value);
		else if(!qstrcmp(field, "framesizex"))			def.framesizex = SDL_atoi(value);
		else if(!qstrcmp(field, "framesizey"))			def.framesizey = SDL_atoi(value);
		else if(!qstrcmp(field, "framerate"))			def.framerate = SDL_atoi(value);
		else if(!qstrcmp(field, "skybox"))				def.skybox = SDL_atoi(value);
		else if(!qstrcmp(field, "attach"))				def.attachflags = SDL_atoi(value);
		else if(!qstrcmp(field, "nocull"))				def.nocull = SDL_atoi(value);
		else if(!qstrcmp(field, "spawnchance"))			def.spawnchance = SDL_atoi(value);
		else if(!qstrcmp(field, "minlight"))			def.minlight = SDL_atof(value);
		else if(!qstrcmp(field, "maxlight"))			def.maxlight = SDL_atof(value);
		else if(!qstrcmp(field, "collide_exp"))			def.collide_exp = SDL_atoi(value);
		else if(!qstrcmp(field, "collide_bmodels"))		def.collide_bmodels = SDL_atoi(value);
		else if(!qstrcmp(field, "globs"))				def.globs = SDL_atoi(value);
		else if(!qstrcmp(field, "globsize"))			def.globsize = SDL_atoi(value);
		else if(!qstrcmp(field, "numglobparticles"))	def.numglobparticles = SDL_atoi(value);
		else if(!qstrcmp(field, "softoff"))				def.softoff = SDL_atoi(value);
		else if(!qstrcmp(field, "softofftime"))			def.softofftime = SDL_atof(value);
		else if(!qstrcmp(field, "nofog"))				def.nofog = (SDL_atoi(value) == 1) ? true : false;
		else if(!qstrcmp(field, "texture"))				def.texturepath = value;
		else
		{
			printf("Warning! Unknown field: %s in %s\n", field, pstrScriptFile);
		}
	}
	


	CString f;
	f << "$particlescript" << NEWLINE;
	f << "{" << NEWLINE;
	
	f << "\t$shape ";
	switch(def.shapetype)
	{
	case shape_point:
		f << "point";
		break;
	case shape_box:
		f << "box";
		break;
	case shape_playerplane:
		f << "playerplane";
		break;
	}
	f << NEWLINE;

	if(def.systemsize != 0)
		f << "\t$system_size " << def.systemsize << NEWLINE;

	if(def.maxheight != 0)
		f << "\t$playerplane_max_height " << def.maxheight << NEWLINE;

	if(def.minvel != 0)
		f << "\t$min_velocity " << def.minvel << NEWLINE;
	if(def.maxvel != 0)
		f << "\t$max_velocity " << def.maxvel << NEWLINE;
	if(def.maxofs != 0)
		f << "\t$max_offset " << def.maxofs << NEWLINE;
	if(def.fadeintime != 0)
		f << "\t$fade_in_time " << def.fadeintime << NEWLINE;
	if(def.fadeoutdelay != 0)
		f << "\t$fade_out_delay " << def.fadeoutdelay << NEWLINE;
	if(def.velocitydamp != 0)
		f << "\t$velocity_damping " << def.velocitydamp << NEWLINE;
	if(def.veldampdelay != 0)
		f << "\t$velocity_damping_delay " << def.veldampdelay << NEWLINE;
	if(def.maxlife != 0)
		f << "\t$lifetime " << def.maxlife << NEWLINE;
	if(def.maxlifevar != 0)
		f << "\t$lifetime_variation " << def.maxlifevar << NEWLINE;

	if(def.randomdir || def.globs || def.softoff)
	{
		bool first = true;
		f << "\t$flags ";
		
		if(def.randomdir)
		{
			if(first)
				first = false;
			else
				f << " ";

			f << "random_direction";
		}

		if(def.globs)
		{
			if(first)
				first = false;
			else
				f << " ";

			f << "globs";
		}

		if(def.softoff)
		{
			if(first)
				first = false;
			else
				f << " ";

			f << "softoff";
		}

		f << NEWLINE;
	}


	if(!def.primarycolor.IsZero())
		f << "\t$primary_color " << (Int32)(def.primarycolor.x) << " " << (Int32)(def.primarycolor.y) << " " << (Int32)(def.primarycolor.z) << NEWLINE;
	if(!def.secondarycolor.IsZero())
		f << "\t$secondary_color " << (Int32)(def.secondarycolor.x) << " " << (Int32)(def.secondarycolor.y) << " " << (Int32)(def.secondarycolor.z) << NEWLINE;

	if(def.transitiondelay != 0)
		f << "\t$color_transition_delay " << def.transitiondelay << NEWLINE;
	if(def.transitiontime != 0)
		f << "\t$color_transition_duration " << def.transitiontime << NEWLINE;
	if(def.transitionvar != 0)
		f << "\t$color_transition_variance " << def.transitionvar << NEWLINE;

	if(def.scale != 0)
		f << "\t$scale " << def.scale << NEWLINE;
	if(def.scalevar != 0)
		f << "\t$scale_variation " << def.scalevar << NEWLINE;
	if(def.scaledampdelay != 0)
		f << "\t$scale_damping_delay " << def.scaledampdelay << NEWLINE;
	if(def.scaledampfactor != 0)
		f << "\t$scale_damping " << def.scaledampfactor << NEWLINE;

	if(def.maxparticles != 0)
		f << "\t$max_particles " << def.maxparticles << NEWLINE;
	if(def.particlefreq != 0)
		f << "\t$particle_frequency " << def.particlefreq << NEWLINE;
	if(def.maxparticlevar != 0)
		f << "\t$particle_frequency_variation " << def.maxparticlevar << NEWLINE;
	if(def.startparticles != 0)
		f << "\t$particles_on_spawn " << def.startparticles << NEWLINE;

	if(def.softofftime != 0)
		f << "\t$soft_turnoff_duration " << def.softofftime << NEWLINE;

	if(def.lightcheck != 0)
	{
		bool first = true;
		f << "\t$lighting ";

		if(def.lightcheck & PARTICLE_LIGHTCHECK_NORMAL)
		{
			first = false;
			f << "normal";
		}

		if(def.lightcheck & PARTICLE_LIGHTCHECK_MIXP)
		{
			if(!first)
				f << " ";
			else
				first = false;

			f << "mix_primary_color";
		}

		if(def.lightcheck & PARTICLE_LIGHTCHECK_INTENSITY)
		{
			if(!first)
				f << " ";
			else
				first = false;

			f << "use_intensity";
		}

		if(def.lightcheck & PARTICLE_LIGHTCHECK_ONLYONCE)
		{
			if(!first)
				f << " ";
			else
				first = false;

			f << "only_once";
		}

		f << NEWLINE;
	}

	if(def.collision != 0)
	{
		bool first = true;
		f << "\t$collision ";
		
		switch(def.collision)
		{
		case collide_none:
			f << "none";
			break;
		case collide_die:
			f << "die";
			break;
		case collide_bounce:
			f << "bounce";
			break;
		case collide_decal:
			f << "decal";
			break;
		case collide_stuck:
			f << "stuck";
			break;
		case collide_new_system:
			f << "create_system";
			break;
		}

		f << NEWLINE;
	}

	if(def.collide_exp != 0 || def.collide_bmodels != 0 || def.colwater != 0)
	{
		bool first = true;
		f << "\t$collision_flags ";

		if(def.collide_exp != 0)
		{
			if(first)
				first = false;
			else
				f << " ";

			f << "precise";
		}

		if(def.collide_bmodels != 0)
		{
			if(first)
				first = false;
			else
				f << " ";

			f << "collide_brushmodels";
		}

		if(def.colwater != 0)
		{
			if(first)
				first = false;
			else
				f << " ";

			f << "collide_water";
		}

		f << NEWLINE;
	}

	if(def.gravity != 0)
		f << "\t$gravity " << def.gravity << NEWLINE;

	if(def.impactdamp != 0)
		f << "\t$impact_velocity_dampening " << def.impactdamp << NEWLINE;

	if(!def.create.empty())
		f << "\t$create " << def.create << NEWLINE;

	if(!def.deathcreate.empty())
		f << "\t$create_on_death " << def.deathcreate << NEWLINE;

	if(!def.watercreate.empty())
		f << "\t$create_on_water_impact " << def.watercreate << NEWLINE;

	if(def.spawnchance)
		f << "\t$chance_to_create " << def.spawnchance << NEWLINE;

	if(def.globsize)
		f << "\t$glob_size " << def.globsize << NEWLINE;

	if(def.numglobparticles)
		f << "\t$num_glob_particles " << def.numglobparticles << NEWLINE;	

	f << "\t$rendermode ";
	switch(def.rendermode)
	{
	case render_alpha:
	case render_additive:
		f << "additive";
		break;
	case render_texture:
		f << "alphablend";
		break;
	case render_distort:
		f << "refractive";
		break;
	}
	f << NEWLINE;

	f << "\t$alpha " << def.mainalpha << NEWLINE;

	f << "\t$alignment ";
	switch(def.displaytype)
	{
	case align_tiled:
		f << "tiled";
		break;
	case align_parallel:
		f << "parallel";
		break;
	case align_normal:
		f << "to_normal";
		break;
	case align_tracer:
		f << "tracer";
		break;
	}
	f << NEWLINE;

	if(!def.texturepath.empty())
		f << "\t$texture " << def.texturepath << NEWLINE;

	if(def.overbright != 0 || def.skybox != 0 || def.nocull != 0 || def.nofog != 0)
	{
		bool first = true;
		f << "\t$render_flags ";

		if(def.overbright != 0)
		{
			if(first)
				first = false;
			else
				f << " ";

			f << "overbright";
		}

		if(def.skybox != 0)
		{
			if(first)
				first = false;
			else
				f << " ";

			f << "skybox";
		}

		if(def.nocull != 0)
		{
			if(first)
				first = false;
			else
				f << " ";

			f << "nocull";
		}

		if(def.nofog != 0)
		{
			if(first)
				first = false;
			else
				f << " ";

			f << "nofog";
		}

		f << NEWLINE;
	}

	if(def.minlight != 0)
		f << "\t$min_light_value " << def.minlight << NEWLINE;
	if(def.maxlight != 0)
		f << "\t$max_light_value " << def.maxlight << NEWLINE;

	if(def.rotationvel != 0)
		f << "\t$rotate_z_speed " << def.rotationvel << NEWLINE;
	if(def.rotationvar != 0)
		f << "\t$rotate_z_variation " << def.rotationvar << NEWLINE;
	if(def.rotationdamp != 0)
		f << "\t$rotate_z_dampening " << def.rotationdamp << NEWLINE;
	if(def.rotationdampdelay != 0)
		f << "\t$rotate_z_dampening_delay " << def.rotationdampdelay << NEWLINE;

	if(def.rotxvel != 0)
		f << "\t$rotate_x_speed " << def.rotxvel << NEWLINE;
	if(def.rotxvar != 0)
		f << "\t$rotate_x_variation " << def.rotxvar << NEWLINE;
	if(def.rotxdamp != 0)
		f << "\t$rotate_x_dampening " << def.rotxdamp << NEWLINE;
	if(def.rotxdampdelay != 0)
		f << "\t$rotate_x_dampening_delay " << def.rotxdampdelay << NEWLINE;

	if(def.rotyvel != 0)
		f << "\t$rotate_y_speed " << def.rotyvel << NEWLINE;
	if(def.rotyvar != 0)
		f << "\t$rotate_y_variation " << def.rotyvar << NEWLINE;
	if(def.rotydamp != 0)
		f << "\t$rotate_y_dampening " << def.rotydamp << NEWLINE;
	if(def.rotydampdelay != 0)
		f << "\t$rotate_y_dampening_delay " << def.rotydampdelay << NEWLINE;

	if(def.windx != 0)
		f << "\t$wind_x_velocity " << def.windx << NEWLINE;
	if(def.windy != 0)
		f << "\t$wind_y_velocity " << def.windy << NEWLINE;
	if(def.windvar != 0)
		f << "\t$wind_velocity_variance " << def.windvar << NEWLINE;

	if(def.windtype != 0)
	{
		bool first = false;
		f << "\t$wind_type ";
		switch(def.windtype)
		{
		case wind_linear:
			f << "linear";
			break;
		case wind_sine:
			f << "sine";
			break;
		}
		f << NEWLINE;
	}

	if(def.windmult != 0)
		f << "\t$wind_sine_variance_speed_multiplier " << def.windmult << NEWLINE;
	if(def.minwindmult != 0)
		f << "\t$wind_sine_min_variance " << def.minwindmult << NEWLINE;
	if(def.windmultvar != 0)
		f << "\t$wind_sine_variance " << def.windmultvar << NEWLINE;

	if(def.stuckdie != 0)
		f << "\t$stuck_death_time " << def.stuckdie << NEWLINE;

	if(def.tracerdist != 0)
		f << "\t$tracer_distance " << def.tracerdist << NEWLINE;

	if(def.fadedistnear != 0)
		f << "\t$fade_near_distance " << def.fadedistnear << NEWLINE;
	if(def.fadedistfar != 0)
		f << "\t$fade_far_distance " << def.fadedistfar << NEWLINE;

	if(def.framesizex != 0)
		f << "\t$frame_width " << def.framesizex << NEWLINE;
	if(def.framesizey != 0)
		f << "\t$frame_height " << def.framesizey << NEWLINE;
	if(def.numframes != 0)
		f << "\t$num_frames " << def.numframes << NEWLINE;
	if(def.framerate != 0)
		f << "\t$framerate " << def.framerate << NEWLINE;

	if(def.attachflags != 0)
	{
		bool first = true;
		f << "\t$attachment_flags ";

		if(def.attachflags & PARTICLE_ATTACH_TO_PARENT)
		{
			if(first)
				first = false;
			else
				f << " ";

			f << "to_parent";
		}

		if(def.attachflags & PARTICLE_ATTACH_RELATIVE)
		{
			if(first)
				first = false;
			else
				f << " ";

			f << "relative";
		}

		if(def.attachflags & PARTICLE_ATTACH_TO_ATTACHMENT)
		{
			if(first)
				first = false;
			else
				f << " ";

			f << "to_attachment";
		}

		if(def.attachflags & PARTICLE_ATTACH_ATTACHMENT_VECTOR)
		{
			if(first)
				first = false;
			else
				f << " ";

			f << "attachment_vector";
		}

		if(def.attachflags & PARTICLE_ATTACH_TO_BONE)
		{
			if(first)
				first = false;
			else
				f << " ";

			f << "to_bone";
		}

		f << NEWLINE;
	}

	f << "}" << NEWLINE;

	CString basename;
	Common::Basename(pstrScriptFile, basename);

	CString finalPath;
	finalPath << pstrOutputPath << PATH_SLASH_CHAR << basename << ".txt";

	FILE* pf = fopen(finalPath.c_str(), "wb");
	if(!pf)
	{
		printf("Failed to open '%s' for writing.\n", finalPath.c_str());
		return false;
	}

	Int32 num = fwrite(f.c_str(), sizeof(Char), f.length(), pf);
	fclose(pf);

	if(num != f.length())
	{
		printf("Failed to write %d bytes for '%s', wrote %d instead.\n", f.length(), finalPath.c_str(), num);
		return false;
	}

	return true;
}

//===============================================
// 
//
//===============================================
bool ProcessScript( const Char* pstrPath, const Char* pstrOutputPath )
{
	FILE* pf = fopen(pstrPath, "rb");
	if(!pf)
	{
		printf("Failed to open '%s'.\n", pstrPath);
		return false;
	}

	fseek(pf, 0, SEEK_END);
	Int32 length = ftell(pf);
	fseek(pf, 0, SEEK_SET);

	Char* pBuffer = new Char[length+1];
	fread(pBuffer, sizeof(Char), length, pf);
	fclose(pf);

	pBuffer[length] = '\0';

	// Read first line to determine type
	Char szLine[4096];
	Common::ReadLine(pBuffer, szLine);

	bool result;
	if(qstrstr(szLine, ".txt") == nullptr)
		result = ProcessParticleScript(pstrPath, pBuffer, pstrOutputPath);
	else
		result = ProcessClusterScript(pstrPath, pBuffer, pstrOutputPath);

	delete[] pBuffer;
	return result;
}

//===============================================
// _tmain
//
//===============================================
int _tmain(Int32 argc, Char* argv[])
{
	// Check for usage
	if(argc != 3)
	{
		printf("Usage: <target file/directory>");
		printf("Press any key to exit...\n");
		getchar();
		return -1;
	}

	// Create dir if missing
	if(!DirectoryExists(argv[2]))
		CreateDirectory(argv[2]);

	Uint32 numExported = 0;
	if(qstrstr(argv[1], ".txt"))
	{
		CString filepath(argv[1]);

		// Convert script to new format
		if(ProcessScript(filepath.c_str(), argv[2]))
			numExported++;
	}
	else
	{
		CString searchpath;
		searchpath << argv[1] << PATH_SLASH_CHAR << "*.txt";

		// Parse directory for files
		HANDLE dir;
		WIN32_FIND_DATA file_data;
		if ((dir = FindFirstFile(searchpath.c_str(), &file_data)) == INVALID_HANDLE_VALUE)
		{
			printf("Directory %s not found.\n", argv[1]);
			return -1;
		}

		while (true) 
		{
			CString filepath;
			filepath << argv[1] << PATH_SLASH_CHAR << file_data.cFileName;

			CString texturefilepath(filepath);
			if (qstrcmp(file_data.cFileName, ".") != 0 && qstrcmp(file_data.cFileName, "..") != 0 && qstrstr(file_data.cFileName, ".txt"))
			{
				// Convert script to new format
				if(ProcessScript(filepath.c_str(), argv[2]))
					numExported++;
			}

			if(!FindNextFile(dir, &file_data))
				break;
		}
	}

	if(numExported == 0)
	{
		printf("Error: Particle scripts file(s) not found.\n");
		printf("Press any key to exit...\n");
		getchar();
		return 1;
	}
	else
	{
		printf("%d files converted.\n", numExported);
		printf("Press any key to exit...\n");
		getchar();
		return 0;
	}

	return 0;
}
