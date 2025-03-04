CSD1   0036351d65c73a8ef5e1d0d232e0de97    �
    @   j   �$
 �  	  �  +  "  	  9%  �  �1  	  �:  �  _H  	  vQ  �  Z_  	  qh  �  u  	  #~  �  �  	  .�  �  )�  	  @�  T  ��  ^
  ��  +  �  ^
  {�  �  ��  ^
  ]�  �  ��  ^
  F	 �  * ^
  �! �  #. ^
  �8 �  uE ^
  �O �  �] ^
  ,h T  �v   �� +  ��   �� �  ;�   A� �  ̽   �� �  �� M  � +  .� M  {� �  � M  L �  �! M  $. �  <   &G �  �S   �^   �k   w 
  �   7� c  ��   ��   Ҷ   �� s  c�   �� z  ��   � �  � e  Q �  � e  `(   c5 e  �A 
  �O e  7\ c  �j e  �v   � e  ~� s  � e  V� z  и e  5� �  �   � �  ��   ��   �   � 
  �"   �/ c  V> T  �L �  TY T  �g   �t T  �� 
  	� T  ]� c  �� 	  ׶ �  �� 	  �� S  ;� 	  R� Z  �� 	  �� �  v 	  � j  � 	  $ �  �0 	  �9 �  �G 	  �P #  �^ ^
  Ji �  Du ^
  � S  �� ^
  S� Z  �� ^
  � �  �� ^
  � j  �� ^
  �� �  �� ^
  � �  � ^
  - #  P   V% �  P1   V< S  �H   �S Z  	a   l �  �y M  � �  	� M  V� S  �� M  �� Z  P� M  �� �  P�   n� y  ��    �  �   � �  �&   �1 2  @   <K �  %X   Cc B  �p   �{ I  �   
� �  �� e  � y  �� e  �� �  �� e  &� �  �� e  d� 2  �
 e  � �  �# e  I0 B  �= e  �I I  9X e  �d �  @s   M� y  ƌ   ә �  ��   �� �  ��   �� 2  �� T  � y  �� T  �	 �  �	 T  !	 �  �.	 T  >=	 2  pK	 �  aT	 k
  �^	 8
  i	 k
  os	 �  `|	 e  ň	 8
  ��	 e  b�	 	  y�	 �
  ��	 	  �	 �
  
�	 ^
  h�	 �
  ��	 ^
  K�	 �
  @�	   F�	 �
  �
 M  
 �
  #version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	finalColor = finalColor*texture(texture0, ps_texcoord) + specularColor;
		finalColor.xyz *= 2;
		finalColor = finalColor*color;
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	trans_tangent = RotateVectorByBone( in_tangent, boneindex1, in_boneweights.x );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex2, in_boneweights.y );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex3, in_boneweights.z );
		trans_tangent += RotateVectorByBone( in_tangent, boneindex4, in_boneweights.w );	
		ps_tangent = (normalmatrix*vec4(trans_tangent, 0.0)).xyz;
		
		trans_binormal = cross(trans_normal, trans_tangent);
		ps_binormal = (normalmatrix*vec4(trans_binormal, 0.0)).xyz;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D spectexture;
