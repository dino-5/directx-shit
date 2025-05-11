/*
The MIT License (MIT)

Copyright (c) 2024-2025, Jacco Bikker / Breda University of Applied Sciences.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

// How to use:
//
// Use this in *one* .c or .cpp
//   #define TINYBVH_IMPLEMENTATION
//   #include "tiny_bvh.h"
// Instantiate a BVH and build it for a list of triangles:
//   BVH bvh;
//   bvh.Build( (bvhvec4*)myVerts, numTriangles );
//   Ray ray( bvhvec3( 0, 0, 0 ), bvhvec3( 0, 0, 1 ), 1e30f );
//   bvh.Intersect( ray );
// After this, intersection information is in ray.hit.

// tinybvh can use custom vector types by defining TINYBVH_USE_CUSTOM_VECTOR_TYPES once before inclusion.
// To define custom vector types create a tinybvh namespace with the appropriate using directives, e.g.:
//	 namespace tinybvh
//   {
//     using bvhint2 = math::int2;
//     using bvhint3 = math::int3;
//     using bvhuint2 = math::uint2;
//     using bvhvec2 = math::float2;
//     using bvhvec3 = math::float3;
//     using bvhvec4 = math::float4;
//     using bvhdbl3 = math::double3;
//   }
//
//	 #define TINYBVH_USE_CUSTOM_VECTOR_TYPES
//   #include <tiny_bvh.h>

// tinybvh can be further configured using #defines, to be specified before the #include:
// #define BVHBINS 8        - the number of bins to use in regular BVH construction. Default is 8.
// #define HQBVHBINS 32     - the number of bins to use in SBVH construction. Default is 8.
// #define INST_IDX_BITS 10 - the number of bits to use for the instance index. Default is 32,
//                            which stores the bits in a separate field in tinybvh::Intersection.
// #define C_INT 1          - the estimated cost of a primitive intersection test. Default is 1.
// #define C_TRAV 1         - the estimated cost of a traversal step. Default is 1.

// See tiny_bvh_test.cpp for basic usage. In short:
// instantiate a BVH: tinybvh::BVH bvh;
// build it: bvh.Build( (tinybvh::bvhvec4*)triangleData, TRIANGLE_COUNT );
// ..where triangleData is an array of four-component float vectors:
// - For a single triangle, provide 3 vertices,
// - For each vertex provide x, y and z.
// The fourth float in each vertex is a dummy value and exists purely for
// a more efficient layout of the data in memory.

// More information about the BVH data structure:
// https://jacco.ompf2.com/2022/04/13/how-to-build-a-bvh-part-1-basics

// Further references: See README.md

// Author and contributors:
// Jacco Bikker: BVH code and examples
// Eddy L O Jansson: g++ / clang support
// Aras Pranckevičius: non-Intel architecture support
// Jefferson Amstutz: CMake support
// Christian Oliveros: WASM / EMSCRIPTEN support
// Thierry Cantenot: user-defined alloc & free
// David Peicho: slices & Rust bindings, API advice
// Aytek Aman: C++11 threading implementation

#ifndef TINY_BVH_H_
#define TINY_BVH_H_

// Library version:
#define TINY_BVH_VERSION_MAJOR	1
#define TINY_BVH_VERSION_MINOR	5
#define TINY_BVH_VERSION_SUB	2

// Run-time checks; disabled by default.
// #define PARANOID // checks out-of-bound access of slices
// #define SLICEDUMP // dumps the slice used for building to a file - debug feature.

// Binned BVH building: bin count.
#ifndef BVHBINS
#define BVHBINS 8
#endif
#ifndef HQBVHBINS
#define HQBVHBINS 8
#endif
#define AVXBINS 8 // must stay at 8.

// TLAS setting
// Note: Except when INST_IDX_BITS is set to 32, the instance index is encoded in
// the top bits of the prim idx field.
// Max number of instances in TLAS: 2 ^ INST_IDX_BITS
// Max number of primitives per BLAS: 2 ^ (32 - INST_IDX_BITS)
#ifndef INST_IDX_BITS
#define INST_IDX_BITS 32 // Use 4..~12 to use prim field bits for instance id, or set to 32 to store index in separate field.
#endif

// SAH BVH building: Heuristic parameters
// CPU traversal: C_INT = 1, C_TRAV = 1 seems optimal.
// These are defaults, which initialize the public members c_int and c_trav in
// BVHBase (and thus each BVH instance).
#ifndef C_INT
#define C_INT	1
#endif
#ifndef C_TRAV
#define C_TRAV	1
#endif

// SBVH: "Unsplitting"
#define SBVH_UNSPLITTING
// SBVH: "Ray Distribution Heuristic", WIP.
// #define RDH_FOR_SBVH
#define RDH_MAX_WEIGHT 0.8f

// Triangle intersection: "Watertight"
#define WATERTIGHT_TRITEST

// 'Infinity' values
#define BVH_FAR	1e30f		// actual valid ieee range: 3.40282347E+38
#define BVH_DBL_FAR 1e300	// actual valid ieee range: 1.797693134862315E+308

// Features
#ifndef NO_DOUBLE_PRECISION_SUPPORT
#define DOUBLE_PRECISION_SUPPORT
#endif
// #define TINYBVH_USE_CUSTOM_VECTOR_TYPES
// #define TINYBVH_NO_SIMD
#ifndef NO_INDEXED_GEOMETRY
#define ENABLE_INDEXED_GEOMETRY
#endif
#ifndef NO_CUSTOM_GEOMETRY
#define ENABLE_CUSTOM_GEOMETRY
#endif

// Experimental features

// CWBVH triangle format - doesn't seem to help on GPU?
// #define CWBVH_COMPRESSED_TRIS
// BVH4 triangle format
// #define BVH4_GPU_COMPRESSED_TRIS

// BVH8_CPU align to big boundaries - experimental.
#define BVH8_ALIGN_4K
// #define BVH8_ALIGN_32K

// ============================================================================
//
//        P R E L I M I N A R I E S
//
// ============================================================================

// needful includes
#ifdef _MSC_VER // Visual Studio / C11
#include <malloc.h> // for alloc/free
#include <stdio.h> // for fprintf
#include <math.h> // for sqrtf, fabs
#include <string.h> // for memset
#include <stdlib.h> // for exit(1)
#else // Emscripten / gcc / clang
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cstring>
#endif
#include <cstdint>

// Platform-independent compile-time warnings.
#define EMIT_COMPILER_WARNING_STRINGIFY0(x) #x
#define EMIT_COMPILER_WARNING_STRINGIFY1(x) EMIT_COMPILER_WARNING_STRINGIFY0(x)
#ifdef __GNUC__
#define EMIT_COMPILER_WARNING_COMPOSE(x) GCC warning x
#else
#define EMIT_COMPILER_MESSAGE_PREFACE(type) \
__FILE__ "(" EMIT_COMPILER_WARNING_STRINGIFY1(__LINE__) "): " type ": "
#define EMIT_COMPILER_WARNING_COMPOSE(x) message(EMIT_COMPILER_MESSAGE_PREFACE("warning C0000") x)
#endif
#define WARNING(x) _Pragma(EMIT_COMPILER_WARNING_STRINGIFY1(EMIT_COMPILER_WARNING_COMPOSE(x)))

// SSE/AVX/AVX2/NEON support.
// SSE4.2 availability: Since Nehalem (2008)
// AVX1 availability: Since Sandy Bridge (2011)
// AVX2 availability: Since Haswell (2013)
#ifndef TINYBVH_NO_SIMD
#if defined __x86_64__ || defined _M_X64 || defined __wasm_simd128__ || defined __wasm_relaxed_simd__
#if !defined __SSE4_2__  && !defined _MSC_VER
WARNING( "SSE4.2 not enabled in compilation." )
#else
#define BVH_USESSE
#ifndef __SSE4_2__
#define __SSE4_2__		// msvc doesn't set the SSE flag
#endif
#endif
#if !defined __AVX__
WARNING( "AVX not enabled in compilation." )
#define TINYBVH_NO_SIMD
#else
#define BVH_USEAVX		// required for BuildAVX, BVH_SoA and others
#define BVH_USESSE
#endif
#if !defined __AVX2__ || (!defined __FMA__ && !defined _MSC_VER)
WARNING( "AVX2 and FMA not enabled in compilation." )
#define TINYBVH_NO_SIMD
#else
#define BVH_USEAVX2		// required for BVH8_CPU
#define BVH_USEAVX
#define BVH_USESSE
#endif
#include "immintrin.h"	// for __m128 and __m256
#elif defined __aarch64__ || defined _M_ARM64
#if !defined __NEON__ && !defined(__APPLE__)
WARNING( "NEON not enabled in compilation." )
#define TINYBVH_NO_SIMD
#else
#define BVH_USENEON
#include "arm_neon.h"
#endif
#endif
#endif // TINYBVH_NO_SIMD

// aligned memory allocation
// note: formally, size needs to be a multiple of 'alignment', see:
// https://en.cppreference.com/w/c/memory/aligned_alloc.
// EMSCRIPTEN enforces this.
// Copy of the same construct in tinyocl, in a different namespace.
namespace tinybvh {
inline size_t make_multiple_of( size_t x, size_t alignment ) { return (x + (alignment - 1)) & ~(alignment - 1); }
#ifdef _MSC_VER // Visual Studio / C11
#define ALIGNED( x ) __declspec( align( x ) )
#define _ALIGNED_ALLOC(alignment,size) _aligned_malloc( make_multiple_of( size, alignment ), alignment );
#define _ALIGNED_FREE(ptr) _aligned_free( ptr );
#else // EMSCRIPTEN / gcc / clang
#define ALIGNED( x ) __attribute__( ( aligned( x ) ) )
#if !defined TINYBVH_NO_SIMD && (defined __x86_64__ || defined _M_X64 || defined __wasm_simd128__ || defined __wasm_relaxed_simd__)
#include <xmmintrin.h>
#define _ALIGNED_ALLOC(alignment,size) _mm_malloc( make_multiple_of( size, alignment ), alignment );
#define _ALIGNED_FREE(ptr) _mm_free( ptr );
#else
#if defined __APPLE__ || defined __aarch64__ || (defined __ANDROID_API__ && (__ANDROID_API__ >= 28))
#define _ALIGNED_ALLOC(alignment,size) aligned_alloc( alignment, make_multiple_of( size, alignment ) );
#elif defined __GNUC__
#ifdef __linux__
#define _ALIGNED_ALLOC(alignment,size) aligned_alloc( alignment, make_multiple_of( size, alignment ) );
#else
#define _ALIGNED_ALLOC(alignment,size) _aligned_malloc( alignment, make_multiple_of( size, alignment ) );
#endif
#endif
#define _ALIGNED_FREE(ptr) free( ptr );
#endif
#endif
inline void* malloc64( size_t size, void* = nullptr ) { return size == 0 ? 0 : _ALIGNED_ALLOC( 64, size ); }
inline void* malloc4k( size_t size, void* = nullptr ) { return size == 0 ? 0 : _ALIGNED_ALLOC( 4096, size ); }
inline void* malloc32k( size_t size, void* = nullptr ) { return size == 0 ? 0 : _ALIGNED_ALLOC( 32768, size ); }
inline void free64( void* ptr, void* = nullptr ) { _ALIGNED_FREE( ptr ); }
inline void free4k( void* ptr, void* = nullptr ) { _ALIGNED_FREE( ptr ); }
inline void free32k( void* ptr, void* = nullptr ) { _ALIGNED_FREE( ptr ); }
}; // namespace tiybvh

// Derived TLAS things; for convenience.
#define INST_IDX_SHFT (32 - INST_IDX_BITS)
#if INST_IDX_BITS == 32
#define PRIM_IDX_MASK 0xffffffff // instance index stored separately.
#else
#define PRIM_IDX_MASK ((1 << INST_IDX_SHFT) - 1) // instance index stored in top bits of hit.prim.
#endif

// Forced inlining.
#ifdef _MSC_VER
#define __FORCEINLINE __forceinline
#else
#define __FORCEINLINE __attribute__((always_inline)) inline
#endif

namespace tinybvh {

// We'll use this whenever a layout has no specialized shadow ray query.
#define FALLBACK_SHADOW_QUERY( s ) { Ray r = s; float d = s.hit.t; Intersect( r ); return r.hit.t < d; }

#ifdef _MSC_VER
// Suppress a warning caused by the union of x,y,.. and cell[..] in vectors.
// We need this union to address vector components either by name or by index.
// The warning is re-enabled right after the definition of the data types.
#pragma warning ( push )
#pragma warning ( disable: 4201 /* nameless struct / union */ )
#endif

