/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef SPRITE_H
#define SPRITE_H

#include "sprformat.h"

// Loads a sprite
extern msprite_t* Sprite_Load( const byte* pfile, Uint32 filesize );
extern const mspriteframe_t* Sprite_GetFrame( const msprite_t* psprite, Uint32 frame, Float time );
#endif //SPRITE_H