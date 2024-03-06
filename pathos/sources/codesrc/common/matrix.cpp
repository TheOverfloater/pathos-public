/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include <math.h>

#include "includes.h"
#include "matrix.h"

//=============================================
// @brief Constructor
//
//=============================================
CMatrix::CMatrix ( void ):
	m_pCurrentMatrix(nullptr),
	m_pMatrixHeader(nullptr)
{
	m_pMatrixHeader = new matrix_t;
	m_pCurrentMatrix = m_pMatrixHeader->matrix;

	for(Uint32 i = 0; i < 16; i++)
		m_workingMatrix[i] = 0;

	for(Uint32 i = 0; i < 16; i++)
		m_outMatrix[i] = 0;

	for(Uint32 i = 0; i < 16; i++)
		m_multMatrix[i] = 0;

	for(Uint32 i = 0; i < 16; i++)
		m_rotationMatrix[i] = 0;

	for(Uint32 i = 0; i < 16; i++)
		m_matrixTranspose[i] = 0;
}

//=============================================
// @brief Constructor
//
//=============================================
CMatrix::CMatrix( const CMatrix& src ):
	m_pCurrentMatrix(nullptr),
	m_pMatrixHeader(nullptr)
{
	matrix_t* lastinsert = nullptr;
	matrix_t* pnext = src.m_pMatrixHeader;
	while(pnext)
	{
		matrix_t* pnew = new matrix_t();

		if(!lastinsert)
			m_pMatrixHeader = pnew;
		else
			lastinsert->next = pnew;

		for(Uint32 i = 0; i < 16; i++)
			pnew->matrix[i] = pnext->matrix[i];
		
		pnext = pnext->next;
		lastinsert = pnew;
	}

	m_pCurrentMatrix = m_pMatrixHeader->matrix;

	for(Uint32 i = 0; i < 16; i++)
		m_workingMatrix[i] = 0;

	for(Uint32 i = 0; i < 16; i++)
		m_outMatrix[i] = 0;

	for(Uint32 i = 0; i < 16; i++)
		m_multMatrix[i] = 0;

	for(Uint32 i = 0; i < 16; i++)
		m_rotationMatrix[i] = 0;

	for(Uint32 i = 0; i < 16; i++)
		m_matrixTranspose[i] = 0;
}

//=============================================
// @brief Copy constructor
//
// @param m Source matrix
//=============================================
CMatrix::CMatrix ( const Float *pMatrix ):
	m_pCurrentMatrix(nullptr),
	m_pMatrixHeader(nullptr)
{
	m_pMatrixHeader = new matrix_t;
	m_pCurrentMatrix = m_pMatrixHeader->matrix;

	m_pCurrentMatrix[0] = pMatrix[0]; m_pCurrentMatrix[1] = pMatrix[4]; m_pCurrentMatrix[2] = pMatrix[8]; m_pCurrentMatrix[3] = pMatrix[12]; 
	m_pCurrentMatrix[4] = pMatrix[1]; m_pCurrentMatrix[5] = pMatrix[5]; m_pCurrentMatrix[6] = pMatrix[9]; m_pCurrentMatrix[7] = pMatrix[13]; 
	m_pCurrentMatrix[8] = pMatrix[2]; m_pCurrentMatrix[9] = pMatrix[6]; m_pCurrentMatrix[10] = pMatrix[10]; m_pCurrentMatrix[11] = pMatrix[14];
	m_pCurrentMatrix[12] = pMatrix[3]; m_pCurrentMatrix[13] = pMatrix[7]; m_pCurrentMatrix[14] = pMatrix[11]; m_pCurrentMatrix[15] = pMatrix[15];

	for(Uint32 i = 0; i < 16; i++)
		m_workingMatrix[i] = 0;

	for(Uint32 i = 0; i < 16; i++)
		m_outMatrix[i] = 0;

	for(Uint32 i = 0; i < 16; i++)
		m_multMatrix[i] = 0;

	for(Uint32 i = 0; i < 16; i++)
		m_rotationMatrix[i] = 0;

	for(Uint32 i = 0; i < 16; i++)
		m_matrixTranspose[i] = 0;
}

//=============================================
// @brief Destructor
//
//=============================================
CMatrix::~CMatrix( void )
{
	matrix_t *pstack = m_pMatrixHeader;
	while(pstack)
	{
		matrix_t *pfree = pstack;
		pstack = pfree->next;
		delete pfree;
	}

	m_pMatrixHeader = nullptr;
}