#ifndef TINYBVH_USE_CUSTOM_VECTOR_TYPES

struct bvhvec3;
struct ALIGNED( 16 ) bvhvec4
{
	// vector naming is designed to not cause any name clashes.
	bvhvec4() = default;
	bvhvec4( const float a, const float b, const float c, const float d ) : x( a ), y( b ), z( c ), w( d ) {}
	bvhvec4( const float a ) : x( a ), y( a ), z( a ), w( a ) {}
	bvhvec4( const bvhvec3 & a );
	bvhvec4( const bvhvec3 & a, float b );
	float& operator [] ( const int32_t i ) { return cell[i]; }
	const float& operator [] ( const int32_t i ) const { return cell[i]; }
	union { struct { float x, y, z, w; }; float cell[4]; };
};

struct ALIGNED( 8 ) bvhvec2
{
	bvhvec2() = default;
	bvhvec2( const float a, const float b ) : x( a ), y( b ) {}
	bvhvec2( const float a ) : x( a ), y( a ) {}
	bvhvec2( const bvhvec4 a ) : x( a.x ), y( a.y ) {}
	float& operator [] ( const int32_t i ) { return cell[i]; }
	const float& operator [] ( const int32_t i ) const { return cell[i]; }
	union { struct { float x, y; }; float cell[2]; };
};

struct bvhvec3
{
	bvhvec3() = default;
	bvhvec3( const float a, const float b, const float c ) : x( a ), y( b ), z( c ) {}
	bvhvec3( const float a ) : x( a ), y( a ), z( a ) {}
	bvhvec3( const bvhvec4 a ) : x( a.x ), y( a.y ), z( a.z ) {}
	float& operator [] ( const int32_t i ) { return cell[i]; }
	const float& operator [] ( const int32_t i ) const { return cell[i]; }
	union { struct { float x, y, z; }; float cell[3]; };
};

struct bvhint3
{
	bvhint3() = default;
	bvhint3( const int32_t a, const int32_t b, const int32_t c ) : x( a ), y( b ), z( c ) {}
	bvhint3( const int32_t a ) : x( a ), y( a ), z( a ) {}
	bvhint3( const bvhvec3& a ) { x = (int32_t)a.x, y = (int32_t)a.y, z = (int32_t)a.z; }
	int32_t& operator [] ( const int32_t i ) { return cell[i]; }
	union { struct { int32_t x, y, z; }; int32_t cell[3]; };
};

struct bvhint2
{
	bvhint2() = default;
	bvhint2( const int32_t a, const int32_t b ) : x( a ), y( b ) {}
	bvhint2( const int32_t a ) : x( a ), y( a ) {}
	int32_t x, y;
};

struct bvhuint2
{
	bvhuint2() = default;
	bvhuint2( const uint32_t a, const uint32_t b ) : x( a ), y( b ) {}
	bvhuint2( const uint32_t a ) : x( a ), y( a ) {}
	uint32_t x, y;
};

#endif // TINYBVH_USE_CUSTOM_VECTOR_TYPES

struct ALIGNED( 32 ) bvhaabb
{
	bvhvec3 minBounds; uint32_t dummy1;
	bvhvec3 maxBounds; uint32_t dummy2;
};

struct bvhvec4slice
{
	bvhvec4slice() = default;
	bvhvec4slice( const bvhvec4* data, uint32_t count, uint32_t stride = sizeof( bvhvec4 ) );
	operator bool() const { return !!data; }
	const bvhvec4& operator [] ( size_t i ) const;
	const int8_t* data = nullptr;
	uint32_t count, stride;
};

// Math operations.
// Note: Since this header file is expected to be included in a source file
// of a separate project, the static keyword doesn't provide sufficient
// isolation; hence the tinybvh_ prefix.
inline float tinybvh_safercp( const float x ) { if (x > 1e-12f || x < -1e-12f) return 1.0f / x; else return x >= 0 ? BVH_FAR : -BVH_FAR; }
inline bvhvec3 tinybvh_safercp( const bvhvec3 a ) { return bvhvec3( tinybvh_safercp( a.x ), tinybvh_safercp( a.y ), tinybvh_safercp( a.z ) ); }
inline bvhvec3 tinybvh_rcp( const bvhvec3 a ) { return tinybvh_safercp( a ); /* bvhvec3( 1.0f / a.x, 1.0f / a.y, 1.0f / a.z ); */ }
inline float tinybvh_min( const float a, const float b ) { return a < b ? a : b; }
inline float tinybvh_max( const float a, const float b ) { return a > b ? a : b; }
inline double tinybvh_min( const double a, const double b ) { return a < b ? a : b; }
inline double tinybvh_max( const double a, const double b ) { return a > b ? a : b; }
inline int32_t tinybvh_min( const int32_t a, const int32_t b ) { return a < b ? a : b; }
inline int32_t tinybvh_max( const int32_t a, const int32_t b ) { return a > b ? a : b; }
inline uint32_t tinybvh_min( const uint32_t a, const uint32_t b ) { return a < b ? a : b; }
inline uint32_t tinybvh_max( const uint32_t a, const uint32_t b ) { return a > b ? a : b; }
inline bvhvec3 tinybvh_min( const bvhvec3& a, const bvhvec3& b ) { return bvhvec3( tinybvh_min( a.x, b.x ), tinybvh_min( a.y, b.y ), tinybvh_min( a.z, b.z ) ); }
inline bvhvec4 tinybvh_min( const bvhvec4& a, const bvhvec4& b ) { return bvhvec4( tinybvh_min( a.x, b.x ), tinybvh_min( a.y, b.y ), tinybvh_min( a.z, b.z ), tinybvh_min( a.w, b.w ) ); }
inline bvhvec3 tinybvh_max( const bvhvec3& a, const bvhvec3& b ) { return bvhvec3( tinybvh_max( a.x, b.x ), tinybvh_max( a.y, b.y ), tinybvh_max( a.z, b.z ) ); }
inline bvhvec4 tinybvh_max( const bvhvec4& a, const bvhvec4& b ) { return bvhvec4( tinybvh_max( a.x, b.x ), tinybvh_max( a.y, b.y ), tinybvh_max( a.z, b.z ), tinybvh_max( a.w, b.w ) ); }
inline float tinybvh_clamp( const float x, const float a, const float b ) { return x > a ? (x < b ? x : b) : a; /* NaN safe */ }
inline int32_t tinybvh_clamp( const int32_t x, const int32_t a, const int32_t b ) { return x > a ? (x < b ? x : b) : a; /* NaN safe */ }
template <class T> inline static void tinybvh_swap( T& a, T& b ) { T t = a; a = b; b = t; }
inline float tinybvh_half_area( const bvhvec3& v ) { return v.x < -BVH_FAR ? 0 : (v.x * v.y + v.y * v.z + v.z * v.x); } // for SAH calculations
inline uint32_t tinybvh_maxdim( const bvhvec3& v ) { uint32_t r = fabs( v.x ) > fabs( v.y ) ? 0 : 1; return fabs( v.z ) > fabs( v[r] ) ? 2 : r; }

