/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gameuielements.h"
#include "clientdll.h"
#include "huddraw.h"
#include "fontset.h"
#include "gameuiwindows_shared.h"

// Pointer to game ui manager object
CGameUIManager* CGameUIObject::g_pGUIManager = nullptr;

//====================================
//
//====================================
CGameUIObject::CGameUIObject( Int32 flags, Int32 originX, Int32 originY, Uint32 width, Uint32 height ):
	m_flags(flags),
	m_originX(0),
	m_originY(0),
	m_baseOriginX(originX),
	m_baseOriginY(originY),
	m_width(width),
	m_height(height),
	m_pParent(nullptr),
	m_pLinkedObject(nullptr),
	m_isVisible(true),
	m_isInputFocusObject(false)
{
	if(!(m_flags & (FL_ALIGN_LEFT|FL_ALIGN_RIGHT|FL_ALIGN_CH)))
		m_flags |= FL_ALIGN_LEFT;

	if(!(m_flags & (FL_ALIGN_TOP|FL_ALIGN_BOTTOM|FL_ALIGN_CV)))
		m_flags |= FL_ALIGN_TOP;
}

//====================================
//
//====================================
CGameUIObject::~CGameUIObject( void )
{
	if(!m_pChildrenArray.empty())
	{
		for(Uint32 i = 0; i < m_pChildrenArray.size(); i++)
			delete m_pChildrenArray[i];

		m_pChildrenArray.clear();
	}
}

//====================================
//
//====================================
void CGameUIObject::setParent( CGameUIObject* pparent )
{
	m_pParent = pparent;
	m_pParent->addChild(this);

	// Re-adjust position
	adjustPosition();
}

//====================================
//
//====================================
CGameUIObject* CGameUIObject::getParent( void )
{
	return m_pParent;
}

//====================================
//
//====================================
void CGameUIObject::addChild( CGameUIObject* pchild )
{
	for(Uint32 i = 0; i < m_pChildrenArray.size(); i++)
	{
		if(m_pChildrenArray[i] == pchild)
		{
			cl_engfuncs.pfnCon_Printf("%s - Child already present.\n", __FUNCTION__);
			return;
		}
	}

	m_pChildrenArray.push_back(pchild);
}

//====================================
//
//====================================
void CGameUIObject::setPosition( Int32 xcoord, Int32 ycoord )
{
	m_originX = xcoord;
	m_originY = ycoord;
}

//====================================
//
//====================================
void CGameUIObject::getPosition( Int32& xcoord, Int32& ycoord ) const
{
	xcoord = m_originX;
	ycoord = m_originY;
}

//====================================
//
//====================================
void CGameUIObject::getAbsolutePosition( Int32& xcoord, Int32& ycoord )
{
	if(m_pParent)
	{
		Int32 parentx = 0;
		Int32 parenty = 0;

		m_pParent->getAbsolutePosition(parentx, parenty);

		xcoord = parentx + m_originX;
		ycoord = parenty + m_originY;
	}
}

//====================================
//
//====================================
void CGameUIObject::setSize( Uint32 width, Uint32 height )
{
	m_width = width;
	m_height = height;
}

//====================================
//
//====================================
void CGameUIObject::getSize( Uint32& width, Uint32& height ) const
{
	width = m_width;
	height = m_height;
}

//====================================
//
//====================================
Uint32 CGameUIObject::getWidth( void ) const
{
	return m_width;
}

//====================================
//
//====================================
Uint32 CGameUIObject::getHeight( void ) const
{
	return m_height;
}

//====================================
//
//====================================
void CGameUIObject::setVisible( bool visible )
{
	m_isVisible = visible;
}

//====================================
//
//====================================
bool CGameUIObject::isVisible( void ) const
{
	return m_isVisible;
}

//====================================
//
//====================================
bool CGameUIObject::isMouseOver( Int32 mousex, Int32 mousey )
{
	Int32 absX, absY;
	getAbsolutePosition(absX, absY);

	if(absX > mousex)
		return false;
	if(absX+(Int32)m_width < mousex)
		return false;
	if(absY > mousey)
		return false;
	if(absY+(Int32)m_height < mousey)
		return false;

	return true;
}

//====================================
//
//====================================
bool CGameUIObject::isMouseOver( void )
{
	Int32 absX, absY;
	getAbsolutePosition(absX, absY);

	Int32 mousex, mousey;
	cl_engfuncs.pfnGetMousePosition(mousex, mousey);

	if(absX > mousex)
		return false;
	if(absX+(Int32)m_width < mousex)
		return false;
	if(absY > mousey)
		return false;
	if(absY+(Int32)m_height < mousey)
		return false;

	return true;
}

//====================================
//
//====================================
void CGameUIObject::setInputFocusObject( CGameUIObject* pObject )
{
	if(!m_pParent)
		return;

	m_pParent->setInputFocusObject(pObject);
}

//====================================
//
//====================================
bool CGameUIObject::isInputFocusObject( void )
{
	return m_isInputFocusObject;
}

//====================================
//
//====================================
void CGameUIObject::setIsInputFocusObject( bool isFocusObject )
{
	m_isInputFocusObject = isFocusObject;
}

//====================================
//
//====================================
Int32 CGameUIObject::getFlags( void ) const
{
	return m_flags;
}

//====================================
//
//====================================
void CGameUIObject::think( void )
{
	if(m_pChildrenArray.empty())
		return;

	for(Uint32 i = 0; i < m_pChildrenArray.size(); i++)
		m_pChildrenArray[i]->think();
}

//====================================
//
//====================================
bool CGameUIObject::draw( void )
{
	if(!m_isVisible)
		return true;

	if(m_pChildrenArray.empty())
		return true;

	for(Uint32 i = 0; i < m_pChildrenArray.size(); i++)
	{
		CGameUIObject* pObject = m_pChildrenArray[i];
		if(pObject->getFlags() & FL_DRAW_LINKED_HIGHLIGHT_ONLY)
		{
			CGameUIObject* pLinkedObject = pObject->getLinkedObject();
			if(!pLinkedObject->isMouseOver())
				continue;
		}

		if(!pObject->isVisible())
			continue;

		if(!pObject->draw())
			return false;
	}

	return true;
}

//====================================
//
//====================================
void CGameUIObject::adjustPosition( void )
{
	if(!m_pChildrenArray.empty())
	{
		for(Uint32 i = 0; i < m_pChildrenArray.size(); i++)
			m_pChildrenArray[i]->adjustPosition();
	}
	
	if(!m_pParent)
		return;

	Uint32 parentwidth, parentheight;
	m_pParent->getSize(parentwidth, parentheight);

	if(m_flags & FL_ALIGN_RIGHT)
		m_originX = parentwidth - m_width - m_baseOriginX;
	else if(m_flags & FL_ALIGN_CH)
		m_originX = (parentwidth / 2) - (m_width/2);
	else
		m_originX = m_baseOriginX;

	if(m_flags & FL_ALIGN_BOTTOM)
		m_originY = parentheight - m_height - m_baseOriginY;
	else if(m_flags & FL_ALIGN_CV)
		m_originY = (parentheight / 2) - (m_height/2);
	else
		m_originY = m_baseOriginY;
}

//====================================
//
//====================================
bool CGameUIObject::mouseWheelEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown, Int32 scroll )
{
	if(!m_isVisible)
		return false;

	if(!isInteractive())
		return false;

	if(m_pChildrenArray.empty())
		return false;

	for(Uint32 i = 0; i < m_pChildrenArray.size(); i++)
	{
		CGameUIObject* pObject = m_pChildrenArray[i];
		if(pObject->getFlags() & FL_DRAW_LINKED_HIGHLIGHT_ONLY)
		{
			CGameUIObject* pLinkedObject = pObject->getLinkedObject();
			if(!pLinkedObject->isMouseOver())
				continue;
		}

		if(pObject->mouseWheelEvent(mouseX, mouseY, button, keyDown, scroll))
			return true;
	}

	return false;
}

