CSD1   40cc65d87d734520e1d2b2f0df451b55    �     @      �  `     c  �  �    �  �  #version 130

uniform mat4 projection;
uniform mat4 modelview;

in vec4 in_position;
out vec4 ps_coord;

void main()
{
	vec4 position = in_position*modelview;
	position = position*projection;
	
	gl_Position = position;
	ps_coord = position;
}
#version 130
// Thanks to stevenmarky from gamedev.net for the shader code

uniform sampler2D texture0;
uniform vec2 screensize;
uniform vec2 screenpos;
uniform float distance;
uniform float size;

in vec4 ps_coord;

out vec4 oColor;

void main()
{
	vec2 scrcoords;
	scrcoords.x = (ps_coord.x / ps_coord.w);
	scrcoords.x = scrcoords.x * 0.5 + 0.5;
	
	scrcoords.y = (ps_coord.y / ps_coord.w);
	scrcoords.y = scrcoords.y * 0.5 + 0.5;
	
	vec2 balanced = scrcoords.xy - screenpos.xy;
	balanced.x *= screensize.x/screensize.y;
	
	vec2 normalized = normalize(balanced);
	float dist = length(balanced);
	
	// The strength of the gravitational field at this point:
	float scaled = dist * distance * 0.05 * (1.0 / size);
	float strength = 1 / ( scaled * scaled );
	vec3 rayDirection = vec3(0,0,1);
	vec3 surfaceNormal = normalize( vec3(normalized, 1.0/strength) );
	
	// normalized	
	vec3 newBeam = refract(rayDirection, surfaceNormal, 2.6);
	vec2 newPos = scrcoords.xy + vec2(newBeam.x, newBeam.y);
	
vec4 finalColor = texture(texture0, newPos);
finalColor *= length(newBeam);
	finalColor.a = 1.0f;

	oColor = finalColor;
}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

in vec4 in_position;
out vec4 ps_coord;

void main()
{
	vec4 position = in_position*modelview;
	position = position*projection;
	
	gl_Position = position;
	ps_coord = position;
}
#version 130
#extension GL_ARB_texture_rectangle : enable
// Thanks to stevenmarky from gamedev.net for the shader code

uniform sampler2DRect texture0Rect;
uniform vec2 screensize;
uniform vec2 screenpos;
uniform float distance;
uniform float size;

in vec4 ps_coord;

out vec4 oColor;

void main()
{
	vec2 scrcoords;
	scrcoords.x = (ps_coord.x / ps_coord.w);
	scrcoords.x = scrcoords.x * 0.5 + 0.5;
	
	scrcoords.y = (ps_coord.y / ps_coord.w);
	scrcoords.y = scrcoords.y * 0.5 + 0.5;
	
	vec2 balanced = scrcoords.xy - screenpos.xy;
	balanced.x *= screensize.x/screensize.y;
	
	vec2 normalized = normalize(balanced);
	float dist = length(balanced);
	
	// The strength of the gravitational field at this point:
	float scaled = dist * distance * 0.05 * (1.0 / size);
	float strength = 1 / ( scaled * scaled );
	vec3 rayDirection = vec3(0,0,1);
	vec3 surfaceNormal = normalize( vec3(normalized, 1.0/strength) );
	
	// normalized	
	vec3 newBeam = refract(rayDirection, surfaceNormal, 2.6);
	vec2 newPos = scrcoords.xy + vec2(newBeam.x, newBeam.y);
	
vec4 finalColor = texture(texture0Rect, (newPos)*screensize);
finalColor *= length(newBeam);
	finalColor.a = 1.0f;

	oColor = finalColor;
}
rectangle                          �            