// Operator overloads.
// Only a minimal set is provided.
#ifndef TINYBVH_USE_CUSTOM_VECTOR_TYPES

inline bvhvec2 operator-( const bvhvec2& a ) { return bvhvec2( -a.x, -a.y ); }
inline bvhvec3 operator-( const bvhvec3& a ) { return bvhvec3( -a.x, -a.y, -a.z ); }
inline bvhvec4 operator-( const bvhvec4& a ) { return bvhvec4( -a.x, -a.y, -a.z, -a.w ); }
inline bvhvec2 operator+( const bvhvec2& a, const bvhvec2& b ) { return bvhvec2( a.x + b.x, a.y + b.y ); }
inline bvhvec3 operator+( const bvhvec3& a, const bvhvec3& b ) { return bvhvec3( a.x + b.x, a.y + b.y, a.z + b.z ); }
inline bvhvec4 operator+( const bvhvec4& a, const bvhvec4& b ) { return bvhvec4( a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w ); }
inline bvhvec4 operator+( const bvhvec4& a, const bvhvec3& b ) { return bvhvec4( a.x + b.x, a.y + b.y, a.z + b.z, a.w ); }
inline bvhvec2 operator-( const bvhvec2& a, const bvhvec2& b ) { return bvhvec2( a.x - b.x, a.y - b.y ); }
inline bvhvec3 operator-( const bvhvec3& a, const bvhvec3& b ) { return bvhvec3( a.x - b.x, a.y - b.y, a.z - b.z ); }
inline bvhvec4 operator-( const bvhvec4& a, const bvhvec4& b ) { return bvhvec4( a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w ); }
inline void operator+=( bvhvec2& a, const bvhvec2& b ) { a.x += b.x; a.y += b.y; }
inline void operator+=( bvhvec3& a, const bvhvec3& b ) { a.x += b.x; a.y += b.y; a.z += b.z; }
inline void operator+=( bvhvec4& a, const bvhvec4& b ) { a.x += b.x; a.y += b.y; a.z += b.z; a.w += b.w; }
inline bvhvec2 operator*( const bvhvec2& a, const bvhvec2& b ) { return bvhvec2( a.x * b.x, a.y * b.y ); }
inline bvhvec3 operator*( const bvhvec3& a, const bvhvec3& b ) { return bvhvec3( a.x * b.x, a.y * b.y, a.z * b.z ); }
inline bvhvec4 operator*( const bvhvec4& a, const bvhvec4& b ) { return bvhvec4( a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w ); }
inline bvhvec2 operator*( const bvhvec2& a, float b ) { return bvhvec2( a.x * b, a.y * b ); }
inline bvhvec3 operator*( const bvhvec3& a, float b ) { return bvhvec3( a.x * b, a.y * b, a.z * b ); }
inline bvhvec4 operator*( const bvhvec4& a, float b ) { return bvhvec4( a.x * b, a.y * b, a.z * b, a.w * b ); }
inline bvhvec2 operator*( float b, const bvhvec2& a ) { return bvhvec2( b * a.x, b * a.y ); }
inline bvhvec3 operator*( float b, const bvhvec3& a ) { return bvhvec3( b * a.x, b * a.y, b * a.z ); }
inline bvhvec4 operator*( float b, const bvhvec4& a ) { return bvhvec4( b * a.x, b * a.y, b * a.z, b * a.w ); }
inline bvhvec2 operator/( float b, const bvhvec2& a ) { return bvhvec2( b / a.x, b / a.y ); }
inline bvhvec3 operator/( float b, const bvhvec3& a ) { return bvhvec3( b / a.x, b / a.y, b / a.z ); }
inline bvhvec4 operator/( float b, const bvhvec4& a ) { return bvhvec4( b / a.x, b / a.y, b / a.z, b / a.w ); }
inline void operator*=( bvhvec3& a, const float b ) { a.x *= b; a.y *= b; a.z *= b; }

#endif // TINYBVH_USE_CUSTOM_VECTOR_TYPES

// Vector math: cross and dot.
inline bvhvec3 tinybvh_cross( const bvhvec3& a, const bvhvec3& b )
{
	return bvhvec3( a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x );
}
inline float tinybvh_dot( const bvhvec2& a, const bvhvec2& b ) { return a.x * b.x + a.y * b.y; }
inline float tinybvh_dot( const bvhvec3& a, const bvhvec3& b ) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline float tinybvh_dot( const bvhvec4& a, const bvhvec4& b ) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

// Vector math: common operations.
inline float tinybvh_length( const bvhvec3& a ) { return sqrtf( a.x * a.x + a.y * a.y + a.z * a.z ); }
inline bvhvec3 tinybvh_normalize( const bvhvec3& a )
{
	float l = tinybvh_length( a ), rl = l == 0 ? 0 : (1.0f / l);
	return a * rl;
}
inline bvhvec3 tinybvh_transform_point( const bvhvec3& v, const float* T )
{
	const bvhvec3 res(
		T[0] * v.x + T[1] * v.y + T[2] * v.z + T[3],
		T[4] * v.x + T[5] * v.y + T[6] * v.z + T[7],
		T[8] * v.x + T[9] * v.y + T[10] * v.z + T[11] );
	const float w = T[12] * v.x + T[13] * v.y + T[14] * v.z + T[15];
	if (w == 1) return res; else return res * (1.f / w);
}
inline bvhvec3 tinybvh_transform_vector( const bvhvec3& v, const float* T )
{
	return bvhvec3( T[0] * v.x + T[1] * v.y + T[2] * v.z, T[4] * v.x +
		T[5] * v.y + T[6] * v.z, T[8] * v.x + T[9] * v.y + T[10] * v.z );
}

#ifdef DOUBLE_PRECISION_SUPPORT
// Double-precision math

#ifndef TINYBVH_USE_CUSTOM_VECTOR_TYPES

struct bvhdbl3
{
	bvhdbl3() = default;
	bvhdbl3( const double a, const double b, const double c ) : x( a ), y( b ), z( c ) {}
	bvhdbl3( const double a ) : x( a ), y( a ), z( a ) {}
	bvhdbl3( const bvhvec3 a ) : x( (double)a.x ), y( (double)a.y ), z( (double)a.z ) {}
	double& operator [] ( const int32_t i ) { return cell[i]; }
	const double& operator [] ( const int32_t i ) const { return cell[i]; }
	union { struct { double x, y, z; }; double cell[3]; };
};

#endif // TINYBVH_USE_CUSTOM_VECTOR_TYPES

#ifdef _MSC_VER
#pragma warning ( pop )
#endif

inline bvhdbl3 tinybvh_min( const bvhdbl3& a, const bvhdbl3& b ) { return bvhdbl3( tinybvh_min( a.x, b.x ), tinybvh_min( a.y, b.y ), tinybvh_min( a.z, b.z ) ); }
inline bvhdbl3 tinybvh_max( const bvhdbl3& a, const bvhdbl3& b ) { return bvhdbl3( tinybvh_max( a.x, b.x ), tinybvh_max( a.y, b.y ), tinybvh_max( a.z, b.z ) ); }

#ifndef TINYBVH_USE_CUSTOM_VECTOR_TYPES

inline bvhdbl3 operator-( const bvhdbl3& a ) { return bvhdbl3( -a.x, -a.y, -a.z ); }
inline bvhdbl3 operator+( const bvhdbl3& a, const bvhdbl3& b ) { return bvhdbl3( a.x + b.x, a.y + b.y, a.z + b.z ); }
inline bvhdbl3 operator-( const bvhdbl3& a, const bvhdbl3& b ) { return bvhdbl3( a.x - b.x, a.y - b.y, a.z - b.z ); }
inline void operator+=( bvhdbl3& a, const bvhdbl3& b ) { a.x += b.x; a.y += b.y; a.z += b.z; }
inline bvhdbl3 operator*( const bvhdbl3& a, const bvhdbl3& b ) { return bvhdbl3( a.x * b.x, a.y * b.y, a.z * b.z ); }
inline bvhdbl3 operator*( const bvhdbl3& a, double b ) { return bvhdbl3( a.x * b, a.y * b, a.z * b ); }
inline bvhdbl3 operator*( double b, const bvhdbl3& a ) { return bvhdbl3( b * a.x, b * a.y, b * a.z ); }
inline bvhdbl3 operator/( double b, const bvhdbl3& a ) { return bvhdbl3( b / a.x, b / a.y, b / a.z ); }
inline bvhdbl3 operator*=( bvhdbl3& a, const double b ) { return bvhdbl3( a.x * b, a.y * b, a.z * b ); }

