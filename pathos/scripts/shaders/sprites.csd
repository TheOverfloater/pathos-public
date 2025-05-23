CSD1   503081cf40c727b8b9adb23153d2f92d    �     @      �  �   f  �  3    f    �  w  �  �  !    �  �  �  #version 130

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

uniform sampler2D texture0;

uniform vec2 fogparams;
uniform vec3 fogcolor;

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
	vec4 finalColor = texture(texture0, ps_texcoord);
		finalColor = finalColor*ps_color;
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

uniform sampler2D texture0;

uniform vec2 fogparams;
uniform vec3 fogcolor;

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
	vec4 finalColor = ps_color;
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

uniform sampler2D texture0;

uniform vec2 fogparams;
uniform vec3 fogcolor;

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
	vec4 finalColor = texture(texture0, ps_texcoord);
		finalColor = finalColor*ps_color;
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

uniform sampler2D texture0;

uniform vec2 fogparams;
uniform vec3 fogcolor;

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
	vec4 finalColor = ps_color;
	float fogcoord = length(ps_vertexpos);
		
		float fogfactor = (fogparams.x - fogcoord)*fogparams.y;
		fogfactor = 1.0-SplineFraction(clamp(fogfactor, 0.0, 1.0), 1.0);
		
		finalColor.xyz = mix(finalColor.xyz, fogcolor, fogfactor);
	oColor = finalColor;
}
fog                                �         solid                              �                     