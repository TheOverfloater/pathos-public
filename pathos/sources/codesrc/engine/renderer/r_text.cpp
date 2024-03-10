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

//=======================================
// Constructor
// Purpose:
//=======================================
CText :: CText ( void ):
	m_pLibrary(nullptr),
	m_iR(255),
	m_iG(255),
	m_iB(255),
	m_iA(255),
	m_pShader(nullptr),
	m_uiUniformProjection(0),
	m_uiUniformModelView(0),
	m_uiUniformColor(0),
	m_uiUniformTexture(0),
	m_uiUniformOffset(0),
	m_uiAttribPosition(0),
	m_uiAttribTexCoord(0),
	m_pDefaultSet(nullptr)
{ 
	for(Uint32 i = 0; i < 4; i++)
		m_iRectangle[i] = 0;

	for(Uint32 i = 0; i < 2; i++)
		m_iInset[i] = 0;
}

//=======================================
// Destructor
// Purpose:
//=======================================
CText :: ~CText ( void )
{
	Shutdown();
}

//=======================================
// CText :: Init
// Purpose:
//=======================================
bool CText :: Init ( void )
{ 
	if(FT_Init_FreeType(&m_pLibrary))
	{
		Con_EPrintf("Failed to initialize font library.\n");
		return false;
	}

	return true;
}

//=======================================
// CText :: Shutdown
// Purpose:
//=======================================
void CText :: Shutdown ( void )
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

//=======================================
// CText :: InitGL
// Purpose:
//=======================================
bool CText :: InitGL ( void )
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

		m_uiUniformProjection = m_pShader->InitUniform("projection", CGLSLShader::UNIFORM_MATRIX4);
		m_uiUniformModelView = m_pShader->InitUniform("modelview", CGLSLShader::UNIFORM_MATRIX4);
		m_uiUniformColor = m_pShader->InitUniform("color", CGLSLShader::UNIFORM_FLOAT4);
		m_uiUniformTexture = m_pShader->InitUniform("texture0", CGLSLShader::UNIFORM_INT1);
		m_uiUniformOffset = m_pShader->InitUniform("offset", CGLSLShader::UNIFORM_FLOAT2);

		if(!R_CheckShaderUniform(m_uiUniformProjection, "fogcolor", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_uiUniformModelView, "fogparams", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_uiUniformColor, "modelview", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_uiUniformTexture, "projection", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_uiUniformOffset, "texture0", m_pShader, Sys_ErrorPopup))
			return false;

		m_uiAttribPosition = m_pShader->InitAttribute("in_position", 4, GL_FLOAT, sizeof(font_vertex_t), OFFSET(font_vertex_t, position));
		m_uiAttribTexCoord = m_pShader->InitAttribute("in_texcoord", 2, GL_FLOAT, sizeof(font_vertex_t), OFFSET(font_vertex_t, texcoord));

		if(!R_CheckShaderVertexAttribute(m_uiAttribPosition, "in_position", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_uiAttribTexCoord, "in_texcoord", m_pShader, Sys_ErrorPopup))
			return false;
	}

	for(Uint32 i = 0; i < m_fontSetsArray.size(); i++)
	{
		if(!LoadFont(m_fontSetsArray[i]->name.c_str(), m_fontSetsArray[i]->fontsize, m_fontSetsArray[i]->outline, &m_fontSetsArray[i]->outlinecolor, m_fontSetsArray[i]->outlineradius))
			return false;
	}

	return true;
}

//=======================================
// CText :: ClearGL
// Purpose:
//=======================================
void CText :: ClearGL ( void )
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