#endif // TINYBVH_USE_CUSTOM_VECTOR_TYPES

inline double tinybvh_length( const bvhdbl3& a ) { return sqrt( a.x * a.x + a.y * a.y + a.z * a.z ); }
inline bvhdbl3 tinybvh_normalize( const bvhdbl3& a )
{
	double l = tinybvh_length( a ), rl = l == 0 ? 0 : (1.0 / l);
	return a * rl;
}
inline bvhdbl3 tinybvh_transform_point( const bvhdbl3& v, const double* T )
{
	const bvhdbl3 res(
		T[0] * v.x + T[1] * v.y + T[2] * v.z + T[3],
		T[4] * v.x + T[5] * v.y + T[6] * v.z + T[7],
		T[8] * v.x + T[9] * v.y + T[10] * v.z + T[11] );
	const double w = T[12] * v.x + T[13] * v.y + T[14] * v.z + T[15];
	if (w == 1) return res; else return res * (1. / w);
}
inline bvhdbl3 tinybvh_transform_vector( const bvhdbl3& v, const double* T )
{
	return bvhdbl3( T[0] * v.x + T[1] * v.y + T[2] * v.z, T[4] * v.x +
		T[5] * v.y + T[6] * v.z, T[8] * v.x + T[9] * v.y + T[10] * v.z );
}

inline bvhdbl3 tinybvh_cross( const bvhdbl3& a, const bvhdbl3& b )
{
	return bvhdbl3( a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x );
}
inline double tinybvh_dot( const bvhdbl3& a, const bvhdbl3& b ) { return a.x * b.x + a.y * b.y + a.z * b.z; }

inline double tinybvh_half_area( const bvhdbl3& v ) { return v.x < -BVH_FAR ? 0 : (v.x * v.y + v.y * v.z + v.z * v.x); } // for SAH calculations

#endif // DOUBLE_PRECISION_SUPPORT

// SIMD typedef, helps keeping the interface generic
#ifdef BVH_USESSE
typedef __m128 SIMDVEC4;
typedef __m128i SIMDIVEC4;
#define SIMD_SETVEC(a,b,c,d) _mm_set_ps( a, b, c, d )
#define SIMD_SETRVEC(a,b,c,d) _mm_set_ps( d, c, b, a )
#elif defined BVH_USENEON
typedef float32x4_t SIMDVEC4;
typedef int32x4_t SIMDIVEC4;
inline float32x4_t SIMD_SETVEC( float w, float z, float y, float x )
{
	ALIGNED( 64 ) float data[4] = { x, y, z, w };
	return vld1q_f32( data );
}
inline float32x4_t SIMD_SETRVEC( float x, float y, float z, float w )
{
	ALIGNED( 64 ) float data[4] = { x, y, z, w };
	return vld1q_f32( data );
}
#else
typedef bvhvec4 SIMDVEC4;
typedef struct { int x, y, z, w; } SIMDIVEC4;
#define SIMD_SETVEC(a,b,c,d) bvhvec4( d, c, b, a )
#define SIMD_SETRVEC(a,b,c,d) bvhvec4( a, b, c, d )
#endif
#ifdef BVH_USEAVX
typedef __m256 SIMDVEC8;
typedef __m256i SIMDIVEC8;
#elif defined BVH_USENEON
typedef float32x4x2_t SIMDVEC8;
typedef int32x4x2_t SIMDIVEC8;
inline uint32x4_t SIMD_SETRVECU( uint32_t x, uint32_t y, uint32_t z, uint32_t w )
{
	ALIGNED( 64 ) uint32_t data[4] = { x, y, z, w };
	return vld1q_u32( data );
}
#else
typedef struct { float v0, v1, v2, v3, v4, v5, v6, v7; } SIMDVEC8;
typedef struct { int v0, v1, v2, v3, v4, v5, v6, v7; } SIMDIVEC8;
#endif

// ============================================================================
//
//        T I N Y _ B V H   I N T E R F A C E
//
// ============================================================================

#if defined _MSC_VER || defined __GNUC__
#pragma pack(push, 4) // is there a good alternative for Clang / EMSCRIPTEN?
#endif
struct Intersection
{
	// An intersection result is designed to fit in no more than
	// four 32-bit values. This allows efficient storage of a result in
	// GPU code. The obvious missing result is an instance id; consider
	// squeezing this in the 'prim' field in some way.
	// Using this data and the original triangle data, all other info for
	// shading (such as normal, texture color etc.) can be reconstructed.
#if INST_IDX_BITS == 32
	uint32_t inst;	// instance index. Stored in top bits of prim if INST_IDX_BITS != 32.
#endif
	float t, u, v;	// distance along ray & barycentric coordinates of the intersection
	uint32_t prim;	// primitive index
	// 64 byte of custom data -
	// assuming struct Ray is aligned, this starts at a cache line boundary.
	void* auxData;
	union
	{
		unsigned char userChar[56];
		float userFloat[14];
		uint32_t userInt32[14];
		double userDouble[7];
		uint64_t userInt64[7];
	};
};
#if defined _MSC_VER || defined __GNUC__
#pragma pack(pop) // is there a good alternative for Clang / EMSCRIPTEN?
#endif

// 16 bits of mask to tell which BLASInstance to intersect or not.
// By default the mask will be initialized to intersect all instances.
#define RAY_MASK_INTERSECT_ALL 0xFFFF

struct ALIGNED( 64 ) Ray
{
	// Basic ray class. Note: For single blas traversal it is expected
	// that Ray::rD is properly initialized. For tlas/blas traversal this
	// field is typically updated for each blas.
	Ray() = default;
	Ray( bvhvec3 origin, bvhvec3 direction, float t = BVH_FAR, uint32_t rayMask = RAY_MASK_INTERSECT_ALL )
	{
		memset( this, 0, sizeof( Ray ) );
		O = origin, D = tinybvh_normalize( direction ), rD = tinybvh_rcp( D );
		hit.t = t;
		mask = rayMask & RAY_MASK_INTERSECT_ALL;
	}
	ALIGNED( 16 ) bvhvec3 O; uint32_t mask = RAY_MASK_INTERSECT_ALL;
	ALIGNED( 16 ) bvhvec3 D; uint32_t instIdx = 0;
	ALIGNED( 16 ) bvhvec3 rD;
#if INST_IDX_BITS != 32
	uint32_t dummy2; // align to 16 bytes if field 'hit' is 16 bytes; otherwise don't.
#endif
	Intersection hit;
};

#ifdef DOUBLE_PRECISION_SUPPORT

struct IntersectionEx
{
	// Double-precision hit record.
	double t, u, v;	// distance along ray & barycentric coordinates of the intersection
	uint64_t inst, prim; // instance and primitive index
};

struct RayEx
{
	// Double-precision ray definition.
	RayEx() = default;
	RayEx( bvhdbl3 origin, bvhdbl3 direction, double tmax = BVH_DBL_FAR, uint32_t rayMask = RAY_MASK_INTERSECT_ALL )
	{
		memset( this, 0, sizeof( RayEx ) );
		O = origin, D = direction;
		double rl = 1.0 / sqrt( D.x * D.x + D.y * D.y + D.z * D.z );
		D.x *= rl, D.y *= rl, D.z *= rl;
		rD.x = 1.0 / D.x, rD.y = 1.0 / D.y, rD.z = 1.0 / D.z;
		hit.u = hit.v = 0, hit.t = tmax;
		instIdx = 0;
		mask = rayMask & RAY_MASK_INTERSECT_ALL;
	}
	bvhdbl3 O, D, rD;
	IntersectionEx hit;
	uint64_t instIdx;
	uint64_t mask;
};

#endif

struct BVHContext
{
	void* (*malloc)(size_t size, void* userdata) = malloc64;
	void (*free)(void* ptr, void* userdata) = free64;
	void* userdata = nullptr;
};

enum TraceDevice : uint32_t { USE_CPU = 1, USE_GPU };

