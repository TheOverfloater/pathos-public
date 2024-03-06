/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef VECTOR_INLINE_HPP
#define VECTOR_INLINE_HPP

//=============================================
// @brief Constructor
//
//=============================================
inline Vector::Vector( void ):
	x(0),
	y(0),
	z(0)
{
}

//=============================================
// @brief Constructor
//
// @param srcX Value on X coordinate
// @param srcY Value on Y coordinate
// @param srcZ Value on Z coordinate
//=============================================
inline Vector::Vector( Float srcX, Float srcY, Float srcZ ):
	x(srcX),
	y(srcY),
	z(srcZ)
{
}

//=============================================
// @brief Constructor
//
// @param src Vector object to copy
//=============================================
inline Vector::Vector( const Vector& src ):
	x(src.x),
	y(src.y),
	z(src.z)
{
}

//=============================================
// @brief Constructor
//
// @param coords Array of coordinates to copy
//=============================================
inline Vector::Vector( Float coords[3] ):
	x(coords[0]),
	y(coords[1]),
	z(coords[2])
{
}

//=============================================
// @brief Negation operator
//
// @param rhv Reference to right hand Vector object
//=============================================
inline Vector Vector::operator-( void ) const
{
	return Vector(-x, -y, -z);
}

//=============================================
// @brief Equality operator
//
// @param rhv Reference to right hand Vector object
// @return Comparison result
//=============================================
inline bool Vector::operator==( const Vector& rhv ) const
{
	return (rhv.x == x && rhv.y == y && rhv.z == z);
}

//=============================================
// @brief Not equal operator
//
// @param rhv Reference to right hand Vector object
// @return Comparison result
//=============================================
inline bool Vector::operator!=( const Vector& rhv ) const
{
	return (rhv.x != x || rhv.y != y || rhv.z != z);
}

//=============================================
// @brief Addition operator
//
// @param rhv Reference to right hand Vector object
// @return Vector object containing the result
//=============================================
inline Vector Vector::operator+( const Vector& rhv ) const
{
	return Vector(x + rhv.x, y + rhv.y, z + rhv.z);
}

//=============================================
// @brief Plus equals operator
//
// @param rhv Reference to right hand Vector object
// @return Reference to this vector object
//=============================================
inline Vector& Vector::operator+=( const Vector& rhv )
{
	x = x + rhv.x;
	y = y + rhv.y;
	z = z + rhv.z;

	return *this;
}

//=============================================
// @brief Subtraction operator
//
// @param rhv Reference to right hand Vector object
// @return Vector object containing the result
//=============================================
inline Vector Vector::operator-( const Vector& rhv ) const
{
	return Vector(x - rhv.x, y - rhv.y, z - rhv.z);
}

//=============================================
// @brief Minus equals operator
//
// @param rhv Reference to right hand Vector object
// @return Reference to this vector object
//=============================================
inline Vector& Vector::operator-=( const Vector& rhv )
{
	x = x - rhv.x;
	y = y - rhv.y;
	z = z - rhv.z;

	return *this;
}

//=============================================
// @brief Multiplication operator
//
// @param rhv Reference to right hand Vector object
// @return Vector object containing the result
//=============================================
inline Vector Vector::operator*( const Vector& rhv ) const
{
	return Vector(x * rhv.x, y * rhv.y, z * rhv.z);
}

//=============================================
// @brief Multiplication and assignment operator
//
// @param rhv Reference to right hand Vector object
// @return Reference to this vector object
//=============================================
inline Vector& Vector::operator*=( const Vector& rhv )
{
	x = x * rhv.x;
	y = y * rhv.y;
	z = z * rhv.z;

	return *this;
}

//=============================================
// @brief Division operator
//
// @param rhv Reference to right hand Vector object
// @return Vector object containing the result
//=============================================
inline Vector Vector::operator/( const Vector& rhv ) const
{
	return Vector(x / rhv.x, y / rhv.y, z / rhv.z);
}

//=============================================
// @brief Division and assignment operator
//
// @param rhv Reference to right hand Vector object
// @return Reference to this vector object
//=============================================
inline Vector& Vector::operator/=( const Vector& rhv )
{
	x = x / rhv.x;
	y = y / rhv.y;
	z = z / rhv.z;

	return *this;
}