//====================================
//
//====================================
bool CGameUIObject::mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown )
{
	if(!m_isVisible)
		return false;

	if(!isInteractive())
		return false;

	if(m_pChildrenArray.empty())
		return false;

	for(Uint32 i = 0; i < m_pChildrenArray.size(); i++)
	{
		CGameUIObject* pObject = m_pChildrenArray[i];
		if(pObject->getFlags() & FL_DRAW_LINKED_HIGHLIGHT_ONLY)
		{
			CGameUIObject* pLinkedObject = pObject->getLinkedObject();
			if(!pLinkedObject->isMouseOver())
				continue;
		}

		if(pObject->mouseButtonEvent(mouseX, mouseY, button, keyDown))
			return true;
	}

	return false;
}

//====================================
//
//====================================
bool CGameUIObject::keyEvent( Int32 button, Int16 mod, bool keyDown )
{
	if(!m_isVisible)
		return false;

	if(!isInteractive())
		return false;

	if(m_pChildrenArray.empty())
		return false;

	for(Uint32 i = 0; i < m_pChildrenArray.size(); i++)
	{
		CGameUIObject* pObject = m_pChildrenArray[i];
		if(pObject->getFlags() & FL_DRAW_LINKED_HIGHLIGHT_ONLY)
		{
			CGameUIObject* pLinkedObject = pObject->getLinkedObject();
			if(!pLinkedObject->isMouseOver())
				continue;
		}

		if(pObject->keyEvent(button, mod, keyDown))
			return true;
	}

	return false;
}

//====================================
//
//====================================
void CGameUIObject::setLinkedObject( CGameUIObject* pObject )
{
	m_pLinkedObject = pObject;
}

//====================================
//
//====================================
CGameUIObject* CGameUIObject::getLinkedObject( void )
{
	return m_pLinkedObject;
}

//====================================
//
//====================================
void CGameUIObject::setButtonsDisabled( bool isDisabled )
{
	if(m_pChildrenArray.empty())
		return;

	for(Uint32 i = 0; i < m_pChildrenArray.size(); i++)
	{
		CGameUIObject* pObject = m_pChildrenArray[i];
		
		if(pObject->isButton())
			pObject->setDisabled(isDisabled);
		else
			pObject->setButtonsDisabled(isDisabled);

	}
}

//====================================
//
//====================================
void CGameUIObject::SetGameUIManager( CGameUIManager* pGUIManager )
{
	g_pGUIManager = pGUIManager;
}

//====================================
//
//====================================
CGameUIBar::CGameUIBar( Int32 flags, const color32_t& color, byte minalpha, Int32 fadeflags, Int32 originX, Int32 originY, Uint32 width, Uint32 height ):
	CGameUIObject(flags, originX, originY, width, height),
	m_fadeFlags(fadeflags),
	m_color(color),
	m_minAlpha(minalpha)
{
}

//====================================
//
//====================================
CGameUIBar::~CGameUIBar( void )
{
}