//=============================================
// @brief Saves the current matrix on the stack
//
//=============================================
void CMatrix::PushMatrix( void )
{
	matrix_t *pnew = new matrix_t;
	memcpy(pnew, m_pMatrixHeader, sizeof(matrix_t));
	pnew->next = m_pMatrixHeader; m_pMatrixHeader = pnew;
	m_pCurrentMatrix = m_pMatrixHeader->matrix;
}

//=============================================
// @brief Restores the previous matrix state on the stack
//
//=============================================
void CMatrix::PopMatrix( void )
{
	assert(m_pMatrixHeader != nullptr);
	matrix_t *pfree = m_pMatrixHeader;
	m_pMatrixHeader = pfree->next;
	m_pCurrentMatrix = m_pMatrixHeader->matrix;
	delete pfree;
}

//=============================================
// @brief Returns the matrix's contents in an output buffer
//
// @return Pointer to matrix output buffer
//=============================================
const Float *CMatrix::GetMatrix( void ) 
{
	m_outMatrix[0] = m_pCurrentMatrix[0]; m_outMatrix[1] = m_pCurrentMatrix[1]; m_outMatrix[2] = m_pCurrentMatrix[2]; m_outMatrix[3] = m_pCurrentMatrix[3];
	m_outMatrix[4] = m_pCurrentMatrix[4]; m_outMatrix[5] = m_pCurrentMatrix[5]; m_outMatrix[6] = m_pCurrentMatrix[6]; m_outMatrix[7] = m_pCurrentMatrix[7];
	m_outMatrix[8] = m_pCurrentMatrix[8]; m_outMatrix[9] = m_pCurrentMatrix[9]; m_outMatrix[10] = m_pCurrentMatrix[10]; m_outMatrix[11] = m_pCurrentMatrix[11];
	m_outMatrix[12] = m_pCurrentMatrix[12]; m_outMatrix[13] = m_pCurrentMatrix[13]; m_outMatrix[14] = m_pCurrentMatrix[14]; m_outMatrix[15] = m_pCurrentMatrix[15];
	return m_outMatrix;
}

