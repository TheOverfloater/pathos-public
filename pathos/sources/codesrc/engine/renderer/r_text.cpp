/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.

===============================================
*/

#include "includes.h"
#include "common.h"
#include "com_math.h"
#include "file.h"
#include "window.h"

#include "r_vbo.h"
#include "r_glsl.h"
#include "r_text.h"
#include "r_glextf.h"
#include "texturemanager.h"
#include "r_main.h"
#include "system.h"
#include "enginestate.h"
#include "r_common.h"
#include "r_vbo.h"
#include "texturemanager.h"

// Font set base directory
const Char CText::FONT_DIRECTORY[] = "fonts";

//
// Class definition
CText gText;

//=============================================
// @brief Constructor
//
//=============================================
CText::CText ( void ):
	m_pLibrary(nullptr),
	m_pBoundFontSet(nullptr),
	m_pCurrentSetInfo(nullptr),
	m_isActive(false),
	m_pShader(nullptr),
	m_pDefaultSet(nullptr)
{ 
	for(Uint32 i = 0; i < 4; i++)
		m_rectangleBounds[i] = 0;

	for(Uint32 i = 0; i < 2; i++)
		m_textInset[i] = 0;
}

//=============================================
// @brief Destructor
//
//=============================================
CText::~CText ( void )
{
	Shutdown();
}

//=============================================
// @brief Initializes the class
//
//=============================================
bool CText::Init ( void )
{ 
	if(FT_Init_FreeType(&m_pLibrary))
	{
		Con_EPrintf("Failed to initialize font library.\n");
		return false;
	}

	return true;
}

//=============================================
// @brief Performs shutdown ops
//
//=============================================
void CText::Shutdown ( void )
{ 
	if(m_pLibrary)
	{
		FT_Done_FreeType(m_pLibrary);
		m_pLibrary = nullptr;
	}

	ClearGL();

	if(!m_fontSetsArray.empty())
	{
		for(Uint32 i = 0; i < m_fontSetsArray.size(); i++)
			delete m_fontSetsArray[i];

		m_fontSetsArray.clear();
	}
};

