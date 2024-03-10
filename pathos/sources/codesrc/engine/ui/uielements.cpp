/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "ui_shared.h"
#include "uielements.h"
#include "textures_shared.h"
#include "fontset.h"
#include "matrix.h"
#include "uimanager.h"

// UI engine function pointers
ui_engine_interface_t CUIObject::g_engineFuncs;

// Width of a tab label
const Uint32 CUITabLabel::TAB_LABEL_WIDTH = 96;
// Height of a tab label
const Uint32 CUITabLabel::TAB_LABEL_HEIGHT = 32;

// Default height for the dropdown list
const Uint32 CUIDropDownList::DROPDOWN_LIST_WIDTH = 256;
// Default height for the dropdown list
const Uint32 CUIDropDownList::DROPDOWN_LIST_HEIGHT = 32;

//=============================================
// @brief Constructor
//
//=============================================
CUIObject::CUIObject( Int32 flags, Uint32 width, Uint32 height, Int32 originx, Int32 originy ):
	m_flags(flags),
	m_pParent(nullptr),
	m_width(width),
	m_baseWidth(width),
	m_height(height),
	m_baseHeight(height),
	m_originX(originx),
	m_baseOriginX(originx),
	m_originY(originy),
	m_baseOriginY(originy),
	m_color(255, 255, 255, 255),
	m_isVisible(true)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CUIObject::~CUIObject( void )
{
	if(!m_childrenArray.empty())
	{
		for(Uint32 i = 0; i < m_childrenArray.size(); i++)
			delete m_childrenArray[i];

		m_childrenArray.clear();
	}
}

//=============================================
// @brief Adds a child object to this element
//
// @param idx Index of the child to return
// @return Pointer to child element
//=============================================
CUIObject* CUIObject::getChildByIndex( Uint32 idx )
{
	assert(idx < m_childrenArray.size());
	return m_childrenArray[idx];
}

//=============================================
// @brief Adds a child to the array
//
// @param pParent Pointer to parent element
//=============================================
void CUIObject::addChild( CUIObject* pChild )
{ 
	// Make sure it's not present twice
	for(Uint32 i = 0; i < m_childrenArray.size(); i++)
	{
		if(m_childrenArray[i] == pChild)
			assert(FALSE); // Shit the bed
	}

	m_childrenArray.push_back(pChild);

	// See if we need to be expanded
	if(m_flags & (UIEL_FL_EXPAND_W|UIEL_FL_EXPAND_H))
	{
		Int32 xPos, yPos;
		pChild->getPosition(xPos, yPos);

		Int32 adjustW = 0;
		Int32 adjustH = 0;
		if(m_flags & UIEL_FL_EXPAND_W)
		{
			Int32 endW = xPos + pChild->getWidth();
			if(endW > (Int32)m_width)
				adjustW = endW - m_width;
		}

		if(m_flags & UIEL_FL_EXPAND_H)
		{
			Int32 endH = yPos + pChild->getHeight();
			if(endH > (Int32)m_height)
				adjustH = endH - m_height;
		}

		// Only call if needed
		if(adjustH || adjustW)
			adjustSize(adjustW, adjustH);
	}
}

//=============================================
// @brief Sets the parent pointer of the UI element
//
// @param pParent Pointer to parent element
//=============================================
void CUIObject::setParent( CUIObject* pParent )
{
	// Force an update on the parent too
	m_pParent = pParent;
	m_pParent->adjustPosition();

	// Now add this and update
	m_pParent->addChild(this);
	adjustPosition();
}

//=============================================
// @brief Sets the input focus upwards till it find the window
//
// @param pFocusObject Pointer to object to set
//=============================================
void CUIObject::setWindowInputFocusObject( CUIObject* pFocusObject )
{
	if(!m_pParent)
		return;

	m_pParent->setWindowInputFocusObject(pFocusObject);
}

//=============================================
// @brief Adjusts the position of the element relative to the parent
//
//=============================================
void CUIObject::adjustPosition( void )
{
	// Adjust children too
	for(Uint32 i = 0; i < m_childrenArray.size(); i++)
		m_childrenArray[i]->adjustPosition();

	if(!m_pParent)
		return;

	// Get width/height info from parent
	Uint32 parentWidth, parentHeight;
	m_pParent->getSize(parentWidth, parentHeight);

	if(m_flags & UIEL_FL_ALIGN_R)
		m_originX = parentWidth - m_baseOriginX - m_width;
	else if(m_flags & UIEL_FL_ALIGN_CH)
		m_originX = parentWidth * 0.5 - m_width * 0.5;
	else
		m_originX = m_baseOriginX;

	if(m_flags & UIEL_FL_ALIGN_B)
		m_originY = parentHeight - m_baseOriginY - m_height;
	else if(m_flags & UIEL_FL_ALIGN_CV)
		m_originY = parentHeight * 0.5 - m_height * 0.5;
	else
		m_originY = m_baseOriginY;
}

//=============================================
// @brief Sets the size of the UI element
//
// @param width Target width of the element
// @param height Target height of the element
//=============================================
void CUIObject::setSize( Int32 width, Int32 height )
{
	m_width = _min(0, width);
	m_height = _min(0, height);
}

//=============================================
// @brief Returns the size of the UI element
//
// @param width Target variable for width of the element
// @param height Target variable for height of the element
//=============================================
void CUIObject::getSize( Uint32& width, Uint32& height )
{
	width = m_width;
	height = m_height;
}

//=============================================
// @brief Sets the alpha value of the UI element
//
//=============================================
void CUIObject::setAlpha( Int32 alpha, bool recursive )
{
	m_color.a = clamp(alpha, 0, 255);

	if(recursive)
	{
		// Set also the children
		for(Uint32 i = 0; i < m_childrenArray.size(); i++)
			m_childrenArray[i]->setAlpha(alpha, true);
	}
}

//=============================================
// @brief Draw function for elements
//
//=============================================
void CUIObject::setColor( Uint32 r, Uint32 g, Uint32 b, Int32 a, bool recursive )
{
	m_color.r = _max(255, r);
	m_color.g = _max(255, g);
	m_color.b = _max(255, b);
	
	if(a != -1)
		m_color.a = _max(255, a);

	if(recursive)
	{
		// Set also the children
		for(Uint32 i = 0; i < m_childrenArray.size(); i++)
			m_childrenArray[i]->setColor(r, g, b, a, true);
	}
}

//=============================================
// @brief Sets the position of the UI element
//
// @param xpos Target position of the element
// @param ypos Target position of the element
// @return TRUE if position was successfuly changed, FALSE otherwise
//=============================================
void CUIObject::setPosition( Int32 xpos, Int32 ypos, bool setBase )
{
	m_originX = xpos;
	m_originY = ypos;

	if(setBase)
	{
		m_baseOriginX = xpos;
		m_baseOriginY = ypos;
	}
}

//=============================================
// @brief Sets the position of the UI element
//
// @param xpos Target variable for position of the element
// @param ypos Target variable for position of the element
//=============================================
void CUIObject::getPosition( Int32& xpos, Int32& ypos )
{
	xpos = m_originX;
	ypos = m_originY;
}

//=============================================
// @brief Sets the position of the UI element
//
// @param xpos Target variable for position of the element
// @param ypos Target variable for position of the element
//=============================================
void CUIObject::getAbsPosition( Int32& xpos, Int32& ypos )
{
	Int32 parentX = 0;
	Int32 parentY = 0;

	if(m_pParent)
		m_pParent->getAbsPosition(parentX, parentY);

	xpos = parentX + m_originX;
	ypos = parentY + m_originY;
}

//=============================================
// @brief Think function for elements
//
//=============================================
void CUIObject::think( void )
{
	// Let the children think too
	for(Uint32 i = 0; i < m_childrenArray.size(); i++)
		m_childrenArray[i]->think();
}

//=============================================
// @brief Think function for elements
//
//=============================================
void CUIObject::postThink( void )
{
	// Let the children think too
	for(Uint32 i = 0; i < m_childrenArray.size(); i++)
		m_childrenArray[i]->postThink();
}

//=============================================
// @brief Think function for elements
//
//=============================================
bool CUIObject::mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown )
{
	if(!m_isVisible)
		return false;

	if(!isMouseOver(mouseX, mouseY))
		return false;

	// Prioritize interactive elements first
	for(Uint32 i = 0; i < m_childrenArray.size(); i++)
	{
		CUIObject* pObject = m_childrenArray[i];

		if(pObject->isInteractive() 
			&& pObject->mouseButtonEvent(mouseX, mouseY, button, keyDown))
			return true;
	}

	// Now check non-interactive elements first
	for(Uint32 i = 0; i < m_childrenArray.size(); i++)
	{
		CUIObject* pObject = m_childrenArray[i];
		if(!pObject->isInteractive() 
			&& pObject->mouseButtonEvent(mouseX, mouseY, button, keyDown))
			return true;
	}

	return false;
}

//=============================================
// @brief Think function for elements
//
//=============================================
bool CUIObject::mouseWheelEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown, Int32 scroll )
{
	if(!m_isVisible)
		return false;

	if(!isMouseOver(mouseX, mouseY))
		return false;

	// Prioritize interactive elements first
	for(Uint32 i = 0; i < m_childrenArray.size(); i++)
	{
		CUIObject* pObject = m_childrenArray[i];

		if(pObject->isInteractive() 
			&& pObject->mouseWheelEvent(mouseX, mouseY, button, keyDown, scroll))
			return true;
	}

	// Now check non-interactive elements first
	for(Uint32 i = 0; i < m_childrenArray.size(); i++)
	{
		CUIObject* pObject = m_childrenArray[i];
		if(!pObject->isInteractive() 
			&& pObject->mouseWheelEvent(mouseX, mouseY, button, keyDown, scroll))
			return true;
	}

	return false;
}

//=============================================
// @brief Determines if the cursor is touching this element
//
//=============================================
bool CUIObject::isMouseOver( Int32 xPos, Int32 yPos )
{
	Int32 absX, absY;
	getAbsPosition(absX, absY);

	if(absX > xPos)
		return false;
	if(absX+(Int32)m_width < xPos)
		return false;
	if(absY > yPos)
		return false;
	if(absY+(Int32)m_height < yPos)
		return false;

	return true;
}

//=============================================
// @brief Draw function for elements
//
//=============================================
bool CUIObject::draw( void )
{
	assert(g_engineFuncs.pfnBasicDrawIsActive);

	// Let the children draw too
	for(Uint32 i = 0; i < m_childrenArray.size(); i++)
	{
		if(!(m_childrenArray[i]->getFlags() & UIEL_FL_ONTOP))
		{
			if(!m_childrenArray[i]->draw())
				return false;
		}
	}

	// Draw ontop elements last
	for(Uint32 i = 0; i < m_childrenArray.size(); i++)
	{
		if(m_childrenArray[i]->getFlags() & UIEL_FL_ONTOP)
		{
			if(!m_childrenArray[i]->draw())
				return false;
		}
	}

	return true;
}

//=============================================
// @brief Adjusts the size by x and y amount
//
//=============================================
bool CUIObject::adjustSize( Int32 x, Int32 y )
{
	if(!isResizable())
		return false;

	Int32 testWidth = (Int32)m_width;
	Int32 testHeight = (Int32)m_height;

	// Add it to the width/height
	if(!(m_flags & UIEL_FL_FIXED_W))
		testWidth += x;

	if(!(m_flags & UIEL_FL_FIXED_H))
		testHeight += y;

	// Make sure the size is valid
	if(testWidth < 0 || testHeight < 0)
		return false;

	// Test if proposed size is valid
	for(Uint32 i = 0; i < m_childrenArray.size(); i++)
	{
		// Helps filter out border and such fixed-size elements
		if(considerElementOnResize(m_childrenArray[i]))
		{
			if(!m_childrenArray[i]->isParentSizeValid((Uint32)testWidth, (Uint32)testHeight, x, y))
			{
				// Try adjusting on only one axis
				if((x && y) && (!adjustSize(x, 0) || adjustSize(0, y)))
					return true;
				else
					return false;
			}
		}
	}

	// Assign final width/height
	m_width = (Uint32)testWidth;
	m_height = (Uint32)testHeight;

	// Adjust size on all children
	for(Uint32 i = 0; i < m_childrenArray.size(); i++)
	{
		if(!considerElementOnResize(m_childrenArray[i]))
			continue;

		m_childrenArray[i]->adjustSize(x, y);
		m_childrenArray[i]->adjustPosition();
	}

	// Re-adjust our position
	adjustPosition();
	return true;
}

//=============================================
// @brief Adjusts the size by x and y amount
//
//=============================================
bool CUIObject::isParentSizeValid( Uint32 testWidth, Uint32 testHeight, Int32 adjX, Int32 adjY )
{
	assert(!isResizable());

	// Adjust origins for test
	Int32 testOriginX;
	if(m_flags & UIEL_FL_ALIGN_R)
		testOriginX = testWidth - m_baseOriginX - m_width;
	else
		testOriginX = m_baseOriginX;

	Int32 testOriginY;
	if(m_flags & UIEL_FL_ALIGN_B)
		testOriginY = testHeight - m_baseOriginY - m_height;
	else
		testOriginY = m_baseOriginY;

	// See if the origin is valid
	Int32 fullWidth = testOriginX + (Int32)m_width;
	if((Int32)testWidth < fullWidth)
		return false;

	Int32 fullHeight = testOriginY + (Int32)m_height;
	if((Int32)testHeight < fullHeight)
		return false;

	return true;
}

//=============================================
// @brief For releasing mouse button press states on buttons
//
//=============================================
void CUIObject::releaseClickStates( void )
{
	for(Uint32 i = 0; i < m_childrenArray.size(); i++)
		m_childrenArray[i]->releaseClickStates();
}

//=============================================
// @brief Returns window that holds this element
//
//=============================================
CUIObject* CUIObject::getParentWindow( void )
{
	CUIObject* pParent = getParent();
	while(pParent)
	{
		if(pParent->isWindow())
			return pParent;

		pParent = pParent->getParent();
	}

	return nullptr;
}

//=============================================
// @brief Requests that other tabs be closed
//
//=============================================
void CUIObject::closeOtherTabs( CUIObject* pCaller )
{
	for(Uint32 i = 0; i < m_childrenArray.size(); i++)
	{
		if(pCaller == m_childrenArray[i])
			continue;

		m_childrenArray[i]->closeOtherTabs(pCaller);
	}
}

//=============================================
// @brief Requests that other tabs be closed
//
//=============================================
void CUIObject::SetRenderInterface( const struct ui_engine_interface_t& renderfuncs )
{
	g_engineFuncs = renderfuncs;
}

//=============================================
// @brief Constructor
//
//=============================================
CUIInteractiveObject::CUIInteractiveObject( Int32 flags, Uint32 width, Uint32 height, Int32 originx, Int32 originy ):
	CUIObject(flags, width, height, originx, originy)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CUIInteractiveObject::~CUIInteractiveObject( void )
{
}

//=============================================
// @brief Determines if the cursor is touching this element
//
//=============================================
bool CUIInteractiveObject::isMouseOver( Int32 xPos, Int32 yPos )
{
	// Check the children first
	for(Uint32 i = 0; i < m_childrenArray.size(); i++)
	{
		if(m_childrenArray[i]->isMouseOver(xPos, yPos))
			return true;
	}

	Int32 absX, absY;
	getAbsPosition(absX, absY);

	if(absX > xPos)
		return false;
	if(absX+(Int32)m_width < xPos)
		return false;
	if(absY > yPos)
		return false;
	if(absY+(Int32)m_height < yPos)
		return false;

	return true;
}

//=============================================
// @brief Constructor
//
//=============================================
CUITexturedObject::CUITexturedObject( Int32 flags, Uint32 width, Uint32 height, Int32 originx, Int32 originy ):
	CUIObject( flags, width, height, originx, originy ),
	m_pDefaultTexture(nullptr),
	m_pFocusTexture(nullptr),
	m_pClickedTexture(nullptr),
	m_pDisabledTexture(nullptr)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CUITexturedObject::~CUITexturedObject( void )
{
}