//=============================================
// @brief Returns the matrix's contents in an output buffer
//
// @return Inverse of the matrix
//=============================================
const Float *CMatrix::GetInverse( void ) 
{
	m_workingMatrix[0] = m_pCurrentMatrix[5]  * m_pCurrentMatrix[10] * m_pCurrentMatrix[15] - m_pCurrentMatrix[5]  * m_pCurrentMatrix[11] * m_pCurrentMatrix[14] - m_pCurrentMatrix[9]  * m_pCurrentMatrix[6]  * m_pCurrentMatrix[15] + m_pCurrentMatrix[9]  * m_pCurrentMatrix[7]  * m_pCurrentMatrix[14] +m_pCurrentMatrix[13] * m_pCurrentMatrix[6]  * m_pCurrentMatrix[11] - m_pCurrentMatrix[13] * m_pCurrentMatrix[7]  * m_pCurrentMatrix[10];
	m_workingMatrix[4] = -m_pCurrentMatrix[4]  * m_pCurrentMatrix[10] * m_pCurrentMatrix[15] + m_pCurrentMatrix[4]  * m_pCurrentMatrix[11] * m_pCurrentMatrix[14] + m_pCurrentMatrix[8]  * m_pCurrentMatrix[6]  * m_pCurrentMatrix[15] - m_pCurrentMatrix[8]  * m_pCurrentMatrix[7]  * m_pCurrentMatrix[14] - m_pCurrentMatrix[12] * m_pCurrentMatrix[6]  * m_pCurrentMatrix[11] + m_pCurrentMatrix[12] * m_pCurrentMatrix[7]  * m_pCurrentMatrix[10];
	m_workingMatrix[8] = m_pCurrentMatrix[4]  * m_pCurrentMatrix[9] * m_pCurrentMatrix[15] - m_pCurrentMatrix[4]  * m_pCurrentMatrix[11] * m_pCurrentMatrix[13] - m_pCurrentMatrix[8]  * m_pCurrentMatrix[5] * m_pCurrentMatrix[15] + m_pCurrentMatrix[8]  * m_pCurrentMatrix[7] * m_pCurrentMatrix[13] + m_pCurrentMatrix[12] * m_pCurrentMatrix[5] * m_pCurrentMatrix[11] - m_pCurrentMatrix[12] * m_pCurrentMatrix[7] * m_pCurrentMatrix[9];
	m_workingMatrix[12] = -m_pCurrentMatrix[4]  * m_pCurrentMatrix[9] * m_pCurrentMatrix[14] + m_pCurrentMatrix[4]  * m_pCurrentMatrix[10] * m_pCurrentMatrix[13] +m_pCurrentMatrix[8]  * m_pCurrentMatrix[5] * m_pCurrentMatrix[14] - m_pCurrentMatrix[8]  * m_pCurrentMatrix[6] * m_pCurrentMatrix[13] - m_pCurrentMatrix[12] * m_pCurrentMatrix[5] * m_pCurrentMatrix[10] + m_pCurrentMatrix[12] * m_pCurrentMatrix[6] * m_pCurrentMatrix[9];
	m_workingMatrix[1] = -m_pCurrentMatrix[1]  * m_pCurrentMatrix[10] * m_pCurrentMatrix[15] + m_pCurrentMatrix[1]  * m_pCurrentMatrix[11] * m_pCurrentMatrix[14] + m_pCurrentMatrix[9]  * m_pCurrentMatrix[2] * m_pCurrentMatrix[15] - m_pCurrentMatrix[9]  * m_pCurrentMatrix[3] * m_pCurrentMatrix[14] - m_pCurrentMatrix[13] * m_pCurrentMatrix[2] * m_pCurrentMatrix[11] + m_pCurrentMatrix[13] * m_pCurrentMatrix[3] * m_pCurrentMatrix[10];
	m_workingMatrix[5] = m_pCurrentMatrix[0]  * m_pCurrentMatrix[10] * m_pCurrentMatrix[15] - m_pCurrentMatrix[0]  * m_pCurrentMatrix[11] * m_pCurrentMatrix[14] - m_pCurrentMatrix[8]  * m_pCurrentMatrix[2] * m_pCurrentMatrix[15] + m_pCurrentMatrix[8]  * m_pCurrentMatrix[3] * m_pCurrentMatrix[14] + m_pCurrentMatrix[12] * m_pCurrentMatrix[2] * m_pCurrentMatrix[11] - m_pCurrentMatrix[12] * m_pCurrentMatrix[3] * m_pCurrentMatrix[10];
	m_workingMatrix[9] = -m_pCurrentMatrix[0]  * m_pCurrentMatrix[9] * m_pCurrentMatrix[15] + m_pCurrentMatrix[0]  * m_pCurrentMatrix[11] * m_pCurrentMatrix[13] + m_pCurrentMatrix[8]  * m_pCurrentMatrix[1] * m_pCurrentMatrix[15] - m_pCurrentMatrix[8]  * m_pCurrentMatrix[3] * m_pCurrentMatrix[13] - m_pCurrentMatrix[12] * m_pCurrentMatrix[1] * m_pCurrentMatrix[11] + m_pCurrentMatrix[12] * m_pCurrentMatrix[3] * m_pCurrentMatrix[9];
	m_workingMatrix[13] = m_pCurrentMatrix[0]  * m_pCurrentMatrix[9] * m_pCurrentMatrix[14] - m_pCurrentMatrix[0]  * m_pCurrentMatrix[10] * m_pCurrentMatrix[13] - m_pCurrentMatrix[8]  * m_pCurrentMatrix[1] * m_pCurrentMatrix[14] + m_pCurrentMatrix[8]  * m_pCurrentMatrix[2] * m_pCurrentMatrix[13] + m_pCurrentMatrix[12] * m_pCurrentMatrix[1] * m_pCurrentMatrix[10] - m_pCurrentMatrix[12] * m_pCurrentMatrix[2] * m_pCurrentMatrix[9];
	m_workingMatrix[2] = m_pCurrentMatrix[1]  * m_pCurrentMatrix[6] * m_pCurrentMatrix[15] - m_pCurrentMatrix[1]  * m_pCurrentMatrix[7] * m_pCurrentMatrix[14] - m_pCurrentMatrix[5]  * m_pCurrentMatrix[2] * m_pCurrentMatrix[15] + m_pCurrentMatrix[5]  * m_pCurrentMatrix[3] * m_pCurrentMatrix[14] + m_pCurrentMatrix[13] * m_pCurrentMatrix[2] * m_pCurrentMatrix[7] - m_pCurrentMatrix[13] * m_pCurrentMatrix[3] * m_pCurrentMatrix[6];
	m_workingMatrix[6] = -m_pCurrentMatrix[0]  * m_pCurrentMatrix[6] * m_pCurrentMatrix[15] + m_pCurrentMatrix[0]  * m_pCurrentMatrix[7] * m_pCurrentMatrix[14] + m_pCurrentMatrix[4]  * m_pCurrentMatrix[2] * m_pCurrentMatrix[15] - m_pCurrentMatrix[4]  * m_pCurrentMatrix[3] * m_pCurrentMatrix[14] - m_pCurrentMatrix[12] * m_pCurrentMatrix[2] * m_pCurrentMatrix[7] + m_pCurrentMatrix[12] * m_pCurrentMatrix[3] * m_pCurrentMatrix[6];
	m_workingMatrix[10] = m_pCurrentMatrix[0]  * m_pCurrentMatrix[5] * m_pCurrentMatrix[15] - m_pCurrentMatrix[0]  * m_pCurrentMatrix[7] * m_pCurrentMatrix[13] - m_pCurrentMatrix[4]  * m_pCurrentMatrix[1] * m_pCurrentMatrix[15] + m_pCurrentMatrix[4]  * m_pCurrentMatrix[3] * m_pCurrentMatrix[13] + m_pCurrentMatrix[12] * m_pCurrentMatrix[1] * m_pCurrentMatrix[7] - m_pCurrentMatrix[12] * m_pCurrentMatrix[3] * m_pCurrentMatrix[5];
	m_workingMatrix[14] = -m_pCurrentMatrix[0]  * m_pCurrentMatrix[5] * m_pCurrentMatrix[14] + m_pCurrentMatrix[0]  * m_pCurrentMatrix[6] * m_pCurrentMatrix[13] + m_pCurrentMatrix[4]  * m_pCurrentMatrix[1] * m_pCurrentMatrix[14] - m_pCurrentMatrix[4]  * m_pCurrentMatrix[2] * m_pCurrentMatrix[13] - m_pCurrentMatrix[12] * m_pCurrentMatrix[1] * m_pCurrentMatrix[6] + m_pCurrentMatrix[12] * m_pCurrentMatrix[2] * m_pCurrentMatrix[5];
	m_workingMatrix[3] = -m_pCurrentMatrix[1] * m_pCurrentMatrix[6] * m_pCurrentMatrix[11] + m_pCurrentMatrix[1] * m_pCurrentMatrix[7] * m_pCurrentMatrix[10] + m_pCurrentMatrix[5] * m_pCurrentMatrix[2] * m_pCurrentMatrix[11] - m_pCurrentMatrix[5] * m_pCurrentMatrix[3] * m_pCurrentMatrix[10] - m_pCurrentMatrix[9] * m_pCurrentMatrix[2] * m_pCurrentMatrix[7] + m_pCurrentMatrix[9] * m_pCurrentMatrix[3] * m_pCurrentMatrix[6];
	m_workingMatrix[7] = m_pCurrentMatrix[0] * m_pCurrentMatrix[6] * m_pCurrentMatrix[11] - m_pCurrentMatrix[0] * m_pCurrentMatrix[7] * m_pCurrentMatrix[10] - m_pCurrentMatrix[4] * m_pCurrentMatrix[2] * m_pCurrentMatrix[11] + m_pCurrentMatrix[4] * m_pCurrentMatrix[3] * m_pCurrentMatrix[10] + m_pCurrentMatrix[8] * m_pCurrentMatrix[2] * m_pCurrentMatrix[7] - m_pCurrentMatrix[8] * m_pCurrentMatrix[3] * m_pCurrentMatrix[6];
	m_workingMatrix[11] = -m_pCurrentMatrix[0] * m_pCurrentMatrix[5] * m_pCurrentMatrix[11] + m_pCurrentMatrix[0] * m_pCurrentMatrix[7] * m_pCurrentMatrix[9] + m_pCurrentMatrix[4] * m_pCurrentMatrix[1] * m_pCurrentMatrix[11] - m_pCurrentMatrix[4] * m_pCurrentMatrix[3] * m_pCurrentMatrix[9] - m_pCurrentMatrix[8] * m_pCurrentMatrix[1] * m_pCurrentMatrix[7] + m_pCurrentMatrix[8] * m_pCurrentMatrix[3] * m_pCurrentMatrix[5];
	m_workingMatrix[15] = m_pCurrentMatrix[0] * m_pCurrentMatrix[5] * m_pCurrentMatrix[10] - m_pCurrentMatrix[0] * m_pCurrentMatrix[6] * m_pCurrentMatrix[9] - m_pCurrentMatrix[4] * m_pCurrentMatrix[1] * m_pCurrentMatrix[10] + m_pCurrentMatrix[4] * m_pCurrentMatrix[2] * m_pCurrentMatrix[9] + m_pCurrentMatrix[8] * m_pCurrentMatrix[1] * m_pCurrentMatrix[6] - m_pCurrentMatrix[8] * m_pCurrentMatrix[2] * m_pCurrentMatrix[5];
	
	Float det = m_pCurrentMatrix[0] * m_workingMatrix[0] + m_pCurrentMatrix[1] * m_workingMatrix[4] + m_pCurrentMatrix[2] * m_workingMatrix[8] + m_pCurrentMatrix[3] * m_workingMatrix[12];
	det = (Float)(1.0 / det);

	for(Uint32 i = 0; i < 16; i++)
		m_matrixTranspose[i] = m_workingMatrix[i] * det;

	return m_matrixTranspose;
}