//=============================================
// @brief Initializes GL objects
//
//=============================================
bool CText::InitGL ( void )
{
	// Initialize shader
	if(!m_pShader)
	{
		Int32 shaderFlags = CGLSLShader::FL_GLSL_SHADER_NONE;
		if(R_IsExtensionSupported("GL_ARB_get_program_binary"))
			shaderFlags |= CGLSLShader::FL_GLSL_BINARY_SHADER_OPS;

		m_pShader = new CGLSLShader(FL_GetInterface(), gGLExtF, "fonts.bss", shaderFlags);
		if(m_pShader->HasError())
		{
			Sys_ErrorPopup("%s - Could not compile shader: %s.", __FUNCTION__, m_pShader->GetError());
			return false;
		}

		m_attribs.u_projection = m_pShader->InitUniform("projection", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_modelview = m_pShader->InitUniform("modelview", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_color = m_pShader->InitUniform("color", CGLSLShader::UNIFORM_FLOAT4);
		m_attribs.u_texture = m_pShader->InitUniform("texture0", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_offset = m_pShader->InitUniform("offset", CGLSLShader::UNIFORM_FLOAT2);

		if(!R_CheckShaderUniform(m_attribs.u_projection, "fogcolor", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_modelview, "fogparams", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_color, "modelview", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_texture, "projection", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_offset, "texture0", m_pShader, Sys_ErrorPopup))
			return false;

		m_attribs.a_position = m_pShader->InitAttribute("in_position", 4, GL_FLOAT, sizeof(font_vertex_t), OFFSET(font_vertex_t, position));
		m_attribs.a_texcoord = m_pShader->InitAttribute("in_texcoord", 2, GL_FLOAT, sizeof(font_vertex_t), OFFSET(font_vertex_t, texcoord));

		if(!R_CheckShaderVertexAttribute(m_attribs.a_position, "in_position", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_texcoord, "in_texcoord", m_pShader, Sys_ErrorPopup))
			return false;
	}

	for(Uint32 i = 0; i < m_fontSetsArray.size(); i++)
	{
		if(!LoadFont(m_fontSetsArray[i]->name.c_str(), m_fontSetsArray[i]->fontsize, m_fontSetsArray[i]->outline, &m_fontSetsArray[i]->outlinecolor, m_fontSetsArray[i]->outlineradius))
			return false;
	}

	return true;
}

//=============================================
// @brief Releases GL objects
//
//=============================================
void CText::ClearGL ( void )
{ 
	if(!m_fontInfoArray.empty())
	{
		for(Uint32 i = 0; i < m_fontInfoArray.size(); i++)
			delete m_fontInfoArray[i];
		m_fontInfoArray.clear();
	}

	if(!m_fontSetsArray.empty())
	{
		for(Uint32 i = 0; i < m_fontSetsArray.size(); i++)
			m_fontSetsArray[i]->infoindex = NO_GL_INFO_INDEX;
	}

	if(m_pShader)
	{
		delete m_pShader;
		m_pShader = nullptr;
	}
}

//=============================================
// @brief Loads a font set
//
//=============================================
const font_set_t *CText::LoadFont ( const Char *pstrFilename, Int32 fontsize, bool outline, const color32_t* poutlinecolor, Uint32 outlineradius )
{
	// Always have this
	color32_t outlinecolor;
	if(poutlinecolor)
		outlinecolor = (*poutlinecolor);

	// Try to find an existing entry
	font_set_t* pset = nullptr;
	for(Uint32 i = 0; i < m_fontSetsArray.size(); i++)
	{
		// Make sure it's actually usable, if not, load the font in again
		font_set_t* pcheck = m_fontSetsArray[i];

		if(!qstrcmp(pstrFilename, pcheck->name) 
			&& pcheck->fontsize == fontsize
			&& pcheck->outline == outline
			&& !memcmp(&pcheck->outlinecolor, &outlinecolor, sizeof(color32_t))
			&& pcheck->outlineradius == outlineradius)
		{
			pset = pcheck;
			break;
		}
	}

	// Only return if it's loaded into GL
	if(pset && pset->infoindex != NO_GL_INFO_INDEX)
		return pset;

	CString filepath;
	filepath << FONT_DIRECTORY << PATH_SLASH_CHAR << pstrFilename;

	// Load in the file
	Uint32 size = 0;
	const byte *pdata = FL_LoadFile(filepath.c_str(), &size);
	if(!pdata)
	{
		Con_EPrintf("Failed to load font '%s'.\n", pstrFilename);
		return nullptr;
	}

	// If OpenGL is not available yet, just add it to the list
	if(!gWindow.IsInitialized())
	{
		if(!pset)
		{
			// It'll be loaded at a later time
			pset = new font_set_t;
			pset->name = pstrFilename;
			pset->fontsize = fontsize;
			pset->outline = outline;
			pset->outlineradius = outlineradius;

			if(poutlinecolor)
				pset->outlinecolor = (*poutlinecolor);

			m_fontSetsArray.push_back(pset);
		}

		FL_FreeFile(pdata);
		return pset;
	}

	// Load the font set
	FT_Face pface = nullptr;
	if(FT_New_Memory_Face(m_pLibrary, pdata, size, 0, &pface))
	{
		Con_EPrintf("Failed to load font '%s'.\n", pstrFilename);
		FL_FreeFile(pdata);
		return nullptr;
	}

	// Set pixel sizes
	if(FT_Set_Pixel_Sizes(pface, 0, fontsize))
	{
		Con_EPrintf("Font size %i is not available for '%s'.\n", size, pstrFilename);
		FL_FreeFile(pdata);
		FT_Done_Face(pface);
		return nullptr;
	}

	// Determine ideal texture size
	Uint32 glyphsize, sizex, sizey, padding;
	GetIdealSizes(fontsize, &sizex, &sizey, &glyphsize, &padding); 

	// Pack outline chars into same texture
	Int32 basesizey = sizey;
	if(outline)
		sizey *= 2;

	// Offset into height
	Uint32 yoffset = 0;

	// Allocate new slot
	font_set_t* pnew = nullptr;
	if(!pset)
		pnew = new font_set_t;
	else
		pnew = pset;

	// Allocate buffer to store data into for outlines
	// Allocate GL info struct
	pnew->infoindex = m_fontInfoArray.size();
	fontsetglinfo_t* pglinfo = new fontsetglinfo_t;

	// Create VBO
	pglinfo->pvbo = new CVBO(gGLExtF, true, false);

	// Create texture buffer
	Uint32 imagedatasize = sizex*sizey*4;
	byte* ptexturedata = new byte[imagedatasize];
	memset(ptexturedata, 0, sizeof(byte)*imagedatasize);

	// Render normal glyph set
	if(!RenderGlyphs(pnew, pglinfo, pnew->glyphs, pface, sizex, yoffset, basesizey, sizey, glyphsize, padding, 0, false, ptexturedata, 0))
	{
		if(!pset)
			delete pnew;

		if(pglinfo)
			delete pglinfo;

		Con_Printf("Could not load font set '%s'.\n", pstrFilename);
		FT_Done_Face(pface);
		FL_FreeFile(pdata);
		return nullptr;
	}

	if(outline)
	{
		// Set offset
		pglinfo->index_offset_outline = pglinfo->pvbo->GetVBODataSize() / sizeof(font_vertex_t);

		yoffset += basesizey;
		byte *poutlinedata = ptexturedata + (sizex*basesizey*4);

		if(!RenderGlyphs(pnew, pglinfo, pnew->glyphs_outline, pface, sizex, yoffset, basesizey, sizey, glyphsize, padding, pglinfo->index_offset_outline, true, poutlinedata, outlineradius))
		{
			if(!pset)
				delete pnew;

			if(pglinfo)
				delete pglinfo;

			Con_Printf("Could not load font set '%s'.\n", pstrFilename);
			FT_Done_Face(pface);
			FL_FreeFile(pdata);
			return nullptr;
		}
	}

	// Bind it into OGL
	pglinfo->palloc = CTextureManager::GetInstance()->GenTextureIndex(RS_WINDOW_LEVEL);

	glBindTexture(GL_TEXTURE_2D, pglinfo->palloc->gl_index);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, sizex, sizey, 0, GL_RGBA, GL_UNSIGNED_BYTE, ptexturedata );
	delete[] ptexturedata;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

	m_fontInfoArray.push_back(pglinfo);

	pnew->name = pstrFilename;
	pnew->fontsize = fontsize;
	pnew->outline = outline;
	pnew->outlineradius = outlineradius;

	// If specified, use outline color.
	// Otherwise it's black by default
	if(poutlinecolor)
		pnew->outlinecolor = (*poutlinecolor);

	FT_Done_Face(pface);
	FL_FreeFile(pdata);

	if(!pset)
		m_fontSetsArray.push_back(pnew);

	return pnew;
};

//=============================================
// @brief Renders glyphs for a font set
//
//=============================================
bool CText::RenderGlyphs( font_set_t *pset, fontsetglinfo_t* psetinfo, font_glyph_t* pglyphs, FT_Face pface, Uint32 sizex, Uint32 yoffset, Uint32 basesizey, Uint32 sizey, Uint32 glyphsize, Uint32 padding, Uint32 bufferoffset, bool outline, byte* poutbuffer, Uint32 outlineradius )
{
	// Allocate buffer to store data into
	byte *pglyphbuffer[NUM_GLYPHS] = { nullptr };

	FT_Stroker pstroker = nullptr;
	if(outline)
	{
		FT_Stroker_New(m_pLibrary, &pstroker);
		FT_Stroker_Set(pstroker, outlineradius * 64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
	}
	
	// Get data from the .ttf
	bool result = true;
	for(Uint32 i = 32; i < NUM_GLYPHS; i++)
	{
		FT_UInt glyphindex = FT_Get_Char_Index(pface, i);
		if(FT_Load_Glyph(pface, glyphindex, FT_LOAD_DEFAULT))
		{
			result = false;
			break;
		}

		FT_Glyph glyph;
		if(FT_Get_Glyph(pface->glyph, &glyph))
		{
			result = false;
			break;
		}

		if(outline && pstroker)
		{
			if(FT_Glyph_StrokeBorder(&glyph, pstroker, false, true))
			{
				result = false;
				break;
			}
		}

		if(FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true))
		{
			result = false;
			break;
		}

		FT_BitmapGlyph bitmapglyph = reinterpret_cast<FT_BitmapGlyph>(glyph);

		// Fill in data from the glyph
		font_glyph_t *pglyph = &pglyphs[i];
		pglyph->width = bitmapglyph->bitmap.width;
		pglyph->height = bitmapglyph->bitmap.rows;
		pglyph->bitmap_left = bitmapglyph->left;
		pglyph->bitmap_top = bitmapglyph->top;
		pglyph->advancex = (glyph->advance.x >> 16);
		pglyph->advancey = (glyph->advance.y >> 16);
		pglyph->pitch = bitmapglyph->bitmap.pitch;

		if(pglyph->height > pset->maxheight)
			pset->maxheight = pglyph->height;

		Uint32 gliphdatasize = pglyph->height*pglyph->pitch;
		pglyphbuffer[i] = new byte[gliphdatasize];
		memcpy(pglyphbuffer[i], bitmapglyph->bitmap.buffer, sizeof(byte)*gliphdatasize);

		FT_Glyph pfontglyph = nullptr;
		if(FT_Get_Glyph(pface->glyph, &pfontglyph))
		{
			result = false;
			break;
		}

		FT_BBox pbbox;
		FT_Glyph_Get_CBox(pfontglyph, FT_GLYPH_BBOX_TRUNCATE, &pbbox);
		pglyph->ymin = pbbox.xMin;

		// Get row and column
		Int32 column = (i-32)%(sizex/glyphsize);
		Int32 row = (i-32)/(sizex/glyphsize);

		// Fill in texcoords
		pglyph->texcoords[0][0] = (column*glyphsize+(padding/2.0f))/static_cast<Float>(sizex);
		pglyph->texcoords[0][1] = (row*glyphsize+(padding/2.0f)+yoffset)/static_cast<Float>(sizey);
		pglyph->texcoords[1][0] = (column*glyphsize+pglyph->width+(padding/2.0f))/static_cast<Float>(sizex);
		pglyph->texcoords[1][1] = (row*glyphsize+(padding/2.0f)+yoffset)/static_cast<Float>(sizey);
		pglyph->texcoords[2][0] = (column*glyphsize+pglyph->width+(padding/2.0f))/static_cast<Float>(sizex);
		pglyph->texcoords[2][1] = (row*glyphsize+pglyph->height+(padding/2.0f)+yoffset)/static_cast<Float>(sizey);
		pglyph->texcoords[3][0] = (column*glyphsize+(padding/2.0f))/static_cast<Float>(sizex);
		pglyph->texcoords[3][1] = (row*glyphsize+pglyph->height+(padding/2.0f)+yoffset)/static_cast<Float>(sizey);

		FT_Done_Glyph(glyph);
		FT_Done_Glyph(pfontglyph);
	}

	if(!result)
	{
		// Clear anything that's been allocated
		for(Uint32 j = 0; j < NUM_GLYPHS; j++)
		{
			if(pglyphbuffer[j]) 
				delete[] pglyphbuffer[j];
		}
	
		return false;
	}

	// Fill into output
	Uint32 bufferindex = 0;
	for (Uint32 y = 0; y < basesizey; y++) 
	{
		for (Uint32 x = 0; x < sizex; x++) 
		{
			Uint32 col = x / glyphsize;
			Uint32 row = y / glyphsize;
			Uint32 order = row * (sizex/glyphsize) + col;
			Uint32 glyph_index = order + 32;

			if (glyph_index > 32 && glyph_index < NUM_GLYPHS) 
			{
				Uint32 x_loc = x % glyphsize - padding / 2;
				Uint32 y_loc = y % glyphsize - padding / 2;

				if (x_loc < 0 || y_loc < 0 || x_loc >= pglyphs[glyph_index].width || y_loc >= pglyphs[glyph_index].height) 
				{
					poutbuffer[bufferindex++] = 255;
					poutbuffer[bufferindex++] = 255;
					poutbuffer[bufferindex++] = 255;
					poutbuffer[bufferindex++] = 0;
				} 
				else 
				{
					Int32 byte_order_in_glyph = y_loc * pglyphs[glyph_index].width + x_loc;
					poutbuffer[bufferindex++] = 255;
					poutbuffer[bufferindex++] = 255;
					poutbuffer[bufferindex++] = 255;
					poutbuffer[bufferindex++] = pglyphbuffer[glyph_index][byte_order_in_glyph];
				}
			} 
			else 
			{
				poutbuffer[bufferindex++] = 255;
				poutbuffer[bufferindex++] = 255;
				poutbuffer[bufferindex++] = 255;
				poutbuffer[bufferindex++] = 0;
			}
		}
	}

	// Delete the data we used
	for(Uint32 i = 0; i < NUM_GLYPHS; i++)
	{
		if(pglyphbuffer[i]) 
			delete[] pglyphbuffer[i];
	}

	// Now create the VBO
	font_vertex_t *pbuffer = new font_vertex_t[NUM_GLYPHS*6];

	Uint32 k = 0;
	for(Uint32 j = 0; j < NUM_GLYPHS; j++)
	{
		font_glyph_t *pglyph = &pglyphs[j];
		pglyph->start_vertex = bufferoffset + k;

		Float flwidth = pglyph->width;
		Float flheight = pglyph->height;

		Float xstart = static_cast<Float>(pglyph->bitmap_left);
		Float ystart = static_cast<Float>(-pglyph->bitmap_top);

		pbuffer[k].texcoord[0] = pglyph->texcoords[0][0];
		pbuffer[k].texcoord[1] = pglyph->texcoords[0][1];
		pbuffer[k].position[0] = xstart; 
		pbuffer[k].position[1] = ystart;
		pbuffer[k].position[2] = -1;
		pbuffer[k].position[3] = 1;
		k++;

		pbuffer[k].texcoord[0] = pglyph->texcoords[1][0];
		pbuffer[k].texcoord[1] = pglyph->texcoords[1][1];
		pbuffer[k].position[0] = xstart+flwidth; 
		pbuffer[k].position[1] = ystart;
		pbuffer[k].position[2] = -1;
		pbuffer[k].position[3] = 1;
		k++;

		pbuffer[k].texcoord[0] = pglyph->texcoords[2][0];
		pbuffer[k].texcoord[1] = pglyph->texcoords[2][1];
		pbuffer[k].position[0] = xstart+flwidth; 
		pbuffer[k].position[1] = ystart+flheight;
		pbuffer[k].position[2] = -1;
		pbuffer[k].position[3] = 1;
		k++;

		pbuffer[k].texcoord[0] = pglyph->texcoords[0][0];
		pbuffer[k].texcoord[1] = pglyph->texcoords[0][1];
		pbuffer[k].position[0] = xstart; 
		pbuffer[k].position[1] = ystart;
		pbuffer[k].position[2] = -1;
		pbuffer[k].position[3] = 1;
		k++;

		pbuffer[k].texcoord[0] = pglyph->texcoords[2][0];
		pbuffer[k].texcoord[1] = pglyph->texcoords[2][1];
		pbuffer[k].position[0] = xstart+flwidth; 
		pbuffer[k].position[1] = ystart+flheight;
		pbuffer[k].position[2] = -1;
		pbuffer[k].position[3] = 1;
		k++;

		pbuffer[k].texcoord[0] = pglyph->texcoords[3][0];
		pbuffer[k].texcoord[1] = pglyph->texcoords[3][1];
		pbuffer[k].position[0] = xstart; 
		pbuffer[k].position[1] = ystart+flheight;
		pbuffer[k].position[2] = -1;
		pbuffer[k].position[3] = 1;
		k++;
	}

	psetinfo->pvbo->Append(pbuffer, NUM_GLYPHS*6*sizeof(font_vertex_t), nullptr, 0);
	delete[] pbuffer;

	FT_Stroker_Done(pstroker);

	return true;
}

//=============================================
// @brief Returns the ideal resolution for a font set texture
//
//=============================================
void CText::GetIdealSizes( Uint32 fontsize, Uint32 *resx, Uint32 *resy, Uint32 *pglyphsize, Uint32 *padding )
{
	Uint32 glyphsize = 2;
	while(glyphsize < fontsize)
		glyphsize = glyphsize*2;

	Uint32 sizex, sizey;
	sizex = sizey = 128;
	Uint32 size = NUM_GLYPHS*glyphsize*glyphsize;

	while(sizex*sizey < size)
	{
		sizex = sizex*2;

		if(sizex*sizey < size)
			sizey = sizey*2;
	}

	if(padding)
		(*padding) = glyphsize-fontsize;

	if(resx)
		(*resx) = sizex; 
	
	if(resy)
		(*resy) = sizey;

	if(pglyphsize)
		(*pglyphsize) = glyphsize;
};

//=============================================
// @brief Sets the drawing rectangle bounds
//
//=============================================
void CText::SetRectangle( Int16 minx, Int16 miny, Int16 maxx, Int16 maxy, Int32 insetx, Int32 insety )
{
	m_rectangleBounds[0] = minx;
	m_rectangleBounds[1] = miny;
	m_rectangleBounds[2] = maxx;
	m_rectangleBounds[3] = maxy;

	m_textInset[0] = insetx;
	m_textInset[1] = insety;
};

//=============================================
// @brief Returns the size of a string in pixels
//
//=============================================
void CText::GetStringSize( const font_set_t *pset, const Char *pstring, Uint32 *width, Uint32 *height, Int32 *ymin )
{
	if(!pset)
	{
		Con_Printf("%s - Font set is nullptr.\n", __FUNCTION__);
		return;
	}

	if(width)
		*width = 0;

	if(height)
		*height = 0;

	if(ymin)
		*ymin = 0;

	if(!pstring)
		return;

	const Char *pscan = pstring;
	while(*pscan != '\0')
	{
		Uint32 glyphindex = static_cast<byte>(*pscan);
		if(glyphindex > 254)
			glyphindex = 254;

		if(height)
		{
			if(*height < pset->glyphs[glyphindex].height)
				*height = pset->glyphs[glyphindex].height;
		}

		if(ymin)
		{
			if(*ymin < pset->glyphs[glyphindex].ymin)
				*ymin = pset->glyphs[glyphindex].ymin;
		}

		if(width)
			*width += pset->glyphs[glyphindex].advancex;

		pscan++;
	}
};

//=============================================
// @brief Tells if the string should be newlined
//
//=============================================
bool CText::ShouldNewline( Int32 offsx, Int32 x, const font_set_t *pFont, const Char *pString )
{
	if(!pString)
		return true;

	if(*pString == '\n' || *pString == '\r' || *pString == '\0')
		return true;

	Uint32 screenwidth = gWindow.GetWidth();

	// Try to break full words off
	if(*pString == ' ')
	{
		Int32 xadd = 0;
		while(*pString == ' ') 
		{ 
			Uint32 glyphindex = static_cast<byte>(*pString);
			if(glyphindex > 254)
				glyphindex = 254;

			xadd += pFont->glyphs[glyphindex].advancex;
			pString++; 
		}

		while(*pString != ' ' && *pString != '\n' && *pString != '\0')
		{
			Uint32 glyphindex = static_cast<byte>(*pString);
			if(glyphindex > 254)
				glyphindex = 254;

			xadd += pFont->glyphs[glyphindex].advancex;
			pString++;
		}

		if(m_rectangleBounds[2] != 0 && m_rectangleBounds[3] != 0)
		{
			if(offsx+xadd > x+m_rectangleBounds[2]-m_textInset[0])
			{
				return true;
			}
		}
		else if(offsx+xadd > static_cast<Int32>(screenwidth))
		{
			return true;
		}
	}

	Uint32 glyphindex = static_cast<byte>(*pString);
	if(glyphindex > 254)
		glyphindex = 254;

	if(m_rectangleBounds[2] != 0 && m_rectangleBounds[3] != 0)
	{
		if(offsx+pFont->glyphs[glyphindex].advancex > x+m_rectangleBounds[2]-m_textInset[0])
		{
			return true;
		}
	}
	else if(offsx+pFont->glyphs[glyphindex].advancex > static_cast<Int32>(screenwidth))
	{
		return true;
	}

	return false;
}

//=============================================
// @brief Estimates the height of a string drawn onscreen
//
//=============================================
Int32 CText::EstimateHeight( const font_set_t *pFontSet, const Char *pstrString, Uint32 minlineheight )
{
	if(!pstrString)
		return 0;

	Int32 x;
	x = m_rectangleBounds[0]+m_textInset[0];

	// Set current beginning
	line_chunk_t curchunk;
	line_t curline;
	curchunk.pstringbegin = pstrString;
	const Char* pstrerrorpos = nullptr;

	Int32 totalHeight = 0;
	const Char *pstr = pstrString;
	while(true)
	{
		Uint32 glyphindex = static_cast<byte>(*pstr);
		if(glyphindex > 254)
			glyphindex = 254;

		const font_glyph_t *pglyph = &pFontSet->glyphs[glyphindex];
		if(ShouldNewline(x, 0, pFontSet, pstr))
		{
			// Add the new line
			if(curline.lineheight < static_cast<Int32>(minlineheight))
				curline.lineheight = static_cast<Int32>(minlineheight);

			totalHeight += curline.lineheight;

			// Skip spaces
			while(*pstr == ' ')
				pstr++;
			
			if(*pstr == '\0') 
				break;

			// Reset this
			if(pstrerrorpos && (*pstr == '\r' || *pstr == '\n'))
				pstrerrorpos = nullptr;

			if(pstr[0] == '\r' && pstr[1] == '\n')
				pstr += 2;

			if(*pstr == '\n' || *pstr == '\r')
				pstr++;

			// Reset this
			curchunk.length = 0;
			curline.lineheight = 0;

			x = m_rectangleBounds[0]+m_textInset[0];
			continue;
		}

		if(pstr[0] == '[')
		{
			if(pstrerrorpos != pstr 
				&& !qstrncmp(pstr, "[color", 6))
			{
				// Clear this
				pstrerrorpos = nullptr;
				// Skip the [color token
				const Char* pstr_parse = pstr + 6;

				// Read out the values
				while(true)
				{
					// Reached end
					if(*pstr_parse == ']')
						break;

					// Parse till spaces are gone
					while(*pstr_parse 
						&& SDL_isspace(*pstr_parse)
						&& *pstr_parse != '\r'
						&& *pstr_parse != '\n')
						pstr_parse++;

					if(!*pstr_parse
						|| *pstr_parse == '\n'
						|| *pstr_parse == '\r')
					{
						pstrerrorpos = pstr;
						break;
					}

					// Verify that the token type
					if(*pstr_parse != 'r'
						&& *pstr_parse != 'g'
						&& *pstr_parse != 'b')
					{
						pstrerrorpos = pstr;
						break;
					}

					// Move ahead one character
					pstr_parse++;

					// Mark error if present
					if(!SDL_isdigit(*pstr_parse))
					{
						pstrerrorpos = pstr;
						break;
					}

					// Next should be a series of digits
					while(SDL_isdigit(*pstr_parse))
						pstr_parse++;
				}

				// Reposition to end if all went well
				if(*pstr_parse == ']')
					pstr = pstr_parse + 1;

				// Start from beginning
				continue;
			}
			else if(!qstrncmp(pstr, "[/color]", 8))
			{
				if(!pstrerrorpos)
				{
					// Skip the token
					pstr += 8;
					continue;
				}
				else
				{
					// Clear this
					pstrerrorpos = nullptr;
				}
			}
		}

		if(curline.lineheight < pglyph->height)
			curline.lineheight = pglyph->height;

		if(*pstr == '\0')
			break;

		pstr++;
		curchunk.length++;
		x += pglyph->advancex;
	}

	if(totalHeight < minlineheight)
		totalHeight = minlineheight;

	return totalHeight;
}

//=============================================
// @brief Draws a string using faster routines without newlining or bounds checks
//
//=============================================
bool CText::DrawSimpleString( const Char *pstrString, Int32 x, Int32 y, Int32 maxlenght )
{
	if(!pstrString)
		return true;

	if(!pstrString || *pstrString == '\0')
		return true;

	if(!m_isActive)
	{
		Con_Printf("%s - Shader was not bound prior to calling.\n", __FUNCTION__);
		return true;
	}

	if(!m_pBoundFontSet)
	{
		Con_Printf("%s - No font set was bound.\n");
		return true;
	}
	
	if(!m_pCurrentSetInfo)
	{
		Con_Printf("%s - No font set info was set.\n", __FUNCTION__);
		return true;
	}

	// Reset
	Int32 _x = x + m_rectangleBounds[0] + m_textInset[0];
	Int32 _y = y + m_rectangleBounds[1] + m_textInset[1];

	R_Bind2DTexture(GL_TEXTURE0, m_pCurrentSetInfo->palloc->gl_index);

	if(m_pBoundFontSet->outline && (m_pBoundFontSet->outlinecolor.r != m_textColor.r
		|| m_pBoundFontSet->outlinecolor.g != m_textColor.g
		|| m_pBoundFontSet->outlinecolor.b != m_textColor.b))
	{
		Int32 _a = m_textColor.a;
		if(m_pBoundFontSet->outlinecolor.a != 0)
			_a *= static_cast<Float>(m_pBoundFontSet->outlinecolor.a) / 255.0f;

		// Draw the outline characters
		DrawSimpleStringChars(pstrString, m_pBoundFontSet->glyphs_outline, _x, _y, m_pBoundFontSet->outlinecolor.r, 
			m_pBoundFontSet->outlinecolor.b, m_pBoundFontSet->outlinecolor.b, _a, maxlenght, true);
	}

	// Draw the normal characters
	DrawSimpleStringChars(pstrString, m_pBoundFontSet->glyphs, _x, _y, m_textColor.r, m_textColor.g, m_textColor.b, m_textColor.a, maxlenght, false);

	return true;
}

//=============================================
// @brief Draws simple string characters
//
//=============================================
void CText::DrawSimpleStringChars( const Char* pstrString, const font_glyph_t* pglyphs, Int32 x, Int32 y, Int32 r, Int32 g, Int32 b, Int32 a, Int32 maxlenght, bool outline, Int32* padvancex, Int32* padvancey  )
{
	m_pShader->SetUniform4f(m_attribs.u_color, r/255.0f, g/255.0f, b/255.0f, a/255.0f);

	R_ValidateShader(m_pShader);

	Int32 _x = x;
	Int32 _y = y;

	const char *pstr = pstrString;
	while(*pstr)
	{
		if(maxlenght && (pstr - pstrString) >= maxlenght)
			break;

		Uint32 glyphindex = static_cast<byte>(*pstr);
		if(glyphindex > 254)
			glyphindex = 254;

		const font_glyph_t *pglyph = &pglyphs[glyphindex];

		if(!SDL_isspace(*pstr))
		{
			m_pShader->SetUniform2f(m_attribs.u_offset, static_cast<Float>(_x), static_cast<Float>(_y));
			glDrawArrays(GL_TRIANGLES, pglyph->start_vertex, 6);
		}

		_x += pglyph->advancex; 
		_y += pglyph->advancey;

		if(padvancex)
			(*padvancex) += pglyph->advancex;

		if(padvancey)
			(*padvancey) += pglyph->advancey;

		pstr++;
	}
}

//=============================================
// @brief Draws a single string on the screen
//
//=============================================
bool CText::DrawString( const Char *pstrString, Int32 x, Int32 y, bool reverse, Uint32 lineoffset, Uint32 minlineheight, Uint32 xoffset )
{
	if(!pstrString || *pstrString == '\0')
		return true;

	if(!m_isActive)
	{
		Con_Printf("%s - Shader was not bound prior to calling.\n", __FUNCTION__);
		return true;
	}

	if(!m_pBoundFontSet)
	{
		Con_Printf("%s - No font set was bound.\n");
		return true;
	}
	
	if(!m_pCurrentSetInfo)
	{
		Con_Printf("%s - No font set info was set.\n", __FUNCTION__);
		return true;
	}

	Int32 _x;
	_x = x+m_rectangleBounds[0]+m_textInset[0]+xoffset;

	// Set current beginning
	line_chunk_t curchunk;
	line_t curline;
	curchunk.pstringbegin = pstrString;
	const Char* pstrerrorpos = nullptr;

	const Char *pstr = pstrString;
	while(true)
	{
		// Grab current glyph
		Uint32 glyphindex = static_cast<byte>(*pstr);
		if(glyphindex > 254)
			glyphindex = 254;

		const font_glyph_t *pglyph = &m_pBoundFontSet->glyphs[glyphindex];

		// Check for newlining because of border edge, newline, etc
		if(ShouldNewline(_x, x, m_pBoundFontSet, pstr))
		{
			// Add the new line
			if(curline.lineheight < static_cast<Int32>(minlineheight))
				curline.lineheight = static_cast<Int32>(minlineheight);

			curline.chunks.radd(curchunk);
			m_linesList.add(curline);

			// Skip spaces
			while(*pstr == ' ')
				pstr++;

			if(*pstr == '\0') 
				break;

			// Reset this
			if(*pstr == '\r' || *pstr == '\n')
			{
				curchunk.color = color24_t();
				pstrerrorpos = nullptr;
				pstr++;
			}

			if(pstr[0] == '\r' && pstr[1] == '\n')
				pstr += 2;

			if(*pstr == '\n')
				pstr++;

			curchunk.length = 0;
			curchunk.pstringbegin = pstr;
			curline.lineheight = 0;
			curline.chunks.clear();

			_x = x+m_rectangleBounds[0]+m_textInset[0];
			continue;
		}

		// Manage any color related parameters
		if(pstr[0] == '[')
		{
			if(pstrerrorpos != pstr 
				&& !qstrncmp(pstr, "[color", 6))
			{
				// Color used for chunk
				color24_t color;
				char buffer[4];

				// Skip the [color token
				const Char* pstr_parse = pstr + 6;

				// Read out the values
				while(true)
				{
					// Reached end
					if(*pstr_parse == ']')
						break;

					// Parse till spaces are gone
					while(*pstr_parse 
						&& SDL_isspace(*pstr_parse)
						&& *pstr_parse != '\r'
						&& *pstr_parse != '\n')
						pstr_parse++;

					if(!*pstr_parse
						|| *pstr_parse == '\n'
						|| *pstr_parse == '\r')
					{
						pstrerrorpos = pstr;
						break;
					}

					// Verify that the token type
					if(*pstr_parse != 'r'
						&& *pstr_parse != 'g'
						&& *pstr_parse != 'b')
					{
						pstrerrorpos = pstr;
						break;
					}

					byte* pcolorelement;
					if(*pstr_parse == 'r')
						pcolorelement = &color.r;
					else if(*pstr_parse == 'g')
						pcolorelement = &color.g;
					else if(*pstr_parse == 'b')
						pcolorelement = &color.b;
					else
					{
						// Shouldn't possibly happen
						pcolorelement = nullptr;
						assert(false);
					}

					// Move ahead one character
					pstr_parse++;

					if(!SDL_isdigit(*pstr_parse))
					{
						pstrerrorpos = pstr;
						break;
					}

					// Next should be a digit
					Uint32 length = 0;
					while(SDL_isdigit(*pstr_parse))
					{
						buffer[length] = *pstr_parse;
						pstr_parse++;
						length++;

						if(length == 3)
						{
							while(SDL_isdigit(*pstr_parse))
								pstr_parse++;
							break;
						}
					}

					// terminate
					buffer[length] = '\0';

					// Read into integer
					int component = SDL_atoi(buffer);
					component = clamp(component, 0, 255);

					// Put into final position
					if(pcolorelement)
						(*pcolorelement) = component;
				}

				// Reposition to end
				if(*pstr_parse == ']')
				{
					// Skip the token
					pstr = pstr_parse + 1;

					if(curchunk.length)
					{
						// Retire current chunk
						curline.chunks.radd(curchunk);
						curchunk.length = 0;
						curchunk.pstringbegin = pstr;
					}
					else
					{
						// Simply reset beginning
						curchunk.pstringbegin = pstr;
					}
				
					// Set color
					curchunk.color = color;
				}

				// Start from beginning
				continue;
			}
			else if(!qstrncmp(pstr, "[/color]", 8))
			{
				if(!pstrerrorpos)
				{
					// Skip the token
					pstr += 8;

					if(curchunk.length)
					{
						// Break chunk at this point
						curline.chunks.radd(curchunk);
						curchunk.length = 0;
						curchunk.pstringbegin = pstr;
					}
					else
					{
						// Just reset beginning
						curchunk.pstringbegin = pstr;
					}

					// Reset color
					curchunk.color = color24_t();
					continue;
				}
				else
				{
					// Clear this
					pstrerrorpos = nullptr;
				}
			}
		}

		if(curline.lineheight < pglyph->height)
			curline.lineheight = pglyph->height;

		if(*pstr == '\0')
			break;

		pstr++;
		curchunk.length++;
		_x += pglyph->advancex;
	}

	// Reset
	_x = x+m_rectangleBounds[0]+m_textInset[0]+ static_cast<Int32>(xoffset);

	Int32 _y;
	if(reverse) 
		_y = y + m_rectangleBounds[3] - m_textInset[1] - m_pBoundFontSet->fontsize;
	else 
		_y = y + m_rectangleBounds[1] + m_textInset[1] + m_pBoundFontSet->fontsize;

	// Cap the offset at the last line
	if(lineoffset > m_linesList.size())
		lineoffset = m_linesList.size()-1;

	R_ValidateShader(m_pShader);

	R_Bind2DTexture(GL_TEXTURE0, m_pCurrentSetInfo->palloc->gl_index);

	if(!reverse)
		m_linesList.rbegin();
	else
		m_linesList.begin();

	// Draw line by line
	Uint32 lineIdx = 0;
	while(!m_linesList.end())
	{	
		line_t& line = m_linesList.get();

		if(lineIdx < lineoffset)
		{
			if(!reverse)
				m_linesList.prev();
			else
				m_linesList.next();

			lineIdx++;
			continue;
		}

		if(reverse)
		{
			if(m_rectangleBounds[2] != 0 &&m_rectangleBounds[3] != 0)
			{
				// Only exclude if ALL of the line is invisible
				if((_y - line.lineheight) < (y+m_rectangleBounds[1]+m_textInset[1])
					&& _y < (y+m_rectangleBounds[1]+m_textInset[1]))
					break;
			}
			else
			{
				// Only exclude if ALL of the line is invisible
				if((_y - line.lineheight) < y && _y < y)
					break;
			}
		}
		else
		{
			if(m_rectangleBounds[2] != 0 &&m_rectangleBounds[3] != 0)
			{
				// Only exclude if ALL of the line is invisible
				if((_y + line.lineheight) > (y+m_rectangleBounds[3]-m_textInset[1])
					&& _y > (y+m_rectangleBounds[3]-m_textInset[1]))
					break;
			}
			else
			{
				// Only exclude if ALL of the line is invisible
				if((_y + line.lineheight) > static_cast<Int32>(gWindow.GetHeight())
					&& _y > static_cast<Int32>(gWindow.GetHeight()))
					break;
			}
		}

		line.chunks.begin();
		while(!line.chunks.end())
		{
			line_chunk_t& chunk = line.chunks.get();
			if(!chunk.length)
			{
				line.chunks.next();
				continue;
			}

			// Set chunk color
			Int32 _r, _g, _b;
			if(chunk.color.r != 0 || chunk.color.g != 0 || chunk.color.b != 0)
			{
				_r = chunk.color.r;
				_g = chunk.color.g;
				_b = chunk.color.b;
			}
			else
			{
				_r = m_textColor.r;
				_g = m_textColor.g;
				_b = m_textColor.b;
			}

			if(m_pBoundFontSet->outline && (m_pBoundFontSet->outlinecolor.r != _r
				|| m_pBoundFontSet->outlinecolor.g != _g
				|| m_pBoundFontSet->outlinecolor.b != _b))
			{
				Int32 _or = m_pBoundFontSet->outlinecolor.r;
				Int32 _og = m_pBoundFontSet->outlinecolor.g;
				Int32 _ob = m_pBoundFontSet->outlinecolor.b;
				Int32 _a = m_textColor.a;
				if(m_pBoundFontSet->outlinecolor.a != 0)
					_a *= static_cast<Float>(m_pBoundFontSet->outlinecolor.a) / 255.0f;

				// Draw the string
				DrawSimpleStringChars(chunk.pstringbegin, m_pBoundFontSet->glyphs_outline, _x, _y, _or, _og, _ob, _a, chunk.length, true);
			}

			// Draw chunk
			Int32 advanceX = 0;
			Int32 advanceY = 0;

			DrawSimpleStringChars(chunk.pstringbegin, m_pBoundFontSet->glyphs, _x, _y, _r, _g, _b, m_textColor.a, chunk.length, false, &advanceX, &advanceY);

			_x += advanceX;
			_y += advanceY;

			line.chunks.next();
		}

		// Go down
		if(reverse)
			_y = _y - line.lineheight;
		else
			_y = _y + line.lineheight;

		_x = x+m_rectangleBounds[0]+m_textInset[0];

		if(!reverse)
			m_linesList.prev();
		else
			m_linesList.next();
	}

	// Clear out lines
	m_linesList.clear();

	return true;
};

//=============================================
// @brief Draws a single character on the screen
//
//=============================================
bool CText::DrawChar( Char character, Int32 x, Int32 y, Uint32 r, Uint32 g, Uint32 b, Uint32 a )
{
	if(SDL_isspace(character))
		return true;

	if(!m_isActive)
	{
		Con_Printf("%s - Shader was not bound prior to calling.\n", __FUNCTION__);
		return true;
	}

	if(!m_pBoundFontSet)
	{
		Con_Printf("%s - No font set was bound.\n");
		return true;
	}
	
	if(!m_pCurrentSetInfo)
	{
		Con_Printf("%s - No font set info was set.\n", __FUNCTION__);
		return true;
	}

	// bind texture
	R_Bind2DTexture(GL_TEXTURE0, m_pCurrentSetInfo->palloc->gl_index);

	// Draw with the outline first
	if(m_pBoundFontSet->outline && (m_pBoundFontSet->outlinecolor.r != r
		|| m_pBoundFontSet->outlinecolor.g != g
		|| m_pBoundFontSet->outlinecolor.b != b))
	{
		Int32 _r = m_pBoundFontSet->outlinecolor.r;
		Int32 _g = m_pBoundFontSet->outlinecolor.g;
		Int32 _b = m_pBoundFontSet->outlinecolor.b;

		Int32 _a = a;
		if(m_pBoundFontSet->outlinecolor.a != 0)
			_a *= static_cast<Float>(m_pBoundFontSet->outlinecolor.a) / 255.0f;

		m_pShader->SetUniform4f(m_attribs.u_color, static_cast<Float>(_r)/255.0f, static_cast<Float>(_g)/255.0f, static_cast<Float>(_b)/255.0f, static_cast<Float>(_a)/255.0f);

		Uint32 glyphindex = static_cast<byte>(character);
		if(glyphindex > 254)
			glyphindex = 254;

		const font_glyph_t *pglyph = &m_pBoundFontSet->glyphs_outline[glyphindex];

		R_ValidateShader(m_pShader);

		m_pShader->SetUniform2f(m_attribs.u_offset, static_cast<Float>(x), static_cast<Float>(y));
		glDrawArrays(GL_TRIANGLES, pglyph->start_vertex, 6);
	}

	// Draw normal character
	m_pShader->SetUniform4f(m_attribs.u_color, static_cast<Float>(r)/255.0f, static_cast<Float>(g)/255.0f, static_cast<Float>(b)/255.0f, static_cast<Float>(a)/255.0f);

	Uint32 glyphindex = static_cast<byte>(character);
	if(glyphindex > 254)
		glyphindex = 254;

	const font_glyph_t *pglyph = &m_pBoundFontSet->glyphs[glyphindex];

	R_ValidateShader(m_pShader);

	m_pShader->SetUniform2f(m_attribs.u_offset, static_cast<Float>(x), static_cast<Float>(y));
	glDrawArrays(GL_TRIANGLES, pglyph->start_vertex, 6);

	return true;
};

//=============================================
// @brief Prepares font rendering by binding shaders, setting matrices, etc
//
//=============================================
bool CText::Prepare( void )
{
	// Clear these anyway
	m_pBoundFontSet = nullptr;
	m_pCurrentSetInfo = nullptr;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	rns.view.projection.PushMatrix();
	rns.view.projection.LoadIdentity();
	rns.view.projection.Ortho(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO, 0.1f, 100);

	rns.view.modelview.PushMatrix();
	rns.view.modelview.LoadIdentity();
	rns.view.modelview.Scale(1.0f/gWindow.GetWidth(), 1.0f/gWindow.GetHeight(), 1.0);

	// Reset color
	m_textColor.r = m_textColor.g = m_textColor.b = m_textColor.a = 255;

	if(!m_pShader->EnableShader())
	{
		m_isActive = false;
		return false;
	}

	m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, rns.view.projection.GetMatrix());
	m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());
	m_pShader->SetUniform1i(m_attribs.u_texture, 0);

	m_isActive = true;
	return true;
};