//=============================================
// @brief Draw function for elements
//
//=============================================
bool CUITexturedObject::draw( void )
{
	assert(g_engineFuncs.pfnBasicDrawIsActive);
	
	if(!isVisible())
		return true;

	// Only draw if it's a rendered element
	if(m_pDefaultTexture != nullptr && m_height > 0 && m_width > 0)
	{
		Int32 xpos = 0;
		Int32 ypos = 0;

		// Positions are relative to parent
		if(m_pParent)
			m_pParent->getAbsPosition(xpos, ypos);

		xpos += m_originX;
		ypos += m_originY;

		if(!g_engineFuncs.pfnBasicDrawEnableTextures())
			return false;

		// Determine texture to use
		const en_texture_t* ptexture = nullptr;
		if(m_pDisabledTexture && (isDisabled() || (m_pParent && m_pParent->isDisabled())))
		{
			ptexture = m_pDisabledTexture;
		}
		else if(m_pClickedTexture && (isClickedOn() || (m_pParent && m_pParent->isClickedOn())))
		{
			ptexture = m_pClickedTexture;
		}
		else if(m_pFocusTexture)
		{
			CUIObject* pWindow = getParentWindow();

			if(pWindow && pWindow->isInFocus())
			{
				Int32 mX, mY;
				g_engineFuncs.pfnGetMousePosition(mX, mY);

				if(isMouseOver(mX, mY))
					ptexture = m_pFocusTexture;
			}
		}

		// Use default otherwise
		if(!ptexture)
			ptexture = m_pDefaultTexture;

		g_engineFuncs.pfnBind2DTexture(GL_TEXTURE0_ARB, ptexture->palloc->gl_index, false);

		g_engineFuncs.pfnBasicDrawColor4f((Float)m_color.r/255.0f, 
			(Float)m_color.g/255.0f, 
			(Float)m_color.b/255.0f, 
			(Float)m_color.a/255.0f);

		// Textures are tiled based on size, for now
		Float tcmax_x = (Float)((Float)m_width / (Float)ptexture->width);
		Float tcmax_y = (Float)((Float)m_height / (Float)ptexture->height);

		g_engineFuncs.pfnValidateBasicDraw();

		g_engineFuncs.pfnBasicDrawBegin(GL_TRIANGLES);
		g_engineFuncs.pfnBasicDrawTexCoord2f(0, tcmax_y);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xpos, (Float)ypos + m_height, -1);

		g_engineFuncs.pfnBasicDrawTexCoord2f(0, 0);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xpos, (Float)ypos, -1);

		g_engineFuncs.pfnBasicDrawTexCoord2f(tcmax_x, 0);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xpos+m_width, (Float)ypos, -1);

		g_engineFuncs.pfnBasicDrawTexCoord2f(0, tcmax_y);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xpos, (Float)ypos + m_height, -1);

		g_engineFuncs.pfnBasicDrawTexCoord2f(tcmax_x, 0);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xpos+m_width, (Float)ypos, -1);

		g_engineFuncs.pfnBasicDrawTexCoord2f(tcmax_x, tcmax_y);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xpos+m_width, (Float)ypos+m_height, -1);
		g_engineFuncs.pfnBasicDrawEnd();
	}

	// Call base class to render children
	return CUIObject::draw();
}

//=============================================
// @brief Constructor
//
//=============================================
CUISurface::CUISurface( Int32 flags, Uint32 width, Uint32 height, Int32 originx, Int32 originy ):
	CUIObject( flags, width, height, originx, originy ),
	m_pBackground(nullptr),
	m_pBtmLeftCorner(nullptr),
	m_pBtmRightCorner(nullptr),
	m_pTopLeftCorner(nullptr),
	m_pTopRightCorner(nullptr),
	m_pLeftBorder(nullptr),
	m_pRightBorder(nullptr),
	m_pBottomBorder(nullptr),
	m_pTopBorder(nullptr),
	m_pScheme(nullptr)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CUISurface::~CUISurface( void )
{
}

//=============================================
// @brief Tells if the parent size is valid for us
//
//=============================================
bool CUISurface::isParentSizeValid( Uint32 testWidth, Uint32 testHeight, Int32 adjX, Int32 adjY )
{
	// Determine my adjusted size
	Int32 myTestWidth = (Int32)m_width;
	Int32 myTestHeight = (Int32)m_height;

	// Add it to the width/height
	if(!(m_flags & UIEL_FL_FIXED_W))
		myTestWidth += adjX;

	if(!(m_flags & UIEL_FL_FIXED_H))
		myTestHeight += adjY;

	// Determine minimum width/height
	Int32 minWidth = m_pLeftBorder->getWidth() + m_pRightBorder->getWidth();
	Int32 minHeight = m_pTopBorder->getHeight() + m_pBottomBorder->getHeight();
	if(myTestWidth < minWidth || myTestHeight < minHeight)
		return false;

	// Adjust origins for test
	Int32 testOriginX;
	if(m_flags & UIEL_FL_ALIGN_R)
		testOriginX = testWidth - m_baseOriginX - m_width;
	else
		testOriginX = m_baseOriginX;

	Int32 testOriginY;
	if(m_flags & UIEL_FL_ALIGN_B)
		testOriginY = testHeight - m_baseOriginY - m_height;
	else
		testOriginY = m_baseOriginY;

	// Compare against parent size too
	Int32 fullWidth = myTestWidth + testOriginX;
	Int32 fullHeight = myTestHeight + testOriginY;
	if(fullWidth > (Int32)testWidth || fullHeight > (Int32)testHeight)
		return false;

	// Test for children too
	for(Uint32 i = 0; i < m_childrenArray.size(); i++)
	{
		if(considerElementOnResize(m_childrenArray[i]))
		{
			if(!m_childrenArray[i]->isParentSizeValid((Uint32)myTestWidth, (Uint32)myTestHeight, adjX, adjY))
				return false;
		}
	}

	return true;
}

//=============================================
// @brief Creates a UI element from a schema object
//
//=============================================
CUITexturedObject* CUISurface::createObject( const ui_schemeinfo_t* pscheme, const Char* objectName )
{
	// Get it from the schema
	const ui_schemeobject_t* pobj = pscheme->getObject(objectName);
	if(!pobj)
	{
		g_engineFuncs.pfnCon_Printf("Error: Schema '%s' is missing the '%s' definition!\n", pscheme->schemeName.c_str(), objectName);
		return nullptr;
	}

	// Allocate the new element
	CUITexturedObject* pNew = new CUITexturedObject(UIEL_FL_NONE, pobj->width, pobj->height, 0, 0);

	// Set properties
	pNew->setDefaultTexture(pobj->defaultTexture);
	pNew->setFocusTexture(pobj->focusTexture);
	pNew->setClickedTexture(pobj->clickTexture);
	pNew->setDisabledTexture(pobj->disabledTexture);
	pNew->setParent(this);

	return pNew;
}

//=============================================
// @brief Initializes the tab element with a schema
//
//=============================================
bool CUISurface::init( const Char* pstrSchemaName )
{
	m_pScheme = g_engineFuncs.pfnUILoadSchemaFile(pstrSchemaName);
	if(!m_pScheme)
		return false;

	// Set background element
	m_pBackground = createObject(m_pScheme, "Body");
	if(!m_pBackground)
		return false;

	// Set bottom left corner element
	m_pLeftBorder = createObject(m_pScheme, "Left");
	if(!m_pLeftBorder)
		return false;

	// Set bottom right corner element
	m_pRightBorder = createObject(m_pScheme, "Right");
	if(!m_pRightBorder)
		return false;

	// Init bottom elements
	if(!initBottomElements())
		return false;

	// Init top elements
	if(!initTopElements())
		return false;

	return true;
}

//=============================================
// @brief Sets the bottom border elements
//
//=============================================
bool CUISurface::initBottomElements( void )
{
	// Set top left corner element
	m_pBottomBorder = createObject(m_pScheme, "Bottom");
	if(!m_pBottomBorder)
		return false;

	// Set bottom left corner element
	m_pBtmLeftCorner = createObject(m_pScheme, "BottomLeft");
	if(!m_pBtmLeftCorner)
		return false;

	// Set bottom right corner element
	m_pBtmRightCorner = createObject(m_pScheme, "BottomRight");
	if(!m_pBtmRightCorner)
		return false;

	return true;
}

//=============================================
// @brief Sets the bottom border elements
//
//=============================================
bool CUISurface::initTopElements( void )
{
	// Set top right corner element
	m_pTopBorder = createObject(m_pScheme, "Top");
	if(!m_pTopBorder)
		return false;

	// Set top left corner element
	m_pTopLeftCorner = createObject(m_pScheme, "TopLeft");
	if(!m_pTopLeftCorner)
		return false;

	// Set top right corner element
	m_pTopRightCorner = createObject(m_pScheme, "TopRight");
	if(!m_pTopRightCorner)
		return false;

	// Call to finalize
	adjustPosition();

	return true;
}

//=============================================
// @brief Tells if a child element should be queried on size checks
//
//=============================================
bool CUISurface::considerElementOnResize( CUIObject* pObject )
{
	if(pObject == m_pBackground || pObject == m_pTopBorder
		|| pObject == m_pBottomBorder || pObject == m_pRightBorder
		|| pObject == m_pLeftBorder || pObject == m_pTopRightCorner
		|| pObject == m_pTopLeftCorner || pObject == m_pBtmRightCorner
		|| pObject == m_pBtmLeftCorner)
		return false;
	
	return true;
}

//=============================================
// @brief Sets the alpha value of the UI element
//
//=============================================
void CUISurface::setAlpha( Int32 alpha, bool recursive )
{
	// Call base class to manage basics
	CUIObject::setAlpha(alpha, recursive);

	// If not recursive, we need to set for the base elements
	if(!recursive)
	{
		m_pBackground->setAlpha(m_color.a);
		if(m_pTopLeftCorner)
			m_pTopLeftCorner->setAlpha(m_color.a);

		if(m_pTopRightCorner)
			m_pTopRightCorner->setAlpha(m_color.a);

		m_pLeftBorder->setAlpha(m_color.a);
		m_pRightBorder->setAlpha(m_color.a);

		if(m_pTopBorder)
			m_pTopBorder->setAlpha(m_color.a);

		if(m_pBtmLeftCorner)
			m_pBtmLeftCorner->setAlpha(m_color.a);
		if(m_pBtmRightCorner)
			m_pBtmRightCorner->setAlpha(m_color.a);
		if(m_pBottomBorder)
			m_pBottomBorder->setAlpha(m_color.a);
	}
}

//=============================================
// @brief Adjusts the size by x and y amount
//
//=============================================
bool CUISurface::adjustSize( Int32 x, Int32 y )
{
	// Call base class to do basics
	if(!CUIObject::adjustSize(x, y))
		return false;

	// Re-adjust bg elements
	if(isResizable())
		adjustBgElements();

	return true;
}

//=============================================
// @brief Repositions the object after a parent's size is changed
//
//=============================================
void CUISurface::adjustPosition( void )
{
	CUIObject::adjustPosition();
	adjustBgElements();
}

//=============================================
// @brief Adjusts the top border element
//
//=============================================
void CUISurface::adjustTopBorder( void )
{
	if(!m_pTopLeftCorner && !m_pTopBorder
		&& !m_pTopRightCorner)
		return;

	// Set the top left corner element
	Int32 originX = 0;
	Int32 originY = 0;
	if(m_pTopLeftCorner)
		m_pTopLeftCorner->setPosition(originX, originY);

	// Set the top border element
	originY = 0;
	if(m_pTopLeftCorner)
		originX = m_pTopLeftCorner->getWidth();
	else
		originY = 0;

	if(m_pTopBorder)
	{
		Int32 width = m_width;
		if(m_pTopLeftCorner)
			width -= m_pTopLeftCorner->getWidth();
		if(m_pTopRightCorner)
			width -= m_pTopRightCorner->getWidth();

		m_pTopBorder->setWidth(width);
		m_pTopBorder->setPosition(originX, originY);
	}

	// Set the top right corner element
	if(m_pTopRightCorner)
	{
		originY = 0;
		originX = m_width - m_pTopRightCorner->getWidth();
		m_pTopRightCorner->setPosition(originX, originY);
	}
}

//=============================================
// @brief Adjusts the background elements
//
//=============================================
void CUISurface::adjustBgElements( void )
{
	if(!m_pBackground || !m_pLeftBorder
		|| !m_pRightBorder)
		return;

	// Done in a separate function to support tabs
	adjustTopBorder();

	// Set the right border
	Int32 originX = 0;
	Int32 originY;
	if(m_pTopLeftCorner)
		originY = m_pTopLeftCorner->getHeight();
	else
		originY = 0;

	m_pRightBorder->setPosition(originX, originY);

	Int32 height = m_height;
	if(m_pTopLeftCorner)
		height -= m_pTopLeftCorner->getHeight();

	if(m_pBtmLeftCorner)
		height -= m_pBtmLeftCorner->getHeight();
	m_pRightBorder->setHeight(height);

	if(m_pBtmLeftCorner)
	{
		// Set the right bottom corner
		originX = 0;
		originY = m_height - m_pBtmLeftCorner->getHeight();
		m_pBtmLeftCorner->setPosition(originX, originY);
	}

	Uint32 width = 0;
	if(m_pBottomBorder && m_pBtmLeftCorner && m_pBtmRightCorner)
	{
		// Set the bottom border
		originX = m_pBtmLeftCorner->getWidth();
		originY = m_height - m_pBottomBorder->getHeight();
		m_pBottomBorder->setPosition(originX, originY);

		width = m_width - m_pBtmLeftCorner->getWidth();
		width -= m_pBtmRightCorner->getWidth();
		m_pBottomBorder->setWidth(width);
	}

	if(m_pBtmRightCorner)
	{
		// Set the bottom right corner
		originX = m_width - m_pBtmRightCorner->getWidth();
		originY = m_height - m_pBtmRightCorner->getHeight();
		m_pBtmRightCorner->setPosition(originX, originY);
	}

	// Set the left border
	originX = m_width - m_pRightBorder->getWidth();
	if(m_pTopRightCorner)
		originY = m_pTopRightCorner->getHeight();
	else
		originY = 0;

	m_pLeftBorder->setPosition(originX, originY);

	if(m_pTopRightCorner)
		height = m_height - m_pTopRightCorner->getHeight();
	else
		height = m_height;

	if(m_pBtmLeftCorner)
		height -= m_pBtmLeftCorner->getHeight();
	m_pLeftBorder->setHeight(height);

	// Set background
	originX = m_pLeftBorder->getWidth();
	if(m_pTopBorder)
		originY = m_pTopBorder->getHeight();
	else
		originY = 0;

	width = m_width - originX;
	width -= m_pRightBorder->getWidth();

	height = m_height - originY;
	if(m_pBottomBorder)
		height -= m_pBottomBorder->getHeight();

	m_pBackground->setPosition(originX, originY);
	m_pBackground->setSize(width, height);
}

//=============================================
// @brief Draw function
//
//=============================================
bool CUISurface::draw( void )
{
	if(!m_isVisible)
		return true;

	// Call base class to draw children
	return CUIObject::draw();
}

//=============================================
// @brief Constructor
//
//=============================================
CUIWindow::CUIWindow( Int32 winFlags, Int32 flags, Uint32 width, Uint32 height, Int32 originx, Int32 originy ):
	CUISurface( flags, width, height, originx, originy ),
	m_winFlags(winFlags),
	m_pTitle(nullptr),
	m_pInputFocusObject(nullptr),
	m_focusIndex(0),
	m_inFocus(false)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CUIWindow::~CUIWindow( void )
{
}

//=============================================
// @brief Initializes the tab element with a schema
//
//=============================================
bool CUIWindow::init( const Char* pstrSchemaName )
{
	// Call base class to set the schema
	if(!CUISurface::init(pstrSchemaName))
		return false;



	// Create the close button if available
	const ui_schemeobject_t* pCloseBtnObj = m_pScheme->getObject("CloseIcon");
	if(pCloseBtnObj)
	{
		// Create the close action object
		CUIWindowCloseAction* pAction = new CUIWindowCloseAction(this);

		// Create the button
		CUIButton* pCloseButton = new CUIButton((UIEL_FL_ALIGN_T|UIEL_FL_ALIGN_R), pCloseBtnObj, pAction, pCloseBtnObj->width, pCloseBtnObj->height, 5, 5);
		if(!pCloseButton->init("defaultbutton.txt"))
		{
			g_engineFuncs.pfnCon_EPrintf("Failed to initialize schema 'defaultbutton.txt' for button.\n");
			return false;
		}

		pCloseButton->setParent(this);
	}

	return true;
}

//=============================================
// @brief Sets the title for the surface
//
//=============================================
void CUIWindow::setTitle( const Char* pstrTitle, const font_set_t* pTitleFont, Uint32 insetX, Uint32 insetY )
{
	if(!m_pTitle)
	{
		Int32 flags = (UIEL_FL_ALIGN_L|UIEL_FL_ALIGN_T);
		m_pTitle = new CUIText(flags, pTitleFont, pstrTitle, insetX, insetY);
		m_pTitle->setParent(this);
	}
	else
	{
		m_pTitle->setText(pstrTitle);
		if(insetX && insetY)
			m_pTitle->setPosition(insetX, insetY);
	}
}

//=============================================
// @brief Moves the window by x and y amount
//
//=============================================
void CUIWindow::move( Int32 x, Int32 y )
{
	// Just add the offsets
	m_originX += x;
	m_originY += y;
}

//=============================================
// @brief Moves the window by x and y amount
//
//=============================================
bool CUIWindow::keyEvent( Int32 button, Int16 mod, bool keyDown )
{
	if(m_pInputFocusObject)
		return m_pInputFocusObject->keyEvent(button, mod, keyDown);

	// We shouldn't let inputs escape
	return true;
}

//=============================================
// @brief Moves the window by x and y amount
//
//=============================================
bool CUIWindow::mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown )
{
	if(CUISurface::mouseButtonEvent(mouseX, mouseY, button, keyDown))
		return true;

	// We shouldn't let inputs escape
	return true;
}

//=============================================
// @brief Moves the window by x and y amount
//
//=============================================
bool CUIWindow::mouseWheelEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown, Int32 scroll )
{
	if(CUISurface::mouseWheelEvent(mouseX, mouseY, button, keyDown, scroll))
		return true;

	// We shouldn't let inputs escape
	return true;
}