class BVHBase
{
public:
	enum BVHType : uint32_t
	{
		// Every BVHJ class is derived from BVHBase, but we don't use virtual functions, for
		// performance reasons. For a TLAS over a mix of BVH layouts we do however need this
		// kind of behavior when transitioning from a TLAS leaf to a BLAS root node.
		UNDEFINED = 0,
		LAYOUT_BVH = 1,
		LAYOUT_BVH_VERBOSE,
		LAYOUT_BVH_DOUBLE,
		LAYOUT_BVH_SOA,
		LAYOUT_BVH_GPU,
		LAYOUT_MBVH,
		LAYOUT_BVH4_CPU,
		LAYOUT_BVH4_GPU,
		LAYOUT_MBVH8,
		LAYOUT_CWBVH,
		LAYOUT_BVH8_AVX2
	};
	struct ALIGNED( 32 ) Fragment
	{
		// A fragment stores the bounds of an input primitive. The name 'Fragment' is from
		// "Parallel Spatial Splits in Bounding Volume Hierarchies", 2016, Fuetterling et al.,
		// and refers to the potential splitting of these boxes for SBVH construction.
		bvhvec3 bmin;				// AABB min x, y and z
		uint32_t primIdx;			// index of the original primitive
		bvhvec3 bmax;				// AABB max x, y and z
		uint32_t clipped = 0;		// Fragment is the result of clipping if > 0.
		bool validBox() { return bmin.x < BVH_FAR; }
	};
	// BVH flags, maintainted by tiny_bvh.
	bool rebuildable = true;		// rebuilds are safe only if a tree has not been converted.
	bool refittable = true;			// refits are safe only if the tree has no spatial splits.
	bool may_have_holes = false;	// threaded builds and MergeLeafs produce BVHs with unused nodes.
	bool bvh_over_aabbs = false;	// a BVH over AABBs is useful for e.g. TLAS traversal.
	bool bvh_over_indices = false;	// a BVH over indices cannot translate primitive index to vertex index.
	BVHContext context;				// context used to provide user-defined allocation functions.
	BVHType layout = UNDEFINED;		// BVH layout identifier.
	// Keep track of allocated buffer size to avoid repeated allocation during layout conversion.
	uint32_t allocatedNodes = 0;	// number of nodes allocated for the BVH.
	uint32_t usedNodes = 0;			// number of nodes used for the BVH.
	uint32_t triCount = 0;			// number of primitives in the BVH.
	uint32_t idxCount = 0;			// number of primitive indices; can exceed triCount for SBVH.
	float c_trav = C_TRAV;			// cost of a traversal step, used to steer SAH construction.
	float c_int = C_INT;			// cost of a primitive intersection, used to steer SAH construction.
	bool l_quads = false;			// some layouts have 4 prims in each leaf; adjust SAH cost for this.
	bvhvec3 aabbMin, aabbMax;		// bounds of the root node of the BVH.
	// Custom memory allocation
	void* AlignedAlloc( size_t size );
	void AlignedFree( void* ptr );
	// Common methods
	void CopyBasePropertiesFrom( const BVHBase& original );	// copy flags from one BVH to another
protected:
	~BVHBase() {}
	__FORCEINLINE void IntersectTri( Ray& ray, const uint32_t idx, const bvhvec4slice& verts, const uint32_t i0, const uint32_t i1, const uint32_t i2 ) const;
	__FORCEINLINE bool TriOccludes( const Ray& ray, const bvhvec4slice& verts, const uint32_t i0, const uint32_t i1, const uint32_t i2 ) const;
	static void PrecomputeTriangle( const bvhvec4slice& vert, const uint32_t ti0, const uint32_t ti1, const uint32_t ti2, float* T );
	static float SA( const bvhvec3& aabbMin, const bvhvec3& aabbMax );
};

class BLASInstance;
class BVH_Verbose;
class BVH : public BVHBase
{
public:
	friend class BVH_GPU;
	friend class BVH_SoA;
	friend class BVH4_CPU;
	friend class BVH8_CPU;
	friend class BVH8_CWBVH;
	template <int M> friend class MBVH;
	enum BuildFlags : uint32_t
	{
		NONE = 0,			// Default building behavior (binned, SAH-driven).
		FULLSPLIT = 1		// Split as far as possible, even when SAH doesn't agree.
	};
	struct BVHNode
	{
		// 'Traditional' 32-byte BVH node layout, as proposed by Ingo Wald.
		// When aligned to a cache line boundary, two of these fit together.
		bvhvec3 aabbMin; uint32_t leftFirst; // 16 bytes
		bvhvec3 aabbMax; uint32_t triCount;	// 16 bytes, total: 32 bytes
		bool isLeaf() const { return triCount > 0; /* empty BVH leaves do not exist */ }
		bool Intersect( const bvhvec3& bmin, const bvhvec3& bmax ) const;
		float SurfaceArea() const { return BVH::SA( aabbMin, aabbMax ); }
	};
	BVH( BVHContext ctx = {} ) { layout = LAYOUT_BVH; context = ctx; }
	BVH( const BVH_Verbose& original ) { layout = LAYOUT_BVH; ConvertFrom( original ); }
	BVH( const bvhvec4* vertices, const uint32_t primCount ) { layout = LAYOUT_BVH; Build( vertices, primCount ); }
	BVH( const bvhvec4slice& vertices ) { layout = LAYOUT_BVH; Build( vertices ); }
	~BVH();
	void ConvertFrom( const BVH_Verbose& original, bool compact = true );
	void SplitLeafs( const uint32_t maxPrims );
	float SAHCost( const uint32_t nodeIdx = 0 ) const;
	int32_t NodeCount() const;
	int32_t LeafCount() const;
	int32_t PrimCount( const uint32_t nodeIdx = 0 ) const;
	void Compact();
	void Save( const char* fileName );
	bool Load( const char* fileName, const bvhvec4* vertices, const uint32_t primCount );
	bool Load( const char* fileName, const bvhvec4* vertices, const uint32_t* indices, const uint32_t primCount );
	bool Load( const char* fileName, const bvhvec4slice& vertices, const uint32_t* indices = 0, const uint32_t primCount = 0 );
	void BuildQuick( const bvhvec4* vertices, const uint32_t primCount );
	void BuildQuick( const bvhvec4slice& vertices );
	void Build( const bvhvec4* vertices, const uint32_t primCount );
	void Build( const bvhvec4slice& vertices );
	void Build( const bvhvec4* vertices, const uint32_t* indices, const uint32_t primCount );
	void Build( const bvhvec4slice& vertices, const uint32_t* indices, const uint32_t primCount );
	void Build( BLASInstance* instances, const uint32_t instCount, BVHBase** blasses, const uint32_t blasCount );
	void Build( void (*customGetAABB)(const unsigned, bvhvec3&, bvhvec3&), const uint32_t primCount );
	void BuildHQ( const bvhvec4* vertices, const uint32_t primCount );
	void BuildHQ( const bvhvec4slice& vertices );
	void BuildHQ( const bvhvec4* vertices, const uint32_t* indices, const uint32_t primCount );
	void BuildHQ( const bvhvec4slice& vertices, const uint32_t* indices, const uint32_t primCount );
	void BuildAVX( const bvhvec4* vertices, const uint32_t primCount );
	void BuildAVX( const bvhvec4slice& vertices );
	void BuildAVX( const bvhvec4* vertices, const uint32_t* indices, const uint32_t primCount );
	void BuildAVX( const bvhvec4slice& vertices, const uint32_t* indices, const uint32_t primCount );
#ifdef BVH_USENEON
	void BuildNEON( const bvhvec4* vertices, const uint32_t primCount );
	void BuildNEON( const bvhvec4slice& vertices );
	void BuildNEON( const bvhvec4* vertices, const uint32_t* indices, const uint32_t primCount );
	void BuildNEON( const bvhvec4slice& vertices, const uint32_t* indices, const uint32_t primCount );
	void PrepareNEONBuild( const bvhvec4slice& vertices, const uint32_t* indices, const uint32_t primCount );
	void BuildNEON();
#endif
	void Refit( const uint32_t nodeIdx = 0 );
	void Optimize( const uint32_t iterations = 25, bool extreme = false );
	uint32_t CombineLeafs( const uint32_t primCount, uint32_t& firstIdx, uint32_t nodeIdx = 0 );
	int32_t Intersect( Ray& ray ) const;
	bool IntersectSphere( const bvhvec3& pos, const float r ) const;
	bool IsOccluded( const Ray& ray ) const;
	void Intersect256Rays( Ray* first ) const;
	void Intersect256RaysSSE( Ray* packet ) const; // requires BVH_USEAVX
	// private:
	void PrepareBuild( const bvhvec4slice& vertices, const uint32_t* indices, const uint32_t primCount );
	void Build();
	void BuildFullSweep();
	bool IsOccludedTLAS( const Ray& ray ) const;
	int32_t IntersectTLAS( Ray& ray ) const;
	void PrepareAVXBuild( const bvhvec4slice& vertices, const uint32_t* indices, const uint32_t primCount );
	void BuildAVX();
	void PrepareHQBuild( const bvhvec4slice& vertices, const uint32_t* indices, const uint32_t prims );
	void BuildHQ();
	bool ClipFrag( const Fragment& orig, Fragment& newFrag, bvhvec3 bmin, bvhvec3 bmax, bvhvec3 minDim, const uint32_t splitAxis );
	void SplitFrag( const Fragment& orig, Fragment& left, Fragment& right, const bvhvec3& minDim, const uint32_t splitAxis, const float splitPos, bool& leftOK, bool& rightOK );
protected:
	template <bool posX, bool posY, bool posZ> int32_t Intersect( Ray& ray ) const;
	template <bool posX, bool posY, bool posZ> int32_t IntersectTLAS( Ray& ray ) const;
	template <bool posX, bool posY, bool posZ> bool IsOccluded( const Ray& ray ) const;
	template <bool posX, bool posY, bool posZ> bool IsOccludedTLAS( const Ray& ray ) const;
	void BuildDefault( const bvhvec4* vertices, const uint32_t primCount );
	void BuildDefault( const bvhvec4slice& vertices );
	void BuildDefault( const bvhvec4* vertices, const uint32_t* indices, const uint32_t primCount );
	void BuildDefault( const bvhvec4slice& vertices, const uint32_t* indices, const uint32_t primCount );
#ifdef RDH_FOR_SBVH
	void BuildRRS();
#endif
	// Helpers
	inline float SplitCostSAH( const float rAparent, const float Aleft, const int Nleft, const float Aright, const int Nright ) const;
	inline float NoSplitCostSAH( const int Nparent ) const;
#ifdef RDH_FOR_SBVH
	inline float SplitCostRDH( const int Pleft, const int Nleft, const int Pright, const int Nright ) const;
	inline float NoSplitCostRDH( const int Nparent ) const;
	inline float RDHSplitWeight( const int Nparent ) const;
#endif
	void QuickSort( const float* centroid, int first, int last );
public:
	// BVH type identification
	bool isTLAS() const { return instList != 0; }
	bool isBLAS() const { return instList == 0; }
	bool isIndexed() const { return vertIdx != 0; }
	bool hasCustomGeom() const { return customIntersect != 0; }
	// Basic BVH data
	bvhvec4slice verts = {};		// pointer to input primitive array: 3x16 bytes per tri.
	uint32_t* vertIdx = 0;			// vertex indices, only used in case the BVH is built over indexed prims.
	uint32_t* primIdx = 0;			// primitive index array.
	uint32_t* rrsHits = 0;			// for RDH: ray hit count per triangle.
	BLASInstance* instList = 0;		// instance array, for top-level acceleration structure.
	BVHBase** blasList = 0;			// blas array, for TLAS traversal.
	uint32_t blasCount = 0;			// number of blasses in blasList.
	BVHNode* bvhNode = 0;			// BVH node pool, Wald 32-byte format. Root is always in node 0.
	uint32_t newNodePtr = 0;		// used during build to keep track of next free node in pool.
	Fragment* fragment = 0;			// input primitive bounding boxes.
	bool useFullSweep = false;		// for experiments only; full-sweep SAH builder.
	// Custom geometry intersection callback
	bool (*customIntersect)(Ray&, const unsigned) = 0;
	bool (*customIsOccluded)(const Ray&, const unsigned) = 0;
};