//=======================================
// CText :: LoadFont
// Purpose:
//=======================================
const font_set_t *CText :: LoadFont ( const Char *pstrFilename, Int32 fontSize, bool outline, const color32_t* poutlinecolor, Uint32 outlineradius )
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
		if(!qstrcmp(pstrFilename, m_fontSetsArray[i]->name) 
			&& m_fontSetsArray[i]->fontsize == fontSize
			&& m_fontSetsArray[i]->outline == outline
			&& !memcmp(&m_fontSetsArray[i]->outlinecolor, &outlinecolor, sizeof(color32_t))
			&& m_fontSetsArray[i]->outlineradius == outlineradius)
		{
			pset = m_fontSetsArray[i];
			break;
		}
	}

	// Only return if it's loaded into GL
	if(pset && pset->infoindex != NO_GL_INFO_INDEX)
		return pset;

	CString filePath;
	filePath << FONT_DIRECTORY << PATH_SLASH_CHAR << pstrFilename;

	// Load in the file
	Uint32 iSize = 0;
	const byte *pData = FL_LoadFile(filePath.c_str(), &iSize);
	if(!pData)
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
			pset->fontsize = fontSize;
			pset->outline = outline;
			pset->outlineradius = outlineradius;

			if(poutlinecolor)
				pset->outlinecolor = (*poutlinecolor);

			m_fontSetsArray.push_back(pset);
		}

		FL_FreeFile(pData);
		return pset;
	}

	// Load the font set
	FT_Face pFace = nullptr;
	if(FT_New_Memory_Face(m_pLibrary, pData, iSize, 0, &pFace))
	{
		Con_EPrintf("Failed to load font '%s'.\n", pstrFilename);
		return nullptr;
	}

	// Set pixel sizes
	if(FT_Set_Pixel_Sizes(pFace, 0, fontSize))
	{
		Con_EPrintf("Font size %i is not available for '%s'.\n", iSize, pstrFilename);
		FT_Done_Face(pFace);
		return nullptr;
	}

	// Determine ideal texture size
	Uint32 iGlyphSize, iResX, iResY, iPadding;
	GetIdealSizes(fontSize, &iResX, &iResY, &iGlyphSize, &iPadding); 

	// Pack outline chars into same texture
	Int32 baseResY = iResY;
	if(outline)
		iResY *= 2;

	// Offset into height
	Uint32 yOffset = 0;

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
	Uint32 imagedatasize = iResX*iResY*4;
	byte* ptexturedata = new byte[imagedatasize];
	memset(ptexturedata, 0, sizeof(byte)*imagedatasize);

	// Render normal glyph set
	if(!RenderGlyphs(pnew, pglinfo, pnew->glyphs, pFace, iResX, yOffset, baseResY, iResY, iGlyphSize, iPadding, 0, false, ptexturedata, 0))
	{
		if(!pset)
			delete pnew;

		if(pglinfo)
			delete pglinfo;

		Con_Printf("Could not load font set '%s'.\n", pstrFilename);
		FT_Done_Face(pFace);
		FL_FreeFile(pData);
		return nullptr;
	}

	if(outline)
	{
		// Set offset
		pglinfo->index_offset_outline = pglinfo->pvbo->GetVBODataSize() / sizeof(font_vertex_t);

		yOffset += baseResY;
		byte *poutlinedata = ptexturedata + (iResX*baseResY*4);

		if(!RenderGlyphs(pnew, pglinfo, pnew->glyphs_outline, pFace, iResX, yOffset, baseResY, iResY, iGlyphSize, iPadding, pglinfo->index_offset_outline, true, poutlinedata, outlineradius))
		{
			if(!pset)
				delete pnew;

			if(pglinfo)
				delete pglinfo;

			Con_Printf("Could not load font set '%s'.\n", pstrFilename);
			FT_Done_Face(pFace);
			FL_FreeFile(pData);
			return nullptr;
		}
	}

	// Bind it into OGL
	pglinfo->palloc = CTextureManager::GetInstance()->GenTextureIndex(RS_WINDOW_LEVEL);

	glBindTexture(GL_TEXTURE_2D, pglinfo->palloc->gl_index);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, iResX, iResY, 0, GL_RGBA, GL_UNSIGNED_BYTE, ptexturedata );
	delete[] ptexturedata;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

	m_fontInfoArray.push_back(pglinfo);

	pnew->name = pstrFilename;
	pnew->fontsize = fontSize;
	pnew->outline = outline;
	pnew->outlineradius = outlineradius;

	// If specified, use outline color.
	// Otherwise it's black by default
	if(poutlinecolor)
		pnew->outlinecolor = (*poutlinecolor);

	FT_Done_Face(pFace);
	FL_FreeFile(pData);

	if(!pset)
		m_fontSetsArray.push_back(pnew);

	return pnew;
};