//=============================================
// @brief Returns the transpose of the matrix
//
// @return Transpose of the matrix
//=============================================
const Float *CMatrix::Transpose( void )
{
    m_outMatrix[0] = m_pCurrentMatrix[0];   m_outMatrix[1] = m_pCurrentMatrix[4];   m_outMatrix[2] = m_pCurrentMatrix[8];   m_outMatrix[3] = m_pCurrentMatrix[12];
    m_outMatrix[4] = m_pCurrentMatrix[1];   m_outMatrix[5] = m_pCurrentMatrix[5];   m_outMatrix[6] = m_pCurrentMatrix[9];   m_outMatrix[7] = m_pCurrentMatrix[13];
    m_outMatrix[8] = m_pCurrentMatrix[2];   m_outMatrix[9] = m_pCurrentMatrix[6];   m_outMatrix[10] = m_pCurrentMatrix[10];  m_outMatrix[11] = m_pCurrentMatrix[14];
    m_outMatrix[12]= m_pCurrentMatrix[3];   m_outMatrix[13] = m_pCurrentMatrix[7];   m_outMatrix[14] = m_pCurrentMatrix[11];  m_outMatrix[15] = m_pCurrentMatrix[15];
	return m_outMatrix;
}

//=============================================
// @brief Sets the values of the matrix
//
// @param m Source matrix to copy values from
//=============================================
void CMatrix::SetMatrix( const Float *pMatrix ) 
{
	m_pCurrentMatrix[0] = pMatrix[0]; m_pCurrentMatrix[1] = pMatrix[4]; m_pCurrentMatrix[2] = pMatrix[8]; m_pCurrentMatrix[3] = pMatrix[12]; 
	m_pCurrentMatrix[4] = pMatrix[1]; m_pCurrentMatrix[5] = pMatrix[5]; m_pCurrentMatrix[6] = pMatrix[9]; m_pCurrentMatrix[7] = pMatrix[13]; 
	m_pCurrentMatrix[8] = pMatrix[2]; m_pCurrentMatrix[9] = pMatrix[6]; m_pCurrentMatrix[10] = pMatrix[10]; m_pCurrentMatrix[11] = pMatrix[14];
	m_pCurrentMatrix[12] = pMatrix[3]; m_pCurrentMatrix[13] = pMatrix[7]; m_pCurrentMatrix[14] = pMatrix[11]; m_pCurrentMatrix[15] = pMatrix[15];
}