//=============================================
// @brief Ends font rendering, unbinds shaders, VBOs, etc
//
//=============================================
void CText::Reset( void )
{
	// Always call this, just to be sure
	UnBindCurrentSet();

	m_pShader->DisableShader();

	rns.view.modelview.PopMatrix();
	rns.view.projection.PopMatrix();

	glDisable(GL_BLEND);

	// Clear these anyway
	m_pBoundFontSet = nullptr;
	m_pCurrentSetInfo = nullptr;
	m_isActive = false;
};

//=============================================
// @brief Sets the font color
//
//=============================================
void CText::SetColor( Uint32 r, Uint32 g, Uint32 b, Uint32 a )
{
	m_textColor.r = r;
	m_textColor.g = g;
	m_textColor.b = b;
	m_textColor.a = a;
};

//=============================================
// @brief Binds a specific font set
//
//=============================================
bool CText::BindSet( const font_set_t *pset )
{
	// Clear these anyway
	m_pBoundFontSet = nullptr;
	m_pCurrentSetInfo = nullptr;

	if(!m_isActive)
	{
		Con_Printf("%s - Shader was not bound prior to calling.\n", __FUNCTION__);
		return true;
	}

	if(pset->infoindex == NO_GL_INFO_INDEX)
	{
		Con_Printf("%s - Font set '%s' has no GL info allocated.\n", __FUNCTION__, pset->name.c_str());
		return true;
	}
	else if(pset->infoindex < 0 || pset->infoindex >= static_cast<Int32>(m_fontInfoArray.size()))
	{
		Con_Printf("%s - Font set '%s' has bogus GL info index.\n", __FUNCTION__, pset->name.c_str());
		return true;
	}

	// Set current font
	m_pBoundFontSet = pset;
	// Get GL info
	m_pCurrentSetInfo = m_fontInfoArray[m_pBoundFontSet->infoindex];

	m_pShader->SetVBO(m_pCurrentSetInfo->pvbo);
	m_pCurrentSetInfo->pvbo->Bind();

	m_pShader->EnableAttribute(m_attribs.a_position);
	m_pShader->EnableAttribute(m_attribs.a_texcoord);

	return true;
};

//=============================================
// @brief Unbinds the currently bound font set
//
//=============================================
void CText::UnBindCurrentSet( void )
{
	if(!m_pBoundFontSet || !m_pCurrentSetInfo)
		return;

	m_pShader->DisableAttribute(m_attribs.a_position);
	m_pShader->DisableAttribute(m_attribs.a_texcoord);

	m_pCurrentSetInfo->pvbo->UnBind();
	m_pShader->SetVBO(nullptr);

	// Clear these anyway
	m_pBoundFontSet = nullptr;
	m_pCurrentSetInfo = nullptr;
};

//=============================================
// @brief Returns the shader's error message
//
// @return Error string pointer
//=============================================
const Char* CText::GetShaderError( void )
{
	if(!m_pShader)
		return "";
	else
		return m_pShader->GetError();
}

// 
//=============================================
// @brief Tells if the shader has an error
//
// @return TRUE if shader has an error, FALSE otherwise
//=============================================
bool CText::HasError( void )
{
	if(!m_pShader)
		return false;
	else
		return m_pShader->HasError();
}