//=============================================
// @brief Performs the close action for the window
//
//=============================================
void CUIWindow::CUIWindowCloseAction::PerformAction( Float param )
{
	if(m_pWindow)
		m_pWindow->setWindowFlags(UIW_FL_KILLME);
}

//=============================================
// @brief Constructor
//
//=============================================
CUIText::CUIText( Int32 flags, const font_set_t* pFont, const Char* pstrText, Int32 originx, Int32 originy, Uint32 maxWidth ):
	CUIObject( (flags|UIEL_FL_FIXED_W|UIEL_FL_FIXED_H), 0, 0, originx, originy ),
	m_pFont(pFont),
	m_maxTextWidth(maxWidth)
{
	setText(pstrText);
}

//=============================================
// @brief Destructor
//
//=============================================
CUIText::~CUIText( void )
{
}

//=============================================
// @brief Sets the string to display
//
//=============================================
void CUIText::setText( const Char* pstrText )
{
	m_string = pstrText;

	if(!m_string.empty())
	{
		g_engineFuncs.pfnGetStringSize(m_pFont, pstrText, &m_width, &m_height, nullptr);

		// Modify these as well
		m_baseWidth = m_width;
		m_baseHeight = m_height;
	}
	else
	{
		m_baseWidth = m_width = 0;
		m_baseHeight = m_height = 0;
	}

	adjustPosition();
}

//=============================================
// @brief Sets the string to display
//
//=============================================
void CUIText::setDisplayText( const Char* pstrText )
{
	m_displayText.clear();
	
	if(m_maxTextWidth)
	{
		// Determine the width of 3 dots
		Int32 dotsWidth = m_pFont->glyphs['.'].advancex;
		dotsWidth *= 3;

		// Determine string length
		Uint32 strWidth = 0;
		Uint32 lastDotWidthChar = 0;
		const Char* pstr = pstrText;
		while(*pstr)
		{
			Int32 charIdx = (byte)(*pstr);
			Int32 charWidth = m_pFont->glyphs[charIdx].advancex;

			if((strWidth+charWidth) > m_maxTextWidth)
			{
				m_displayText.assign(pstrText, lastDotWidthChar-1);
				m_displayText << "...";
				return;
			}

			if((strWidth+dotsWidth) <= m_maxTextWidth)
				lastDotWidthChar++;

			strWidth += charWidth;
			pstr++;
		}
	}

	m_displayText = pstrText;
}

//=============================================
// @brief Draws the string
//
//=============================================
bool CUIText::draw( void )
{
	if(m_string.empty())
		return true;

	// Not happy about so many state switches
	g_engineFuncs.pfnDisableBasicDraw();

	// Determine position to draw in
	Int32 ymin;
	Uint32 width, height;
	g_engineFuncs.pfnGetStringSize(m_pFont, m_string.c_str(), &width, &height, &ymin);

	Int32 absOrgX, absOrgY;
	getAbsPosition(absOrgX, absOrgY);

	if(!g_engineFuncs.pfnDrawSimpleString(m_color, absOrgX, absOrgY+height, m_displayText.c_str(), m_pFont))
		return false;

	// Re-enable basic drawing
	if(!g_engineFuncs.pfnEnableBasicDraw())
		return false;

	CMatrix& projection = g_engineFuncs.pfnGetProjectionMatrix();
	CMatrix& modelview = g_engineFuncs.pfnGetModelViewMatrix();

	// Restore matrices
	g_engineFuncs.pfnBasicDrawSetProjection(projection.GetMatrix());
	g_engineFuncs.pfnBasicDrawSetModelView(modelview.GetMatrix());

	// Re-enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return true;
}

//=============================================
// @brief Draws the string
//
//=============================================
void CUIText::adjustPosition( void )
{
	// Run base functions
	CUIObject::adjustPosition();

	// Update the text displayed
	setDisplayText(m_string.c_str());
}

//=============================================
// @brief Constructor
//
//=============================================
CUIDragger::CUIDragger( Int32 flags, Uint32 width, Uint32 height, Int32 originx, Int32 originy ):
	CUIObject( (flags|UIEL_FL_FIXED_H), width, height, originx, originy ),
	m_lastMouseX(0),
	m_lastMouseY(0),
	m_isClickedOn(false)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CUIDragger::~CUIDragger( void )
{
}

//=============================================
// @brief Sets the string to display
//
//=============================================
bool CUIDragger::mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown )
{
	// Only left mouse click affects draggers
	if(button != SDL_BUTTON_LEFT)
		return false;

	bool mouseOver = isMouseOver(mouseX, mouseY);
	if(mouseOver && keyDown)
	{
		if(!m_isClickedOn)
		{
			m_lastMouseX = mouseX;
			m_lastMouseY = mouseY;
			m_isClickedOn = true;
		}

		return true;
	}
	else if(m_isClickedOn)
	{
		m_lastMouseX = 0;
		m_lastMouseY = 0;
		m_isClickedOn = false;

		if(mouseOver)
			return true;
	}

	return false;
}

//=============================================
// @brief Tells if the parent size is valid for us
//
//=============================================
bool CUIDragger::isParentSizeValid( Uint32 testWidth, Uint32 testHeight, Int32 adjX, Int32 adjY )
{
	// Determine my adjusted size
	Int32 myTestWidth = (Int32)m_width;
	Int32 myTestHeight = (Int32)m_height;

	// Add it to the width/height
	if(!(m_flags & UIEL_FL_FIXED_W))
		myTestWidth += adjX;

	if(!(m_flags & UIEL_FL_FIXED_H))
		myTestHeight += adjY;

	// Determine if we're being inverted on ourselves width/height
	if(myTestWidth < 0 || myTestHeight < 0)
		return false;

	// Adjust origins for test
	Int32 testOriginX;
	if(m_flags & UIEL_FL_ALIGN_R)
		testOriginX = testWidth - m_baseOriginX - m_width;
	else
		testOriginX = m_baseOriginX;

	Int32 testOriginY;
	if(m_flags & UIEL_FL_ALIGN_B)
		testOriginY = testHeight - m_baseOriginY - m_height;
	else
		testOriginY = m_baseOriginY;

	// Compare against parent size too
	Int32 fullWidth = myTestWidth + testOriginX;
	Int32 fullHeight = myTestHeight + testOriginY;
	if(fullWidth > (Int32)testWidth || fullHeight > (Int32)testHeight)
		return false;

	return true;
}

//=============================================
// @brief Draws the string
//
//=============================================
void CUIDragger::think( void )
{
	if(m_isClickedOn)
	{
		Int32 xPos, yPos;
		g_engineFuncs.pfnGetMousePosition(xPos, yPos);

		// Move if the cursor moved
		if(xPos != m_lastMouseX || yPos != m_lastMouseY)
		{
			Int32 deltaX = xPos - m_lastMouseX;
			Int32 deltaY = yPos - m_lastMouseY;

			if(performAdj(deltaX, deltaY))
			{
				m_lastMouseX = xPos;
				m_lastMouseY = yPos;
			}
		}
	}

	// Call base to handle the rest
	CUIObject::think();
}

//=============================================
// @brief Performs the required adjustment
//
//=============================================
bool CUIDragger::performAdj( Int32 x, Int32 y )
{
	m_pParent->move(x, y);
	return true;
}

//=============================================
// @brief Constructor
//
//=============================================
CUIResizer::CUIResizer( Int32 flags, Uint32 width, Uint32 height, Int32 originx, Int32 originy ):
	CUIDragger( (flags|UIEL_FL_FIXED_H|UIEL_FL_FIXED_W), width, height, originx, originy )
{
}

//=============================================
// @brief Destructor
//
//=============================================
CUIResizer::~CUIResizer( void )
{
}

//=============================================
// @brief Performs the required adjustment
//
//=============================================
bool CUIResizer::performAdj( Int32 x, Int32 y )
{
	return m_pParent->adjustSize(x, y);
}

//=============================================
// @brief Constructor
//
//=============================================
CUIButton::CUIButton( Int32 flags, const Char* pstrText, const font_set_t* pFont, CUICallbackEvent* pAction, Uint32 width, Uint32 height, Int32 originx, Int32 originy ):
	CUISurface( (flags|UIEL_FL_FIXED_H|UIEL_FL_FIXED_W), width, height, originx, originy ),
	m_pDisplay(nullptr),
	m_isClickedOn(false),
	m_pAction(pAction),
	m_isDisabled(false)
{
	assert(pstrText && qstrlen(pstrText));

	// Use default font for now
	m_pDisplay = new CUIText((UIEL_FL_ALIGN_CH|UIEL_FL_ALIGN_CV|UIEL_FL_ONTOP), pFont, pstrText, 0, 0);
	m_pDisplay->setParent(this);
}

//=============================================
// @brief Constructor
//
//=============================================
CUIButton::CUIButton( Int32 flags, const ui_schemeobject_t* pScheme, CUICallbackEvent* pAction, Uint32 width, Uint32 height, Int32 originx, Int32 originy ):
	CUISurface( (flags|UIEL_FL_FIXED_H|UIEL_FL_FIXED_W), width, height, originx, originy ),
	m_pDisplay(nullptr),
	m_isClickedOn(false),
	m_pAction(pAction),
	m_isDisabled(false)
{
	assert(pScheme);

	// Create icon object
	CUITexturedObject* pDisplay = new CUITexturedObject((UIEL_FL_ALIGN_CH|UIEL_FL_ALIGN_CV|UIEL_FL_ONTOP), pScheme->width, pScheme->height, 0, 0);
	m_pDisplay = pDisplay;
	
	// Set textures
	pDisplay->setDefaultTexture(pScheme->defaultTexture);
	pDisplay->setFocusTexture(pScheme->focusTexture);
	pDisplay->setClickedTexture(pScheme->clickTexture);

	m_pDisplay->setParent(this);
}

//=============================================
// @brief Constructor
//
//=============================================
CUIButton::CUIButton( Int32 flags, CUICallbackEvent* pAction, Uint32 width, Uint32 height, Int32 originx, Int32 originy ):
	CUISurface( (flags), width, height, originx, originy ),
	m_pDisplay(nullptr),
	m_isClickedOn(false),
	m_pAction(pAction),
	m_isDisabled(false)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CUIButton::~CUIButton( void )
{
	if(m_pAction)
		delete m_pAction;
}

//=============================================
// @brief Sets the string to display
//
//=============================================
bool CUIButton::mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown )
{
	if(m_isDisabled)
		return false;

	// Only left mouse click affects draggers
	if(button != SDL_BUTTON_LEFT)
		return false;

	if(isMouseOver(mouseX, mouseY))
	{
		if(keyDown)
		{
			if(!m_isClickedOn)
			{
				m_isClickedOn = true;
			}
		}
		else if(m_isClickedOn)
		{
			// Peform the action
			if(m_pAction)
				m_pAction->PerformAction(0);

			m_isClickedOn = false;
		}

		return true;
	}

	return false;
}

//=============================================
// @brief Performs think functions
//
//=============================================
void CUIButton::think( void )
{
	if(m_isClickedOn)
	{
		Int32 xPos, yPos;
		g_engineFuncs.pfnGetMousePosition(xPos, yPos);

		if(!isMouseOver(xPos, yPos))
			m_isClickedOn = false;
	}

	// Call base to handle the rest
	CUIObject::think();
}

//=============================================
// @brief Constructor
//
//=============================================
CUITextTab::CUITextTab( Int32 flags, const font_set_t* pFont, Uint32 width, Uint32 height, Int32 originx, Int32 originy ):
	CUISurface( flags, width, height, originx, originy ),
	m_pText(nullptr),
	m_pFont(pFont),
	m_pScroller(nullptr),
	m_textOffset(0),
	m_inset(0)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CUITextTab::~CUITextTab( void )
{
}

//=============================================
// @brief Sets the source text array to render text from
//
//=============================================
void CUITextTab::setText( const Char* pstrText )
{
	m_pText = pstrText;
	updateRangeSize();
}

//=============================================
// @brief Sets the source text array to render text from
//
//=============================================
void CUITextTab::updateRangeSize( void )
{
	if(!m_pText)
	{
		m_pScroller->setFullRange(0);
		return;
	}

	// Re-estimate the height
	g_engineFuncs.pfnSetStringRectangle(0, 0, m_width - m_pScroller->getWidth(), m_height, m_inset, m_inset);
	Int32 textHeight = g_engineFuncs.pfnEstimateStringHeight(m_pFont, m_pText, m_pFont->fontsize);
	m_pScroller->setFullRange(textHeight);

	// Reset rectangle
	g_engineFuncs.pfnSetStringRectangle(0, 0, 0, 0, 0, 0);
}

//=============================================
// @brief Sets the source text array to render text from
//
//=============================================
bool CUITextTab::mouseWheelEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown, Int32 scroll )
{
	if(!isMouseOver(mouseX, mouseY))
		return false;

	if(keyDown && m_pScroller->moveScroller(button, scroll))
		return true;
	else
		return false;
}

//=============================================
// @brief Sets the source text array to render text from
//
//=============================================
bool CUITextTab::init( const Char* pstrSchemaName )
{
	// Call base class to initialize basics
	if(!CUISurface::init(pstrSchemaName))
		return false;

	// Create the scroller
	Int32 flags = UIEL_FL_NONE;
	if(m_flags & UIEL_FL_SCR_REVERSE)
		flags |= UIEL_FL_SCR_REVERSE;

	m_pScroller = new CUIScroller(flags, UIEL_SCROLL_V, m_pFont->fontsize, 0, m_height, 0, 0);
	m_pScroller->setParent(this);

	if(!m_pScroller->init("defaultscroller.txt"))
	{
		g_engineFuncs.pfnCon_EPrintf("Failed to initialize scroller with schema file 'defaultscoller.txt'.\n");
		return false;
	}

	return true;
}

//=============================================
// @brief Draws the element, and the string
//
//=============================================
bool CUITextTab::draw( void )
{
	// Call base class to render the parts
	if(!CUISurface::draw())
		return false;

	// Draw the text itself
	if(m_pText && qstrlen(m_pText))
	{
		// Not happy about so many state switches
		g_engineFuncs.pfnDisableBasicDraw();

		Int32 absOrgX, absOrgY;
		getAbsPosition(absOrgX, absOrgY);

		// Set drawing rectangle
		color32_t color(255, 255, 255, 255);

		if(!g_engineFuncs.pfnDrawStringBox(0, 0, m_width - m_pScroller->getWidth(), m_height, m_inset, m_inset, false, color, absOrgX, absOrgY, m_pText, m_pFont, m_textOffset, m_pFont->fontsize, 0))
			return false;

		// Re-enable basic drawing
		if(!g_engineFuncs.pfnEnableBasicDraw())
			return false;

		CMatrix& projection = g_engineFuncs.pfnGetProjectionMatrix();
		CMatrix& modelview = g_engineFuncs.pfnGetModelViewMatrix();

		// Restore matrices
		g_engineFuncs.pfnBasicDrawSetProjection(projection.GetMatrix());
		g_engineFuncs.pfnBasicDrawSetModelView(modelview.GetMatrix());

		// Re-enable blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	return true;
}

//=============================================
// @brief Sets the offset value for the element
//
//=============================================
void CUITextTab::setOffsetValue( Float offset )
{
	if(!m_pText)
		return;

	g_engineFuncs.pfnSetStringRectangle(0, 0, m_width - m_pScroller->getWidth(), m_height, m_inset, m_inset);
	Float textHeight = g_engineFuncs.pfnEstimateStringHeight(m_pFont, m_pText, m_pFont->fontsize);
	g_engineFuncs.pfnSetStringRectangle(0, 0, 0, 0, 0, 0);

	if(textHeight < m_height)
	{
		m_textOffset = 0;
		return;
	}

	Uint32 totalHeight = textHeight-(m_height-m_pFont->fontsize-m_inset*2);
	Uint32 nbElements = totalHeight/m_pFont->fontsize;
	m_textOffset = nbElements*offset;
}

//=============================================
// @brief Constructor
//
//=============================================
CUITextInputTab::CUITextInputTab( Int32 flags, CUICallbackEvent* pAction, const font_set_t* pFont, Uint32 width, Uint32 height, Int32 originx, Int32 originy ):
	CUISurface( flags, width, height, originx, originy ),
	m_pFont(pFont),
	m_nextBlinkTime(0),
	m_drawMarker(true),
	m_inset(0),
	m_inputPosition(0),
	m_pAction(pAction)
{
	// Clear this
	m_szBuffer[0] = '\0';
}

//=============================================
// @brief Destructor
//
//=============================================
CUITextInputTab::~CUITextInputTab( void )
{
	if(m_pAction)
		delete m_pAction;
}

//=============================================
// @brief Destructor
//
//=============================================
bool CUITextInputTab::mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown )
{
	// Only left mouse click affects this
	if(button != SDL_BUTTON_LEFT)
		return false;

	if(isMouseOver(mouseX, mouseY))
	{
		// Set us as the input focus
		setWindowInputFocusObject(this);
		return true;
	}

	return false;
}

