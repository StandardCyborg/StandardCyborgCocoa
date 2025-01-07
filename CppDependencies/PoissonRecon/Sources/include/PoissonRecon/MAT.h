/*
Copyright (c) 2007, Michael Kazhdan
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of
conditions and the following disclaimer. Redistributions in binary form must reproduce
the above copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the distribution. 

Neither the name of the Johns Hopkins University nor the names of its contributors
may be used to endorse or promote products derived from this software without specific
prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES 
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE.
*/
#ifndef MAT_INCLUDED
#define MAT_INCLUDED
#include "Geometry.h"
#include "Array.h"

template< class Real , unsigned int Dim >
std::vector< TriangleIndex > MinimalAreaTriangulation( ConstPointer( Point< Real , Dim > ) vertices , size_t vCount );

template< class Real , unsigned int Dim >
class _MinimalAreaTriangulation
{
	Pointer( Real ) _bestTriangulation;
	Pointer( int ) _midpoint;
	size_t _vCount;
	ConstPointer( Point< Real , Dim > ) _vertices;

	void _set( void );
	Real _subPolygonArea( size_t i , size_t j );
	void _addTriangles( size_t i , size_t j , std::vector< TriangleIndex >& triangles ) const;
	size_t _subPolygonIndex( size_t i , size_t j ) const;

	_MinimalAreaTriangulation( ConstPointer( Point< Real , Dim > ) vertices , size_t vCount );
	~_MinimalAreaTriangulation( void );
	std::vector< TriangleIndex > getTriangulation( void );
	friend std::vector< TriangleIndex > MinimalAreaTriangulation< Real , Dim >( ConstPointer( Point< Real , Dim > ) vertices , size_t vCount );
};
template< class Real , unsigned int Dim >
std::vector< TriangleIndex > MinimalAreaTriangulation( ConstPointer( Point< Real , Dim > ) vertices , size_t vCount )
{
	_MinimalAreaTriangulation< Real , Dim > MAT( vertices , vCount );
	return MAT.getTriangulation();
}

#include "MAT.inl"

#endif // MAT_INCLUDED