//=============================================
// @brief Restores the matrix to it's identity
//
//=============================================
void CMatrix::LoadIdentity( void )
{
	m_pCurrentMatrix[0] = m_pCurrentMatrix[5] = m_pCurrentMatrix[10] = m_pCurrentMatrix[15] = 1.0f;
	m_pCurrentMatrix[1] = m_pCurrentMatrix[2] = m_pCurrentMatrix[3] = m_pCurrentMatrix[4] = m_pCurrentMatrix[6] = m_pCurrentMatrix[7] = m_pCurrentMatrix[8] = m_pCurrentMatrix[9] = m_pCurrentMatrix[11] = m_pCurrentMatrix[12] = m_pCurrentMatrix[13] = m_pCurrentMatrix[14] = 0.0f;
}

//=============================================
// @brief Multiplies the matrix with another matrix
//
// @param Right hand matrix to multiply with
//=============================================
void CMatrix::MultMatrix ( const Float *pMatrix )
{
	m_multMatrix[0] = pMatrix[0] * m_pCurrentMatrix[0] + pMatrix[4] * m_pCurrentMatrix[1] + pMatrix[8] * m_pCurrentMatrix[2] + pMatrix[12] * m_pCurrentMatrix[3];
	m_multMatrix[1] = pMatrix[1] * m_pCurrentMatrix[0] + pMatrix[5] * m_pCurrentMatrix[1] + pMatrix[9] * m_pCurrentMatrix[2] + pMatrix[13] * m_pCurrentMatrix[3];
	m_multMatrix[2] = pMatrix[2] * m_pCurrentMatrix[0] + pMatrix[6] * m_pCurrentMatrix[1] + pMatrix[10] * m_pCurrentMatrix[2] + pMatrix[14] * m_pCurrentMatrix[3];
	m_multMatrix[3] = pMatrix[3] * m_pCurrentMatrix[0] + pMatrix[7] * m_pCurrentMatrix[1] + pMatrix[11] * m_pCurrentMatrix[2] + pMatrix[15] * m_pCurrentMatrix[3];

	m_multMatrix[4] = pMatrix[0] * m_pCurrentMatrix[4] + pMatrix[4] * m_pCurrentMatrix[5] + pMatrix[8] * m_pCurrentMatrix[6] + pMatrix[12] * m_pCurrentMatrix[7];
	m_multMatrix[5] = pMatrix[1] * m_pCurrentMatrix[4] + pMatrix[5] * m_pCurrentMatrix[5] + pMatrix[9] * m_pCurrentMatrix[6] + pMatrix[13] * m_pCurrentMatrix[7];
	m_multMatrix[6] = pMatrix[2] * m_pCurrentMatrix[4] + pMatrix[6] * m_pCurrentMatrix[5] + pMatrix[10] * m_pCurrentMatrix[6] + pMatrix[14] * m_pCurrentMatrix[7];
	m_multMatrix[7] = pMatrix[3] * m_pCurrentMatrix[4] + pMatrix[7] * m_pCurrentMatrix[5] + pMatrix[11] * m_pCurrentMatrix[6] + pMatrix[15] * m_pCurrentMatrix[7];

	m_multMatrix[8] = pMatrix[0] * m_pCurrentMatrix[8] + pMatrix[4] * m_pCurrentMatrix[9] + pMatrix[8] * m_pCurrentMatrix[10] + pMatrix[12] * m_pCurrentMatrix[11];
	m_multMatrix[9] = pMatrix[1] * m_pCurrentMatrix[8] + pMatrix[5] * m_pCurrentMatrix[9] + pMatrix[9] * m_pCurrentMatrix[10] + pMatrix[13] * m_pCurrentMatrix[11];
	m_multMatrix[10] = pMatrix[2] * m_pCurrentMatrix[8] + pMatrix[6] * m_pCurrentMatrix[9] + pMatrix[10] * m_pCurrentMatrix[10] + pMatrix[14] * m_pCurrentMatrix[11];
	m_multMatrix[11] = pMatrix[3] * m_pCurrentMatrix[8] + pMatrix[7] * m_pCurrentMatrix[9] + pMatrix[11] * m_pCurrentMatrix[10] + pMatrix[15] * m_pCurrentMatrix[11];

	m_multMatrix[12] = pMatrix[0] * m_pCurrentMatrix[12] + pMatrix[4] * m_pCurrentMatrix[13] + pMatrix[8] * m_pCurrentMatrix[14] + pMatrix[12] * m_pCurrentMatrix[15];
	m_multMatrix[13] = pMatrix[1] * m_pCurrentMatrix[12] + pMatrix[5] * m_pCurrentMatrix[13] + pMatrix[9] * m_pCurrentMatrix[14] + pMatrix[13] * m_pCurrentMatrix[15];
	m_multMatrix[14] = pMatrix[2] * m_pCurrentMatrix[12] + pMatrix[6] * m_pCurrentMatrix[13] + pMatrix[10] * m_pCurrentMatrix[14] + pMatrix[14] * m_pCurrentMatrix[15];
	m_multMatrix[15] = pMatrix[3] * m_pCurrentMatrix[12] + pMatrix[7] * m_pCurrentMatrix[13] + pMatrix[11] * m_pCurrentMatrix[14] + pMatrix[15] * m_pCurrentMatrix[15];

	// copy it over
	memcpy(m_pCurrentMatrix, m_multMatrix, sizeof(Float)*16);
}