//=======================================
// CText :: RenderGlyphs
// Purpose:
//=======================================
bool CText :: RenderGlyphs( font_set_t *pset, fontsetglinfo_t* psetinfo, font_glyph_t* pglyphs, FT_Face pFace, Uint32 iResX, Uint32 yOffset, Uint32 baseResY, Uint32 iResY, Uint32 iGlyphSize, Uint32 iPadding, Uint32 bufferoffset, bool outline, byte* poutbuffer, Uint32 outlineradius )
{
	// Allocate buffer to store data into
	byte *pGlyphBuffer[NUM_GLYPHS] = { nullptr };

	FT_Stroker pStroker = nullptr;
	if(outline)
	{
		FT_Stroker_New(m_pLibrary, &pStroker);
		FT_Stroker_Set(pStroker, outlineradius * 64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
	}

	// Get data from the .ttf
	bool result = true;
	for(Uint32 i = 32; i < NUM_GLYPHS; i++)
	{
		FT_UInt glyphIndex = FT_Get_Char_Index(pFace, i);
		if(FT_Load_Glyph(pFace, glyphIndex, FT_LOAD_DEFAULT))
		{
			result = false;
			break;
		}

		FT_Glyph glyph;
		if(FT_Get_Glyph(pFace->glyph, &glyph))
		{
			result = false;
			break;
		}

		if(outline && pStroker)
		{
			if(FT_Glyph_StrokeBorder(&glyph, pStroker, false, true))
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

		FT_BitmapGlyph bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(glyph);

		// Fill in data from the glyph
		font_glyph_t *pGlyph = &pglyphs[i];
		pGlyph->width = bitmapGlyph->bitmap.width;
		pGlyph->height = bitmapGlyph->bitmap.rows;
		pGlyph->bitmap_left = bitmapGlyph->left;
		pGlyph->bitmap_top = bitmapGlyph->top;
		pGlyph->advancex = (glyph->advance.x >> 16);
		pGlyph->advancey = (glyph->advance.y >> 16);
		pGlyph->pitch = bitmapGlyph->bitmap.pitch;

		if(pGlyph->height > pset->maxheight)
			pset->maxheight = pGlyph->height;

		Uint32 gliphDataSize = pGlyph->height*pGlyph->pitch;
		pGlyphBuffer[i] = new byte[gliphDataSize];
		memcpy(pGlyphBuffer[i], bitmapGlyph->bitmap.buffer, sizeof(byte)*gliphDataSize);

		FT_Glyph pFontGlyph = nullptr;
		if(FT_Get_Glyph(pFace->glyph, &pFontGlyph))
		{
			result = false;
			break;
		}

		FT_BBox pBBox;
		FT_Glyph_Get_CBox(pFontGlyph, FT_GLYPH_BBOX_TRUNCATE, &pBBox);
		pGlyph->ymin = pBBox.xMin;

		// Get row and column
		Int32 iColumn = (i-32)%(iResX/iGlyphSize);
		Int32 iRow = (i-32)/(iResX/iGlyphSize);

		// Fill in texcoords
		pGlyph->texcoords[0][0] = (iColumn*iGlyphSize+(iPadding/2.0f))/(Float)iResX;
		pGlyph->texcoords[0][1] = (iRow*iGlyphSize+(iPadding/2.0f)+yOffset)/(Float)iResY;
		pGlyph->texcoords[1][0] = (iColumn*iGlyphSize+pGlyph->width+(iPadding/2.0f))/(Float)iResX;
		pGlyph->texcoords[1][1] = (iRow*iGlyphSize+(iPadding/2.0f)+yOffset)/(Float)iResY;
		pGlyph->texcoords[2][0] = (iColumn*iGlyphSize+pGlyph->width+(iPadding/2.0f))/(Float)iResX;
		pGlyph->texcoords[2][1] = (iRow*iGlyphSize+pGlyph->height+(iPadding/2.0f)+yOffset)/(Float)iResY;
		pGlyph->texcoords[3][0] = (iColumn*iGlyphSize+(iPadding/2.0f))/(Float)iResX;
		pGlyph->texcoords[3][1] = (iRow*iGlyphSize+pGlyph->height+(iPadding/2.0f)+yOffset)/(Float)iResY;

		FT_Done_Glyph(pFontGlyph);
	}

	if(!result)
	{
		// Clear anything that's been allocated
		for(Uint32 j = 0; j < NUM_GLYPHS; j++)
		{
			if(pGlyphBuffer[j]) 
				delete[] pGlyphBuffer[j];
		}
	
		return false;
	}

	// Fill into output
	Uint32 iBufferIndex = 0;
	for (Uint32 y = 0; y < baseResY; y++) 
	{
		for (Uint32 x = 0; x < iResX; x++) 
		{
			Uint32 col = x / iGlyphSize;
			Uint32 row = y / iGlyphSize;
			Uint32 order = row * (iResX/iGlyphSize) + col;
			Uint32 glyph_index = order + 32;

			if (glyph_index > 32 && glyph_index < NUM_GLYPHS) 
			{
				Uint32 x_loc = x % iGlyphSize - iPadding / 2;
				Uint32 y_loc = y % iGlyphSize - iPadding / 2;

				if (x_loc < 0 || y_loc < 0 || x_loc >= pglyphs[glyph_index].width || y_loc >= pglyphs[glyph_index].height) 
				{
					poutbuffer[iBufferIndex++] = 255;
					poutbuffer[iBufferIndex++] = 255;
					poutbuffer[iBufferIndex++] = 255;
					poutbuffer[iBufferIndex++] = 0;
				} 
				else 
				{
					Int32 byte_order_in_glyph = y_loc * pglyphs[glyph_index].width + x_loc;
					poutbuffer[iBufferIndex++] = 255;
					poutbuffer[iBufferIndex++] = 255;
					poutbuffer[iBufferIndex++] = 255;
					poutbuffer[iBufferIndex++] = pGlyphBuffer[glyph_index][byte_order_in_glyph];
				}
			} 
			else 
			{
				poutbuffer[iBufferIndex++] = 255;
				poutbuffer[iBufferIndex++] = 255;
				poutbuffer[iBufferIndex++] = 255;
				poutbuffer[iBufferIndex++] = 0;
			}
		}
	}

	// Delete the data we used
	for(Uint32 i = 0; i < NUM_GLYPHS; i++)
	{
		if(pGlyphBuffer[i]) 
			delete[] pGlyphBuffer[i];
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

		Float xstart = (Float)pglyph->bitmap_left;
		Float ystart = (Float)-pglyph->bitmap_top;

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

	return true;
}

//=======================================
// CText :: GetIdealSizes
// Purpose:
//=======================================
void CText :: GetIdealSizes( Uint32 fontsize, Uint32 *resx, Uint32 *resy, Uint32 *glyphsize, Uint32 *padding )
{
	Uint32 iGlyphSize = 2;
	while(iGlyphSize < fontsize)
		iGlyphSize = iGlyphSize*2;

	Uint32 iResX, iResY;
	iResX = iResY = 128;
	Uint32 iSize = NUM_GLYPHS*iGlyphSize*iGlyphSize;

	while(iResX*iResY < iSize)
	{
		iResX = iResX*2;

		if(iResX*iResY < iSize)
			iResY = iResY*2;
	}

	*padding = iGlyphSize-fontsize;
	*resx = iResX; *resy = iResY;
	*glyphsize = iGlyphSize;
};

//=======================================
// CText :: SetRectangle
// Purpose:
//=======================================
void CText :: SetRectangle( Int16 minx, Int16 miny, Int16 maxx, Int16 maxy, Int32 insetx, Int32 insety )
{
	m_iRectangle[0] = minx;
	m_iRectangle[1] = miny;
	m_iRectangle[2] = maxx;
	m_iRectangle[3] = maxy;

	m_iInset[0] = insetx;
	m_iInset[1] = insety;
};

//=======================================
// CText :: GetStringSize
// Purpose:
//=======================================
void CText :: GetStringSize( const font_set_t *pset, const Char *pstring, Uint32 *width, Uint32 *height, Int32 *ymin )
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
		Uint32 glyphIndex = (byte)(*pscan);
		if(glyphIndex > 254)
			glyphIndex = 254;

		if(height)
		{
			if(*height < pset->glyphs[glyphIndex].height)
				*height = pset->glyphs[glyphIndex].height;
		}

		if(ymin)
		{
			if(*ymin < pset->glyphs[glyphIndex].ymin)
				*ymin = pset->glyphs[glyphIndex].ymin;
		}

		if(width)
			*width += pset->glyphs[glyphIndex].advancex;

		pscan++;
	}
};

//=======================================
// CText :: ShouldNewline
// Purpose:
//=======================================
bool CText :: ShouldNewline( Int32 offsx, Int32 x, const font_set_t *pFont, const Char *pString )
{
	if(!pString)
		return true;

	if(*pString == '\n' || *pString == '\r' || *pString == '\0')
		return true;

	Uint32 screenWidth = gWindow.GetWidth();

	// Try to break full words off
	if(*pString == ' ')
	{
		Int32 xadd = 0;
		while(*pString == ' ') 
		{ 
			Uint32 glyphIndex = (byte)(*pString);
			if(glyphIndex > 254)
				glyphIndex = 254;

			xadd += pFont->glyphs[glyphIndex].advancex;
			pString++; 
		}

		while(*pString != ' ' && *pString != '\n' && *pString != '\0')
		{
			Uint32 glyphIndex = (byte)(*pString);
			if(glyphIndex > 254)
				glyphIndex = 254;

			xadd += pFont->glyphs[glyphIndex].advancex;
			pString++;
		}

		if(m_iRectangle[2] != 0 && m_iRectangle[3] != 0)
		{
			if(offsx+xadd > x+m_iRectangle[2]-m_iInset[0])
			{
				return true;
			}
		}
		else if(offsx+xadd > (Int32)screenWidth)
		{
			return true;
		}
	}

	Uint32 glyphIndex = (byte)(*pString);
	if(glyphIndex > 254)
		glyphIndex = 254;

	if(m_iRectangle[2] != 0 && m_iRectangle[3] != 0)
	{
		if(offsx+pFont->glyphs[glyphIndex].advancex > x+m_iRectangle[2]-m_iInset[0])
		{
			return true;
		}
	}
	else if(offsx+pFont->glyphs[glyphIndex].advancex > (Int32)screenWidth)
	{
		return true;
	}

	return false;
}

//=======================================
// CText :: EstimateHeight
// Purpose:
//=======================================
Int32 CText :: EstimateHeight( const font_set_t *pFontSet, const Char *pstrString, Uint32 minlineheight )
{
	if(!pstrString)
		return 0;

	Int32 iX;
	iX = m_iRectangle[0]+m_iInset[0];

	// Set current beginning
	line_chunk_t curchunk;
	line_t curline;
	curchunk.pstringbegin = pstrString;
	const Char* pstrerrorpos = nullptr;

	Int32 totalHeight = 0;
	const Char *pstr = pstrString;
	while(true)
	{
		Uint32 glyphIndex = (byte)(*pstr);
		if(glyphIndex > 254)
			glyphIndex = 254;

		const font_glyph_t *pGlyph = &pFontSet->glyphs[glyphIndex];
		if(ShouldNewline(iX, 0, pFontSet, pstr))
		{
			// Add the new line
			if(curline.lineheight < (Int32)minlineheight)
				curline.lineheight = (Int32)minlineheight;

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

			iX = m_iRectangle[0]+m_iInset[0];
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

		if(curline.lineheight < pGlyph->height)
			curline.lineheight = pGlyph->height;

		if(*pstr == '\0')
			break;

		pstr++;
		curchunk.length++;
		iX += pGlyph->advancex;
	}

	if(totalHeight < minlineheight)
		totalHeight = minlineheight;

	return totalHeight;
}

//=======================================
// CText :: DrawSimpleString
// Purpose:
//=======================================
bool CText :: DrawSimpleString( const font_set_t *pFontSet, const Char *pstrString, Int32 x, Int32 y, Int32 maxlenght )
{
	if(!pstrString)
		return true;

	if(!pstrString || *pstrString == '\0')
		return true;

	if(pFontSet->infoindex == NO_GL_INFO_INDEX)
	{
		Con_Printf("%s - Font set '%s' has no GL info allocated.\n", __FUNCTION__, pFontSet->name.c_str());
		return true;
	}
	else if(pFontSet->infoindex < 0 || pFontSet->infoindex >= (Int32)m_fontInfoArray.size())
	{
		Con_Printf("%s - Font set '%s' has bogus GL info index.\n", __FUNCTION__, pFontSet->name.c_str());
		return true;
	}

	// Get GL info
	fontsetglinfo_t* pglinfo = m_fontInfoArray[pFontSet->infoindex];

	// Reset
	Int32 iX = x + m_iRectangle[0] + m_iInset[0];
	Int32 iY = y + m_iRectangle[1] + m_iInset[1];

	R_Bind2DTexture(GL_TEXTURE0, pglinfo->palloc->gl_index);

	if(pFontSet->outline && (pFontSet->outlinecolor.r != m_iR
		|| pFontSet->outlinecolor.g != m_iG
		|| pFontSet->outlinecolor.b != m_iB))
	{
		Int32 _a = m_iA;
		if(pFontSet->outlinecolor.a != 0)
			_a *= (Float)pFontSet->outlinecolor.a / 255.0f;

		// Draw the outline characters
		DrawSimpleStringChars(pstrString, pFontSet->glyphs_outline, iX, iY, pFontSet->outlinecolor.r, 
			pFontSet->outlinecolor.b, pFontSet->outlinecolor.b, _a, maxlenght, true);
	}

	// Draw the normal characters
	DrawSimpleStringChars(pstrString, pFontSet->glyphs, iX, iY, m_iR, m_iG, m_iB, m_iA, maxlenght, false);

	return true;
}

//=======================================
// CText :: DrawSimpleStringChars
// Purpose:
//=======================================
void CText :: DrawSimpleStringChars( const Char* pstrString, const font_glyph_t* pglyphs, Int32 iX, Int32 iY, Int32 r, Int32 g, Int32 b, Int32 a, Int32 maxlenght, bool outline, Int32* padvancex, Int32* padvancey  )
{
	m_pShader->SetUniform4f(m_uiUniformColor, r/255.0f, g/255.0f, b/255.0f, a/255.0f);

	R_ValidateShader(m_pShader);

	const char *pstr = pstrString;
	while(*pstr)
	{
		if(maxlenght && (pstr - pstrString) >= maxlenght)
			break;

		Uint32 glyphIndex = (byte)(*pstr);
		if(glyphIndex > 254)
			glyphIndex = 254;

		const font_glyph_t *pGlyph = &pglyphs[glyphIndex];

		if(!SDL_isspace(*pstr))
		{
			m_pShader->SetUniform2f(m_uiUniformOffset, (Float)iX, (Float)iY);
			glDrawArrays(GL_TRIANGLES, pGlyph->start_vertex, 6);
		}

		iX += pGlyph->advancex; 
		iY += pGlyph->advancey;

		if(padvancex)
			(*padvancex) += pGlyph->advancex;

		if(padvancey)
			(*padvancey) += pGlyph->advancey;

		
		pstr++;
	}
}

//=======================================
// CText :: DrawString
// Purpose:
//=======================================
bool CText :: DrawString( const font_set_t *pFontSet, const Char *pstrString, Int32 x, Int32 y, bool reverse, Uint32 lineoffset, Uint32 minlineheight, Uint32 xoffset )
{
	if(!pstrString || *pstrString == '\0')
		return true;

	if(pFontSet->infoindex == NO_GL_INFO_INDEX)
	{
		Con_Printf("%s - Font set '%s' has no GL info allocated.\n", __FUNCTION__, pFontSet->name.c_str());
		return true;
	}
	else if(pFontSet->infoindex < 0 || pFontSet->infoindex >= (Int32)m_fontInfoArray.size())
	{
		Con_Printf("%s - Font set '%s' has bogus GL info index.\n", __FUNCTION__, pFontSet->name.c_str());
		return true;
	}

	Int32 iX, iY;
	iX = x+m_iRectangle[0]+m_iInset[0]+xoffset;

	// Set current beginning
	line_chunk_t curchunk;
	line_t curline;
	curchunk.pstringbegin = pstrString;
	const Char* pstrerrorpos = nullptr;

	const Char *pstr = pstrString;
	while(true)
	{
		// Grab current glyph
		Uint32 glyphIndex = (byte)(*pstr);
		if(glyphIndex > 254)
			glyphIndex = 254;

		const font_glyph_t *pGlyph = &pFontSet->glyphs[glyphIndex];

		// Check for newlining because of border edge, newline, etc
		if(ShouldNewline(iX, x, pFontSet, pstr))
		{
			// Add the new line
			if(curline.lineheight < (Int32)minlineheight)
				curline.lineheight = (Int32)minlineheight;

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

			iX = x+m_iRectangle[0]+m_iInset[0];
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

		if(curline.lineheight < pGlyph->height)
			curline.lineheight = pGlyph->height;

		if(*pstr == '\0')
			break;

		pstr++;
		curchunk.length++;
		iX += pGlyph->advancex;
	}

	// Reset
	iX = x+m_iRectangle[0]+m_iInset[0]+(Int32)xoffset;

	if(reverse) 
		iY = y + m_iRectangle[3] - m_iInset[1] - pFontSet->fontsize;
	else 
		iY = y + m_iRectangle[1] + m_iInset[1] + pFontSet->fontsize;

	// Get GL info
	fontsetglinfo_t* pglinfo = m_fontInfoArray[pFontSet->infoindex];

	// Cap the offset at the last line
	if(lineoffset > m_linesList.size())
		lineoffset = m_linesList.size()-1;

	R_ValidateShader(m_pShader);

	R_Bind2DTexture(GL_TEXTURE0, pglinfo->palloc->gl_index);

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
			if(m_iRectangle[2] != 0 &&m_iRectangle[3] != 0)
			{
				// Only exclude if ALL of the line is invisible
				if((iY - line.lineheight) < (y+m_iRectangle[1]+m_iInset[1])
					&& iY < (y+m_iRectangle[1]+m_iInset[1]))
					break;
			}
			else
			{
				// Only exclude if ALL of the line is invisible
				if((iY - line.lineheight) < x && iY < x)
					break;
			}
		}
		else
		{
			if(m_iRectangle[2] != 0 &&m_iRectangle[3] != 0)
			{
				// Only exclude if ALL of the line is invisible
				if((iY + line.lineheight) > (y+m_iRectangle[3]-m_iInset[1])
					&& iY > (y+m_iRectangle[3]-m_iInset[1]))
					break;
			}
			else
			{
				// Only exclude if ALL of the line is invisible
				if((iY + line.lineheight) > (Int32)gWindow.GetHeight()
					&& iY > (Int32)gWindow.GetHeight())
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
				_r = m_iR;
				_g = m_iG;
				_b = m_iB;
			}

			if(pFontSet->outline && (pFontSet->outlinecolor.r != _r
				|| pFontSet->outlinecolor.g != _g
				|| pFontSet->outlinecolor.b != _b))
			{
				Int32 _or = pFontSet->outlinecolor.r;
				Int32 _og = pFontSet->outlinecolor.g;
				Int32 _ob = pFontSet->outlinecolor.b;
				Int32 _a = m_iA;
				if(pFontSet->outlinecolor.a != 0)
					_a *= (Float)pFontSet->outlinecolor.a / 255.0f;

				// Draw the string
				DrawSimpleStringChars(chunk.pstringbegin, pFontSet->glyphs_outline, iX, iY, _or, _og, _ob, _a, chunk.length, true);
			}

			// Draw chunk
			Int32 advanceX = 0;
			Int32 advanceY = 0;

			DrawSimpleStringChars(chunk.pstringbegin, pFontSet->glyphs, iX, iY, _r, _g, _b, m_iA, chunk.length, false, &advanceX, &advanceY);

			iX += advanceX;
			iY += advanceY;

			line.chunks.next();
		}

		// Go down
		if(reverse)
			iY = iY - line.lineheight;
		else
			iY = iY + line.lineheight;

		iX = x+m_iRectangle[0]+m_iInset[0];

		if(!reverse)
			m_linesList.prev();
		else
			m_linesList.next();
	}

	// Clear out lines
	m_linesList.clear();

	return true;
};

//=======================================
// CText :: DrawChar
// Purpose:
//=======================================
bool CText :: DrawChar( const font_set_t *pFontSet, char character, Int32 x, Int32 y, Uint32 r, Uint32 g, Uint32 b, Uint32 a )
{
	if(SDL_isspace(character))
		return true;

	if(!pFontSet)
		return true;

	if(pFontSet->infoindex == NO_GL_INFO_INDEX)
	{
		Con_Printf("%s - Font set '%s' has no GL info allocated.\n", __FUNCTION__, pFontSet->name.c_str());
		return true;
	}
	else if(pFontSet->infoindex < 0 || pFontSet->infoindex >= (Int32)m_fontInfoArray.size())
	{
		Con_Printf("%s - Font set '%s' has bogus GL info index.\n", __FUNCTION__, pFontSet->name.c_str());
		return true;
	}

	// Get GL info
	fontsetglinfo_t* pglinfo = m_fontInfoArray[pFontSet->infoindex];

	// bind texture
	R_Bind2DTexture(GL_TEXTURE0, pglinfo->palloc->gl_index);

	// Draw with the outline first
	if(pFontSet->outline && (pFontSet->outlinecolor.r != r
		|| pFontSet->outlinecolor.g != g
		|| pFontSet->outlinecolor.b != b))
	{
		Int32 _r = pFontSet->outlinecolor.r;
		Int32 _g = pFontSet->outlinecolor.g;
		Int32 _b = pFontSet->outlinecolor.b;

		Int32 _a = a;
		if(pFontSet->outlinecolor.a != 0)
			_a *= (Float)pFontSet->outlinecolor.a / 255.0f;

		m_pShader->SetUniform4f(m_uiUniformColor, (Float)_r/255.0f, (Float)_g/255.0f, (Float)_b/255.0f, (Float)_a/255.0f);

		Uint32 glyphIndex = (byte)character;
		if(glyphIndex > 254)
			glyphIndex = 254;

		const font_glyph_t *pGlyph = &pFontSet->glyphs_outline[glyphIndex];

		R_ValidateShader(m_pShader);

		m_pShader->SetUniform2f(m_uiUniformOffset, (Float)x, (Float)y);
		glDrawArrays(GL_TRIANGLES, pGlyph->start_vertex, 6);
	}

	// Draw normal character
	m_pShader->SetUniform4f(m_uiUniformColor, (Float)r/255.0f, (Float)g/255.0f, (Float)b/255.0f, (Float)a/255.0f);

	Uint32 glyphIndex = (byte)character;
	if(glyphIndex > 254)
		glyphIndex = 254;

	const font_glyph_t *pGlyph = &pFontSet->glyphs[glyphIndex];

	R_ValidateShader(m_pShader);

	m_pShader->SetUniform2f(m_uiUniformOffset, (Float)x, (Float)y);
	glDrawArrays(GL_TRIANGLES, pGlyph->start_vertex, 6);

	return true;
};

//=======================================
// CText :: Prepare
// Purpose:
//=======================================
bool CText :: Prepare( void )
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	rns.view.projection.PushMatrix();
	rns.view.projection.LoadIdentity();
	rns.view.projection.Ortho(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO, (Float)0.1, 100);

	rns.view.modelview.PushMatrix();
	rns.view.modelview.LoadIdentity();
	rns.view.modelview.Scale(1.0f/gWindow.GetWidth(), 1.0f/gWindow.GetHeight(), 1.0);

	// Reset color
	m_iR = m_iG = m_iB = m_iA = 255;

	return true;
};

