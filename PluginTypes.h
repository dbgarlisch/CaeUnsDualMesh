/****************************************************************************
 *
 * class PluginTypes
 *
 * Proprietary software product of Pointwise, Inc.
 * Copyright (c) 1995-2013 Pointwise, Inc.
 * All rights reserved.
 *
 ***************************************************************************/

#ifndef _PLUGINTYPES_H_
#define _PLUGINTYPES_H_

#include <cassert>
#include <map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "cml.h"

#if defined(WINDOWS) && _MSC_VER < 1600
#   define STDTR1 std::tr1
#else
#   define STDTR1 std
#endif

typedef cml::vector3d                               Vec3;
typedef cml::vector<PWP_UINT32, cml::fixed<2> >     Edge;

typedef std::vector<PWP_UINT32>                     UInt32Array1;
typedef std::vector<UInt32Array1>                   UInt32Array2;
typedef std::vector<Edge>                           EdgeArray1;
typedef std::map<Edge, PWP_UINT32>                  EdgeToUInt32Map;
typedef std::map<PWP_UINT32, PWP_UINT32>            UInt32ToUInt32Map;
typedef STDTR1::unordered_set<PWP_UINT32>           UInt32Set;
typedef std::multimap<PWP_UINT32, PWP_UINT32>       UInt32UInt32Array1MMap;


#define fail(str)   assert(0 == intptr_t(str))

//MSVC++ 12.0 _MSC_VER == 1800 (Visual Studio 2013)
//MSVC++ 11.0 _MSC_VER == 1700 (Visual Studio 2012)
//MSVC++ 10.0 _MSC_VER == 1600 (Visual Studio 2010)
//MSVC++ 9.0  _MSC_VER == 1500 (Visual Studio 2008)
//MSVC++ 8.0  _MSC_VER == 1400 (Visual Studio 2005)
//MSVC++ 7.1  _MSC_VER == 1310 (Visual Studio 2003)
//MSVC++ 7.0  _MSC_VER == 1300
//MSVC++ 6.0  _MSC_VER == 1200
//MSVC++ 5.0  _MSC_VER == 1100

#endif // _PLUGINTYPES_H_