//====================================
//
//====================================
bool CGameUIBar::draw( void )
{
	// Retrieve the absolute position
	Int32 xposition, yposition;
	getAbsolutePosition(xposition, yposition);

	// Set fade
	byte fademod[4];
	for(Uint32 i = 0; i < 4; i++)
		fademod[i] = 255;

	if(m_fadeFlags & FL_BAR_FADE_LEFT)
	{
		fademod[0] = m_minAlpha;
		fademod[1] = m_minAlpha;
	}
	else if(m_fadeFlags & FL_BAR_FADE_RIGHT)
	{
		fademod[2] = m_minAlpha;
		fademod[3] = m_minAlpha;
	}

	if(m_fadeFlags & FL_BAR_FADE_TOP)
	{
		fademod[1] = m_minAlpha;
		fademod[2] = m_minAlpha;
	}
	else if(m_fadeFlags & FL_BAR_FADE_BOTTOM)
	{
		fademod[0] = m_minAlpha;
		fademod[3] = m_minAlpha;
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gHUDDraw.SetOrigin(xposition, yposition);
	gHUDDraw.SetSize(m_width, m_height);
	gHUDDraw.SetAlphaMod(fademod[0], fademod[1], fademod[2], fademod[3]);
	gHUDDraw.SetColor(m_color.r, m_color.g, m_color.b, m_color.a);
	if(!gHUDDraw.DrawQuad(nullptr))
		return false;

	// Reset fade mod
	gHUDDraw.SetAlphaMod(255, 255, 255, 255);

	// Call base class to render
	return CGameUIObject::draw();
}

//====================================
//
//====================================
CGameUISurface::CGameUISurface( Int32 flags, Uint32 edgethickness, const color32_t& color, const color32_t& bgcolor, Int32 originX, Int32 originY, Uint32 width, Uint32 height ):
	CGameUIObject(flags, originX, originY, width, height),
	m_color(color),
	m_backgroundColor(bgcolor),
	m_edgeThickness(edgethickness)
{
}

//====================================
//
//====================================
CGameUISurface::~CGameUISurface( void )
{
}

//====================================
//
//====================================
bool CGameUISurface::draw( void )
{
	Int32 baseOriginX, baseOriginY;
	getAbsolutePosition(baseOriginX, baseOriginY);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if(m_color.r != m_backgroundColor.r|| m_color.g != m_backgroundColor.b
		|| m_color.b != m_backgroundColor.b || m_color.a != m_backgroundColor.a)
	{
		Int32 originx = baseOriginX;
		Int32 originy = baseOriginY;

		if(m_backgroundColor.r != 0 || m_backgroundColor.g != 0 || m_backgroundColor.b != 0 || m_backgroundColor.a != 0)
		{
			// Draw background
			gHUDDraw.SetColor(m_backgroundColor.r, m_backgroundColor.g, m_backgroundColor.b, m_backgroundColor.a);
			gHUDDraw.SetOrigin(baseOriginX+m_edgeThickness, baseOriginY+m_edgeThickness);
			if(!(m_flags & FL_NO_BOTTOM_BORDER))
				gHUDDraw.SetSize(m_width-m_edgeThickness, m_height-m_edgeThickness);
			else
				gHUDDraw.SetSize(m_width-m_edgeThickness, m_height);

			if(!gHUDDraw.DrawQuad(nullptr))
				return false;
		}

		// Draw top edge
		gHUDDraw.SetColor(m_color.r, m_color.g, m_color.b, m_color.a);
		gHUDDraw.SetOrigin(originx, originy);
		gHUDDraw.SetSize(m_width, m_edgeThickness);

		if(!gHUDDraw.DrawQuad(nullptr))
			return false;

		// Draw left edge
		originy = baseOriginY + m_edgeThickness;
		Uint32 verticalBarHeight = m_height - m_edgeThickness;
		if(!(m_flags & FL_NO_BOTTOM_BORDER))
			verticalBarHeight -= m_edgeThickness;

		gHUDDraw.SetOrigin(originx, originy);
		gHUDDraw.SetSize(m_edgeThickness, verticalBarHeight);
		if(!gHUDDraw.DrawQuad(nullptr))
			return false;

		// Draw bottom edge
		if(!(m_flags & FL_NO_BOTTOM_BORDER))
		{
			originy = baseOriginY + m_edgeThickness + verticalBarHeight;
			gHUDDraw.SetOrigin(originx, originy);
			gHUDDraw.SetSize(m_width, m_edgeThickness);
			if(!gHUDDraw.DrawQuad(nullptr))
				return false;
		}

		// Draw right edge
		originx = baseOriginX + (m_width - m_edgeThickness);
		originy = baseOriginY + m_edgeThickness;
		gHUDDraw.SetOrigin(originx, originy);
		gHUDDraw.SetSize(m_edgeThickness, verticalBarHeight);
		if(!gHUDDraw.DrawQuad(nullptr))
			return false;
	}
	else
	{
		// Draw background
		gHUDDraw.SetColor(m_color.r, m_color.g, m_color.b, m_color.a);
		gHUDDraw.SetOrigin(baseOriginX+m_edgeThickness, baseOriginY+m_edgeThickness);
		gHUDDraw.SetSize(m_width-m_edgeThickness, m_height-m_edgeThickness);
		if(!gHUDDraw.DrawQuad(nullptr))
			return false;
	}

	// Call base class to render
	return CGameUIObject::draw();
}

//====================================
//
//====================================
CGameUIButton::CGameUIButton( Int32 flags, CGameUICallbackEvent* pEvent, Uint32 edgethickness, const color32_t& color, const color32_t& bgcolor, const color32_t& highlightcolor, Int32 originX, Int32 originY, Uint32 width, Uint32 height ):
	CGameUIObject(flags, originX, originY, width, height),
	m_isClickedOn(false),
	m_isDisabled(false),
	m_isHighlighted(false),
	m_pEvent(pEvent),
	m_color(color),
	m_backgroundColor(bgcolor),
	m_highlightColor(highlightcolor),
	m_edgeThickness(edgethickness),
	m_pButtonText(nullptr)
{
}

//====================================
//
//====================================
CGameUIButton::CGameUIButton( Int32 flags, CGameUICallbackEvent* pEvent, SDL_Keycode keycode, Uint32 edgethickness, const color32_t& color, const color32_t& bgcolor, const color32_t& highlightcolor, Int32 originX, Int32 originY, Uint32 width, Uint32 height ):
	CGameUIObject(flags, originX, originY, width, height),
	m_isClickedOn(false),
	m_isDisabled(false),
	m_isHighlighted(false),
	m_pEvent(pEvent),
	m_color(color),
	m_backgroundColor(bgcolor),
	m_highlightColor(highlightcolor),
	m_edgeThickness(edgethickness),
	m_pButtonText(nullptr)
{
	m_buttonKeysArray.push_back(keycode);
}

//====================================
//
//====================================
CGameUIButton::CGameUIButton( Int32 flags, CGameUICallbackEvent* pEvent, const CArray<SDL_Keycode>& keycodesArray, Uint32 edgethickness, const color32_t& color, const color32_t& bgcolor, const color32_t& highlightcolor, Int32 originX, Int32 originY, Uint32 width, Uint32 height ):
	CGameUIObject(flags, originX, originY, width, height),
	m_isClickedOn(false),
	m_isDisabled(false),
	m_isHighlighted(false),
	m_pEvent(pEvent),
	m_buttonKeysArray(keycodesArray),
	m_color(color),
	m_backgroundColor(bgcolor),
	m_highlightColor(highlightcolor),
	m_edgeThickness(edgethickness),
	m_pButtonText(nullptr)
{
}

//====================================
//
//====================================
CGameUIButton::~CGameUIButton( void )
{
	if(m_pEvent)
	{
		delete m_pEvent;
		m_pEvent = nullptr;
	}
}

//====================================
//
//====================================
bool CGameUIButton::draw( void )
{
	Int32 baseOriginX, baseOriginY;
	getAbsolutePosition(baseOriginX, baseOriginY);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if(m_color.r == m_backgroundColor.r && m_color.g == m_backgroundColor.g
		&& m_color.b == m_backgroundColor.b && m_color.a == m_backgroundColor.a
		|| m_isClickedOn)
	{
		// Draw background
		gHUDDraw.SetColor(m_color.r, m_color.g, m_color.b, m_color.a);
		gHUDDraw.SetOrigin(baseOriginX, baseOriginY);
		gHUDDraw.SetSize(m_width, m_height);
		if(!gHUDDraw.DrawQuad(nullptr))
			return false;
	}
	else
	{
		Int32 originx = baseOriginX;
		Int32 originy = baseOriginY;

		if(((m_isHighlighted || isMouseOver()) && !m_isDisabled) && (m_highlightColor.r != 0 || m_highlightColor.g != 0 || m_highlightColor.b != 0 || m_highlightColor.a != 0))
		{
			// Draw background with highlight color
			gHUDDraw.SetColor(m_highlightColor.r, m_highlightColor.g, m_highlightColor.b, m_highlightColor.a);
			gHUDDraw.SetOrigin(baseOriginX+m_edgeThickness, baseOriginY+m_edgeThickness);
			gHUDDraw.SetSize(m_width-m_edgeThickness, m_height-m_edgeThickness);
			if(!gHUDDraw.DrawQuad(nullptr))
				return false;
		}
		else if(m_backgroundColor.r != 0 || m_backgroundColor.g != 0 || m_backgroundColor.b != 0 || m_backgroundColor.a != 0)
		{
			// Draw background
			gHUDDraw.SetColor(m_backgroundColor.r, m_backgroundColor.g, m_backgroundColor.b, m_backgroundColor.a);
			gHUDDraw.SetOrigin(baseOriginX+m_edgeThickness, baseOriginY+m_edgeThickness);
			gHUDDraw.SetSize(m_width-m_edgeThickness, m_height-m_edgeThickness);
			if(!gHUDDraw.DrawQuad(nullptr))
				return false;
		}

		// Draw top edge
		gHUDDraw.SetColor(m_color.r, m_color.g, m_color.b, m_color.a);
		gHUDDraw.SetOrigin(originx, originy);
		gHUDDraw.SetSize(m_width, m_edgeThickness);
		if(!gHUDDraw.DrawQuad(nullptr))
			return false;

		// Draw left edge
		originy = baseOriginY + m_edgeThickness;
		Uint32 verticalBarHeight = m_height - m_edgeThickness*2;
		gHUDDraw.SetOrigin(originx, originy);
		gHUDDraw.SetSize(m_edgeThickness, verticalBarHeight);
		if(!gHUDDraw.DrawQuad(nullptr))
			return false;

		// Draw bottom edge
		originy = baseOriginY + m_edgeThickness + verticalBarHeight;
		gHUDDraw.SetOrigin(originx, originy);
		gHUDDraw.SetSize(m_width, m_edgeThickness);
		if(!gHUDDraw.DrawQuad(nullptr))
			return false;

		// Draw right edge
		originx = baseOriginX + (m_width - m_edgeThickness);
		originy = baseOriginY + m_edgeThickness;
		gHUDDraw.SetOrigin(originx, originy);
		gHUDDraw.SetSize(m_edgeThickness, verticalBarHeight);
		if(!gHUDDraw.DrawQuad(nullptr))
			return false;
	}

	// Call base class to render
	return CGameUIObject::draw();
}

//====================================
//
//====================================
bool CGameUIButton::mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown )
{
	// Only left mouse click affects draggers
	if(button != SDL_BUTTON_LEFT)
		return false;

	if(m_isDisabled)
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
			if(m_pEvent)
				m_pEvent->PerformAction(0);

			m_isClickedOn = false;
		}

		return true;
	}

	return false;
}

