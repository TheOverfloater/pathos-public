CSD1   f4cc39629cd639e102b4540214fe6e53    �
     @        `   �  �  6    �  �  $  #version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform vec3 vorigin;
uniform vec3 start;

in vec4 in_position;
in vec2 in_texcoord;
in vec4 in_color;
in float in_width;
in vec3 in_vpoint;

out vec4 ps_color;
out vec2 ps_texcoord;
out vec3 ps_vertexpos;

void main()
{
	ps_texcoord = in_texcoord;
	ps_color = in_color;

	vec3 tangent, dir;
	tangent = start-in_vpoint;
	dir = in_vpoint-vorigin;
	vec3 right = cross(tangent, -dir);
	right = normalize(right);

	vec3 origin = in_position.xyz+right*in_width;
	vec4 position = vec4(origin, 1.0)*modelview;

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

uniform vec3 vorigin;
uniform vec3 start;

in vec4 in_position;
in vec2 in_texcoord;
in vec4 in_color;
in float in_width;
in vec3 in_vpoint;

out vec4 ps_color;
out vec2 ps_texcoord;
out vec3 ps_vertexpos;

void main()
{
	ps_texcoord = in_texcoord;
	ps_color = in_color;

	vec3 tangent, dir;
	tangent = start-in_vpoint;
	dir = in_vpoint-vorigin;
	vec3 right = cross(tangent, -dir);
	right = normalize(right);

	vec3 origin = in_position.xyz+right*in_width;
	vec4 position = vec4(origin, 1.0)*modelview;

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
fog                                	            