//=============================================
// @brief Destructor
//
//=============================================
bool CUITextInputTab::keyEvent( Int32 button, Int16 mod, bool keyDown )
{
	if(!keyDown)
		return true;

	// Get SDL Keycode
	SDL_Keycode sdlKeycode = SDL_GetKeyFromScancode((SDL_Scancode)button);
	
	switch(sdlKeycode)
	{
    case SDLK_KP_1:
		sdlKeycode = SDLK_1;
		break;
    case SDLK_KP_2:
		sdlKeycode = SDLK_2;
		break;
    case SDLK_KP_3:
		sdlKeycode = SDLK_3;
		break;
    case SDLK_KP_4:
		sdlKeycode = SDLK_4;
		break;
    case SDLK_KP_5:
		sdlKeycode = SDLK_5;
		break;
    case SDLK_KP_6:
		sdlKeycode = SDLK_6;
		break;
    case SDLK_KP_7:
		sdlKeycode = SDLK_7;
		break;
    case SDLK_KP_8:
		sdlKeycode = SDLK_8;
		break;
    case SDLK_KP_9:
		sdlKeycode = SDLK_9;
		break;
	case SDLK_KP_0:
		sdlKeycode = SDLK_0;
		break;
	case SDLK_KP_PLUS:
		sdlKeycode = SDLK_PLUS;
		break;
	case SDLK_KP_MINUS:
		sdlKeycode = SDLK_MINUS;
		break;
	case SDLK_KP_MULTIPLY:
		sdlKeycode = SDLK_ASTERISK;
		break;
	case SDLK_KP_DIVIDE:
		sdlKeycode = SDLK_SLASH;
		break;
	}

	// See if it's a valid text input character
	if(sdlKeycode >= SDLK_SPACE && sdlKeycode <= SDLK_z)
	{
		// Avoid buffer over-indexing
		if(m_inputPosition == MAX_INPUT_LENGTH)
			return true;

		// Shift if needed
		Char inputChar = (Char)sdlKeycode;
		if(mod & (KMOD_SHIFT|KMOD_CAPS))
			inputChar = Common::GetShiftedChar(inputChar);

		// Shift elements if needed
		if(m_szBuffer[m_inputPosition] != '\0')
		{
			// Insert it inbetween
			Uint32 bufLength = qstrlen(m_szBuffer);
			Char *psrc = m_szBuffer + bufLength;
			Char *pdst = m_szBuffer + m_inputPosition - 1;

			while(psrc != pdst)
			{
				Char *pcdst = psrc+1;
				*pcdst = *psrc;

				psrc--;
			}

			m_szBuffer[m_inputPosition] = inputChar;
		}
		else
		{
			// Set the input in the array
			m_szBuffer[m_inputPosition] = inputChar;
			m_szBuffer[m_inputPosition+1] = '\0';
		}

		// Always draw after input is added
		m_nextBlinkTime = g_engineFuncs.pfnGetEngineTime() + 0.5;
		m_drawMarker = true;

		m_inputPosition++;
	}
	else if(sdlKeycode == SDLK_LEFT)
	{
		if(m_inputPosition > 0)
		{
			// Always draw after input is added
			m_nextBlinkTime = g_engineFuncs.pfnGetEngineTime() + 0.5;
			m_drawMarker = true;

			m_inputPosition--;
		}
	}
	else if(sdlKeycode == SDLK_RIGHT)
	{
		if(m_szBuffer[m_inputPosition] != '\0')
		{
			// Always draw after input is added
			m_nextBlinkTime = g_engineFuncs.pfnGetEngineTime() + 0.5;
			m_drawMarker = true;

			m_inputPosition++;
		}
	}
	else if(sdlKeycode == SDLK_BACKSPACE)
	{
		if(m_inputPosition)
		{
			if(m_szBuffer[m_inputPosition] != '\0')
			{
				// Remove inbetween
				Uint32 bufLength = qstrlen(m_szBuffer);
				Char *psrc = m_szBuffer + m_inputPosition - 1;
				Char *pdst = m_szBuffer + bufLength;

				while(psrc != pdst)
				{
					Char *pcsrc = psrc+1;
					*psrc = *pcsrc;

					psrc++;
				}
			}
			else
			{
				Char* pstr = m_szBuffer+m_inputPosition;
				*(pstr-1) = *pstr;
			}

			m_inputPosition--;
		}
	}

	// Sent a key event to the handler
	if(m_pAction)
		m_pAction->KeyEvent(button, mod, keyDown);

	return true;
}

//=============================================
// @brief Destructor
//
//=============================================
void CUITextInputTab::clearText( void )
{
	m_szBuffer[0] = '\0';
	m_inputPosition = 0;
}

//=============================================
// @brief Sets the contents of the text buffer
//
//=============================================
void CUITextInputTab::setText( const Char* pstrText )
{
	if(!pstrText)
	{
		m_inputPosition = 0;
		m_szBuffer[0] = '\0';
		return;
	}

	qstrcpy(m_szBuffer, pstrText);
	m_inputPosition = qstrlen(pstrText);
}

//=============================================
// @brief Peforms think functions
//
//=============================================
void CUITextInputTab::think( void )
{
	// Call base class to handle basics
	CUISurface::think();

	// Time input market
	if(g_engineFuncs.pfnGetEngineTime() >= m_nextBlinkTime)
	{
		m_nextBlinkTime = g_engineFuncs.pfnGetEngineTime() + 0.5;
		m_drawMarker = !m_drawMarker;
	}
}

//=============================================
// @brief Destructor
//
//=============================================
bool CUITextInputTab::draw( void )
{
	// Call base class to handle rendering
	if(!CUISurface::draw())
		return false;

	// Two pixels for now
	Int32 markerWidth = 2;

	// For the marker
	Char* pstr = nullptr;
	Uint32 numChars = 0;

	if(m_szBuffer[0] != '\0')
	{
		// Not happy about so many state switches
		g_engineFuncs.pfnDisableBasicDraw();

		// Determine position to draw in
		Uint32 textHeight = g_engineFuncs.pfnEstimateStringHeight(m_pFont, m_szBuffer, 0);

		Int32 absOrgX, absOrgY;
		getAbsPosition(absOrgX, absOrgY);

		// Draw in the vertical center
		Uint32 yOrg = absOrgY + m_height/2 + textHeight/2;

		// Draw only the parts that fit in
		Uint32 stringWidth = 0;
		numChars = qstrlen(m_szBuffer)-1;
		pstr = m_szBuffer+numChars;
		while(numChars > 0)
		{
			Uint32 charWidth = m_pFont->glyphs[*pstr].advancex;
			if(m_inset+charWidth+stringWidth+markerWidth >= (m_width-m_inset))
			{
				pstr++;
				break;
			}

			stringWidth += charWidth;

			numChars--;
			pstr--;
		}

		// Draw the required chars
		color32_t color(255, 255, 255, 255);
		if(!g_engineFuncs.pfnDrawSimpleString(color, absOrgX+m_inset, yOrg, pstr, m_pFont))
			return false;

		// Re-enable basic drawing
		if(!g_engineFuncs.pfnEnableBasicDraw())
			return false;

		CMatrix& projection = g_engineFuncs.pfnGetProjectionMatrix();
		CMatrix& modelview = g_engineFuncs.pfnGetModelViewMatrix();

		// Restore matrices
		g_engineFuncs.pfnBasicDrawSetProjection(projection.GetMatrix());
		g_engineFuncs.pfnBasicDrawSetModelView(modelview.GetMatrix());

		// Re-enable blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	// Draw marker if needed
	if(m_drawMarker && m_pParent->getFocusState())
	{
		Int32 absOrgX, absOrgY;
		getAbsPosition(absOrgX, absOrgY);

		Int32 xOrg = absOrgX + m_inset;
		Int32 yOrg = absOrgY - m_height/2 + m_pFont->fontsize;
		Int32 markerHeight = m_pFont->fontsize;

		// Determine offset
		if(pstr != nullptr)
		{
			if(m_inputPosition < numChars)
				return true;

			Uint32 nbChars = m_inputPosition - numChars;
			for(Uint32 i = 0; i < nbChars; i++)
				xOrg += m_pFont->glyphs[pstr[i]].advancex;
		}

		if(!g_engineFuncs.pfnBasicDrawDisableTextures())
			return false;

		g_engineFuncs.pfnBasicDrawColor4f(1.0, 1.0, 1.0, 1.0);

		g_engineFuncs.pfnValidateBasicDraw();

		g_engineFuncs.pfnBasicDrawBegin(GL_TRIANGLES);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xOrg, (Float)yOrg + markerHeight, -1);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xOrg, (Float)yOrg, -1);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xOrg+markerWidth, (Float)yOrg, -1);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xOrg, (Float)yOrg + markerHeight, -1);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xOrg+markerWidth, (Float)yOrg, -1);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xOrg+markerWidth, (Float)yOrg+markerHeight, -1);
		g_engineFuncs.pfnBasicDrawEnd();

		if(!g_engineFuncs.pfnBasicDrawEnableTextures())
			return false;
	}

	return true;
}

//=============================================
// @brief Constructor
//
//=============================================
CUIDragButton::CUIDragButton( Int32 flags, ui_scroller_align_t alignment, CUICallbackEvent* pAction, Uint32 width, Uint32 height, Int32 originx, Int32 originy ):
	CUIButton( flags, pAction, width, height, originx, originy ),
	m_alignment(alignment),
	m_lastMouseX(0),
	m_lastMouseY(0),
	m_startInset(0),
	m_endInset(0),
	m_position(0),
	m_lastParentLength(0)
{
	if(alignment == UIEL_SCROLL_V)
		m_flags |= UIEL_FL_FIXED_W | UIEL_FL_ALIGN_R;
	else
		m_flags |= UIEL_FL_FIXED_H | UIEL_FL_ALIGN_B;
}

//=============================================
// @brief Destructor
//
//=============================================
CUIDragButton::~CUIDragButton( void )
{
}

//=============================================
// @brief Sets the string to display
//
//=============================================
void CUIDragButton::setBounds( Int32 start, Int32 end )
{
	m_startInset = start;
	m_endInset = end;
}

//=============================================
// @brief Sets the string to display
//
//=============================================
void CUIDragButton::getBounds( Int32& start, Int32& end )
{
	start = m_startInset;
	end = m_endInset;
}

//=============================================
// @brief Sets the string to display
//
//=============================================
bool CUIDragButton::mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown )
{
	// Only left mouse click affects draggers
	if(button != SDL_BUTTON_LEFT)
		return false;

	if(isMouseOver(mouseX, mouseY))
	{
		if(keyDown)
		{
			if(!m_isClickedOn)
			{
				m_lastMouseX = mouseX;
				m_lastMouseY = mouseY;
				m_isClickedOn = true;
			}

			return true;
		}
	}

	if(m_isClickedOn && !keyDown)
	{
		// Peform the action
		if(m_pAction)
			m_pAction->PerformAction(0);

		m_isClickedOn = false;
		m_lastMouseX = 0;
		m_lastMouseY = 0;
	}

	return false;
}

//=============================================
// @brief Performs think functions
//
//=============================================
void CUIDragButton::think( void )
{
	if(m_isClickedOn)
	{
		Int32 xPos, yPos;
		g_engineFuncs.pfnGetMousePosition(xPos, yPos);

		Int32 prevVal;
		Int32 curVal;
		if(m_alignment == UIEL_SCROLL_V)
		{
			prevVal = m_lastMouseY;
			curVal = yPos;
		}
		else
		{
			prevVal = m_lastMouseX;
			curVal = xPos;
		}

		// Peform the action
		if(curVal != prevVal)
		{
			Int32 delta = curVal - prevVal;
			if(adjPosition(delta, true))
			{
				m_lastMouseX = xPos;
				m_lastMouseY = yPos;
			}

			if(m_pAction)
				m_pAction->PerformAction(0);
		}
	}

	// Call base to handle the rest
	CUIObject::think();
}

//=============================================
// @brief Sets the length of the drag button
//
//=============================================
void CUIDragButton::setLength( Uint32 length )
{
	if(m_alignment == UIEL_SCROLL_V)
	{
		Int32 finalHeight = length - m_startInset - m_endInset;
		m_height = _min(0, finalHeight);

		Uint32 minHeight = m_pTopBorder->getHeight() + m_pBottomBorder->getHeight();
		if(m_height < minHeight)
			m_height = minHeight;
	}
	else
	{
		Int32 finalWidth = length - m_startInset - m_endInset;
		m_width = _min(0, finalWidth);

		Uint32 minWidth = m_pLeftBorder->getWidth() + m_pRightBorder->getWidth();
		if(m_width < minWidth)
			m_width = minWidth;
	}

	adjustBgElements();
}

//=============================================
// @brief Sets the position of the drag button
//
//=============================================
void CUIDragButton::setPosition( Float position )
{
	m_position = clamp(position, 0.0, 1.0);
}

//=============================================
// @brief Gets the position of the drag button
//
//=============================================
Double CUIDragButton::getPosition( void )
{
	return m_position;
}

//=============================================
// @brief Sets the position of the drag button
//
//=============================================
bool CUIDragButton::adjPosition( Int32 adjAmt, bool isMouseDrag, bool callEvent )
{
	if(m_position == 1.0 && adjAmt > 0)
		return false;
	else if(m_position == 0.0 && adjAmt < 0)
		return false;

	// Get parent length
	Uint32 parentLength;
	if(m_alignment == UIEL_SCROLL_V)
		parentLength = m_pParent->getHeight();
	else
		parentLength = m_pParent->getWidth();

	// Determine my own range
	Int32 myRange = parentLength-getLength()-m_startInset-m_endInset;
	
	// No range, just set to base
	if(!myRange)
	{
		// Set appropriate position
		if(m_alignment == UIEL_SCROLL_V)
			m_originY = m_startInset;
		else
			m_originX = m_startInset;

		return true;
	}

	Uint32 referenceRange;
	if(isMouseDrag)
		referenceRange = myRange;
	else
		referenceRange = m_pParent->getFullRange();

	// Make adjustments
	Double adjFrac = (Double)adjAmt/(Double)(referenceRange);

	m_position += adjFrac;
	m_position = clamp(m_position, 0.0, 1.0);

	// Set appropriate position
	if(m_alignment == UIEL_SCROLL_V)
		m_originY = m_startInset + m_position*myRange;
	else
		m_originX = m_startInset + m_position*myRange;

	// Call the action to reset the position
	if(callEvent && m_pAction)
		m_pAction->PerformAction(0);

	// Remember last parent length
	m_lastParentLength = parentLength;
	return true;
}

//=============================================
// @brief Gets the length of the drag button
//
//=============================================
Uint32 CUIDragButton::getLength( void )
{
	if(m_alignment == UIEL_SCROLL_V)
		return m_height;
	else
		return m_width;
}

//=============================================
// @brief Repositions the object after a parent's size is changed
//
//=============================================
void CUIDragButton::adjustPosition( void )
{
	// Call base class first
	CUIButton::adjustPosition();

	if(m_lastParentLength)
	{
		// Reset the position
		Uint32 parentLength;
		if(m_alignment == UIEL_SCROLL_V)
			parentLength = m_pParent->getHeight();
		else
			parentLength = m_pParent->getWidth();

		// Determine previous full position
		Double prevRelativePosition = m_position*m_lastParentLength;
		m_lastParentLength = parentLength;

		// Determine current position based on this
		Double currentPosition = (Double)prevRelativePosition/(Double)parentLength;
		m_position = clamp(currentPosition, 0.0, 1.0);

		// Determine my own range
		Int32 myRange = parentLength-getLength()-m_startInset-m_endInset;

		// Set appropriate position
		Double offAdj = m_position*myRange;
		if(!offAdj)
			m_position = 0;

		if(m_alignment == UIEL_SCROLL_V)
			m_originY = m_startInset + offAdj;
		else
			m_originX = m_startInset + offAdj;

		// Call the action to reset the position
		if(m_pAction)
			m_pAction->PerformAction(0);
	}
}

//=============================================
// @brief Performs the close action for the window
//
//=============================================
void CUIScroller::CUIScrollerArrowBtnAction::PerformAction( Float param )
{
	assert(m_pScroller != nullptr);

	// Get the dragger button
	CUIDragButton* pButton = m_pScroller->getDragButton();
	Int32 adjAmt = m_jumpSize * m_direction;

	pButton->adjPosition(adjAmt, false);
}

//=============================================
// @brief Performs the close action for the window
//
//=============================================
void CUIScroller::CUIScrollerDragBtnAction::PerformAction( Float param )
{
	assert(m_pScroller != nullptr);

	// Get the dragger button
	CUIDragButton* pButton = m_pScroller->getDragButton();
	Float position = pButton->getPosition();

	// Get parent of the scroller button
	CUIObject* pParent = m_pScroller->getParent();
	if(pParent)
		pParent->setOffsetValue(position);
}

//=============================================
// @brief Constructor
//
//=============================================
CUIScroller::CUIScroller( Int32 flags, ui_scroller_align_t alignment, Uint32 unitSize, Uint32 width, Uint32 height, Int32 originx, Int32 originy ):
	CUITexturedObject( flags, width, height, originx, originy ),
	m_pButtonStart(nullptr),
	m_pButtonEnd(nullptr),
	m_pDragButton(nullptr),
	m_alignment(alignment),
	m_fullRangeSize(0),
	m_prevFullRangeSize(0),
	m_unitSize(unitSize),
	m_pScheme(nullptr)
{
	if(alignment == UIEL_SCROLL_V)
		m_flags |= UIEL_FL_FIXED_W | UIEL_FL_ALIGN_R;
	else
		m_flags |= UIEL_FL_FIXED_H | UIEL_FL_ALIGN_B;
}