//=============================================
// @brief Translates the matrix with a vector
//
// @param x Value on x coordinate to translate by
// @param y Value on y coordinate to translate by
// @param z Value on z coordinate to translate by
//=============================================
void CMatrix::Translate ( Float x, Float y, Float z )
{
	m_workingMatrix[0]  = 1;  m_workingMatrix[1]  = 0;  m_workingMatrix[2]  = 0;  m_workingMatrix[3]  = x;
	m_workingMatrix[4]  = 0;  m_workingMatrix[5]  = 1;  m_workingMatrix[6]  = 0;  m_workingMatrix[7]  = y;
	m_workingMatrix[8]  = 0;  m_workingMatrix[9]  = 0;  m_workingMatrix[10] = 1;  m_workingMatrix[11] = z;
	m_workingMatrix[12] = 0;  m_workingMatrix[13] = 0;  m_workingMatrix[14] = 0;  m_workingMatrix[15] = 1;
	MultMatrix(m_workingMatrix);
}

//=============================================
// @brief Rotates the matrix with a vector
//
//=============================================
void CMatrix::Rotate ( Float angle, Float x, Float y, Float z )
{
	Float sinAngle, cosAngle;
	Float mag = sqrtf(x * x + y * y + z * z);

	sinAngle = sinf ( (Float)(angle * M_PI / 180.0f) );
	cosAngle = cosf ( (Float)(angle * M_PI / 180.0f) );
	if ( mag > 0.0f )
	{
		Float xx, yy, zz, xy, yz, zx, xs, ys, zs;
		Float oneMinusCos;

		x /= mag;
		y /= mag;
		z /= mag;

		xx = x * x;
		yy = y * y;
		zz = z * z;
		xy = x * y;
		yz = y * z;
		zx = z * x;
		xs = x * sinAngle;
		ys = y * sinAngle;
		zs = z * sinAngle;
		oneMinusCos = 1.0f - cosAngle;

		m_workingMatrix[0] = (oneMinusCos * xx) + cosAngle;
		m_workingMatrix[1] = (oneMinusCos * xy) - zs;
		m_workingMatrix[2] = (oneMinusCos * zx) + ys;
		m_workingMatrix[3] = 0.0F; 

		m_workingMatrix[4] = (oneMinusCos * xy) + zs;
		m_workingMatrix[5] = (oneMinusCos * yy) + cosAngle;
		m_workingMatrix[6] = (oneMinusCos * yz) - xs;
		m_workingMatrix[7] = 0.0F;

		m_workingMatrix[8] = (oneMinusCos * zx) - ys;
		m_workingMatrix[9] = (oneMinusCos * yz) + xs;
		m_workingMatrix[10] = (oneMinusCos * zz) + cosAngle;
		m_workingMatrix[11] = 0.0F; 

		m_workingMatrix[12] = 0.0F;
		m_workingMatrix[13] = 0.0F;
		m_workingMatrix[14] = 0.0F;
		m_workingMatrix[15] = 1.0F;

		MultMatrix(m_workingMatrix);
   }
}