//=======================================
// CText :: Reset
// Purpose:
//=======================================
void CText :: Reset( void )
{
	rns.view.modelview.PopMatrix();
	rns.view.projection.PopMatrix();

	glDisable(GL_BLEND);
};

//=======================================
// CText :: SetColor
// Purpose:
//=======================================
void CText :: SetColor( Uint32 r, Uint32 g, Uint32 b, Uint32 a )
{
	m_iR = r;
	m_iG = g;
	m_iB = b;
	m_iA = a;
};

//=======================================
// CText :: BindSet
// Purpose:
//=======================================
bool CText :: BindSet( const font_set_t *pset )
{
	if(pset->infoindex == NO_GL_INFO_INDEX)
	{
		Con_Printf("%s - Font set '%s' has no GL info allocated.\n", __FUNCTION__, pset->name.c_str());
		return true;
	}
	else if(pset->infoindex < 0 || pset->infoindex >= (Int32)m_fontInfoArray.size())
	{
		Con_Printf("%s - Font set '%s' has bogus GL info index.\n", __FUNCTION__, pset->name.c_str());
		return true;
	}

	// Get GL info
	fontsetglinfo_t* pglinfo = m_fontInfoArray[pset->infoindex];

	m_pShader->SetVBO(pglinfo->pvbo);
	pglinfo->pvbo->Bind();

	if(!m_pShader->EnableShader())
		return false;

	m_pShader->SetUniformMatrix4fv(m_uiUniformProjection, rns.view.projection.GetMatrix());
	m_pShader->SetUniformMatrix4fv(m_uiUniformModelView, rns.view.modelview.GetMatrix());
	m_pShader->SetUniform1i(m_uiUniformTexture, 0);

	m_pShader->EnableAttribute(m_uiAttribPosition);
	m_pShader->EnableAttribute(m_uiAttribTexCoord);

	return true;
};

//=======================================
// CText :: UnBind
// Purpose:
//=======================================
void CText :: UnBind( const font_set_t *pset )
{
	if(pset->infoindex == NO_GL_INFO_INDEX)
	{
		Con_Printf("%s - Font set '%s' has no GL info allocated.\n", __FUNCTION__, pset->name.c_str());
		return;
	}
	else if(pset->infoindex < 0 || pset->infoindex >= (Int32)m_fontInfoArray.size())
	{
		Con_Printf("%s - Font set '%s' has bogus GL info index.\n", __FUNCTION__, pset->name.c_str());
		return;
	}

	m_pShader->DisableAttribute(m_uiAttribPosition);
	m_pShader->DisableAttribute(m_uiAttribTexCoord);
	m_pShader->DisableShader();

	// Get GL info
	fontsetglinfo_t* pglinfo = m_fontInfoArray[pset->infoindex];

	pglinfo->pvbo->UnBind();
	m_pShader->SetVBO(nullptr);
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