//====================================
//
//====================================
bool CGameUIButton::keyEvent( Int32 button, Int16 mod, bool keyDown )
{
	if(m_buttonKeysArray.empty())
		return false;

	for(Uint32 i = 0; i < m_buttonKeysArray.size(); i++)
	{
		SDL_Keycode buttonKey = m_buttonKeysArray[i];
		if(buttonKey == SDLK_UNKNOWN)
			return false;

		if(m_isDisabled)
			return false;

		SDL_Keycode sdlKeycode = SDL_GetKeyFromScancode((SDL_Scancode)button);
		if(buttonKey == sdlKeycode && keyDown)
		{
			m_isClickedOn = true;

			if(m_pEvent)
				m_pEvent->PerformAction(0);

			break;
		}
	}

	return false;
}

//====================================
//
//====================================
void CGameUIButton::think( void )
{
	if(m_isClickedOn)
	{
		if(!isMouseOver())
			m_isClickedOn = false;
	}

	// Call base to handle the rest
	CGameUIObject::think();
}

//====================================
//
//====================================
void CGameUIButton::setText( const Char* pstrText )
{
	if(!m_pButtonText)
	{
		m_pButtonText = new CGameUIText(FL_ALIGN_CV|FL_ALIGN_CH, m_color, nullptr, pstrText, 0, 0);
		m_pButtonText->setParent(this);
	}
	else
		m_pButtonText->setText(pstrText);
}

//====================================
//
//====================================
void CGameUIButton::setDisabled( bool isDisabled )
{
	m_isDisabled = isDisabled;
}

//====================================
//
//====================================
bool CGameUIButton::isDisabled( void )
{
	return m_isDisabled;
}

//====================================
//
//====================================
void CGameUIButton::setHighlighted( bool isHighlighted )
{
	m_isHighlighted = isHighlighted;
}

//====================================
// 
//====================================
void CGameUIButton::setBgColor( const color32_t& color )
{
	m_backgroundColor = color;
}

//====================================
//
//====================================
CGameUIText::CGameUIText( Int32 flags, const color32_t& color, const font_set_t* pfontset, const Char* pstrText, Int32 originX, Int32 originY ):
	CGameUIObject(flags, originX, originY, 0, 0),
	m_color(color),
	m_text(pstrText),
	m_pFontSet(pfontset),
	m_textInset(0),
	m_maxWidth(0),
	m_maxHeight(0)
{
	if(!m_pFontSet)
		m_pFontSet = gGameUIManager.GetDefaultFontSet();

	// Call to adjust size
	adjustSize();
}

//====================================
//
//====================================
CGameUIText::CGameUIText( Int32 flags, const color32_t& color, const font_set_t* pfontset, Int32 originX, Int32 originY ):
	CGameUIObject(flags, originX, originY, 0, 0),
	m_color(color),
	m_pFontSet(pfontset),
	m_textInset(0),
	m_maxWidth(0),
	m_maxHeight(0)
{
	if(!m_pFontSet)
		m_pFontSet = gGameUIManager.GetDefaultFontSet();
}

//====================================
//
//====================================
CGameUIText::CGameUIText( Int32 flags, const color32_t& color, const font_set_t* pfontset, const Char* pstrText, Int32 originX, Int32 originY, Uint32 maxwidth, Uint32 maxheight, Uint32 textInset ):
	CGameUIObject(flags, originX, originY, 0, 0),
	m_color(color),
	m_text(pstrText),
	m_pFontSet(pfontset),
	m_textInset(textInset),
	m_maxWidth(maxwidth),
	m_maxHeight(maxheight)
{
	if(!m_pFontSet)
		m_pFontSet = gGameUIManager.GetDefaultFontSet();

	// Call to adjust size
	adjustSize();
}

//====================================
//
//====================================
CGameUIText::~CGameUIText( void )
{
}

//====================================
//
//====================================
bool CGameUIText::draw( void )
{
	gHUDDraw.FinishDraw();

	Int32 xpos, ypos;
	getAbsolutePosition(xpos, ypos);

	if(!m_maxWidth || !m_maxHeight)
	{
		Uint32 height = cl_renderfuncs.pfnEstimateStringHeight(m_pFontSet, m_text.c_str(), 0);
		if(!cl_renderfuncs.pfnDrawSimpleString(m_color, xpos, ypos+height, m_text.c_str(), m_pFontSet))
			return false;
	}
	else
	{
		if(!cl_renderfuncs.pfnDrawStringBox(0, 0, 
			m_maxWidth, m_maxHeight, 
			m_textInset, m_textInset, 
			false, 
			m_color, 
			xpos, ypos, 
			m_text.c_str(),
			m_pFontSet,
			0, 
			m_pFontSet->fontsize,
			0))
			return false;
	}

	if(!gHUDDraw.SetupDraw())
		return false;

	// Call base to handle the rest
	return CGameUIObject::draw();
}

//====================================
//
//====================================
void CGameUIText::setText( const Char* pstrText )
{
	m_text = pstrText;
	adjustSize();
}

//====================================
//
//====================================
void CGameUIText::setFontSet( const font_set_t* pFontSet )
{
	m_pFontSet = pFontSet;
	adjustSize();
}

//====================================
//
//====================================
void CGameUIText::setColor( const color32_t& fontcolor )
{
	m_color = fontcolor;
}

//====================================
//
//====================================
void CGameUIText::adjustSize( void )
{
	// Calculate string size
	Uint32 stringwidth = 0;
	Uint32 stringheight = 0;
	
	const Char* pstr = m_text.c_str();
	while(*pstr != '\0')
	{
		stringwidth += m_pFontSet->glyphs[*pstr].advancex;
		if(stringheight < m_pFontSet->glyphs[*pstr].height)
			stringheight = m_pFontSet->glyphs[*pstr].height;

		pstr++;
	}

	// Assign final size
	m_width = stringwidth;
	m_height = stringheight;

	// Adjust position relative to parent
	adjustPosition();
}

//====================================
//
//====================================
CGameUITextTab::CGameUITextTab( Int32 flags, const font_set_t* pfontset, Uint32 textinset, Uint32 edgethickness, const color32_t& color, const color32_t& bgcolor, const color32_t& highlightcolor, const color32_t& textcolor, Int32 originX, Int32 originY, Uint32 width, Uint32 height ):
	CGameUISurface(flags, edgethickness, color, bgcolor, originX, originY, width, height),
	m_textInset(textinset),
	m_pFontSet(pfontset),
	m_textOffset(0),
	m_textColor(textcolor),
	m_pScroller(nullptr)
{
	if(!m_pFontSet)
		m_pFontSet = gGameUIManager.GetDefaultFontSet();

	m_pScroller = new CGameUIScroller(CGameUIObject::FL_NONE, CGameUIObject::FL_SCROLL_V, edgethickness, color, highlightcolor, highlightcolor, m_pFontSet->fontsize, 16, m_height - m_edgeThickness*2, 0, m_edgeThickness);
	m_pScroller->setParent(this);
}

//====================================
//
//====================================
CGameUITextTab::~CGameUITextTab( void )
{
}

//====================================
//
//====================================
bool CGameUITextTab::draw( void )
{
	// Call base class to render
	if(!CGameUISurface::draw())
		return false;

	if(!m_displayText.empty())
	{
		// Draw the text
		gHUDDraw.FinishDraw();

		Int32 originx, originy;
		getAbsolutePosition(originx, originy);

		// Set drawing rectangle
		if(!cl_renderfuncs.pfnDrawStringBox(0, 0, 
			m_width - m_pScroller->getWidth(), m_height, 
			m_textInset, m_textInset, 
			false, 
			m_textColor, 
			originx, originy, 
			m_displayText.c_str(), 
			m_pFontSet, 
			m_textOffset, 
			m_pFontSet->fontsize, 
			0))
			return false;

		// Reset HUD renderer
		if(!gHUDDraw.SetupDraw())
			return false;
	}

	return true;
}