//=============================================
// @brief Scales the matrix
//
// @param x Value on x coordinate to scale width
// @param y Value on y coordinate to scale width
// @param z Value on z coordinate to scale width
//=============================================
void CMatrix::Scale ( Float x, Float y, Float z )
{
	m_workingMatrix[0] = x; m_workingMatrix[5] = y; m_workingMatrix[10] = z; m_workingMatrix[15] = 1;
	m_workingMatrix[1] = m_workingMatrix[2] = m_workingMatrix[3] = m_workingMatrix[4] = m_workingMatrix[6] = m_workingMatrix[7] = m_workingMatrix[8] = m_workingMatrix[9] = m_workingMatrix[11] = m_workingMatrix[12] = m_workingMatrix[13] = m_workingMatrix[14] = 0;
	MultMatrix(m_workingMatrix);
}

//=============================================
// @brief Sets the matrix as a view frustum matrix
//
//=============================================
void CMatrix::SetFrustum( Float l, Float r, Float b, Float t, Float n, Float f )
{
	memset(m_workingMatrix, 0, sizeof(m_workingMatrix));
	m_workingMatrix[0]  = 2 * n / (r - l);
	m_workingMatrix[2]  = (r + l) / (r - l);
	m_workingMatrix[5]  = 2 * n / (t - b);
	m_workingMatrix[6]  = (t + b) / (t - b);
	m_workingMatrix[10] = -(f + n) / (f - n);
	m_workingMatrix[11] = -(2 * f * n) / (f - n);
	m_workingMatrix[14] = -1;
	m_workingMatrix[15] = 0;
	MultMatrix(m_workingMatrix);
}

//=============================================
// @brief Sets up an orthogonical projection matrix
//
//=============================================
void CMatrix::Ortho(Float l, Float r, Float b, Float t, Float n, Float f)
{
	Float r_l = r - l;
	Float t_b = t - b;
	Float f_n = f - n;
	Float tx = - (r + l) / (r - l);
	Float ty = - (t + b) / (t - b);
	Float tz = - (f + n) / (f - n);

	m_workingMatrix[0] = 2.0f / r_l;
	m_workingMatrix[1] = 0.0f;
	m_workingMatrix[2] = 0.0f;
	m_workingMatrix[3] = tx;

	m_workingMatrix[4] = 0.0f;
	m_workingMatrix[5] = 2.0f / t_b;
	m_workingMatrix[6] = 0.0f;
	m_workingMatrix[7] = ty;

	m_workingMatrix[8] = 0.0f;
	m_workingMatrix[9] = 0.0f;
	m_workingMatrix[10] = -2.0f / f_n;
	m_workingMatrix[11] = tz;

	m_workingMatrix[12] = 0.0f;
	m_workingMatrix[13] = 0.0f;
	m_workingMatrix[14] = 0.0f;
	m_workingMatrix[15] = 1.0f;
	MultMatrix(m_workingMatrix);
}

