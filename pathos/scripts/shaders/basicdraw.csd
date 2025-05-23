CSD1   fe37d48d689b8c5acb4fbf12e1d3510d    �#     @      �$  �   E    a  f  E  �  a    d  p	  O  �  d  #  O  r  a  �  �  �  a  �  �  �  �    �  �  �  4   �  #version 130

uniform mat4 projection;
uniform mat4 modelview;

in vec4 in_position;
in vec2 in_texcoord;
in vec4 in_color;

out vec4 ps_color;
out vec2 ps_texcoord;
out vec3 ps_vertexpos;

void main()
{
ps_color = in_color;
	vec4 position = in_position*modelview;
	
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable
uniform vec2 fogparams;
uniform vec3 fogcolor;
uniform float multiplier;

in vec4 ps_color;
in vec2 ps_texcoord;
in vec3 ps_vertexpos;

out vec4 oColor;

float SplineFraction( float value, float scale )
{
	float valueSquared;

	value = scale * value;
	valueSquared = value * value;

	// Nice little ease-in, ease-out spline-like curve
	return 3 * valueSquared - 2 * valueSquared * value;
}

void main()
{
	vec4 finalColor = vec4(1.0, 1.0, 1.0, 1.0);
finalColor = finalColor*ps_color*multiplier;
	
	oColor = finalColor;
}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

in vec4 in_position;
in vec2 in_texcoord;
in vec4 in_color;

out vec4 ps_color;
out vec2 ps_texcoord;
out vec3 ps_vertexpos;

void main()
{
ps_color = in_color;
	vec4 position = in_position*modelview;
	
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable
uniform vec2 fogparams;
uniform vec3 fogcolor;
uniform float multiplier;

in vec4 ps_color;
in vec2 ps_texcoord;
in vec3 ps_vertexpos;

out vec4 oColor;

float SplineFraction( float value, float scale )
{
	float valueSquared;

	value = scale * value;
	valueSquared = value * value;

	// Nice little ease-in, ease-out spline-like curve
	return 3 * valueSquared - 2 * valueSquared * value;
}

void main()
{
	vec4 finalColor = vec4(1.0, 1.0, 1.0, 1.0);
finalColor = finalColor*ps_color*multiplier;
	
	oColor = finalColor;
}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

in vec4 in_position;
in vec2 in_texcoord;
in vec4 in_color;

out vec4 ps_color;
out vec2 ps_texcoord;
out vec3 ps_vertexpos;

void main()
{
ps_color = in_color;
	vec4 position = in_position*modelview;
	
	ps_vertexpos = position.xyz;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable
uniform vec2 fogparams;
uniform vec3 fogcolor;
uniform float multiplier;

in vec4 ps_color;
in vec2 ps_texcoord;
in vec3 ps_vertexpos;

out vec4 oColor;

float SplineFraction( float value, float scale )
{
	float valueSquared;

	value = scale * value;
	valueSquared = value * value;

	// Nice little ease-in, ease-out spline-like curve
	return 3 * valueSquared - 2 * valueSquared * value;
}

void main()
{
	vec4 finalColor = vec4(1.0, 1.0, 1.0, 1.0);
finalColor = finalColor*ps_color*multiplier;
	
	float fogcoord = length(ps_vertexpos);
		
		float fogfactor = (fogparams.x - fogcoord)*fogparams.y;
		fogfactor = 1.0-SplineFraction(clamp(fogfactor, 0.0, 1.0), 1.0);
		
		finalColor.xyz = mix(finalColor.xyz, fogcolor, fogfactor);
	oColor = finalColor;
}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

in vec4 in_position;
in vec2 in_texcoord;
in vec4 in_color;

out vec4 ps_color;
out vec2 ps_texcoord;
out vec3 ps_vertexpos;

void main()
{
ps_color = in_color;
	vec4 position = in_position*modelview;
	
	ps_vertexpos = position.xyz;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable
uniform vec2 fogparams;
uniform vec3 fogcolor;
uniform float multiplier;

in vec4 ps_color;
in vec2 ps_texcoord;
in vec3 ps_vertexpos;

out vec4 oColor;

float SplineFraction( float value, float scale )
{
	float valueSquared;

	value = scale * value;
	valueSquared = value * value;

	// Nice little ease-in, ease-out spline-like curve
	return 3 * valueSquared - 2 * valueSquared * value;
}

void main()
{
	vec4 finalColor = vec4(1.0, 1.0, 1.0, 1.0);
finalColor = finalColor*ps_color*multiplier;
	
	float fogcoord = length(ps_vertexpos);
		
		float fogfactor = (fogparams.x - fogcoord)*fogparams.y;
		fogfactor = 1.0-SplineFraction(clamp(fogfactor, 0.0, 1.0), 1.0);
		
		finalColor.xyz = mix(finalColor.xyz, fogcolor, fogfactor);
	oColor = finalColor;
}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

in vec4 in_position;
in vec2 in_texcoord;
in vec4 in_color;

out vec4 ps_color;
out vec2 ps_texcoord;
out vec3 ps_vertexpos;

void main()
{
ps_texcoord = in_texcoord;
ps_color = in_color;
	vec4 position = in_position*modelview;
	
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable
uniform sampler2D texture0;
	uniform vec2 fogparams;
uniform vec3 fogcolor;
uniform float multiplier;

in vec4 ps_color;
in vec2 ps_texcoord;
in vec3 ps_vertexpos;

out vec4 oColor;

float SplineFraction( float value, float scale )
{
	float valueSquared;

	value = scale * value;
	valueSquared = value * value;

	// Nice little ease-in, ease-out spline-like curve
	return 3 * valueSquared - 2 * valueSquared * value;
}

void main()
{
	vec4 finalColor = vec4(1.0, 1.0, 1.0, 1.0);
finalColor = texture(texture0, ps_texcoord);
	finalColor = finalColor*ps_color*multiplier;
	
	oColor = finalColor;
}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

in vec4 in_position;
in vec2 in_texcoord;
in vec4 in_color;

out vec4 ps_color;
out vec2 ps_texcoord;
out vec3 ps_vertexpos;

void main()
{
ps_texcoord = in_texcoord;
ps_color = in_color;
	vec4 position = in_position*modelview;
	
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable
uniform sampler2DRect rectangle0;
	uniform vec2 fogparams;
uniform vec3 fogcolor;
uniform float multiplier;

in vec4 ps_color;
in vec2 ps_texcoord;
in vec3 ps_vertexpos;

out vec4 oColor;

float SplineFraction( float value, float scale )
{
	float valueSquared;

	value = scale * value;
	valueSquared = value * value;

	// Nice little ease-in, ease-out spline-like curve
	return 3 * valueSquared - 2 * valueSquared * value;
}

void main()
{
	vec4 finalColor = vec4(1.0, 1.0, 1.0, 1.0);
finalColor = texture(rectangle0, ps_texcoord);
	finalColor = finalColor*ps_color*multiplier;
	
	oColor = finalColor;
}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

in vec4 in_position;
in vec2 in_texcoord;
in vec4 in_color;

out vec4 ps_color;
out vec2 ps_texcoord;
out vec3 ps_vertexpos;

void main()
{
ps_texcoord = in_texcoord;
ps_color = in_color;
	vec4 position = in_position*modelview;
	
	ps_vertexpos = position.xyz;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable
uniform sampler2D texture0;
	uniform vec2 fogparams;
uniform vec3 fogcolor;
uniform float multiplier;

in vec4 ps_color;
in vec2 ps_texcoord;
in vec3 ps_vertexpos;

out vec4 oColor;

float SplineFraction( float value, float scale )
{
	float valueSquared;

	value = scale * value;
	valueSquared = value * value;

	// Nice little ease-in, ease-out spline-like curve
	return 3 * valueSquared - 2 * valueSquared * value;
}

void main()
{
	vec4 finalColor = vec4(1.0, 1.0, 1.0, 1.0);
finalColor = texture(texture0, ps_texcoord);
	finalColor = finalColor*ps_color*multiplier;
	
	float fogcoord = length(ps_vertexpos);
		
		float fogfactor = (fogparams.x - fogcoord)*fogparams.y;
		fogfactor = 1.0-SplineFraction(clamp(fogfactor, 0.0, 1.0), 1.0);
		
		finalColor.xyz = mix(finalColor.xyz, fogcolor, fogfactor);
	oColor = finalColor;
}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

in vec4 in_position;
in vec2 in_texcoord;
in vec4 in_color;

out vec4 ps_color;
out vec2 ps_texcoord;
out vec3 ps_vertexpos;

void main()
{
ps_texcoord = in_texcoord;
ps_color = in_color;
	vec4 position = in_position*modelview;
	
	ps_vertexpos = position.xyz;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable
uniform sampler2DRect rectangle0;
	uniform vec2 fogparams;
uniform vec3 fogcolor;
uniform float multiplier;

in vec4 ps_color;
in vec2 ps_texcoord;
in vec3 ps_vertexpos;

out vec4 oColor;

float SplineFraction( float value, float scale )
{
	float valueSquared;

	value = scale * value;
	valueSquared = value * value;

	// Nice little ease-in, ease-out spline-like curve
	return 3 * valueSquared - 2 * valueSquared * value;
}

void main()
{
	vec4 finalColor = vec4(1.0, 1.0, 1.0, 1.0);
finalColor = texture(rectangle0, ps_texcoord);
	finalColor = finalColor*ps_color*multiplier;
	
	float fogcoord = length(ps_vertexpos);
		
		float fogfactor = (fogparams.x - fogcoord)*fogparams.y;
		fogfactor = 1.0-SplineFraction(clamp(fogfactor, 0.0, 1.0), 1.0);
		
		finalColor.xyz = mix(finalColor.xyz, fogcolor, fogfactor);
	oColor = finalColor;
}
texture                            h$         fog                                x$         rectangle                          �$                                             