CSD1   b8621bc764820c3e0b07ebfe4104ff35    �	     @      "
  p       P  �    �  Q  /    >  �  #version 130

uniform mat4 projection;
uniform mat4 modelview;

in vec4 in_position;
in vec2 in_texcoord;

out vec2 ps_texcoord;

void main()
{
	ps_texcoord = in_texcoord;

	vec4 position = in_position*modelview;
	gl_Position = position*projection;
	
}
#version 130

uniform sampler2D texture0;
uniform float size;

in vec2 ps_texcoord;
out vec4 oColor;

void main()
{
	float offset[5] = float[]( 0.0, 1.0, 2.0, 3.0, 4.0 );
	float weight[5] = float[]( 0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162 );
	
	vec4 outcolor;
	outcolor = texture(texture0, ps_texcoord)*weight[0];	
	
		for(int i = 1; i < 5; i++)
		{
			outcolor += texture(texture0, ps_texcoord+vec2(0, offset[i]/size))*weight[i];
				outcolor += texture(texture0, ps_texcoord-vec2(0, offset[i]/size))*weight[i];
			}
	oColor = outcolor;
}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

in vec4 in_position;
in vec2 in_texcoord;

out vec2 ps_texcoord;

void main()
{
	ps_texcoord = in_texcoord;

	vec4 position = in_position*modelview;
	gl_Position = position*projection;
	
}
#version 130

uniform sampler2D texture0;
uniform float size;

in vec2 ps_texcoord;
out vec4 oColor;

void main()
{
	float offset[5] = float[]( 0.0, 1.0, 2.0, 3.0, 4.0 );
	float weight[5] = float[]( 0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162 );
	
	vec4 outcolor;
	outcolor = texture(texture0, ps_texcoord)*weight[0];	
	
		for(int i = 1; i < 5; i++)
		{
			outcolor += texture(texture0, ps_texcoord+vec2(offset[i]/size, 0))*weight[i];
				outcolor += texture(texture0, ps_texcoord-vec2(offset[i]/size, 0))*weight[i];	
			}
	oColor = outcolor;
}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

in vec4 in_position;
in vec2 in_texcoord;

out vec2 ps_texcoord;

void main()
{
	ps_texcoord = in_texcoord;

	vec4 position = in_position*modelview;
	gl_Position = position*projection;
	
}
#version 130

uniform sampler2D texture0;
uniform float size;

in vec2 ps_texcoord;
out vec4 oColor;

void main()
{
	float offset[5] = float[]( 0.0, 1.0, 2.0, 3.0, 4.0 );
	float weight[5] = float[]( 0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162 );
	
	vec4 outcolor;
	vec2 texcoord = vec2(ps_texcoord.x, 1.0 - ps_texcoord.y);
		outcolor = texture(texture0, texcoord);
	oColor = outcolor;
}
type                               
             