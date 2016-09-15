#pragma once

// std::vector macros

#define VecBuffer(vec)				(vec.size() > 0 ? &vec[0] : nullptr)
#define VecBufferFrom(vec, place)	(vec.size() > place ? &vec[place] : nullptr)
#define VecElementSize(vec)			(vec.size() > 0 ? sizeof(vec[0]) : 0)
#define VecBufferSize(vec)			(vec.size() > 0 ? vec.size()*sizeof(vec[0]) : 0)
#define VecDuplicateLeftHalf(vec)	{if (vec.size() > 0) { memcpy(&vec[vec.size()/2], &vec[0], (vec.size()/2)*sizeof(vec[0])); } else {}}

// ZeroMemory() for structs

#define ZeroStruct(structure) (ZeroMemory(&structure, sizeof(structure)))