//=============================================
// @brief Destructor
//
//=============================================
CUIScroller::~CUIScroller( void )
{
}

//=============================================
// @brief Sets the source text array to render text from
//
//=============================================
bool CUIScroller::init( const Char* pstrSchemaName )
{
	m_pScheme = g_engineFuncs.pfnUILoadSchemaFile(pstrSchemaName);
	if(!m_pScheme)
		return false;

	// Set background for this element
	const ui_schemeobject_t* pObject = m_pScheme->getObject("ScrollerBase");
	if(!pObject)
	{
		g_engineFuncs.pfnCon_EPrintf("Schema file %s is missing 'ScrollerBase'.\n", pstrSchemaName);
		return false;
	}

	m_pDefaultTexture = pObject->defaultTexture;

	// Final height/width to set
	Int32 finalWidth = -1;
	Int32 finalHeight = -1;

	// Set based on alignment
	CString objectName;
	Int32 flags = UIEL_FL_NONE;
	if(m_alignment == UIEL_SCROLL_V)
	{
		flags |= UIEL_FL_ALIGN_T;
		objectName = "ArrowUp";
		finalWidth = pObject->width;
	}
	else
	{
		flags |= UIEL_FL_ALIGN_L;
		objectName = "ArrowLeft";
		finalHeight = pObject->height;
	}

	pObject = m_pScheme->getObject(objectName.c_str());
	if(!pObject)
	{
		g_engineFuncs.pfnCon_EPrintf("Schema file %s is missing 'ScrollerBase'.\n", pstrSchemaName);
		return false;
	}

	// Create the start button
	CUIScrollerArrowBtnAction* pAction = new CUIScrollerArrowBtnAction(this, -1, m_unitSize);
	m_pButtonStart = new CUIButton(flags, pObject, pAction, pObject->width, pObject->height, 0, 0);
	m_pButtonStart->setParent(this);

	if(!m_pButtonStart->init(pstrSchemaName))
	{
		g_engineFuncs.pfnCon_EPrintf("Failed to initialize button for scroller with schema file '%s'.\n", pstrSchemaName);
		return false;
	}

	// Set based on alignment
	flags = UIEL_FL_NONE;
	if(m_alignment == UIEL_SCROLL_V)
	{
		flags |= UIEL_FL_ALIGN_B;
		objectName = "ArrowDown";

		if((Int32)pObject->width > finalWidth)
			finalWidth = pObject->width;
	}
	else
	{
		flags |= UIEL_FL_ALIGN_R;
		objectName = "ArrowRight";

		if((Int32)pObject->height > finalHeight)
			finalHeight = pObject->height;
	}

	pObject = m_pScheme->getObject(objectName.c_str());
	if(!pObject)
	{
		g_engineFuncs.pfnCon_EPrintf("Schema file %s is missing 'ScrollerBase'.\n", pstrSchemaName);
		return false;
	}

	// Create the start button
	pAction = new CUIScrollerArrowBtnAction(this, 1, m_unitSize);
	m_pButtonEnd = new CUIButton(flags, pObject, pAction, pObject->width, pObject->height, 0, 0);
	m_pButtonEnd->setParent(this);

	if(!m_pButtonEnd->init(pstrSchemaName))
	{
		g_engineFuncs.pfnCon_EPrintf("Failed to initialize button for scroller with schema file '%s'.\n", pstrSchemaName);
		return false;
	}

	// Set if needed
	if(finalWidth != -1)
		m_width = finalWidth;

	if(finalHeight != -1)
		m_height = finalHeight;

	// Adjust positions
	m_pButtonStart->adjustPosition();
	m_pButtonEnd->adjustPosition();

	// Middle button is a bit more complicated
	flags = UIEL_FL_NONE;
	Uint32 dragBtnWidth = 0;
	Uint32 dragBtnHeight = 0;
	Int32 dragBtnOriginX = 0;
	Int32 dragBtnOriginY = 0;

	if(m_alignment == UIEL_SCROLL_V)
	{
		flags |= UIEL_FL_FIXED_W;
		dragBtnWidth = finalWidth;

		dragBtnOriginY = m_pButtonStart->getHeight();
		dragBtnHeight = m_height - dragBtnOriginY - m_pButtonEnd->getHeight();
	}
	else
	{
		flags |= UIEL_FL_FIXED_H;
		dragBtnHeight = finalWidth;

		dragBtnOriginX = m_pButtonStart->getWidth();
		dragBtnWidth = m_width - dragBtnOriginX - m_pButtonEnd->getWidth();
	}

	// Create middle button
	CUIScrollerDragBtnAction* pDragAction = new CUIScrollerDragBtnAction(this);
	m_pDragButton = new CUIDragButton(flags, m_alignment, pDragAction, dragBtnWidth, dragBtnHeight, dragBtnOriginX, dragBtnOriginY);
	m_pDragButton->setParent(this);

	// Set bounds for the scroller
	if(m_alignment == UIEL_SCROLL_V)
		m_pDragButton->setBounds(m_pButtonStart->getHeight(), m_pButtonEnd->getHeight());
	else
		m_pDragButton->setBounds(m_pButtonStart->getWidth(), m_pButtonEnd->getWidth());

	if(!m_pDragButton->init(pstrSchemaName))
	{
		g_engineFuncs.pfnCon_EPrintf("Failed to initialize button for scroller with schema file '%s'.\n", pstrSchemaName);
		return false;
	}
	
	adjustPosition();
	return true;
}

//=============================================
// @brief Tells if the parent size is valid for us
//
//=============================================
void CUIScroller::setFullRange( Uint32 fullRangeSize )
{ 
	if(!m_prevFullRangeSize)
		m_prevFullRangeSize = fullRangeSize;

	m_fullRangeSize = fullRangeSize;
	readjustDragButton();
}

//=============================================
// @brief Tells if the parent size is valid for us
//
//=============================================
void CUIScroller::readjustDragButton( void )
{
	Int32 insetStart, insetEnd;
	m_pDragButton->getBounds(insetStart, insetEnd);

	// Determine coverage
	Uint32 viewSize = 0;
	if(m_alignment == UIEL_SCROLL_V)
		viewSize = m_height;
	else
		viewSize = m_width;

	// Set to maximum if empty
	if(!m_fullRangeSize)
	{
		m_pDragButton->setLength(viewSize);
		m_pDragButton->setPosition(0);
	}

	// Determine coverage
	Float coverage = viewSize/(Float)m_fullRangeSize;
	Uint32 draggerLength = coverage*viewSize;
	if(draggerLength > viewSize)
		draggerLength = viewSize;

	m_pDragButton->setLength(draggerLength);

	if(m_flags & UIEL_FL_SCR_REVERSE)
	{
		// Always set to end
		m_pDragButton->setPosition(1.0);
	}
	else
	{
		Double prevPos = m_pDragButton->getPosition();
		Uint32 prevRealPos = m_prevFullRangeSize*prevPos;

		Double newPos = (Double)prevRealPos/(Double)m_fullRangeSize;
		m_pDragButton->setPosition((Float)newPos);
	}

	// Force a reset
	m_pDragButton->adjPosition(0, false);
	m_prevFullRangeSize = m_fullRangeSize;
}

//=============================================
// @brief Adjusts the size by x and y amount
//
//=============================================
bool CUIScroller::adjustSize( Int32 x, Int32 y )
{
	// Call base class to adjust size
	if(!CUIObject::adjustSize(x, y))
		return false;

	// Update range size on parent
	m_pParent->updateRangeSize();

	readjustDragButton();
	return true;
}

//=============================================
// @brief Tells if the parent size is valid for us
//
//=============================================
bool CUIScroller::isParentSizeValid( Uint32 testWidth, Uint32 testHeight, Int32 adjX, Int32 adjY )
{
	// Determine my adjusted size
	Int32 myTestWidth = (Int32)m_width;
	Int32 myTestHeight = (Int32)m_height;

	// Add it to the width/height
	if(!(m_flags & UIEL_FL_FIXED_W))
		myTestWidth += adjX;

	if(!(m_flags & UIEL_FL_FIXED_H))
		myTestHeight += adjY;

	// Adjust origins for test
	Int32 testOriginX;
	if(m_flags & UIEL_FL_ALIGN_R)
		testOriginX = testWidth - m_baseOriginX - m_width;
	else
		testOriginX = m_baseOriginX;

	Int32 testOriginY;
	if(m_flags & UIEL_FL_ALIGN_B)
		testOriginY = testHeight - m_baseOriginY - m_height;
	else
		testOriginY = m_baseOriginY;

	// Compare against parent size too
	Int32 fullWidth = myTestWidth + testOriginX;
	Int32 fullHeight = myTestHeight + testOriginY;
	if(fullWidth > (Int32)testWidth || fullHeight > (Int32)testHeight)
		return false;

	// Test for children too
	for(Uint32 i = 0; i < m_childrenArray.size(); i++)
	{
		if(considerElementOnResize(m_childrenArray[i]))
		{
			if(!m_childrenArray[i]->isParentSizeValid((Uint32)myTestWidth, (Uint32)myTestHeight, adjX, adjY))
				return false;
		}
	}

	return true;
}

//=============================================
// @brief Tells if a child element should be queried on size checks
//
//=============================================
bool CUIScroller::considerElementOnResize( CUIObject* pObject )
{
	if(!CUITexturedObject::considerElementOnResize(pObject))
		return false;

	if(pObject == m_pDragButton)
		return false;
	
	return true;
}

//=============================================
// @brief Moves the scroller in the specified direction
//
//=============================================
bool CUIScroller::moveScroller( Int32 button, Int32 scrollAmount )
{
	if((button == MOUSE_WHEEL_UP || button == MOUSE_WHEEL_DOWN) && m_alignment != UIEL_SCROLL_V)
		return false;

	if((button == MOUSE_WHEEL_RIGHT || button == MOUSE_WHEEL_LEFT) && m_alignment != UIEL_SCROLL_H)
		return false;

	Int32 moveAmount = scrollAmount * m_unitSize;
	if(button == MOUSE_WHEEL_UP || button == MOUSE_WHEEL_RIGHT)
		moveAmount = -moveAmount;

	m_pDragButton->adjPosition(moveAmount, false);

	return true;
}

//=============================================
// @brief Constructor
//
//=============================================
CUITabLabel::CUITabLabel( Int32 flags, const Char* pstrLabelTitle, const font_set_t* pFont, CUICallbackEvent* pAction, Int32 originx, Int32 originy ):
	CUIButton( flags, pstrLabelTitle, pFont, pAction, TAB_LABEL_WIDTH, TAB_LABEL_HEIGHT, originx, originy ),
	m_pTabBody(nullptr)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CUITabLabel::~CUITabLabel( void )
{
}

//=============================================
// @brief Constructor
//
//=============================================
CUITabBody::CUITabBody( Int32 flags, CUITabLabel* pTabLabel, Uint32 width, Uint32 height, Int32 originx, Int32 originy ):
	CUISurface( flags, width, height, originx, originy ),
	m_pTabLabel(pTabLabel),
	m_pTopLeftInnerCorner(nullptr),
	m_pTopRightInnerCorner(nullptr),
	m_pTopLeftBorderBit(nullptr),
	m_pTopRightBorderBit(nullptr)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CUITabBody::~CUITabBody( void )
{
}

//=============================================
// @brief Initializes the top border elements
//
//=============================================
bool CUITabBody::initTopElements( void )
{
	// Set top middle element
	m_pTopBorder = createObject(m_pScheme, "Body");
	if(!m_pTopBorder)
		return false;

	// Set top left border bit element
	m_pTopRightBorderBit = createObject(m_pScheme, "Top");
	if(!m_pTopRightBorderBit)
		return false;

	// Check if we're the tab element
	Int32 originX, originY;
	m_pTabLabel->getPosition(originX, originY);

	if(m_originX != originX)
	{
		// Set top left corner element
		m_pTopLeftCorner = createObject(m_pScheme, "TopLeft");
		if(!m_pTopLeftCorner)
			return false;

		// Set top left border bit element
		m_pTopLeftBorderBit = createObject(m_pScheme, "Top");
		if(!m_pTopLeftBorderBit)
			return false;

		// Set top left inner corner element
		m_pTopLeftInnerCorner = createObject(m_pScheme, "TopInnerLeft");
		if(!m_pTopLeftInnerCorner)
			return false;
	}

	// Set top right corner element
	m_pTopRightCorner = createObject(m_pScheme, "TopRight");
	if(!m_pTopRightCorner)
		return false;

	// Set top right inner corner element
	m_pTopRightInnerCorner = createObject(m_pScheme, "TopInnerRight");
	if(!m_pTopRightInnerCorner)
		return false;

	return true;
}

//=============================================
// @brief Adjusts the top border element
//
//=============================================
void CUITabBody::adjustTopBorder( void )
{
	if(!m_pTopRightInnerCorner || !m_pTopRightCorner 
		|| !m_pTopBorder || !m_pTopRightBorderBit)
		return;

	// Get the label's x origin
	Int32 labelOriginX, labelOriginY;
	m_pTabLabel->getPosition(labelOriginX, labelOriginY);

	Int32 originY = 0;
	Int32 originX = 0;

	// Set top left corner(if present)
	if(m_pTopLeftCorner)
	{
		m_pTopLeftCorner->setPosition(originX, originY);
		originX += m_pTopLeftCorner->getWidth();
	}
	else
		originX += m_pLeftBorder->getWidth();

	// Set the inner left(if present)
	if(m_pTopLeftBorderBit)
	{
		Uint32 width = labelOriginX - m_originX - originX;
		m_pTopLeftBorderBit->setPosition(originX, originY);
		m_pTopLeftBorderBit->setWidth(width);
	}

	// Next is the inner left corner
	if(m_pTopLeftInnerCorner)
	{
		originX = labelOriginX - m_originX;
		m_pTopLeftInnerCorner->setPosition(originX, originY);
		originX += m_pTopLeftInnerCorner->getWidth();
	}
	else
		originX = m_pLeftBorder->getWidth();
	
	// Set the middle bit
	Uint32 width = m_pTabLabel->getWidth();
	if(m_pTopLeftInnerCorner)
		width -= m_pTopLeftInnerCorner->getWidth();

	width -= m_pTopRightInnerCorner->getWidth();
	if(!m_pTopLeftInnerCorner)
		width -= m_pLeftBorder->getWidth();

	m_pTopBorder->setPosition(originX, originY);
	m_pTopBorder->setWidth(width);

	// Set the inner right corner
	originX = labelOriginX - m_originX + m_pTabLabel->getWidth();
	originX -= m_pTopRightInnerCorner->getWidth();
	m_pTopRightInnerCorner->setPosition(originX, originY);

	// Set the right border bit
	originX += m_pTopRightInnerCorner->getWidth();
	width = m_width - originX - m_pTopRightCorner->getWidth();
	m_pTopRightBorderBit->setPosition(originX, originY);
	m_pTopRightBorderBit->setWidth(width);

	// Set top right corner
	originX = m_width - m_pTopRightCorner->getWidth();
	m_pTopRightCorner->setPosition(originX, originY);
}

//=============================================
// @brief Tells if a child element should be queried on size checks
//
//=============================================
bool CUITabBody::considerElementOnResize( CUIObject* pObject )
{
	if(!CUISurface::considerElementOnResize(pObject))
		return false;

	if(pObject == m_pTopLeftInnerCorner || pObject == m_pTopRightInnerCorner
		|| pObject == m_pTopLeftBorderBit || pObject == m_pTopRightBorderBit)
		return false;
	
	return true;
}

//=============================================
// @brief Sets the alpha value of the UI element
//
//=============================================
void CUITabBody::setAlpha( Int32 alpha, bool recursive )
{
	// Call base class to manage basics
	CUISurface::setAlpha(alpha, recursive);

	// If not recursive, we need to set for the base elements
	if(!recursive)
	{
		if(m_pTopLeftInnerCorner)
			m_pTopLeftInnerCorner->setAlpha(m_color.a);
		if(m_pTopRightInnerCorner)
			m_pTopRightInnerCorner->setAlpha(m_color.a);
		if(m_pTopLeftBorderBit)
			m_pTopLeftBorderBit->setAlpha(m_color.a);
		if(m_pTopRightBorderBit)
			m_pTopRightBorderBit->setAlpha(m_color.a);
	}
}

//=============================================
// @brief Constructor
//
//=============================================
CUITabList::CUITabList( Int32 flags, CUICallbackEvent* pSelectEvent, const font_set_t* pFont, Uint32 width, Uint32 height, Int32 originx, Int32 originy ):
	CUIObject( flags, width, height, originx, originy ),
	m_pTabSelectEvent(pSelectEvent),
	m_pFont(pFont)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CUITabList::~CUITabList( void )
{
	if(m_pTabSelectEvent)
		delete m_pTabSelectEvent;
}