//====================================
//
//====================================
void CGameUITextTab::initData( const byte* pdata, Uint32 datasize )
{
	// Set the text to display
	m_displayText.assign(reinterpret_cast<const Char*>(pdata), datasize);
	updateRangeSize();
}

//====================================
//
//====================================
void CGameUITextTab::setFontSet( const font_set_t* pfontset )
{
	m_pFontSet = pfontset;
	m_pScroller->setUnitSize(m_pFontSet->fontsize);
}

//====================================
//
//====================================
void CGameUITextTab::setTextColor( const color32_t& textcolor )
{
	m_textColor = textcolor;
}

//=============================================
// @brief Sets the offset value for the element
//
//=============================================
void CGameUITextTab::setOffsetValue( Float offset )
{
	if(m_displayText.empty())
		return;

	cl_renderfuncs.pfnSetStringRectangle(0, 0, m_width - m_pScroller->getWidth(), m_height, m_textInset, m_textInset);
	Float textHeight = cl_renderfuncs.pfnEstimateStringHeight(m_pFontSet, m_displayText.c_str(), m_pFontSet->fontsize);
	cl_renderfuncs.pfnSetStringRectangle(0, 0, 0, 0, 0, 0);

	if(textHeight < m_height)
	{
		m_textOffset = 0;
		return;
	}

	Uint32 totalHeight = textHeight-(m_height-m_pFontSet->fontsize-m_textInset*2);
	Uint32 nbElements = totalHeight/m_pFontSet->fontsize;
	m_textOffset = nbElements*offset;
}

//=============================================
// @brief Sets the source text array to render text from
//
//=============================================
void CGameUITextTab::updateRangeSize( void )
{
	if(m_displayText.empty())
	{
		m_pScroller->setFullRange(0);
		return;
	}

	// Re-estimate the height
	cl_renderfuncs.pfnSetStringRectangle(0, 0, m_width - m_pScroller->getWidth(), m_height, m_textInset, m_textInset);
	Int32 textHeight = cl_renderfuncs.pfnEstimateStringHeight(m_pFontSet, m_displayText.c_str(), m_pFontSet->fontsize);
	m_pScroller->setFullRange(textHeight);

	// Reset rectangle
	cl_renderfuncs.pfnSetStringRectangle(0, 0, 0, 0, 0, 0);
}

//=============================================
// @brief Sets the source text array to render text from
//
//=============================================
bool CGameUITextTab::mouseWheelEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown, Int32 scroll )
{
	if(keyDown && m_pScroller->moveScroller(button, scroll))
		return true;
	else
		return false;
}

//=============================================
// @brief Sets the source text array to render text from
//
//=============================================
bool CGameUITextTab::keyEvent( Int32 button, Int16 mod, bool keyDown )
{
	if(!keyDown)
		return false;

	// Get SDL Keycode
	SDL_Keycode sdlKeycode = SDL_GetKeyFromScancode((SDL_Scancode)button);

	switch(sdlKeycode)
	{
	case SDLK_DOWN:
		m_pScroller->moveScroller(button, 1);
		return true;
	case SDLK_UP:
		m_pScroller->moveScroller(button, -1);
		return true;
	default:
		return false;
	}
}

//====================================
//
//====================================
CGameUIWindow::CGameUIWindow( Int32 flags, Int32 originX, Int32 originY, Uint32 width, Uint32 height ):
	CGameUIObject(FL_NONE, originX, originY, width, height),
	m_windowFlags(flags),
	m_pInputFocusObject(nullptr),
	m_windowRemoveTime(0)
{
}

//====================================
//
//====================================
CGameUIWindow::~CGameUIWindow( void )
{
}

//====================================
//
//====================================
void CGameUIWindow::initBackground( Uint32& verticalbarheight, Uint32& middlebarwidth, Uint32& barThickness )
{
	// Create black background
	CGameUISurface* pBackground = new CGameUISurface(CGameUIObject::FL_NONE, 
		0, 
		GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
		GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
		0, 0,
		m_width,
		m_height);
	pBackground->setParent(this);

	//
	// Create upper bars
	//

	// Scale width by the relative size
	barThickness = gHUDDraw.ScaleX(GAMEUIWINDOW_BAR_THICKNESS);
	Int32 hBarYOrigin = gHUDDraw.ScaleY(GAMEUIWINDOW_H_BAR_Y_ORIGIN);
	Int32 vBarXOrigin = gHUDDraw.ScaleY(GAMEUIWINDOW_V_BAR_X_ORIGIN);

	// Create horizontal upper left bar
	if(!(m_windowFlags & FL_WINDOW_NO_BGBORDERS))
	{
		CGameUIBar* pUpperHorizontalLeftBar = new CGameUIBar((FL_ALIGN_LEFT|FL_ALIGN_TOP), 
			GAMEUIWINDOW_BARS_COLOR, 
			0, 
			CGameUIBar::FL_BAR_FADE_LEFT, 
			0, 
			hBarYOrigin, 
			vBarXOrigin, 
			barThickness);
		pUpperHorizontalLeftBar->setParent(this);
	}

	// Create horizontal middle upper bar
	middlebarwidth = m_width - vBarXOrigin*2;
	if(!(m_windowFlags & FL_WINDOW_NO_BGBORDERS))
	{
		CGameUIBar* pUpperHorizontalMiddleBar = new CGameUIBar((FL_ALIGN_LEFT|FL_ALIGN_TOP), 
			GAMEUIWINDOW_BARS_COLOR, 
			0, 
			CGameUIBar::FL_BAR_FADE_NONE, 
			vBarXOrigin, 
			hBarYOrigin, 
			middlebarwidth, 
			barThickness);
		pUpperHorizontalMiddleBar->setParent(this);
	}

	// Create horizontal upper right bar
	Uint32 upperrightbarorigin = vBarXOrigin + middlebarwidth;
	if(!(m_windowFlags & FL_WINDOW_NO_BGBORDERS))
	{
		CGameUIBar* pUpperHorizontalRightBar = new CGameUIBar((FL_ALIGN_LEFT|FL_ALIGN_TOP), 
			GAMEUIWINDOW_BARS_COLOR, 
			0, 
			CGameUIBar::FL_BAR_FADE_RIGHT, 
			upperrightbarorigin, 
			hBarYOrigin, 
			vBarXOrigin, 
			barThickness);
		pUpperHorizontalRightBar->setParent(this);
	}

	//
	// Create lower bars
	//

	// Create horizontal upper left bar
	if(!(m_windowFlags & FL_WINDOW_NO_BGBORDERS))
	{
		CGameUIBar* pLowerHorizontalLeftBar = new CGameUIBar((FL_ALIGN_LEFT|FL_ALIGN_BOTTOM), 
			GAMEUIWINDOW_BARS_COLOR, 
			0, 
			CGameUIBar::FL_BAR_FADE_LEFT, 
			0, 
			hBarYOrigin, 
			vBarXOrigin, 
			barThickness);
		pLowerHorizontalLeftBar->setParent(this);
	}

	// Create horizontal middle upper bar
	if(!(m_windowFlags & FL_WINDOW_NO_BGBORDERS))
	{
		CGameUIBar* pLowerHorizontalMiddleBar = new CGameUIBar((FL_ALIGN_LEFT|FL_ALIGN_BOTTOM), 
			GAMEUIWINDOW_BARS_COLOR, 
			0, 
			CGameUIBar::FL_BAR_FADE_NONE, 
			vBarXOrigin, 
			hBarYOrigin, 
			middlebarwidth, 
			barThickness);
		pLowerHorizontalMiddleBar->setParent(this);

		// Create horizontal upper right bar
		CGameUIBar* pLowerHorizontalRightBar = new CGameUIBar((FL_ALIGN_LEFT|FL_ALIGN_BOTTOM), 
			GAMEUIWINDOW_BARS_COLOR, 
			0, 
			CGameUIBar::FL_BAR_FADE_RIGHT, 
			upperrightbarorigin, 
			hBarYOrigin, 
			vBarXOrigin, 
			barThickness);
		pLowerHorizontalRightBar->setParent(this);
	}

	// Create left bar
	verticalbarheight = m_height - (hBarYOrigin*2 + barThickness*2);

	if(!(m_windowFlags & FL_WINDOW_NO_BGBORDERS))
	{
		CGameUIBar* pLeftBar = new CGameUIBar((FL_ALIGN_LEFT|FL_ALIGN_TOP), 
			GAMEUIWINDOW_BARS_COLOR, 
			0, 
			CGameUIBar::FL_BAR_FADE_NONE, 
			vBarXOrigin, 
			hBarYOrigin+barThickness, 
			barThickness, 
			verticalbarheight);
		pLeftBar->setParent(this);

		// Create right bar
		CGameUIBar* pRightBar = new CGameUIBar((FL_ALIGN_RIGHT|FL_ALIGN_TOP), 
			GAMEUIWINDOW_BARS_COLOR, 
			0, 
			CGameUIBar::FL_BAR_FADE_NONE, 
			vBarXOrigin, 
			hBarYOrigin+barThickness, 
			barThickness, 
			verticalbarheight);
		pRightBar->setParent(this);
	}
}