#ifdef DOUBLE_PRECISION_SUPPORT

class BLASInstanceEx;
class BVH_Double : public BVHBase
{
public:
	struct BVHNode
	{
		// Double precision 'traditional' BVH node layout.
		// Compared to the default BVHNode, child node indices and triangle indices
		// are also expanded to 64bit values to support massive scenes.
		bvhdbl3 aabbMin, aabbMax; // 2x24 bytes
		uint64_t leftFirst; // 8 bytes
		uint64_t triCount; // 8 bytes, total: 64 bytes
		bool isLeaf() const { return triCount > 0; /* empty BVH leaves do not exist */ }
		double Intersect( const RayEx& ray ) const;
		double SurfaceArea() const;
	};
	struct Fragment
	{
		// Double-precision version of the fragment sruct.
		bvhdbl3 bmin, bmax;			// AABB
		uint64_t primIdx;			// index of the original primitive
	};
	BVH_Double( BVHContext ctx = {} ) { layout = LAYOUT_BVH_DOUBLE; context = ctx; }
	~BVH_Double();
	void Build( const bvhdbl3* vertices, const uint64_t primCount );
	void Build( BLASInstanceEx* bvhs, const uint64_t instCount, BVH_Double** blasses, const uint64_t blasCount );
	void Build( void (*customGetAABB)(const uint64_t, bvhdbl3&, bvhdbl3&), const uint64_t primCount );
	void PrepareBuild( const bvhdbl3* vertices, const uint64_t primCount );
	void Build();
	double SAHCost( const uint64_t nodeIdx = 0 ) const;
	int32_t Intersect( RayEx& ray ) const;
	bool IsOccluded( const RayEx& ray ) const;
	bool IsOccludedTLAS( const RayEx& ray ) const;
	int32_t IntersectTLAS( RayEx& ray ) const;
	bvhdbl3* verts = 0;				// pointer to input primitive array, double-precision, 3x24 bytes per tri.
	Fragment* fragment = 0;			// input primitive bounding boxes, double-precision.
	BVHNode* bvhNode = 0;			// BVH node, double precision format.
	uint64_t* primIdx = 0;			// primitive index array for double-precision bvh.
	BLASInstanceEx* instList = 0;	// instance array, for top-level acceleration structure.
	BVH_Double** blasList = 0;		// blas array, for TLAS traversal.
	uint64_t blasCount = 0;			// number of blasses in blasList.
	// 64-bit base overrides
	uint64_t newNodePtr = 0;		// next free bvh pool entry to allocate
	uint64_t usedNodes = 0;			// number of nodes used for the BVH.
	uint64_t allocatedNodes = 0;	// number of nodes allocated for the BVH.
	uint64_t triCount = 0;			// number of primitives in the BVH.
	uint64_t idxCount = 0;			// number of primitive indices.
	bvhdbl3 aabbMin, aabbMax;		// bounds of the root node of the BVH.
	// Custom geometry intersection callback
	bool (*customIntersect)(RayEx&, uint64_t) = 0;
	bool (*customIsOccluded)(const RayEx&, uint64_t) = 0;
};

#endif // DOUBLE_PRECISION_SUPPORT

class BVH_GPU : public BVHBase
{
public:
	struct BVHNode
	{
		// Alternative 64-byte BVH node layout, which specifies the bounds of
		// the children rather than the node itself. This layout is used by
		// Aila and Laine in their seminal GPU ray tracing paper.
		bvhvec3 lmin; uint32_t left;
		bvhvec3 lmax; uint32_t right;
		bvhvec3 rmin; uint32_t triCount;
		bvhvec3 rmax; uint32_t firstTri; // total: 64 bytes
		bool isLeaf() const { return triCount > 0; }
	};
	BVH_GPU( BVHContext ctx = {} ) { layout = LAYOUT_BVH_GPU; context = ctx; }
	BVH_GPU( const BVH& original ) { /* DEPRICATED */ ConvertFrom( original ); }
	~BVH_GPU();
	void Build( const bvhvec4* vertices, const uint32_t primCount );
	void Build( const bvhvec4slice& vertices );
	void Build( const bvhvec4* vertices, const uint32_t* indices, const uint32_t primCount );
	void Build( const bvhvec4slice& vertices, const uint32_t* indices, const uint32_t primCount );
	void Build( BLASInstance* instances, const uint32_t instCount, BVHBase** blasses, const uint32_t blasCount );
	void BuildHQ( const bvhvec4* vertices, const uint32_t primCount );
	void BuildHQ( const bvhvec4slice& vertices );
	void BuildHQ( const bvhvec4* vertices, const uint32_t* indices, const uint32_t primCount );
	void BuildHQ( const bvhvec4slice& vertices, const uint32_t* indices, const uint32_t primCount );
	void Optimize( const uint32_t iterations = 25, bool extreme = false );
	float SAHCost( const uint32_t nodeIdx = 0 ) const { return bvh.SAHCost( nodeIdx ); }
	void ConvertFrom( const BVH& original, bool compact = true );
	int32_t Intersect( Ray& ray ) const;
	bool IsOccluded( const Ray& ray ) const { FALLBACK_SHADOW_QUERY( ray ); }
	// BVH data
	BVHNode* bvhNode = 0;			// BVH node in Aila & Laine format.
	BVH bvh;						// BVH4 is created from BVH and uses its data.
	bool ownBVH = true;				// False when ConvertFrom receives an external bvh.
};

class BVH_SoA : public BVHBase
{
public:
	struct BVHNode
	{
		// Second alternative 64-byte BVH node layout, same as BVHAilaLaine but
		// with child AABBs stored in SoA order.
		SIMDVEC4 xxxx, yyyy, zzzz;
		uint32_t left, right, triCount, firstTri; // total: 64 bytes
		bool isLeaf() const { return triCount > 0; }
	};
	BVH_SoA( BVHContext ctx = {} ) { layout = LAYOUT_BVH_SOA; context = ctx; }
	BVH_SoA( const BVH& original ) { /* DEPRICATED */ layout = LAYOUT_BVH_SOA; ConvertFrom( original ); }
	~BVH_SoA();
	void Build( const bvhvec4* vertices, const uint32_t primCount );
	void Build( const bvhvec4slice& vertices );
	void Build( const bvhvec4* vertices, const uint32_t* indices, const uint32_t primCount );
	void Build( const bvhvec4slice& vertices, const uint32_t* indices, const uint32_t primCount );
	void BuildHQ( const bvhvec4* vertices, const uint32_t primCount );
	void BuildHQ( const bvhvec4slice& vertices );
	void BuildHQ( const bvhvec4* vertices, const uint32_t* indices, const uint32_t primCount );
	void BuildHQ( const bvhvec4slice& vertices, const uint32_t* indices, const uint32_t primCount );
	void Optimize( const uint32_t iterations = 25, bool extreme = false );
	float SAHCost( const uint32_t nodeIdx = 0 ) const { return bvh.SAHCost( nodeIdx ); }
	void Save( const char* fileName );
	bool Load( const char* fileName, const bvhvec4* vertices, const uint32_t primCount );
	bool Load( const char* fileName, const bvhvec4* vertices, const uint32_t* indices, const uint32_t primCount );
	bool Load( const char* fileName, const bvhvec4slice& vertices, const uint32_t* indices = 0, const uint32_t primCount = 0 );
	void ConvertFrom( const BVH& original, bool compact = true );
	int32_t Intersect( Ray& ray ) const;
	bool IsOccluded( const Ray& ray ) const;
	// BVH data
	BVHNode* bvhNode = 0;			// BVH node in 'structure of arrays' format.
	BVH bvh;						// BVH_SoA is created from BVH and uses its data.
	bool ownBVH = true;				// False when ConvertFrom receives an external bvh.
};