//=============================================
// @brief Creates a tab object and returns a 
// pointer to the body
//
//=============================================
CUITabBody* CUITabList::createTab( const Char* pstrName )
{
	Int32 originX = 0;
	Int32 originY = 0;

	// Determine the label's origin
	Int32 labelWidth = CUITabLabel::TAB_LABEL_WIDTH;
	Int32 labelOriginX = originX + labelWidth*m_pTabsArray.size();
	if(labelOriginX + labelWidth > (Int32)m_width - 32)
	{
		g_engineFuncs.pfnCon_EPrintf("Couldn't create CUITabBody with name %s - all space used up on width.\n", pstrName);
		return nullptr;
	}

	// Callback event for the label button
	Uint32 tabIndex = m_pTabsArray.size();
	CUITabSelectCallback* pCallbackEvent = new CUITabSelectCallback(this, tabIndex);

	// Create the label
	CUITabLabel* pTabLabel = new CUITabLabel(UIEL_FL_NONE, pstrName, m_pFont, pCallbackEvent, labelOriginX, originY);
	if(!pTabLabel->init("defaultwindow.txt"))
	{
		g_engineFuncs.pfnCon_EPrintf("Failed to set schema 'defaultbutton.txt' for CUITabLabel object %s.\n", pstrName);
		delete pTabLabel;
		return nullptr;
	}

	pTabLabel->setParent(this);
	pTabLabel->setAlpha(255); // Never has alpha

	// Determine the tab's position
	originY += pTabLabel->getHeight();
	Uint32 bodyWidth = m_width;
	Uint32 bodyHeight = m_height - originY;

	// Create the tab body
	CUITabBody* pTabBody = new CUITabBody(UIEL_FL_NONE, pTabLabel, bodyWidth, bodyHeight, originX, originY);
	if(!pTabBody->init("defaultwindow.txt"))
	{
		g_engineFuncs.pfnCon_EPrintf("Failed to set schema 'defaultbutton.txt' for the CUITabBody %s.\n", pstrName);
		delete pTabLabel;
		return nullptr;
	}

	pTabBody->setParent(this);
	pTabBody->setAlpha(255); // Never has alpha
	pTabLabel->setTabBody(pTabBody);
	pTabBody->adjustBgElements();

	// Set first element as visible
	if(tabIndex > 0)
		pTabBody->setVisible(false);

	// Add it to the list
	tab_t tabPair;
	tabPair.plabel = pTabLabel;
	tabPair.pbody = pTabBody;

	m_pTabsArray.push_back(tabPair);
	return pTabBody;
}

//=============================================
// @brief Enables a single tab and hides the rest
//
//=============================================
void CUITabList::showTab( Uint32 tabIndex )
{
	assert(tabIndex < m_pTabsArray.size());

	// Hide all the tabs except this one
	for(Uint32 i = 0; i < m_pTabsArray.size(); i++)
	{
		if(i == tabIndex)
			continue;

		m_pTabsArray[i].pbody->setVisible(false);
	}

	// Enable the tab
	m_pTabsArray[tabIndex].pbody->setVisible(true);

	// Call the event if needed
	if(m_pTabSelectEvent)
		m_pTabSelectEvent->PerformAction(tabIndex);
}

//=============================================
// @brief Constructor
//
//=============================================
CUIScrollableSurface::CUIScrollableSurface( Int32 flags, Uint32 width, Uint32 height, Int32 originx, Int32 originy ):
	CUIObject( (flags|UIEL_FL_EXPAND_H), width, height, originx, originy ),
	m_pSurface(nullptr),
	m_pScroller(nullptr),
	m_scrollOffset(0),
	m_baseYOffset(0),
	m_pFont(nullptr)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CUIScrollableSurface::~CUIScrollableSurface( void )
{
}

//=============================================
// @brief Sets the source text array to render text from
//
//=============================================
bool CUIScrollableSurface::init( const Char* pstrSchemaName )
{
	// The surface is a seperate object so it isn't affected by the scroller
	m_pSurface = new CUISurface(m_flags, m_width, m_height, 0, 0);
	m_pSurface->setParent(this);

	if(!m_pSurface->init(pstrSchemaName))
	{
		g_engineFuncs.pfnCon_EPrintf("Failed to set schema file '%s' for 'CUIScrollableSurface'.\n", pstrSchemaName);
		return false;
	}

	m_pSurface->setAlpha(250);

	// Create the scroller
	Int32 flags = UIEL_FL_NONE;
	if(m_flags & UIEL_FL_SCR_REVERSE)
		flags |= UIEL_FL_SCR_REVERSE;

	m_pScroller = new CUIScroller(flags, UIEL_SCROLL_V, 16, 0, m_height, 0, 0);
	m_pScroller->setParent(this);

	if(!m_pScroller->init("defaultscroller.txt"))
	{
		g_engineFuncs.pfnCon_EPrintf("Failed to initialize CUIScroller with schema file 'defaultscoller.txt' for 'CUIScrollableSurface'.\n");
		return false;
	}

	return true;
}

//=============================================
// @brief Sets the source text array to render text from
//
//=============================================
void CUIScrollableSurface::setOffsetValue( Float offset )
{
	m_scrollOffset = offset;
}

//=============================================
// @brief Adjusts the size by x and y amount
//
//=============================================
bool CUIScrollableSurface::adjustSize( Int32 x, Int32 y )
{
	if(CUIObject::adjustSize(x, y))
	{
		m_pScroller->setFullRange(m_height+m_baseYOffset);
		return true;
	}

	return false;
}

//=============================================
// @brief Draws the string
//
//=============================================
bool CUIScrollableSurface::draw( void )
{
	if(!m_isVisible)
		return true;

	// Draw non-scrolling elements first
	for(Uint32 i = 0; i < m_childrenArray.size(); i++)
	{
		CUIObject* pObject = m_childrenArray[i];
		if(shouldShiftChild(pObject))
			continue;

		if(!pObject->draw())
			return false;
	}

	// Get the parent's size to set up the scissor
	Int32 absX, absY;
	m_pSurface->getAbsPosition(absX, absY);

	Uint32 width, height;
	m_pSurface->getSize(width, height);

	Uint32 scrwidth, scrheight;
	g_engineFuncs.pfnGetWindowSize(scrwidth, scrheight);

	Int32 originY = (Int32)scrheight - absY - height;

	glEnable(GL_SCISSOR_TEST);
	glScissor(absX + 2, originY + 2, width - 4, height - 4);

	// Offset in modelview by the offset
	Int32 minusCoverage = m_height - m_baseHeight + m_baseYOffset + 4;
	if(minusCoverage < 0)
		minusCoverage = 0;

	Int32 offsetAmount = minusCoverage * m_scrollOffset;

	// Draw the scrolled elements
	for(Uint32 i = 0; i < m_childrenArray.size(); i++)
	{
		CUIObject* pObject = m_childrenArray[i];
		if(!isChildVisible(pObject, offsetAmount))
			continue;

		if(!shouldShiftChild(pObject))
			continue;

		if(!(pObject->getFlags() & UIEL_FL_ONTOP))
		{
			// Get current origin
			Int32 savedOriginX, savedOriginY;
			pObject->getPosition(savedOriginX, savedOriginY);

			// Offset by the offset amount and draw
			pObject->setPosition(savedOriginX, savedOriginY + m_baseYOffset - offsetAmount);
			if(!pObject->draw())
				return false;
			
			// Restore it
			pObject->setPosition(savedOriginX, savedOriginY);
		}
	}

	// Draw ontop elements last
	for(Uint32 i = 0; i < m_childrenArray.size(); i++)
	{
		CUIObject* pObject = m_childrenArray[i];
		if(!isChildVisible(pObject, offsetAmount))
			continue;

		if(!shouldShiftChild(pObject))
			continue;

		if(pObject->getFlags() & UIEL_FL_ONTOP)
		{
			// Get current origin
			Int32 savedOriginX, savedOriginY;
			pObject->getPosition(savedOriginX, savedOriginY);

			// Offset by the offset amount and draw
			pObject->setPosition(savedOriginX, savedOriginY + m_baseYOffset - offsetAmount);
			if(!pObject->draw())
				return false;
			
			// Restore it
			pObject->setPosition(savedOriginX, savedOriginY);
		}
	}

	glDisable(GL_SCISSOR_TEST);

	return true;
}

//=============================================
// @brief Think function for elements
//
//=============================================
bool CUIScrollableSurface::mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown )
{
	if(!m_isVisible)
		return false;

	if(!isMouseOver(mouseX, mouseY))
		return false;

	// Offset in modelview by the offset
	Int32 minusCoverage = m_height - m_baseHeight + m_baseYOffset + 4;
	if(minusCoverage < 0)
		minusCoverage = 0;

	Int32 offsetAmount = minusCoverage * m_scrollOffset;

	// Prioritize interactive elements first
	for(Uint32 i = 0; i < m_childrenArray.size(); i++)
	{
		CUIObject* pObject = m_childrenArray[i];
		if(!pObject->isInteractive())
			continue;

		Int32 savedOriginX = 0;
		Int32 savedOriginY = 0;
		if(shouldShiftChild(pObject))
		{
			if(!isChildVisible(pObject, offsetAmount))
				continue;

			// Get current origin
			pObject->getPosition(savedOriginX, savedOriginY);
			// Set new origin and test
			pObject->setPosition(savedOriginX, savedOriginY + m_baseYOffset - offsetAmount);
		}

		bool bResult = pObject->mouseButtonEvent(mouseX, mouseY, button, keyDown);

		// Restore it
		if(shouldShiftChild(pObject))
			pObject->setPosition(savedOriginX, savedOriginY);

		if(bResult)
			return true;
	}

	// Now check non-interactive elements
	for(Uint32 i = 0; i < m_childrenArray.size(); i++)
	{
		CUIObject* pObject = m_childrenArray[i];
		if(pObject->isInteractive())
			continue;

		Int32 savedOriginX = 0;
		Int32 savedOriginY = 0;

		if(shouldShiftChild(pObject))
		{
			if(!isChildVisible(pObject, offsetAmount))
				continue;

			// Get current origin
			pObject->getPosition(savedOriginX, savedOriginY);
			// Set new origin and test
			pObject->setPosition(savedOriginX, savedOriginY + m_baseYOffset - offsetAmount);
		}

		bool bResult = pObject->mouseButtonEvent(mouseX, mouseY, button, keyDown);

		// Restore it
		if(shouldShiftChild(pObject))
			pObject->setPosition(savedOriginX, savedOriginY);

		if(bResult)
			return true;
	}

	return true;
}

//=============================================
// @brief Sets the source text array to render text from
//
//=============================================
bool CUIScrollableSurface::mouseWheelEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown, Int32 scroll )
{
	if(keyDown && m_pScroller->moveScroller(button, scroll))
		return true;
	else
		return false;
}

//=============================================
// @brief Think function for elements
//
//=============================================
void CUIScrollableSurface::think( void )
{
	// Offset in modelview by the offset
	Int32 minusCoverage = m_height - m_baseHeight + m_baseYOffset + 4;
	if(minusCoverage < 0)
		minusCoverage = 0;

	Int32 offsetAmount = minusCoverage * m_scrollOffset;

	// Let the children think too
	for(Uint32 i = 0; i < m_childrenArray.size(); i++)
	{
		CUIObject* pObject = m_childrenArray[i];

		Int32 savedOriginX = 0;
		Int32 savedOriginY = 0;

		if(pObject->isInteractive() && shouldShiftChild(pObject))
		{
			if(!isChildVisible(pObject, offsetAmount))
				continue;

			pObject->getPosition(savedOriginX, savedOriginY);
			pObject->setPosition(savedOriginX, savedOriginY + m_baseYOffset - offsetAmount);
		}

		pObject->think();

		if(pObject->isInteractive() && shouldShiftChild(pObject))
			pObject->setPosition(savedOriginX, savedOriginY);
	}
}

//=============================================
// @brief Determines if the cursor is touching this element
//
//=============================================
bool CUIScrollableSurface::isMouseOver( Int32 xPos, Int32 yPos )
{
	Uint32 surfWidth, surfHeight;
	m_pSurface->getSize(surfWidth, surfHeight);

	Int32 absX, absY;
	getAbsPosition(absX, absY);

	if(absX > xPos)
		return false;
	if(absX+(Int32)surfWidth < xPos)
		return false;
	if(absY > yPos)
		return false;
	if(absY+(Int32)surfHeight+(Int32)m_baseYOffset < yPos)
		return false;

	return true;
}

//=============================================
// @brief Checks if the element is visible
//
//=============================================
bool CUIScrollableSurface::isChildVisible( CUIObject* pChild, Int32 offset )
{
	Int32 xPos, yPos;
	pChild->getPosition(xPos, yPos);
	yPos -= offset;
	yPos += m_baseYOffset;

	Uint32 width, height;
	pChild->getSize(width, height);

	// Get the size and location of the background
	Int32 surfX, surfY;
	m_pSurface->getPosition(surfX, surfY);

	Uint32 surfWidth, surfHeight;
	m_pSurface->getSize(surfWidth, surfHeight);

	if(xPos >= surfX+(Int32)surfWidth)
		return false;
	else if(xPos+(Int32)width < surfX)
		return false;
	else if(yPos >= surfY+(Int32)surfHeight)
		return false;
	else if(yPos+(Int32)height < surfY)
		return false;

	return true;
}

//=============================================
// @brief Tells if the child object should be shifted
//
//=============================================
bool CUIScrollableSurface::shouldShiftChild( CUIObject* pChild )
{
	if(pChild == m_pScroller 
		|| pChild == m_pSurface)
		return false;

	return true;
}

//=============================================
// @brief Constructor
//
//=============================================
CUIList::CUIList( Int32 flags, const font_set_t* pFont, Uint32 rowHeight, Uint32 nbColumns, Uint32 width, Uint32 height, Int32 originx, Int32 originy ):
	CUIScrollableSurface( flags, width, height, originx, originy ),
	m_nbColumns(nbColumns),
	m_rowHeight(rowHeight),
	m_pHeader(nullptr),
	m_listHeight(0),
	m_pHighlightedRow(nullptr),
	m_pListFont(pFont)
{
	if(!(flags & UIEL_FL_NO_HEADER))
	{
		// Set this so base class knows
		m_baseYOffset = m_rowHeight;
	}
}

//=============================================
// @brief Destructor
//
//=============================================
CUIList::~CUIList( void )
{
}

//=============================================
// @brief Sets the source text array to render text from
//
//=============================================
bool CUIList::init( const Char* pstrSchemaName )
{
	if(!CUIScrollableSurface::init(pstrSchemaName))
		return false;

	if(!(m_flags & UIEL_FL_NO_HEADER))
	{
		// Create the header object
		Uint32 headerWidth = m_width - m_pScroller->getWidth();
		m_pHeader = new CUIListHeader(UIEL_FL_NONE, m_pListFont, m_nbColumns, headerWidth, m_rowHeight);
		m_pHeader->setParent(this);

		if(!m_pHeader->init("defaultwindow.txt"))
		{
			g_engineFuncs.pfnCon_EPrintf("Failed to initialize CUIListHeader for CUIScrollableSurface.\n");
			return false;
		}

		// Alter the surface
		m_pSurface->adjustSize(0, -(Int32)m_rowHeight);

		Int32 surfX, surfY;
		m_pSurface->getPosition(surfX, surfY);
		m_pSurface->setPosition(surfX, surfY+m_rowHeight, true);
	}

	return true;
}

//=============================================
// @brief Tells if the child object should be shifted
//
//=============================================
bool CUIList::shouldShiftChild( CUIObject* pChild )
{
	if(!CUIScrollableSurface::shouldShiftChild(pChild))
		return false;

	if(pChild == m_pHeader)
		return false;

	return true;
}

//=============================================
// @brief Sets the header column text
//
//=============================================
void CUIList::setHeaderColumnName( Uint32 index, const Char* pstrName )
{
	assert(m_pHeader != nullptr);
	m_pHeader->setColumnName(index, pstrName);
}

//=============================================
// @brief Creates a new row and returns it's pointer
//
//=============================================
CUIListRow* CUIList::createNewRow( CUICallbackEvent* pEvent, Uint32 textInset )
{
	Uint32 rowIndex = m_rowsArray.size();
	Uint32 rowWidth = m_width - m_pScroller->getWidth();

	Int32 flags = UIEL_FL_NONE;
	if(m_flags & UIEL_FL_HOVER_HIGHLIGHT)
		flags |= UIEL_FL_HOVER_HIGHLIGHT;

	CUIListRow* pNewRow = new CUIListRow(flags, rowIndex, textInset, m_pHeader, pEvent, m_nbColumns, m_listHeight, rowWidth, m_rowHeight);
	pNewRow->setParent(this);

	m_rowsArray.push_back(pNewRow);
	m_listHeight += pNewRow->getHeight() + 1;
	return pNewRow;
}

//=============================================
// @brief Creates a new row and returns it's pointer
//
//=============================================
void CUIList::createSeparator( Uint32 textInset, const Char* pstrName, Uint32 height )
{
	Uint32 rowWidth = m_width - m_pScroller->getWidth();

	CUIListSeparator* pSeparator = new CUIListSeparator(UIEL_FL_NONE, textInset, m_listHeight, rowWidth, height);
	pSeparator->setParent(this);

	CUIText* pText = new CUIText((UIEL_FL_ALIGN_CH|UIEL_FL_ALIGN_CV), m_pListFont, pstrName, 0, 0, 0); 

	pSeparator->setColumnContents(0, pText);
	pSeparator->setColumnColor(255, 192, 64);
	m_listHeight += pSeparator->getHeight();

	m_separatorsArray.push_back(pSeparator);
}