//====================================
//
//====================================
void CGameUIWindow::setInputFocusObject( CGameUIObject* pObject )
{
	if(m_pInputFocusObject)
		m_pInputFocusObject->setIsInputFocusObject(false);

	m_pInputFocusObject = pObject;
	m_pInputFocusObject->setIsInputFocusObject(true);
}

//====================================
//
//====================================
bool CGameUIWindow::mouseWheelEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown, Int32 scroll )
{
	if(!m_isVisible)
		return false;

	if(!isInteractive())
		return false;

	if(m_pChildrenArray.empty())
		return false;

	// Allow input focus object to take any inputs first
	if(m_pInputFocusObject && m_pInputFocusObject->mouseWheelEvent(mouseX, mouseY, button, keyDown, scroll))
		return true;

	// If not, allow others to take the input
	for(Uint32 i = 0; i < m_pChildrenArray.size(); i++)
	{
		if(m_pChildrenArray[i]->mouseWheelEvent(mouseX, mouseY, button, keyDown, scroll))
			return true;
	}

	return false;
}

//====================================
//
//====================================
bool CGameUIWindow::mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown )
{
	if(!m_isVisible)
		return false;

	if(!isInteractive())
		return false;

	if(m_pChildrenArray.empty())
		return false;

	// Allow input focus object to take any inputs first
	if(m_pInputFocusObject && m_pInputFocusObject->mouseButtonEvent(mouseX, mouseY, button, keyDown))
		return true;

	// If not, allow others to take the input
	for(Uint32 i = 0; i < m_pChildrenArray.size(); i++)
	{
		if(m_pChildrenArray[i]->mouseButtonEvent(mouseX, mouseY, button, keyDown))
			return true;
	}

	return false;
}

//====================================
//
//====================================
bool CGameUIWindow::keyEvent( Int32 button, Int16 mod, bool keyDown )
{
	if(!m_isVisible)
		return false;

	if(!isInteractive())
		return false;

	if(m_pChildrenArray.empty())
		return false;

	// Allow input focus object to take any inputs first
	if(m_pInputFocusObject && m_pInputFocusObject->keyEvent(button, mod, keyDown))
		return true;

	// If not, allow others to take the input
	for(Uint32 i = 0; i < m_pChildrenArray.size(); i++)
	{
		if(m_pChildrenArray[i]->keyEvent(button, mod, keyDown))
			return true;
	}

	return false;
}

//====================================
//
//====================================
void CGameUIWindow::setDelayedRemoval( Double delay )
{
	if(!(m_windowFlags & FL_WINDOW_WAIT_TILL_NEXT))
	{
		m_windowRemoveTime = cl_engfuncs.pfnGetClientTime() + delay;
		m_windowFlags |= FL_WINDOW_DELAY_REMOVE;
	}

	setButtonsDisabled(true);
}

//=============================================
// @brief Constructor
//
//=============================================
CGameUIDragButton::CGameUIDragButton( Int32 flags, gui_scroller_align_t alignment, CGameUICallbackEvent* pEvent, Uint32 edgethickness, const color32_t& color, const color32_t& bgcolor, const color32_t& highlightcolor, Uint32 width, Uint32 height, Int32 originx, Int32 originy ):
	CGameUIButton( flags, pEvent, edgethickness, color, bgcolor, highlightcolor, originx, originy, width, height ),
	m_alignment(alignment),
	m_lastMouseX(0),
	m_lastMouseY(0),
	m_startInset(0),
	m_endInset(0),
	m_position(0),
	m_lastParentLength(0)
{
	if(alignment == CGameUIObject::FL_SCROLL_V)
		m_flags |= CGameUIObject::FL_ALIGN_RIGHT;
	else
		m_flags |= CGameUIObject::FL_ALIGN_BOTTOM;
}

//=============================================
// @brief Destructor
//
//=============================================
CGameUIDragButton::~CGameUIDragButton( void )
{
}

//=============================================
// @brief Sets the string to display
//
//=============================================
void CGameUIDragButton::setBounds( Int32 start, Int32 end )
{
	m_startInset = start;
	m_endInset = end;
}

//=============================================
// @brief Sets the string to display
//
//=============================================
void CGameUIDragButton::getBounds( Int32& start, Int32& end )
{
	start = m_startInset;
	end = m_endInset;
}

//=============================================
// @brief Sets the string to display
//
//=============================================
bool CGameUIDragButton::mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown )
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
		if(m_pEvent)
			m_pEvent->PerformAction(0);

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
void CGameUIDragButton::think( void )
{
	if(m_isClickedOn)
	{
		Int32 xPos, yPos;
		cl_engfuncs.pfnGetMousePosition(xPos, yPos);

		Int32 prevVal;
		Int32 curVal;
		if(m_alignment == CGameUIObject::FL_SCROLL_V)
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

			if(m_pEvent)
				m_pEvent->PerformAction(0);
		}
	}

	// Call base to handle the rest
	CGameUIObject::think();
}

//=============================================
// @brief Sets the length of the drag button
//
//=============================================
void CGameUIDragButton::setLength( Uint32 length )
{
	if(m_alignment == CGameUIObject::FL_SCROLL_V)
	{
		Int32 finalHeight = length - m_startInset - m_endInset;
		m_height = _min(0, finalHeight);
	}
	else
	{
		Int32 finalWidth = length - m_startInset - m_endInset;
		m_width = _min(0, finalWidth);
	}
}

//=============================================
// @brief Sets the position of the drag button
//
//=============================================
void CGameUIDragButton::setPosition( Float position )
{
	m_position = clamp(position, 0.0, 1.0);
}

//=============================================
// @brief Gets the position of the drag button
//
//=============================================
Float CGameUIDragButton::getPosition( void )
{
	return m_position;
}