class BVH_Verbose : public BVHBase
{
public:
	struct BVHNode
	{
		// This node layout has some extra data per node: It stores left and right
		// child node indices explicitly, and stores the index of the parent node.
		// This format exists primarily for the BVH optimizer.
		bvhvec3 aabbMin; uint32_t left;
		bvhvec3 aabbMax; uint32_t right;
		uint32_t triCount, firstTri, parent;
		float dummy[5]; // total: 64 bytes.
		bool isLeaf() const { return triCount > 0; }
		float SA() const { return BVH::SA( aabbMin, aabbMax ); }
	};
	BVH_Verbose( BVHContext ctx = {} ) { layout = LAYOUT_BVH_VERBOSE; context = ctx; }
	BVH_Verbose( const BVH& original ) { /* DEPRECATED */ layout = LAYOUT_BVH_VERBOSE; ConvertFrom( original ); }
	~BVH_Verbose() { AlignedFree( bvhNode ); }
	void ConvertFrom( const BVH& original, bool compact = true );
	float SAHCost( const uint32_t nodeIdx = 0 ) const;
	int32_t NodeCount() const;
	int32_t PrimCount( const uint32_t nodeIdx = 0 ) const;
	void Refit( const uint32_t nodeIdx = 0, bool skipLeafs = false );
	void CheckFit( const uint32_t nodeIdx = 0, bool skipLeafs = false );
	void Compact();
	void SplitLeafs( const uint32_t maxPrims = 1 );
	void MergeLeafs();
	void Optimize( const uint32_t iterations = 25, bool extreme = false );
private:
	struct SortItem { uint32_t idx; float cost; };
	void RefitUp( uint32_t nodeIdx );
	float SAHCostUp( uint32_t nodeIdx ) const;
	uint32_t FindBestNewPosition( const uint32_t Lid ) const;
	uint32_t CountSubtreeTris( const uint32_t nodeIdx, uint32_t* counters );
	void MergeSubtree( const uint32_t nodeIdx, uint32_t* newIdx, uint32_t& newIdxPtr );
public:
	// BVH data
	bvhvec4slice verts = {};		// pointer to input primitive array: 3x16 bytes per tri.
	Fragment* fragment = 0;			// input primitive bounding boxes, double-precision.
	uint32_t* primIdx = 0;			// primitive index array - pointer copied from original.
	BVHNode* bvhNode = 0;			// BVH node with additional info, for BVH optimizer.
};

template <int M> class MBVH : public BVHBase
{
public:
	struct MBVHNode
	{
		// M-wide (aka 'shallow') BVH layout.
		bvhvec3 aabbMin; uint32_t firstTri;
		bvhvec3 aabbMax; uint32_t triCount;
		uint32_t child[M];
		uint32_t childCount;
		uint32_t dummy[((30 - M) & 3) + 1]; // dummies are for alignment.
		bool isLeaf() const { return triCount > 0; }
	};
	MBVH( BVHContext ctx = {} ) { layout = LAYOUT_MBVH; context = ctx; }
	MBVH( const BVH& original ) { /* DEPRECATED */ layout = LAYOUT_MBVH; ConvertFrom( original ); }
	~MBVH();
	void Build( const bvhvec4* vertices, const uint32_t primCount );
	void Build( const bvhvec4slice& vertices );
	void Build( const bvhvec4* vertices, const uint32_t* indices, const uint32_t primCount );
	void Build( const bvhvec4slice& vertices, const uint32_t* indices, const uint32_t primCount );
	void BuildHQ( const bvhvec4* vertices, const uint32_t primCount );
	void BuildHQ( const bvhvec4slice& vertices );
	void BuildHQ( const bvhvec4* vertices, const uint32_t* indices, const uint32_t primCount );
	void BuildHQ( const bvhvec4slice& vertices, const uint32_t* indices, const uint32_t primCount );
	void Optimize( const uint32_t iterations = 25, bool extreme = false );
	void Refit( const uint32_t nodeIdx = 0 );
	uint32_t LeafCount( const uint32_t nodeIdx = 0 ) const;
	float SAHCost( const uint32_t nodeIdx = 0 ) const;
	void ConvertFrom( const BVH& original, bool compact = true );
	// BVH data
	MBVHNode* mbvhNode = 0;			// BVH node for M-wide BVH.
	BVH bvh;						// MBVH<M> is created from BVH and uses its data.
	bool ownBVH = true;				// False when ConvertFrom receives an external bvh.
};

class BVH4_GPU : public BVHBase
{
public:
	struct BVHNode // actual struct is unused; left here to show structure of data in bvh4Data.
	{
		// 4-way BVH node, optimized for GPU rendering
		struct aabb8 { uint8_t xmin, ymin, zmin, xmax, ymax, zmax; }; // quantized
		bvhvec3 aabbMin; uint32_t c0Info;			// 16
		bvhvec3 aabbExt; uint32_t c1Info;			// 16
		aabb8 c0bounds, c1bounds; uint32_t c2Info;	// 16
		aabb8 c2bounds, c3bounds; uint32_t c3Info;	// 16; total: 64 bytes
		// childInfo, 32bit:
		// msb:        0=interior, 1=leaf
		// leaf:       16 bits: relative start of triangle data, 15 bits: triangle count.
		// interior:   31 bits: child node address, in float4s from BVH data start.
		// Triangle data: directly follows nodes with leaves. Per tri:
		// - bvhvec4 vert0, vert1, vert2
		// - uint vert0.w stores original triangle index.
		// We can make the node smaller by storing child nodes sequentially, but
		// there is no way we can shave off a full 16 bytes, unless aabbExt is stored
		// as chars as well, as in CWBVH.
	};
	BVH4_GPU( BVHContext ctx = {} ) { layout = LAYOUT_BVH4_GPU; context = ctx; }
	BVH4_GPU( const MBVH<4>& bvh4 ) { /* DEPRECATED */ layout = LAYOUT_BVH4_GPU; ConvertFrom( bvh4 ); }
	~BVH4_GPU();
	void Build( const bvhvec4* vertices, const uint32_t primCount );
	void Build( const bvhvec4slice& vertices );
	void Build( const bvhvec4* vertices, const uint32_t* indices, const uint32_t primCount );
	void Build( const bvhvec4slice& vertices, const uint32_t* indices, const uint32_t primCount );
	void BuildHQ( const bvhvec4* vertices, const uint32_t primCount );
	void BuildHQ( const bvhvec4slice& vertices );
	void BuildHQ( const bvhvec4* vertices, const uint32_t* indices, const uint32_t primCount );
	void BuildHQ( const bvhvec4slice& vertices, const uint32_t* indices, const uint32_t primCount );
	void Optimize( const uint32_t iterations = 25, bool extreme = false );
	void ConvertFrom( const MBVH<4>& original, bool compact = true );
	float SAHCost( const uint32_t nodeIdx = 0 ) const { return bvh4.SAHCost( nodeIdx ); }
	int32_t Intersect( Ray& ray ) const;
	bool IsOccluded( const Ray& ray ) const { FALLBACK_SHADOW_QUERY( ray ); }
	// BVH data
	bvhvec4* bvh4Data = 0;			// 64-byte 4-wide BVH node for efficient GPU rendering.
	uint32_t allocatedBlocks = 0;	// node data and triangles are stored in 16-byte blocks.
	uint32_t usedBlocks = 0;		// actually used storage.
	MBVH<4> bvh4;					// BVH4_GPU is created from BVH4 and uses its data.
	bool ownBVH4 = true;			// False when ConvertFrom receives an external bvh.
};

class BVH4_CPU : public BVHBase
{
public:
	enum { EMPTY_BIT = 1 << 31, LEAF_BIT = 1 << 30 };
	struct BVHNode
	{
		// 4-way BVH node, optimized for CPU rendering.
		// SSE version.
		SIMDVEC4 xmin4, xmax4;
		SIMDVEC4 ymin4, ymax4;
		SIMDVEC4 zmin4, zmax4;
		SIMDIVEC4 child4, perm4; // total 128 bytes.
	};
	struct CacheLine { SIMDVEC4 a, b, c, d; };
	BVH4_CPU( BVHContext ctx = {} ) { layout = LAYOUT_BVH4_CPU; context = ctx; c_int = 2; l_quads = true; }
	~BVH4_CPU();
	void Save( const char* fileName );
	bool Load( const char* fileName, const uint32_t expectedTris );
	void Build( const bvhvec4* vertices, const uint32_t primCount );
	void Build( const bvhvec4slice& vertices );
	void Build( const bvhvec4* vertices, const uint32_t* indices, const uint32_t prims );
	void Build( const bvhvec4slice& vertices, const uint32_t* indices, uint32_t prims );
	void BuildHQ( const bvhvec4* vertices, const uint32_t primCount );
	void BuildHQ( const bvhvec4slice& vertices );
	void BuildHQ( const bvhvec4* vertices, const uint32_t* indices, const uint32_t prims );
	void BuildHQ( const bvhvec4slice& vertices, const uint32_t* indices, uint32_t prims );
	void Optimize( const uint32_t iterations, bool extreme );
	void Refit();
	float SAHCost( const uint32_t nodeIdx ) const;
	void ConvertFrom( MBVH<4>& original );
	int32_t Intersect( Ray& ray ) const;
	bool IsOccluded( const Ray& ray ) const;
	// Intersect / IsOccluded specialize for ray octant using templated functions.
	template <bool posX, bool posY, bool posZ> int32_t Intersect( Ray& ray ) const;
	template <bool posX, bool posY, bool posZ> bool IsOccluded( const Ray& ray ) const;
	// BVH8 data
	CacheLine* bvh4Data = 0;		// Interleaved interior (128b) and leaf (192b) data.
	MBVH<4> bvh4;					// BVH4_CPU is created from BVH4 and uses its data.
	bool ownBVH4 = true;			// false when ConvertFrom receives an external bvh4.
	uint32_t allocatedBlocks = 0;	// node data and triangles are stored in 16-byte blocks.
	uint32_t usedBlocks = 0;		// the amount of data actually used.
};

