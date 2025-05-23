CSD1   77228a4661754d5de3a58151cb839f39            @      N  P   �  �  g  #version 130

uniform mat4 projection;
uniform mat4 modelview;

in vec4 in_position;
in vec2 in_texcoord;
in vec4 in_color;

out vec4 ps_color;
out vec2 ps_texcoord;
out vec4 ps_screenpos;

void main()
{
	ps_texcoord = in_texcoord;

	ps_color = in_color;
	vec4 position = in_position*modelview;
	position = position*projection;
	
	ps_screenpos = position;
	gl_Position = position;
}
#version 130

uniform sampler2D texture0;
uniform sampler2D texture1;

in vec4 ps_color;
in vec2 ps_texcoord;
in vec4 ps_screenpos;

out vec4 oColor;

void main()
{
	vec4 finalColor = texture(texture0, ps_texcoord);
	
	vec2 scrCoord;
	scrCoord.x = (ps_screenpos[0]/ps_screenpos[3]) * 0.5 + 0.5;
	scrCoord.y = (ps_screenpos[1]/ps_screenpos[3]) * 0.5 + 0.5;	
	scrCoord.y = 1.0 - scrCoord.y;
	
	vec4 bgColor = texture(texture1, scrCoord);
	float alpha = (bgColor.x + bgColor.y + bgColor.z)/3.0f;
	
	oColor.xyz = finalColor.xyz * ps_color.xyz;
	oColor.w = alpha * finalColor.w * ps_color.w;
}