//=============================================
// @brief Sets the position of the drag button
//
//=============================================
bool CGameUIDragButton::adjPosition( Int32 adjAmt, bool isMouseDrag, bool callEvent )
{
	if(m_position == 1.0 && adjAmt > 0)
		return false;
	else if(m_position == 0.0 && adjAmt < 0)
		return false;

	// Get parent length
	Uint32 parentLength;
	if(m_alignment == CGameUIObject::FL_SCROLL_V)
		parentLength = m_pParent->getHeight();
	else
		parentLength = m_pParent->getWidth();

	// Determine my own range
	Int32 myRange = parentLength-getLength()-m_startInset-m_endInset;
	
	// No range, just set to base
	if(!myRange)
	{
		// Set appropriate position
		if(m_alignment == CGameUIObject::FL_SCROLL_V)
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
	Float adjFrac = (Float)adjAmt/(Float)(referenceRange);

	m_position += adjFrac;
	m_position = clamp(m_position, 0.0, 1.0);

	// Set appropriate position
	if(m_alignment == CGameUIObject::FL_SCROLL_V)
		m_originY = m_startInset + m_position*myRange;
	else
		m_originX = m_startInset + m_position*myRange;

	// Call the action to reset the position
	if(callEvent && m_pEvent)
		m_pEvent->PerformAction(0);

	// Remember last parent length
	m_lastParentLength = parentLength;
	return true;
}

//=============================================
// @brief Gets the length of the drag button
//
//=============================================
Uint32 CGameUIDragButton::getLength( void )
{
	if(m_alignment == CGameUIObject::FL_SCROLL_V)
		return m_height;
	else
		return m_width;
}

//=============================================
// @brief Repositions the object after a parent's size is changed
//
//=============================================
void CGameUIDragButton::adjustPosition( void )
{
	// Call base class first
	CGameUIButton::adjustPosition();

	if(m_lastParentLength)
	{
		// Reset the position
		Uint32 parentLength;
		if(m_alignment == CGameUIObject::FL_SCROLL_V)
			parentLength = m_pParent->getHeight();
		else
			parentLength = m_pParent->getWidth();

		// Determine previous full position
		Uint32 prevRelativePosition = m_position*m_lastParentLength;
		m_lastParentLength = parentLength;

		// Determine current position based on this
		Float currentPosition = (Float)prevRelativePosition/(Float)parentLength;
		m_position = clamp(currentPosition, 0.0, 1.0);

		// Determine my own range
		Int32 myRange = parentLength-getLength()-m_startInset-m_endInset;

		// Set appropriate position
		Int32 offAdj = m_position*myRange;
		if(!offAdj)
			m_position = 0;

		if(m_alignment == CGameUIObject::FL_SCROLL_V)
			m_originY = m_startInset + offAdj;
		else
			m_originX = m_startInset + offAdj;

		// Call the action to reset the position
		if(m_pEvent)
			m_pEvent->PerformAction(0);
	}
}

//=============================================
// @brief Performs the close action for the window
//
//=============================================
void CGameUIScroller::CGameUIScrollerDragBtnAction::PerformAction( Float param )
{
	assert(m_pScroller != nullptr);

	// Get the dragger button
	CGameUIDragButton* pButton = m_pScroller->getDragButton();
	Float position = pButton->getPosition();

	// Get parent of the scroller button
	CGameUIObject* pParent = m_pScroller->getParent();
	if(pParent)
		pParent->setOffsetValue(position);
}

//=============================================
// @brief Constructor
//
//=============================================
CGameUIScroller::CGameUIScroller( Int32 flags, gui_scroller_align_t alignment, Uint32 edgethickness, const color32_t& color, const color32_t& bgcolor, const color32_t& highlightcolor, Uint32 unitSize, Uint32 width, Uint32 height, Int32 originx, Int32 originy ):
	CGameUISurface( flags, edgethickness, color, bgcolor, originx, originy, width, height ),
	m_pDragButton(nullptr),
	m_alignment(alignment),
	m_fullRangeSize(0),
	m_prevFullRangeSize(0),
	m_unitSize(unitSize)
{
	if(alignment == CGameUIObject::FL_SCROLL_V)
		m_flags |= CGameUIObject::FL_ALIGN_RIGHT;
	else
		m_flags |= CGameUIObject::FL_ALIGN_BOTTOM;

	// Middle button is a bit more complicated
	Int32 btnflags = CGameUIObject::FL_NONE;
	Int32 dragBtnOriginX = m_edgeThickness;
	Int32 dragBtnOriginY = m_edgeThickness;

	Uint32 dragBtnWidth = 0;
	Uint32 dragBtnHeight = 0;
	if(m_alignment == CGameUIObject::FL_SCROLL_V)
	{
		dragBtnWidth = m_width - m_edgeThickness*2;
		dragBtnHeight = m_height - dragBtnOriginY - m_edgeThickness;
	}
	else
	{
		dragBtnHeight = m_height - m_edgeThickness*2;
		dragBtnWidth = m_width - dragBtnOriginX - m_edgeThickness;
	}

	// Create middle button
	CGameUIScrollerDragBtnAction* pDragAction = new CGameUIScrollerDragBtnAction(this);
	m_pDragButton = new CGameUIDragButton(btnflags, m_alignment, pDragAction, m_edgeThickness, m_color, m_backgroundColor, highlightcolor, dragBtnWidth, dragBtnHeight, dragBtnOriginX, dragBtnOriginY);
	m_pDragButton->setParent(this);
	
	adjustPosition();
}

//=============================================
// @brief Destructor
//
//=============================================
CGameUIScroller::~CGameUIScroller( void )
{
}

//=============================================
// @brief Tells if the parent size is valid for us
//
//=============================================
void CGameUIScroller::setFullRange( Int32 fullRangeSize )
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
void CGameUIScroller::readjustDragButton( void )
{
	Int32 insetStart, insetEnd;
	m_pDragButton->getBounds(insetStart, insetEnd);

	// Determine coverage
	Uint32 viewSize = 0;
	if(m_alignment == CGameUIObject::FL_SCROLL_V)
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

	if(m_flags & CGameUIObject::FL_SCROLL_REVERSE)
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
// @brief Moves the scroller in the specified direction
//
//=============================================
bool CGameUIScroller::moveScroller( Int32 button, Int32 scrollAmount )
{
	if((button == MOUSE_WHEEL_UP || button == MOUSE_WHEEL_DOWN) && m_alignment != CGameUIObject::FL_SCROLL_V)
		return false;

	if((button == MOUSE_WHEEL_RIGHT || button == MOUSE_WHEEL_LEFT) && m_alignment != CGameUIObject::FL_SCROLL_H)
		return false;

	Int32 moveAmount = scrollAmount * m_unitSize;
	if(button == MOUSE_WHEEL_UP || button == MOUSE_WHEEL_RIGHT)
		moveAmount = -moveAmount;

	m_pDragButton->adjPosition(moveAmount, false);

	return true;
}

//=============================================
// @brief Sets the source text array to render text from
//
//=============================================
void CGameUIScroller::setUnitSize( Uint32 unitsize )
{
	m_unitSize = unitsize;
}

//=============================================
// @brief Constructor
//
//=============================================
CGameUITextInputTab::CGameUITextInputTab( Int32 flags, CGameUICallbackEvent* pAction, Uint32 textinset, Uint32 edgethickness, const color32_t& color, const color32_t& bgcolor, const color32_t& highlightcolor, const font_set_t* pFont, Int32 originx, Int32 originy, Uint32 width, Uint32 height ):
	CGameUIObject( flags, originx, originy, width, height ),
	m_pFont(pFont),
	m_nextBlinkTime(0),
	m_drawMarker(true),
	m_inset(textinset),
	m_color(color),
	m_backgroundColor(bgcolor),
	m_highlightColor(highlightcolor),
	m_edgeThickness(edgethickness),
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
CGameUITextInputTab::~CGameUITextInputTab( void )
{
	if(m_pAction)
		delete m_pAction;
}

//=============================================
// @brief Destructor
//
//=============================================
bool CGameUITextInputTab::mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown )
{
	// Only left mouse click affects this
	if(button != SDL_BUTTON_LEFT)
		return false;

	if(isMouseOver(mouseX, mouseY))
	{
		// Set us as the input focus
		setInputFocusObject(this);
		return true;
	}

	return false;
}

//=============================================
// @brief Destructor
//
//=============================================
bool CGameUITextInputTab::keyEvent( Int32 button, Int16 mod, bool keyDown )
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

	if(sdlKeycode == SDLK_RETURN || sdlKeycode == SDLK_KP_ENTER)
		return false;

	// See if it's a valid text input character
	if(sdlKeycode >= SDLK_SPACE && sdlKeycode <= SDLK_z)
	{
		// Avoid buffer over-indexing
		if(m_inputPosition == GAMEUI_MAX_INPUT_LENGTH)
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
		m_nextBlinkTime = cl_engfuncs.pfnGetClientTime() + 0.5;
		m_drawMarker = true;

		m_inputPosition++;
	}
	else if(sdlKeycode == SDLK_LEFT)
	{
		if(m_inputPosition > 0)
		{
			// Always draw after input is added
			m_nextBlinkTime = cl_engfuncs.pfnGetClientTime() + 0.5;
			m_drawMarker = true;

			m_inputPosition--;
		}
	}
	else if(sdlKeycode == SDLK_RIGHT)
	{
		if(m_szBuffer[m_inputPosition] != '\0')
		{
			// Always draw after input is added
			m_nextBlinkTime = cl_engfuncs.pfnGetClientTime() + 0.5;
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
void CGameUITextInputTab::clearText( void )
{
	m_szBuffer[0] = '\0';
	m_inputPosition = 0;
}

//=============================================
// @brief Sets the contents of the text buffer
//
//=============================================
void CGameUITextInputTab::setText( const Char* pstrText )
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
void CGameUITextInputTab::think( void )
{
	// Call base class to handle basics
	CGameUIObject::think();

	// Time input market
	if(cl_engfuncs.pfnGetClientTime() >= m_nextBlinkTime)
	{
		m_nextBlinkTime = cl_engfuncs.pfnGetClientTime() + 0.5;
		m_drawMarker = !m_drawMarker;
	}
}

//=============================================
// @brief Destructor
//
//=============================================
bool CGameUITextInputTab::draw( void )
{
	Int32 baseOriginX, baseOriginY;
	getAbsolutePosition(baseOriginX, baseOriginY);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if(m_color.r == m_backgroundColor.r && m_color.g == m_backgroundColor.g
		&& m_color.b == m_backgroundColor.b && m_color.a == m_backgroundColor.a)
	{
		// Draw background
		gHUDDraw.SetColor(m_color.r, m_color.g, m_color.b, m_color.a);
		gHUDDraw.SetOrigin(baseOriginX, baseOriginY);
		gHUDDraw.SetSize(m_width, m_height);
		if(!gHUDDraw.DrawQuad(nullptr))
			return false;
	}
	else
	{
		Int32 originx = baseOriginX;
		Int32 originy = baseOriginY;

		if(isInputFocusObject() && (m_highlightColor.r != 0 || m_highlightColor.g != 0 || m_highlightColor.b != 0 || m_highlightColor.a != 0))
		{
			// Draw background with highlight color
			gHUDDraw.SetColor(m_highlightColor.r, m_highlightColor.g, m_highlightColor.b, m_highlightColor.a);
			gHUDDraw.SetOrigin(baseOriginX+m_edgeThickness, baseOriginY+m_edgeThickness);
			gHUDDraw.SetSize(m_width-m_edgeThickness, m_height-m_edgeThickness);
			if(!gHUDDraw.DrawQuad(nullptr))
				return false;
		}
		else if(m_backgroundColor.r != 0 || m_backgroundColor.g != 0 || m_backgroundColor.b != 0 || m_backgroundColor.a != 0)
		{
			// Draw background
			gHUDDraw.SetColor(m_backgroundColor.r, m_backgroundColor.g, m_backgroundColor.b, m_backgroundColor.a);
			gHUDDraw.SetOrigin(baseOriginX+m_edgeThickness, baseOriginY+m_edgeThickness);
			gHUDDraw.SetSize(m_width-m_edgeThickness, m_height-m_edgeThickness);
			if(!gHUDDraw.DrawQuad(nullptr))
				return false;
		}

		// Draw top edge
		gHUDDraw.SetColor(m_color.r, m_color.g, m_color.b, m_color.a);
		gHUDDraw.SetOrigin(originx, originy);
		gHUDDraw.SetSize(m_width, m_edgeThickness);
		if(!gHUDDraw.DrawQuad(nullptr))
			return false;

		// Draw left edge
		originy = baseOriginY + m_edgeThickness;
		Uint32 verticalBarHeight = m_height - m_edgeThickness*2;
		gHUDDraw.SetOrigin(originx, originy);
		gHUDDraw.SetSize(m_edgeThickness, verticalBarHeight);
		if(!gHUDDraw.DrawQuad(nullptr))
			return false;

		// Draw bottom edge
		originy = baseOriginY + m_edgeThickness + verticalBarHeight;
		gHUDDraw.SetOrigin(originx, originy);
		gHUDDraw.SetSize(m_width, m_edgeThickness);
		if(!gHUDDraw.DrawQuad(nullptr))
			return false;

		// Draw right edge
		originx = baseOriginX + (m_width - m_edgeThickness);
		originy = baseOriginY + m_edgeThickness;
		gHUDDraw.SetOrigin(originx, originy);
		gHUDDraw.SetSize(m_edgeThickness, verticalBarHeight);
		if(!gHUDDraw.DrawQuad(nullptr))
			return false;
	}

	// Two pixels for now
	Int32 markerWidth = 2;

	// For the marker
	Char* pstr = nullptr;
	Uint32 numChars = 0;
	Uint32 stringWidth = 0;

	if(m_szBuffer[0] != '\0')
	{
		// Not happy about so many state switches
		gHUDDraw.FinishDraw();

		// Determine position to draw in
		Uint32 textHeight = cl_renderfuncs.pfnEstimateStringHeight(m_pFont, m_szBuffer, 0);

		// Draw in the vertical center
		Uint32 yOrg = baseOriginY + m_height/2 + textHeight/2;

		// Draw only the parts that fit in
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
		if(!cl_renderfuncs.pfnDrawSimpleString(color, baseOriginX+m_inset, yOrg, pstr, m_pFont))
			return false;

		// Re-enable basic drawing
		if(!gHUDDraw.SetupDraw())
			return false;

		// Re-enable blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	// Draw marker if needed
	if(m_drawMarker && isInputFocusObject())
	{
		Int32 xOrg = baseOriginX + m_inset;
		Int32 yOrg = baseOriginY + m_height/2 - m_pFont->fontsize/2;
		Int32 markerHeight = m_pFont->fontsize;

		// Determine offset
		if(pstr != nullptr)
		{
			if(m_inputPosition < numChars)
				return CGameUIObject::draw();

			Uint32 nbChars = m_inputPosition - numChars;
			for(Uint32 i = 0; i < nbChars; i++)
				xOrg += m_pFont->glyphs[pstr[i]].advancex;
		}

		gHUDDraw.SetColor(255, 255, 255, 255);

		gHUDDraw.SetOrigin(xOrg, yOrg);
		gHUDDraw.SetSize(markerWidth, markerHeight);
		if(!gHUDDraw.DrawQuad(nullptr))
			return false;
	}

	// Call base class to handle rendering
	return CGameUIObject::draw();
}