//=============================================
// @brief Assignment operator
//
// @param str Reference to CString to copy
// @return Vector object containing the result
//=============================================
inline Vector& Vector::operator=( const Vector& rhv )
{
	x = rhv.x;
	y = rhv.y;
	z = rhv.z;

	return *this;
}

//=============================================
// @brief Indexing operator
//
// @param str Reference to CString to copy
// @return Indexed value
//=============================================
inline Float& Vector::operator[]( Uint32 n )
{
	assert(n < 3);

	if(n == 0)
		return x;
	else if(n == 1)
		return y;
	else
		return z;
}

//=============================================
// @brief Indexing operator
//
// @param str Reference to CString to copy
// @return Indexed value
//=============================================
inline Float Vector::operator[]( Uint32 n ) const
{
	assert(n < 3);

	if(n == 0)
		return x;
	else if(n == 1)
		return y;
	else
		return z;
}

//=============================================
// @brief Indexing operator
//
// @param str Reference to CString to copy
// @return Indexed value
//=============================================
inline Float& Vector::operator[]( Int32 n )
{
	assert(n < 3);

	if(n == 0)
		return x;
	else if(n == 1)
		return y;
	else
		return z;
}

//=============================================
// @brief Indexing operator
//
// @param str Reference to CString to copy
// @return Indexed value
//=============================================
inline Float Vector::operator[]( Int32 n ) const
{
	assert(n < 3);

	if(n == 0)
		return x;
	else if(n == 1)
		return y;
	else
		return z;
}

//=============================================
// @brief Indexing operator
//
// @param str Reference to CString to copy
// @return Indexed value
//=============================================
inline Vector Vector::operator*(Float value) const
{
	return Vector(x*value, y*value, z*value);
}

//=============================================
// @brief Indexing operator
//
// @param str Reference to CString to copy
// @return Indexed value
//=============================================
inline Vector Vector::operator/(Float value) const
{
	return Vector(x/value, y/value, z/value);
}

//=============================================
// @brief Returns a pointer to the first element
//
// @return Pointer to first element
//=============================================
inline Vector::operator Float *( void )
{
	return &x;
}

//=============================================
// @brief Returns a const pointer to the first element
//
// @return Const pointer to first element
//=============================================
inline Vector::operator const Float *( void ) const
{
	return &x;
}

//=============================================
// @brief Gets the length of the vector
//
// @return Length of the vector
//=============================================
inline Float Vector::Length( void ) const
{
	return sqrt(x*x + y*y + z*z);
}

//=============================================
// @brief Gets the 2d length of the vector
//
// @return Length of the vector
//=============================================
inline Float Vector::Length2D( void ) const
{
	return sqrt(x*x + y*y);
}

//=============================================
// @brief Returns the normalized vector
//
// @return Normalized vector
//=============================================
inline Vector& Vector::Normalize( void )
{
	Float length = Length();

	if (length)
	{
		Float ilength = 1/length;
		x *= ilength;
		y *= ilength;
		z *= ilength;
	}

	return *this;
}

//=============================================
// @brief Clears the vector to zero
//
// @return Normalized vector
//=============================================
inline void Vector::Clear( void )
{
	x = y = z = 0;
}

//=============================================
// @brief Tells if the vector is zero
//
// @return TRUE if zero, FALSE otherwise
//=============================================
inline bool Vector::IsZero( void ) const
{
	return (x == 0 && y == 0 && z == 0) ? true : false;
}

//=============================================
// @brief Tells if the vector is NAN
//
// @return TRUE if zero, FALSE otherwise
//=============================================
inline bool Vector::IsNAN( Int32 index ) const
{
	assert(index >= 0 && index < 3);
	
	Float fvar = this->operator[](index);
	return (*reinterpret_cast<Int32*>(&(fvar)) & NANMASK) == NANMASK ? true : false;
}

//=============================================
// @brief Multiplies a Vector with a float
//
// @return Resulting vector
//=============================================
inline Vector operator*( Float lhv, const Vector& rhv )
{
	return Vector(rhv.x*lhv, rhv.y*lhv, rhv.z*lhv);
}
#endif // VECTOR_INLINE_HPP