//=============================================
// @brief Sets the header column text
//
//=============================================
void CUIList::clearHighlight( void )
{
	if(!m_pHighlightedRow)
		return;

	m_pHighlightedRow->setHighlight(false);
	m_pHighlightedRow->setHighlightedColumn(-1);
	m_pHighlightedRow = nullptr;
}

//=============================================
// @brief Sets the header column text
//
//=============================================
void CUIList::setHighlightOnRow( Uint32 index, bool isHighlighted )
{
	clearHighlight();

	assert(index < m_rowsArray.size());
	m_rowsArray[index]->setHighlight(isHighlighted);

	if(isHighlighted)
		m_pHighlightedRow = m_rowsArray[index];
}

//=============================================
// @brief Sets the header column text
//
//=============================================
void CUIList::setHighlightOnRowColumn( Int32 columnIndex )
{
	if(!m_pHighlightedRow)
		return;

	m_pHighlightedRow->setHighlightedColumn(columnIndex);
}

//=============================================
// @brief Gets the text from the column of a row
//
//=============================================
CUIObject* CUIList::getRowColumnObject( Uint32 rowIndex, Uint32 columnIndex )
{
	assert(rowIndex < m_rowsArray.size());

	CUIListRow* pRow = m_rowsArray[rowIndex];
	return pRow->getColumnContents(columnIndex);
}

//=============================================
// @brief Clears the list contents
//
//=============================================
void CUIList::clearList( void )
{
	if(!m_rowsArray.empty())
	{
		for(Uint32 i = 0; i < m_rowsArray.size(); i++)
		{
			removeChild(m_rowsArray[i]);
			delete m_rowsArray[i];
		}

		m_rowsArray.clear();
	}

	if(!m_separatorsArray.empty())
	{
		for(Uint32 i = 0; i < m_separatorsArray.size(); i++)
		{
			removeChild(m_separatorsArray[i]);
			delete m_separatorsArray[i];
		}

		m_separatorsArray.clear();
	}

	m_listHeight = 0;
	m_height = m_baseHeight;

	m_pHighlightedRow = nullptr;
}

//=============================================
// @brief Constructor
//
//=============================================
CUIListHeader::CUIListHeader( Int32 flags, const font_set_t* pFont, Uint32 numColumns, Uint32 width, Uint32 height ):
	CUIObject( flags, width, height, 0, 0 ),
	m_pFont(pFont)
{
	m_columns.resize(numColumns);
}

//=============================================
// @brief Destructor
//
//=============================================
CUIListHeader::~CUIListHeader( void )
{
}

//=============================================
// @brief Initializes the class
//
//=============================================
bool CUIListHeader::init( const Char* pstrSchemaName )
{
	// Make sure we're valid
	if(m_columns.empty())
	{
		g_engineFuncs.pfnCon_EPrintf("No columns specified for CUIListHeader object.\n");
		return false;
	}

	Uint32 nbColumns = m_columns.size();
	Uint32 colWidth = m_width/nbColumns;

	// Create each column
	for(Uint32 i = 0; i < nbColumns; i++)
	{
		hdr_column_t& column = m_columns[i];

		Int32 colXOrg = colWidth*i;
		Int32 colYOrg = 0;

		// Adjust last one to completely fill the width
		if(i == (nbColumns-1))
			colWidth = m_width-colXOrg;

		// Create the surface
		column.psurface = new CUISurface(UIEL_FL_NONE, colWidth, m_height, colXOrg, colYOrg);
		column.psurface->setParent(this);
		if(!column.psurface->init(pstrSchemaName))
		{
			g_engineFuncs.pfnCon_EPrintf("Failed to init CUISurface object for CUIListHeader.\n");
			return false;
		}

		// Create the text
		column.ptext = new CUIText(UIEL_FL_ALIGN_CV, m_pFont, m_columns[i].name.c_str(), 8, 0);
		column.ptext->setParent(column.psurface);
	}

	return true;
}

//=============================================
// @brief Sets the string for a column
//
//=============================================
void CUIListHeader::setColumnName( Uint32 index, const Char* pstrName )
{
	assert(index < m_columns.size());

	hdr_column_t& column = m_columns[index];
	column.name = pstrName;

	// If the text object is already present
	if(column.ptext)
		column.ptext->setText(pstrName);
}

//=============================================
// @brief Constructor
//
//=============================================
CUIListRow::CUIListRow( Int32 flags, Uint32 rowIndex, Uint32 textInset, CUIListHeader* pHeader, CUICallbackEvent* pEvent, Uint32 numColumns, Uint32 rowOffs, Uint32 width, Uint32 height ):
	CUIObject( flags, width, height, 0, rowOffs ),
	m_rowIndex(rowIndex),
	m_pHeader(pHeader),
	m_pEvent(pEvent),
	m_isHighlighted(false),
	m_highlightedColumnIdx(-1)
{
	m_columns.resize(numColumns);
	m_columnContents.resize(numColumns);
	Uint32 columnWidth = (width/numColumns);
	for(Uint32 i = 0; i < numColumns; i++)
	{
		Int32 xPos = columnWidth*i;
		CUIObject* pColumnObject = new CUIObject(UIEL_FL_NONE, columnWidth, m_height, xPos, 0);
		pColumnObject->setParent(this);

		m_columns[i] = pColumnObject;
	}
}

//=============================================
// @brief Destructor
//
//=============================================
CUIListRow::~CUIListRow( void )
{
	if(m_pEvent)
		delete m_pEvent;
}

//=============================================
// @brief Sets the text for a column
//
//=============================================
void CUIListRow::setColumnContents( Uint32 index, CUIObject* pObject )
{
	assert(index < m_columns.size());
	pObject->setParent(m_columns[index]);
	m_columnContents[index] = pObject;
}

//=============================================
// @brief Gets the text for a column
//
//=============================================
CUIObject* CUIListRow::getColumnContents( Uint32 index )
{
	assert(index < m_columns.size());
	return m_columnContents[index];
}

//=============================================
// @brief Sets the color for a column
//
//=============================================
void CUIListRow::setColumnColor( byte r, byte g, byte b )
{
	for(Uint32 i = 0; i < m_columns.size(); i++)
		m_columns[i]->setColor(r, g, b);
}

//=============================================
// @brief Handles a mouse button event
//
//=============================================
bool CUIListRow::mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown )
{
	// Only left mouse click affects draggers
	if(button != SDL_BUTTON_LEFT)
		return false;

	if(isMouseOver(mouseX, mouseY))
	{
		if(m_pEvent)
			m_pEvent->MouseButtonEvent(mouseX, mouseY, button, keyDown);

		return true;
	}

	return false;
}

//=============================================
// @brief Draws the string
//
//=============================================
bool CUIListRow::draw( void )
{
	bool shouldHighlight = false;
	if(m_isHighlighted)
		shouldHighlight = true;

	if(m_flags & UIEL_FL_HOVER_HIGHLIGHT)
	{
		CUIObject* pParentWindow = getParentWindow();
		if(pParentWindow && pParentWindow->isInFocus())
		{
			Int32 mouseX, mouseY;
			g_engineFuncs.pfnGetMousePosition(mouseX, mouseY);

			if(isMouseOver(mouseX, mouseY))
				shouldHighlight = true;
		}
	}

	if(shouldHighlight)
	{
		Int32 xpos, ypos;
		getAbsPosition(xpos, ypos);

		if(!g_engineFuncs.pfnBasicDrawDisableTextures())
			return false;

		g_engineFuncs.pfnBasicDrawColor4f(1.0, 1.0, 1.0, 0.25);

		g_engineFuncs.pfnValidateBasicDraw();

		g_engineFuncs.pfnBasicDrawBegin(GL_TRIANGLES);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xpos, (Float)ypos + m_height, -1);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xpos, (Float)ypos, -1);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xpos+m_width, (Float)ypos, -1);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xpos, (Float)ypos + m_height, -1);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xpos+m_width, (Float)ypos, -1);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xpos+m_width, (Float)ypos+m_height, -1);
		g_engineFuncs.pfnBasicDrawEnd();

		if(!g_engineFuncs.pfnBasicDrawEnableTextures())
			return false;
	}

	if(m_highlightedColumnIdx != -1)
	{
		Int32 xpos, ypos;
		getAbsPosition(xpos, ypos);

		Int32 colWidth = (m_width/m_columns.size());
		Int32 colXPos = m_highlightedColumnIdx*colWidth;
		xpos += colXPos;

		if(m_highlightedColumnIdx == (((Int32)m_columns.size())-1))
			colWidth = (Int32)m_width - colXPos;

		if(!g_engineFuncs.pfnBasicDrawDisableTextures())
			return false;

		g_engineFuncs.pfnBasicDrawColor4f(1.0, 0.75, 0.25, 1.0);

		g_engineFuncs.pfnValidateBasicDraw();

		g_engineFuncs.pfnBasicDrawBegin(GL_TRIANGLES);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xpos, (Float)ypos + m_height, -1);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xpos, (Float)ypos, -1);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xpos+colWidth, (Float)ypos, -1);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xpos, (Float)ypos + m_height, -1);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xpos+colWidth, (Float)ypos, -1);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xpos+colWidth, (Float)ypos+m_height, -1);
		g_engineFuncs.pfnBasicDrawEnd();

		if(!g_engineFuncs.pfnBasicDrawEnableTextures())
			return false;
	}

	// Draw everything else as normal
	return CUIObject::draw();
}

//=============================================
// @brief Constructor
//
//=============================================
CUIListSeparator::CUIListSeparator( Int32 flags, Uint32 textInset, Uint32 rowOffs, Uint32 width, Uint32 height ):
	CUIListRow( flags, 0, textInset, nullptr, nullptr, 1, rowOffs, width, height )
{
}

//=============================================
// @brief Destructor
//
//=============================================
CUIListSeparator::~CUIListSeparator( void )
{
}

//=============================================
// @brief Draws the string
//
//=============================================
bool CUIListSeparator::draw( void )
{
	// Draw everything as normal
	if(!CUIListRow::draw())
		return false;

	if(!g_engineFuncs.pfnBasicDrawDisableTextures())
		return false;

	Int32 xpos, ypos;
	getAbsPosition(xpos, ypos);

	Int32 sepHeight = 1;
	ypos += m_height - sepHeight;

	g_engineFuncs.pfnBasicDrawColor4f(0.0, 0.0, 0.0, 0.25);

	Int32 sepInset = 8;
	xpos += sepInset;
	Int32 sepWidth = m_width - sepInset*2;

	g_engineFuncs.pfnValidateBasicDraw();

	g_engineFuncs.pfnBasicDrawBegin(GL_TRIANGLES);
	g_engineFuncs.pfnBasicDrawVertex3f((Float)xpos, (Float)ypos + sepHeight, -1);
	g_engineFuncs.pfnBasicDrawVertex3f((Float)xpos, (Float)ypos, -1);
	g_engineFuncs.pfnBasicDrawVertex3f((Float)xpos+sepWidth, (Float)ypos, -1);
	g_engineFuncs.pfnBasicDrawVertex3f((Float)xpos, (Float)ypos + sepHeight, -1);
	g_engineFuncs.pfnBasicDrawVertex3f((Float)xpos+sepWidth, (Float)ypos, -1);
	g_engineFuncs.pfnBasicDrawVertex3f((Float)xpos+sepWidth, (Float)ypos+sepHeight, -1);
	g_engineFuncs.pfnBasicDrawEnd();

	if(!g_engineFuncs.pfnBasicDrawEnableTextures())
		return false;

	return true;
}

//=============================================
// @brief Constructor
//
//=============================================
CUIDropDownList::CUIDropDownList( Int32 flags, CUICallbackEvent* pEvent, CUICallbackEvent* pListToggleEvent, const font_set_t* pFont, Uint32 width, Uint32 height, Int32 originx, Int32 originy ):
	CUISurface( flags, width, height, originx, originy ),
	m_pCurrentValue(nullptr),
	m_pDropdownButton(nullptr),
	m_pDropdownList(nullptr),
	m_selectionIdx(-1),
	m_pEvent(pEvent),
	m_pFont(pFont),
	m_pListToggleEvent(pListToggleEvent)
{
	Uint32 textInset = 6;
	Int32 maxWidth = width - 24 - textInset*2;
	maxWidth = _min(0, maxWidth);

	// Create the text object
	m_pCurrentValue = new CUIText((UIEL_FL_ALIGN_CV|UIEL_FL_ALIGN_L|UIEL_FL_ONTOP), m_pFont, "", 6, 0, maxWidth);
	m_pCurrentValue->setParent(this);
}

//=============================================
// @brief Destructor
//
//=============================================
CUIDropDownList::~CUIDropDownList( void )
{
	if(m_pEvent)
		delete m_pEvent;

	if(m_pListToggleEvent)
		delete m_pListToggleEvent;
}

//=============================================
// @brief Initializes the class
//
//=============================================
bool CUIDropDownList::init( const Char* pstrSchemaName )
{
	// Init base class
	if(!CUISurface::init(pstrSchemaName))
		return false;

	const ui_schemeobject_t* pObject = m_pScheme->getObject("ArrowDown");
	if(!pObject)
	{
		g_engineFuncs.pfnCon_EPrintf("UI schema '%s' is missing 'ArrowDown'.\n", pstrSchemaName);
		return false;
	}

	// Create the button
	CUIDropDownButtonEvent* pDropDownBtnEvent = new CUIDropDownButtonEvent(this);
	CUIButton* pDropButton = new CUIButton((UIEL_FL_ALIGN_CV|UIEL_FL_ALIGN_R), pObject, pDropDownBtnEvent, 24, m_height, 0, 0);
	pDropButton->setParent(this);
	if(!pDropButton->init("defaultbutton.txt"))
	{
		g_engineFuncs.pfnCon_EPrintf("Failed to initialize 'CUIButton' for 'CUIDropDownList'.\n");
		return false;
	}

	// Create the dropdown list
	m_pDropdownList = new CUIList((UIEL_FL_NO_HEADER|UIEL_FL_HOVER_HIGHLIGHT), m_pFont, m_height, 1, m_width, m_height*4, 0, m_height);
	m_pDropdownList->setParent(this);
	if(!m_pDropdownList->init("indentedwindow.txt"))
	{
		g_engineFuncs.pfnCon_EPrintf("Failed to initialize 'CUIList' for 'CUIDropDownList'.\n");
		return false;
	}

	// Set to hidden by default
	m_pDropdownList->setVisible(false);

	return true;
}

//=============================================
// @brief Think function for elements
//
//=============================================
bool CUIDropDownList::mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown )
{
	if(CUISurface::mouseButtonEvent(mouseX, mouseY, button, keyDown))
		return true;

	if(m_pDropdownList->mouseButtonEvent(mouseX, mouseY, button, keyDown))
		return true;

	// Hide the list if we clicked away
	if(m_pDropdownList->isVisible() && keyDown)
		toggleList();

	return false;
}

//=============================================
// @brief Toggles visibility of the dropdown list
//
//=============================================
void CUIDropDownList::toggleList( void )
{
	if(m_pDropdownList->isVisible())
	{
		m_pDropdownList->setVisible(false);
		m_flags &= ~UIEL_FL_ONTOP;

		if(m_pListToggleEvent)
			m_pListToggleEvent->PerformAction(false);
	}
	else
	{
		m_pDropdownList->setVisible(true);
		m_flags |= UIEL_FL_ONTOP;

		m_pParent->closeOtherTabs(this);

		if(m_pListToggleEvent)
			m_pListToggleEvent->PerformAction(true);
	}
}

//=============================================
// @brief Sets the current selection
//
//=============================================
void CUIDropDownList::setSelection( Int32 rowIndex )
{
	if(rowIndex == -1)
	{
		m_pCurrentValue->setText("");
		return;
	}

	if(m_pDropdownList->getNbRows() < 1)
		return;

	CUIObject* pObject = m_pDropdownList->getRowColumnObject(rowIndex, 0);
	if(!pObject)
		return;

	CUIText* pText = reinterpret_cast<CUIText*>(pObject);
	const Char* pstrValue = pText->getText();

	m_pCurrentValue->setText(pstrValue);
	m_selectionIdx = rowIndex;
}

//=============================================
// @brief Sets the current selection
//
//=============================================
void CUIDropDownList::manageSelectionEvent( Uint32 rowIndex )
{
	setSelection(rowIndex);
	toggleList();

	// Call the callback function
	if(m_pEvent)
		m_pEvent->PerformAction(rowIndex);
}

//=============================================
// @brief Adds a choice to the dropdown list
//
//=============================================
void CUIDropDownList::addChoice( const Char* pstrText )
{
	Int32 rowIdx = m_pDropdownList->getNbRows();

	// Create the new row
	CUIDropDownRowClickEvent* pEvent = new CUIDropDownRowClickEvent(this, rowIdx);
	CUIListRow* pNew = m_pDropdownList->createNewRow(pEvent, 4);
	
	// Allocate new element and set
	CUIText* pText = new CUIText((UIEL_FL_ALIGN_CV|UIEL_FL_ALIGN_CH), m_pFont, pstrText, 0, 0, 0);
	pNew->setColumnContents(0, pText);

	// Set the selection
	if(m_selectionIdx == -1)
		setSelection(rowIdx);
}