uniform sampler2D lumtexture;
uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
vec3 halfVec = normalize(l_dir - v_dir);
	return max(dot(halfVec, v_normal), 0.0);
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	// Use basic specular as it looks better with the base lighting
		outColor.w = CalcSpecular(v_origin, v_normal, s_lightdir, specularStrength);
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2 * specularStrength;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
vec3 out_normal = (2.0 * texture2D (normalmap, texcoord).xyz) - 1.0;
	return normalize(tbnMatrix * out_normal);
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	finalColor.xyz += texture(lumtexture, ps_texcoord).xyz;
	// Get the specular texture if needed
	vec4 specTexture = texture(spectexture, ps_texcoord);
		specularFactor = (specTexture.x + specTexture.y + specTexture.z) / 3.0;
	// Build the TBN matrix if required
	tbnMatrix = mat3(ps_tangent, ps_binormal, ps_normal);
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	lightColor = AddBaseLight(skylight_dir, skylight_ambient, skylight_diffuse, v_normal, ps_vertexpos, specularFactor);
		
		// Add in the diffuse light
			finalColor.xyz = AddDiffuseLight(finalColor.xyz, lightColor.xyz);
		// Add to specular
		specularColor.xyz = AddSpecularLight(specularColor.xyz, lightColor.w, skylight_diffuse);
	// Normal single pass rendering
	// Lighting without textures but with alpha
	finalColor += specularColor;
		finalColor.w = texture(texture0, ps_texcoord).w;
	// Solid color
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	// Normal single pass rendering
	// Lighting without textures but with alpha
	// Solid color
	finalColor = color;
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	// Normal single pass rendering
	// Lighting without textures but with alpha
	// Solid color
	finalColor = color;
	// Scope
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	// Normal single pass rendering
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	float fldot = -dot(skylight_dir, ps_normal);
		finalColor.xyz += skylight_ambient+fldot*skylight_diffuse;

		vec2 texcoord;
		texcoord = ps_texcoord - 0.5;
		texcoord *= scope_scale;

		texcoord = texcoord + 0.5;
		texcoord.x = 1.0-texcoord.x;
		texcoord.y = 1.0-texcoord.y;
		
		texcoord.x *= scope_scrsize.x;
		texcoord.y *= scope_scrsize.y;

		vec4 texcolor = texture(texture0, ps_texcoord);
		vec4 rectcolor = texture(rectangle, texcoord);

		finalColor = mix(rectcolor, texcolor*finalColor, texcolor.w);
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	// Normal single pass rendering
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	float fldot = -dot(skylight_dir, ps_normal);
		finalColor.xyz += skylight_ambient+fldot*skylight_diffuse;

		vec2 texcoord;
		texcoord = ps_texcoord - 0.5;
		texcoord *= scope_scale;

		texcoord = texcoord + 0.5;
		texcoord.x = 1.0-texcoord.x;
		texcoord.y = 1.0-texcoord.y;
		
		texcoord.x *= scope_scrsize.x;
		texcoord.y *= scope_scrsize.y;

		vec4 texcolor = texture(texture0, ps_texcoord);
		vec4 rectcolor = texture(rectangle, texcoord);

		finalColor = mix(rectcolor, texcolor*finalColor, texcolor.w);
	// Texture with no lighting
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	// Normal single pass rendering
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	finalColor = texture(texture0, ps_texcoord);	
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	// Normal single pass rendering
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	finalColor = texture(texture0, ps_texcoord);	
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	// Normal single pass rendering
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	finalColor = texture(texture0, ps_texcoord);	
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	// Normal single pass rendering
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	finalColor = texture(texture0, ps_texcoord);	
	oColor = finalColor;
	
	float alpha = (finalColor.a - 0.5) / max(fwidth(finalColor.a), 0.0001) + 0.5;
		if(alpha < 0.5)
			discard;
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	// Normal single pass rendering
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	finalColor = texture(texture0, ps_texcoord);	
	oColor = finalColor;
	
	}
#version 130

uniform mat4 projection;
uniform mat4 modelview;

uniform mat4 normalmatrix;
uniform mat4 lightmatrix;

uniform vec4 bones[96];

uniform vec3 v_origin;
uniform vec3 v_right;

uniform vec2 scroll;

uniform sampler2D flextexture;
uniform float flextexture_size;
in vec3 in_position;
in vec3 in_tangent;
in vec3 in_normal;
in vec2 in_texcoord;
in vec4 in_boneindexes;
in vec4 in_boneweights;
in vec2 in_flexcoord;

out vec3 ps_vertexpos;
out vec3 ps_tangent;
out vec3 ps_binormal;
out vec3 ps_normal;
out vec2 ps_texcoord;
out vec4 ps_vertexcoord;

vec3 TransformVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz)+bones[boneindex].w;
	tmp[1] = dot(vin, bones[boneindex+1].xyz)+bones[boneindex+1].w;
	tmp[2] = dot(vin, bones[boneindex+2].xyz)+bones[boneindex+2].w;
	
	return tmp*weight;
}

vec3 RotateVectorByBone( vec3 vin, int boneindex, float weight )
{
	vec3 tmp;
	tmp[0] = dot(vin, bones[boneindex].xyz);
	tmp[1] = dot(vin, bones[boneindex+1].xyz);
	tmp[2] = dot(vin, bones[boneindex+2].xyz);
	
	return tmp*weight;
}