class BVH8_CWBVH : public BVHBase
{
public:
	BVH8_CWBVH( BVHContext ctx = {} ) { layout = LAYOUT_CWBVH; context = ctx; }
	BVH8_CWBVH( MBVH<8>& bvh8 ) { /* DEPRECATED */ layout = LAYOUT_CWBVH; ConvertFrom( bvh8 ); }
	~BVH8_CWBVH();
	void Save( const char* fileName );
	bool Load( const char* fileName, const uint32_t expectedTris );
	void Build( const bvhvec4* vertices, const uint32_t primCount );
	void Build( const bvhvec4slice& vertices );
	void Build( const bvhvec4* vertices, const uint32_t* indices, const uint32_t primCount );
	void Build( const bvhvec4slice& vertices, const uint32_t* indices, const uint32_t primCount );
	void BuildHQ( const bvhvec4* vertices, const uint32_t primCount );
	void BuildHQ( const bvhvec4slice& vertices );
	void BuildHQ( const bvhvec4* vertices, const uint32_t* indices, const uint32_t primCount );
	void BuildHQ( const bvhvec4slice& vertices, const uint32_t* indices, const uint32_t primCount );
	void Optimize( const uint32_t iterations = 25, bool extreme = false );
	void ConvertFrom( MBVH<8>& original, bool compact = true );
	float SAHCost( const uint32_t nodeIdx = 0 ) const;
	int32_t Intersect( Ray& ray ) const;
	bool IsOccluded( const Ray& ray ) const { FALLBACK_SHADOW_QUERY( ray ); }
	// BVH8 data
	bvhvec4* bvh8Data = 0;			// nodes in CWBVH format.
	bvhvec4* bvh8Tris = 0;			// triangle data for CWBVH nodes.
	uint32_t allocatedBlocks = 0;	// node data is stored in blocks of 16 byte.
	uint32_t usedBlocks = 0;		// actually used blocks.
	MBVH<8> bvh8;					// BVH8_CWBVH is created from BVH8 and uses its data.
	bool ownBVH8 = true;			// false when ConvertFrom receives an external bvh8.
};

// Storage for up to four triangles, in SoA layout, for BVH8_CPU.
struct BVHTri4Leaf
{
	SIMDVEC4 v0x4, v0y4, v0z4;
	SIMDVEC4 e1x4, e1y4, e1z4;
	SIMDVEC4 e2x4, e2y4, e2z4;
	uint32_t primIdx[4];		// total: 160 bytes.
	SIMDVEC4 dummy0, dummy1;	// pad to 3 full cachelines.
	inline void SetData( const bvhvec3& v0, const bvhvec3& e1, const bvhvec3& e2, const uint32_t pidx, const uint32_t slot )
	{
		((float*)&v0x4)[slot] = v0.x, ((float*)&v0y4)[slot] = v0.y, ((float*)&v0z4)[slot] = v0.z;
		((float*)&e1x4)[slot] = e1.x, ((float*)&e1y4)[slot] = e1.y, ((float*)&e1z4)[slot] = e1.z;
		((float*)&e2x4)[slot] = e2.x, ((float*)&e2y4)[slot] = e2.y, ((float*)&e2z4)[slot] = e2.z, primIdx[slot] = pidx;
	}
};

// Storage for a single triangle, for BVH8_CPU.
struct BVHTri1Leaf
{
	bvhvec3 v0, e1, e2;
	uint32_t primIdx;			// total: 40 bytes
};

class BVH8_CPU : public BVHBase
{
public:
	enum { EMPTY_BIT = 1 << 31, LEAF_BIT = 1 << 30 };
	struct BVHNode
	{
		// 8-way BVH node, optimized for CPU rendering.
		// Based on: "Accelerated Single Ray Tracing for Wide Vector Units", Fuetterling et al., 2017,
		// and the implementation by Mathijs Molenaar, https://github.com/mathijs727/pandora
		SIMDVEC8 xmin8, xmax8;
		SIMDVEC8 ymin8, ymax8;
		SIMDVEC8 zmin8, zmax8;
		SIMDIVEC8 child8; // bits: 31..29 = flags, 28..0: node index.
		SIMDIVEC8 perm8;
		// flag bits: 000 is an empty node, 010 is an interior node. 1xx is leaf; xx = tricount.
	};
	struct BVHNodeCompact
	{
		// Novel 128-byte 8-way BVH node.
		SIMDIVEC8 child8, perm8;									// 64
		float d0, bminx, bminy, bminz, d1, bextx, bexty, bextz;		// 32
		SIMDIVEC4 cbminmaxx8, cbminmaxy8;							// 32, total: 128
	};
	struct CacheLine { SIMDVEC8 a, b; };
	BVH8_CPU( BVHContext ctx = {} ) { layout = LAYOUT_BVH8_AVX2; context = ctx; c_int = 2; l_quads = true; }
	~BVH8_CPU();
	void Save( const char* fileName );
	bool Load( const char* fileName, const uint32_t expectedTris );
	void Build( const bvhvec4* vertices, const uint32_t primCount );
	void Build( const bvhvec4slice& vertices );
	void Build( const bvhvec4* vertices, const uint32_t* indices, const uint32_t prims );
	void Build( const bvhvec4slice& vertices, const uint32_t* indices, uint32_t prims );
	void BuildHQ( const bvhvec4* vertices, const uint32_t primCount );
	void BuildHQ( const bvhvec4slice& vertices );
	void BuildHQ( const bvhvec4* vertices, const uint32_t* indices, const uint32_t prims );
	void BuildHQ( const bvhvec4slice& vertices, const uint32_t* indices, uint32_t prims );
	void Optimize( const uint32_t iterations, bool extreme );
	void Refit();
	float SAHCost( const uint32_t nodeIdx ) const;
	void ConvertFrom( MBVH<8>& original );
	int32_t Intersect( Ray& ray ) const;
	bool IsOccluded( const Ray& ray ) const;
	// Intersect / IsOccluded specialize for ray octant using templated functions.
	template <bool posX, bool posY, bool posZ> int32_t Intersect( Ray& ray ) const;
	template <bool posX, bool posY, bool posZ> bool IsOccluded( const Ray& ray ) const;
	// BVH8 data
	CacheLine* bvh8Data = 0;		// Interleaved interior (256b) and leaf (192b) data.
	MBVH<8> bvh8;					// BVH8_CPU is created from BVH8 and uses its data.
	bool ownBVH8 = true;			// false when ConvertFrom receives an external bvh8.
	uint32_t allocatedBlocks = 0;	// node data and triangles are stored in 16-byte blocks.
	uint32_t usedBlocks = 0;		// the amount of data actually used.
};

// BLASInstance: A TLAS is built over BLAS instances, where a single BLAS can be
// used with multiple transforms, and multiple BLASses can be combined in a complex
// scene. The TLAS is built over the world-space AABBs of the BLAS root nodes.
class ALIGNED( 64 ) BLASInstance
{
public:
	BLASInstance() = default;
	BLASInstance( uint32_t idx ) : blasIdx( idx ) {}
	float transform[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }; // identity
	float invTransform[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }; // identity
	bvhvec3 aabbMin = bvhvec3( BVH_FAR );
	uint32_t blasIdx = 0;
	bvhvec3 aabbMax = bvhvec3( -BVH_FAR );
	uint32_t mask = (uint32_t)RAY_MASK_INTERSECT_ALL; // set to intersect with all rays by default
	uint32_t dummy[8]; // pad struct to 64 byte
	void Update( BVHBase * blas );
	void InvertTransform();
};

#ifdef DOUBLE_PRECISION_SUPPORT

// BLASInstanceEx: Double-precision version of BLASInstance.
class BLASInstanceEx
{
public:
	BLASInstanceEx() = default;
	BLASInstanceEx( uint64_t idx ) : blasIdx( idx ) {}
	double transform[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }; // identity
	double invTransform[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }; // identity
	bvhdbl3 aabbMin = bvhdbl3( BVH_DBL_FAR );
	uint64_t blasIdx = 0;
	bvhdbl3 aabbMax = bvhdbl3( -BVH_DBL_FAR );
	uint64_t mask = RAY_MASK_INTERSECT_ALL; // set to intersect with all rays by default
	void Update( BVH_Double* blas );
	void InvertTransform();
};

#endif

} // namespace tinybvh

#endif // TINY_BVH_H_

// ============================================================================
//
//        I M P L E M E N T A T I O N
//
// ============================================================================