//=============================================
// @brief Requests that other tabs be closed
//
//=============================================
void CUIDropDownList::closeOtherTabs( CUIObject* pCaller )
{
	// Hide the list if we clicked away
	if(m_pDropdownList->isVisible())
		toggleList();
}

//=============================================
// @brief Performs the close action for the window
//
//=============================================
void CUIDropDownList::CUIDropDownButtonEvent::PerformAction( Float param )
{
	if(m_pList)
		m_pList->toggleList();
}

//=============================================
// @brief Manages a mouse button event
//
//=============================================
bool CUIDropDownList::CUIDropDownRowClickEvent::MouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown )
{
	if(!m_pList)
		return false;

	if(button != SDL_BUTTON_LEFT)
		return false;

	// Register that the button was clicked on us
	if(button == SDL_BUTTON_LEFT && keyDown)
		m_wasClickedOn = true;

	// Only left mouse click affects this
	if(button == SDL_BUTTON_LEFT && !keyDown && m_wasClickedOn)
	{
		m_pList->manageSelectionEvent(m_rowIndex);
		m_wasClickedOn = false;
		return true;
	}

	return true;
}

//=============================================
// @brief Determines if the cursor is touching this element
//
//=============================================
bool CUIDropDownList::isMouseOver( Int32 xPos, Int32 yPos )
{
	Int32 absX, absY;
	getAbsPosition(absX, absY);

	Uint32 height = m_height;
	if(m_pDropdownList->isVisible())
		height += m_pDropdownList->getHeight();

	if(absX > xPos)
		return false;
	if(absX+(Int32)m_width < xPos)
		return false;
	if(absY > yPos)
		return false;
	if(absY+(Int32)height < yPos)
		return false;

	return true;
}

//=============================================
// @brief Constructor
//
//=============================================
CUITickBox::CUITickBox( Int32 flags, CUICallbackEvent* pAction, Uint32 width, Uint32 height, Int32 originx, Int32 originy ):
	CUIButton( (flags|UIEL_FL_FIXED_H|UIEL_FL_FIXED_W), pAction, width, height, originx, originy ),
	m_pTickFlagObject(nullptr),
	m_isEnabled(false)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CUITickBox::~CUITickBox( void )
{
}

//=============================================
// @brief Destructor
//
//=============================================
bool CUITickBox::init( const Char* pstrSchemaName )
{
	m_pScheme = g_engineFuncs.pfnUILoadSchemaFile(pstrSchemaName);
	if(!m_pScheme)
		return false;

	// Get the schema for the checkbox outline
	const ui_schemeobject_t* pOutlineObject = m_pScheme->getObject("CheckBoxOutline");
	if(!pOutlineObject)
	{
		g_engineFuncs.pfnCon_EPrintf("%s - Cannot find schema object 'CheckBoxOutline' in '%s'.\n", __FUNCTION__, m_pScheme->schemeName.c_str());
		return false;
	}

	// Create icon object
	CUITexturedObject* pOutline = new CUITexturedObject((UIEL_FL_ALIGN_CH|UIEL_FL_ALIGN_CV), pOutlineObject->width, pOutlineObject->height, 0, 0);
	m_pDisplay = pOutline;
	m_pDisplay->setParent(this);

	// Set textures
	pOutline->setDefaultTexture(pOutlineObject->defaultTexture);
	pOutline->setFocusTexture(pOutlineObject->focusTexture);
	pOutline->setClickedTexture(pOutlineObject->clickTexture);

	// Add the tick flag
	const ui_schemeobject_t* pFlagObject = m_pScheme->getObject("CheckBoxFlag");
	if(!pFlagObject)
	{
		g_engineFuncs.pfnCon_EPrintf("%s - Cannot find schema object 'CheckBoxFlag' in '%s'.\n", __FUNCTION__, m_pScheme->schemeName.c_str());
		return false;
	}

	CUITexturedObject* pTickFlag = new CUITexturedObject((UIEL_FL_ALIGN_CH|UIEL_FL_ALIGN_CV), pFlagObject->width, pFlagObject->height, 0, 0);
	m_pTickFlagObject = pTickFlag;
	m_pTickFlagObject->setParent(this);

	pTickFlag->setDefaultTexture(pFlagObject->defaultTexture);
	pTickFlag->setFocusTexture(pFlagObject->focusTexture);
	pTickFlag->setClickedTexture(pFlagObject->clickTexture);

	// By default it's off
	m_pTickFlagObject->setVisible(false);
	
	return true;
}

//=============================================
// @brief Sets the string to display
//
//=============================================
void CUITickBox::setText( const Char* pstrText, const font_set_t* pFont, Uint32 textOffset )
{
	// Create the description text
	CUIText* pText = new CUIText(UIEL_FL_ALIGN_CV, pFont, pstrText, m_width + textOffset, 0, 0);
	pText->setParent(this);
}

//=============================================
// @brief Sets the string to display
//
//=============================================
void CUITickBox::setState( bool isEnabled )
{
	m_isEnabled = isEnabled;
	m_pTickFlagObject->setVisible(isEnabled);
}

//=============================================
// @brief Sets the string to display
//
//=============================================
bool CUITickBox::mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown )
{
	// Only left mouse click affects draggers
	if(button != SDL_BUTTON_LEFT)
		return false;

	if(isMouseOver(mouseX, mouseY))
	{
		if(keyDown)
		{
			if(!m_isClickedOn)
			{
				m_isClickedOn = true;
			}
		}
		else if(m_isClickedOn)
		{
			// Toggle us
			if(m_isEnabled)
			{
				m_isEnabled = false;
				m_pTickFlagObject->setVisible(false);
			}
			else
			{
				m_isEnabled = true;
				m_pTickFlagObject->setVisible(true);
			}

			// Peform the action
			if(m_pAction)
				m_pAction->PerformAction(m_isEnabled);

			m_isClickedOn = false;
		}

		return true;
	}

	return false;
}

//=============================================
// @brief Constructor
//
//=============================================
CUISlider::CUISlider( Int32 flags, CUICallbackEvent* pEvent, Uint32 width, Uint32 height, Int32 originx, Int32 originy, Float minValue, Float maxValue, Float markerdistance ):
	CUIObject( (flags|UIEL_FL_FIXED_H|UIEL_FL_FIXED_W), width, height, originx, originy ),
	m_minValue(minValue),
	m_maxValue(maxValue),
	m_markerDistance( markerdistance ),
	m_value(0),
	m_pEvent(pEvent),
	m_pSliderButton(nullptr),
	m_pLeftObject(nullptr),
	m_pBodyObject(nullptr),
	m_pRightObject(nullptr)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CUISlider::~CUISlider( void )
{
	if(m_pEvent)
		delete m_pEvent;
}

//=============================================
// @brief Destructor
//
//=============================================
bool CUISlider::draw( void )
{
	// Determine marker height
	Float markerHeight = (Float)m_height/2.0f - (Float)m_pBodyObject->getHeight()/2.0f - 4;
	Float markerWidth = 1.0f; // 2 pixels

	Float distJump = (m_maxValue - m_minValue) / (1.0/m_markerDistance);

	// Draw the markers
	for(Float dist = 0; dist <= m_width; dist += distJump)
	{
		Int32 absX, absY;
		getAbsPosition(absX, absY);

		// Draw the marker
		Float xPos = absX + dist;
		Float yPos = absY + m_height - markerHeight + (Float)m_pBodyObject->getHeight()/2.0f;

		if(!g_engineFuncs.pfnBasicDrawDisableTextures())
			return false;

		g_engineFuncs.pfnBasicDrawColor4f(1.0, 1.0, 1.0, 1.0);

		g_engineFuncs.pfnValidateBasicDraw();

		// Draw it
		g_engineFuncs.pfnBasicDrawBegin(GL_TRIANGLES);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xPos,				(Float)yPos + markerHeight,		-1);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xPos,				(Float)yPos,					-1);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xPos+markerWidth,	(Float)yPos,					-1);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xPos,				(Float)yPos + markerHeight,		-1);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xPos+markerWidth,	(Float)yPos,					-1);
		g_engineFuncs.pfnBasicDrawVertex3f((Float)xPos+markerWidth,	(Float)yPos + markerHeight,		-1);
		g_engineFuncs.pfnBasicDrawEnd();
	}

	// Call base class to draw children
	return CUIObject::draw();
}

//=============================================
// @brief Destructor
//
//=============================================
bool CUISlider::init( const Char* pstrSchemaName )
{
	// Get the height and width of the elements
	ui_schemeinfo_t* pschemeinfo = g_engineFuncs.pfnUILoadSchemaFile(pstrSchemaName);
	if(!pschemeinfo)
	{
		g_engineFuncs.pfnCon_EPrintf("Failed to load schema file '%s'.\n", pstrSchemaName);
		return false;
	}

	const ui_schemeobject_t* pLeftObject = pschemeinfo->getObject("Left");
	if(!pLeftObject)
	{
		g_engineFuncs.pfnCon_EPrintf("Schema file '%s' is missing the 'Left' object.\n", pstrSchemaName);
		return false;
	}

	const ui_schemeobject_t* pBodyObject = pschemeinfo->getObject("Body");
	if(!pBodyObject)
	{
		g_engineFuncs.pfnCon_EPrintf("Schema file '%s' is missing the 'Body' object.\n", pstrSchemaName);
		return false;
	}

	const ui_schemeobject_t* pRightObject = pschemeinfo->getObject("Right");
	if(!pRightObject)
	{
		g_engineFuncs.pfnCon_EPrintf("Schema file '%s' is missing the 'Right' object.\n", pstrSchemaName);
		return false;
	}

	// Create the left object
	m_pLeftObject = new CUITexturedObject(UIEL_FL_ALIGN_CV, pLeftObject->width, pLeftObject->height, 0, 0);
	m_pLeftObject->setParent(this);

	en_texture_t* pLeftTexture = pLeftObject->defaultTexture;
	m_pLeftObject->setDefaultTexture(pLeftTexture);

	// Create the body object
	Uint32 bodyWidth = m_width - pLeftObject->width - pRightObject->width;
	m_pBodyObject = new CUITexturedObject(UIEL_FL_ALIGN_CV, bodyWidth, pBodyObject->height, pBodyObject->width, 0);
	m_pBodyObject->setParent(this);

	en_texture_t* pBodyTexture = pBodyObject->defaultTexture;
	m_pBodyObject->setDefaultTexture(pBodyTexture);

	// Create the left object
	Uint32 originx = pLeftObject->width + m_pBodyObject->getWidth();
	m_pRightObject = new CUITexturedObject(UIEL_FL_ALIGN_CV, pRightObject->width, pRightObject->height, originx, 0);
	m_pRightObject->setParent(this);

	en_texture_t* pRightTexture = pRightObject->defaultTexture;
	m_pRightObject->setDefaultTexture(pRightTexture);

	if(m_minValue != 0.0 || m_maxValue != 1.0)
	{
		// Determine marker height
		Float markerHeight = (Float)m_height/2.0f - (Float)m_pBodyObject->getHeight()/2.0f - 4;

		Char valueStr[32];
		sprintf(valueStr, "%0.1f", m_minValue);

		Uint32 width, height;
		g_engineFuncs.pfnGetStringSize(gUIManager.GetDefaultFontSet(), valueStr, &width, &height, nullptr);

		// Create the "minimum value" string object
		Int32 originY = m_height - height + 4 + markerHeight;
		Int32 originX = -(width*0.5);
		CUIText* pMinValueText = new CUIText(UIEL_FL_NONE, gUIManager.GetDefaultFontSet(), valueStr, originX, originY, 0);
		pMinValueText->setParent(this);

		sprintf(valueStr, "%0.1f", m_maxValue);
		g_engineFuncs.pfnGetStringSize(gUIManager.GetDefaultFontSet(), valueStr, &width, &height, nullptr);

		// Create the "maximum value" string object
		originY = m_height - height + 4 + markerHeight;
		originX = m_width - width * 0.5;
		CUIText* pMaxValueText = new CUIText(UIEL_FL_NONE, gUIManager.GetDefaultFontSet(), valueStr, originX, originY, 0);
		pMaxValueText->setParent(this);
	}

	// Create the button
	Uint32 buttonHeight = m_height;
	Uint32 buttonWidth = ((m_maxValue - m_minValue) / (1.0/m_markerDistance)) / 2.0f;
	
	CUISliderDragBtnAction *pAction = new CUISliderDragBtnAction(this);
	m_pSliderButton = new CUIDragButton(UIEL_FL_ALIGN_CV, UIEL_SCROLL_H, pAction, buttonWidth, buttonHeight, 0, 0);
	m_pSliderButton->setParent(this);

	if(!m_pSliderButton->init("defaultbutton.txt"))
	{
		g_engineFuncs.pfnCon_EPrintf("Failed to initialize slider object 'CUISliderButton'.\n");
		return false;
	}

	return true;
}

//=============================================
// @brief Destructor
//
//=============================================
void CUISlider::setValueFromPosition( Double position )
{
	m_value = (m_maxValue-m_minValue)*position + m_minValue;

	// Call the event
	if(m_pEvent)
		m_pEvent->PerformAction(m_value);
}

//=============================================
// @brief Destructor
//
//=============================================
void CUISlider::setValue( Float value )
{
	if(value < m_minValue)
	{
		g_engineFuncs.pfnCon_EPrintf("Value too low for 'CUISlider', minimum is %f.\n", m_minValue);
		return;
	}
	else if(value > m_maxValue)
	{
		g_engineFuncs.pfnCon_EPrintf("Value too high for 'CUISlider', maximum is %f.\n", m_maxValue);
		return;
	}

	m_value = value;

	// Set the slider pos
	Float position = (m_value-m_minValue) / (m_maxValue-m_minValue);
	if(position < 0)
		position = 0;
	if(position > 1.0)
		position = 1.0;

	m_pSliderButton->setPosition(position);
	m_pSliderButton->adjPosition(0, true);
}

//=============================================
// @brief Performs the close action for the window
//
//=============================================
void CUISlider::CUISliderDragBtnAction::PerformAction( Float param )
{
	assert(m_pSlider != nullptr);

	// Get the dragger button
	CUIDragButton* pButton = m_pSlider->getSliderButton();
	Double position = pButton->getPosition();

	// Get parent of the scroller button
	m_pSlider->setValueFromPosition(position);
}

//=============================================
// @brief Constructor
//
//=============================================
CUIProgressBar::CUIProgressBar( Int32 flags, Uint32 width, Uint32 height, Int32 originx, Int32 originy ):
	CUISurface( (flags), width, height, originx, originy ),
	m_meterValue(0)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CUIProgressBar::~CUIProgressBar( void )
{
}

//=============================================
// @brief Draws the progress bar
//
//=============================================
bool CUIProgressBar::draw( void )
{
	// Call base class to draw the surface
	if(!CUISurface::draw())
		return false;

	if(m_meterValue <= 0)
		return true;

	// Get the sizes of the borders
	Uint32 bottomHeight = m_pBottomBorder->getHeight();
	Uint32 topHeight = m_pTopBorder->getHeight();

	Uint32 leftWidth = m_pLeftBorder->getWidth();
	Uint32 rightWidth = m_pRightBorder->getWidth();

	// Determine dimensions
	Float meterHeight = m_height - bottomHeight - topHeight;
	Float meterFullWidth = m_width - leftWidth - rightWidth;
	Float meterWidth = meterFullWidth * m_meterValue;

	// Draw the meter itself
	Int32 absX, absY;
	getAbsPosition(absX, absY);

	if(!g_engineFuncs.pfnBasicDrawDisableTextures())
		return false;

	g_engineFuncs.pfnBasicDrawColor4f(1.0, 1.0, 1.0, 1.0);

	Int32 xPos = absX + leftWidth;
	Int32 yPos = absY + topHeight;

	g_engineFuncs.pfnValidateBasicDraw();

	// Draw it
	g_engineFuncs.pfnBasicDrawBegin(GL_TRIANGLES);
	g_engineFuncs.pfnBasicDrawVertex3f((Float)xPos,					(Float)yPos + meterHeight,		-1);
	g_engineFuncs.pfnBasicDrawVertex3f((Float)xPos,					(Float)yPos,					-1);
	g_engineFuncs.pfnBasicDrawVertex3f((Float)xPos+meterWidth,		(Float)yPos,					-1);
	g_engineFuncs.pfnBasicDrawVertex3f((Float)xPos,					(Float)yPos + meterHeight,		-1);
	g_engineFuncs.pfnBasicDrawVertex3f((Float)xPos+meterWidth,		(Float)yPos,					-1);
	g_engineFuncs.pfnBasicDrawVertex3f((Float)xPos+meterWidth,		(Float)yPos + meterHeight,		-1);
	g_engineFuncs.pfnBasicDrawEnd();

	return true;
}

//=============================================
// @brief Draws the progress bar
//
//=============================================
void CUIProgressBar::setValue( Float value )
{
	// Do not allow values outside bounds
	m_meterValue = clamp(value, 0, 1);
}