void main()
{
	int boneindex1 = int(in_boneindexes.x);
	int boneindex2 = int(in_boneindexes.y);
	int boneindex3 = int(in_boneindexes.z);
	int boneindex4 = int(in_boneindexes.w);
	
	vec3 vert_origin = in_position;
	vec3 vert_normal = in_normal;
	
	vert_origin = vert_origin + textureLod(flextexture, in_flexcoord, 0.0).xyz;
		
		vec2 texcoord_normal = vec2(in_flexcoord.x + 1.0/flextexture_size, in_flexcoord.y);
		vert_normal = normalize(vert_normal + textureLod(flextexture, texcoord_normal, 0.0).xyz);
	vec3 trans_origin;
	trans_origin = TransformVectorByBone( vert_origin, boneindex1, in_boneweights.x );
	trans_origin += TransformVectorByBone( vert_origin, boneindex2, in_boneweights.y );
	trans_origin += TransformVectorByBone( vert_origin, boneindex3, in_boneweights.z );
	trans_origin += TransformVectorByBone( vert_origin, boneindex4, in_boneweights.w );
	
	vec3 trans_normal;
	trans_normal = RotateVectorByBone( vert_normal, boneindex1, in_boneweights.x );
	trans_normal += RotateVectorByBone( vert_normal, boneindex2, in_boneweights.y );
	trans_normal += RotateVectorByBone( vert_normal, boneindex3, in_boneweights.z );
	trans_normal += RotateVectorByBone( vert_normal, boneindex4, in_boneweights.w );		
	
	vec3 trans_tangent;
	vec3 trans_binormal;
	vec4 position = vec4(trans_origin, 1.0)*modelview;

	ps_vertexpos = position.xyz;
	ps_normal = (normalmatrix*vec4(trans_normal, 0.0)).xyz;
	
	ps_texcoord = in_texcoord + scroll;
	vec3 chromeup, chromeright, tmp;
		tmp = v_origin * -1;
		tmp += trans_origin;

		tmp = normalize(tmp);
		chromeup = cross(tmp, v_right);
		chromeup = normalize(chromeup);

		chromeright = cross(tmp, chromeup);
		chromeright = normalize(chromeright);

		// This is adding 0.5 only and not dividing by 2 because eye glint looks better this way
		float n = dot(trans_normal, chromeright);
		ps_texcoord.x = n + 0.5;

		n = dot(trans_normal, chromeup);
		ps_texcoord.y = n + 0.5;
	gl_Position = position*projection;
}
#version 130
#extension GL_ARB_texture_rectangle : enable

uniform sampler2D texture0;
uniform sampler2DRect rectangle;

uniform vec3 skylight_ambient;
uniform vec3 skylight_diffuse;
uniform vec3 skylight_dir;

uniform sampler2D normalmap;

uniform vec4 color;

uniform float phong_exponent;
uniform float specfactor;

uniform float scope_scale;
uniform vec2 scope_scrsize;

in vec3 ps_vertexpos;
in vec3 ps_tangent;
in vec3 ps_binormal;
in vec3 ps_normal;
in vec2 ps_texcoord;
in vec4 ps_vertexcoord;

out vec4 oColor;

float CalcShininess( vec3 v_dir, vec3 v_normal, vec3 l_dir )
{
return 1.0;
}

float CalcSpecular( vec3 v_origin, vec3 v_normal, vec3 lightDir, float specularStrength )
{
	// Eye vector (towards the camera)
	// All coordinates are in eye space
	vec3 eyeVec = normalize( -v_origin );
	
	// Direction in which the triangle reflects the light
	vec3 reflectVec = reflect( lightDir, v_normal );
	
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( eyeVec, reflectVec ), 0, 1 );	
	
	// Currently specular color is light color times two
	return specfactor * pow( cosAlpha, phong_exponent ) * specularStrength;
}

vec4 AddBaseLight( vec3 s_lightdir, vec3 s_ambient, vec3 s_diffuse, vec3 v_normal, vec3 v_origin, float specularStrength )
{
	float fldot = -dot(s_lightdir, v_normal);
	
	vec4 outColor;
	outColor.xyz = s_ambient+fldot*s_diffuse;
	
	return outColor;
}

vec3 AddDiffuseLight ( vec3 colorIn1, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	outValue += colorIn2;
	return outValue;
}

vec3 AddSpecularLight ( vec3 colorIn1, float specularStrength, vec3 colorIn2 )
{
	vec3 outValue = colorIn1;
	return outValue;
}

vec3 GetNormal( vec2 texcoord, sampler2D normalmap, vec3 v_normal, mat3 tbnMatrix )
{
return v_normal;
}

void main()
{
	mat3 tbnMatrix;
	float specularFactor = 0.0;
	vec4 specularColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	// Add luminance of any
	// Get the specular texture if needed
	// Build the TBN matrix if required
	// Shut up compiler warnings
		tbnMatrix = mat3(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
	// Normal depends on rendering type
	vec3 v_normal = GetNormal(ps_texcoord, normalmap, ps_normal, tbnMatrix);
	
	// Base lighting
	vec4 lightColor;
	// Normal single pass rendering
	// Lighting without textures but with alpha
	// Solid color
	// Scope
	// Texture with no lighting
	finalColor = texture(texture0, ps_texcoord);	
	oColor = finalColor;
	
	}
shadertype                         �
        bumpmapping                        �
        chrome                             � 
        flex                               i!
        alphatest                          ="
        specular                           #
        luminance                          �#
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 