//=============================================
// @brief Sets up a matrix that looks at a given coordinate
//
//=============================================
void CMatrix::LookAt( Float eyex, Float eyey, Float eyez, Float centerx, Float centery, Float centerz, Float upx, Float upy, Float upz )
{
	Float x[3], y[3], z[3];
	Float mag;

	/* Make rotation matrix */

	/* Z vector */
	z[0] = eyex - centerx;
	z[1] = eyey - centery;
	z[2] = eyez - centerz;
	mag = sqrt( z[0]*z[0] + z[1]*z[1] + z[2]*z[2] );
	if (mag) {  /* mpichler, 19950515 */
		z[0] /= mag;
		z[1] /= mag;
		z[2] /= mag;
	}

	/* Y vector */
	y[0] = upx;
	y[1] = upy;
	y[2] = upz;

	/* X vector = Y cross Z */
	x[0] =  y[1]*z[2] - y[2]*z[1];
	x[1] = -y[0]*z[2] + y[2]*z[0];
	x[2] =  y[0]*z[1] - y[1]*z[0];

	/* Recompute Y = Z cross X */
	y[0] =  z[1]*x[2] - z[2]*x[1];
	y[1] = -z[0]*x[2] + z[2]*x[0];
	y[2] =  z[0]*x[1] - z[1]*x[0];

	/* mpichler, 19950515 */
	/* cross product gives area of parallelogram, which is < 1.0 for
	* non-perpendicular unit-length vectors; so normalize x, y here
	*/

	mag = sqrt( x[0]*x[0] + x[1]*x[1] + x[2]*x[2] );
	if (mag) {
		x[0] /= mag;
		x[1] /= mag;
		x[2] /= mag;
	}

	mag = sqrt( y[0]*y[0] + y[1]*y[1] + y[2]*y[2] );
	if (mag) {
		y[0] /= mag;
		y[1] /= mag;
		y[2] /= mag;
	}

	#define M(row,col)  m_workingMatrix[col*4+row]
	M(0,0) = x[0];  M(1,0) = x[1];  M(2,0) = x[2];  M(3,0) = 0.0;
	M(0,1) = y[0];  M(1,1) = y[1];  M(2,1) = y[2];  M(3,1) = 0.0;
	M(0,2) = z[0];  M(1,2) = z[1];  M(2,2) = z[2];  M(3,2) = 0.0;
	M(0,3) = 0.0;   M(1,3) = 0.0;   M(2,3) = 0.0;   M(3,3) = 1.0;
	#undef M

	MultMatrix(m_workingMatrix);
	Translate(-eyex, -eyey, -eyez);
}

//=============================================
// @brief Sets up a matrix that looks at a given coordinate
//
//=============================================
void CMatrix::operator=( const CMatrix& src )
{
	// Clear existing
	if(m_pMatrixHeader)
	{
		matrix_t *pstack = m_pMatrixHeader;
		while(pstack)
		{
			matrix_t *pfree = pstack;
			pstack = pfree->next;
			delete pfree;
		}

		m_pMatrixHeader = nullptr;
	}

	// Reset
	m_pCurrentMatrix = nullptr;

	matrix_t* lastinsert = nullptr;
	matrix_t* pnext = src.m_pMatrixHeader;
	while(pnext)
	{
		matrix_t* pnew = new matrix_t();

		if(!lastinsert)
			m_pMatrixHeader = pnew;
		else
			lastinsert->next = pnew;

		for(Uint32 i = 0; i < 16; i++)
			pnew->matrix[i] = pnext->matrix[i];
		
		pnext = pnext->next;
		lastinsert = pnew;
	}

	for(Uint32 i = 0; i < 16; i++)
		m_workingMatrix[i] = 0;

	for(Uint32 i = 0; i < 16; i++)
		m_outMatrix[i] = 0;

	for(Uint32 i = 0; i < 16; i++)
		m_multMatrix[i] = 0;

	for(Uint32 i = 0; i < 16; i++)
		m_rotationMatrix[i] = 0;

	for(Uint32 i = 0; i < 16; i++)
		m_matrixTranspose[i] = 0;
}