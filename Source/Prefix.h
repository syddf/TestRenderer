#pragma once
#include <glm/glm.hpp>
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <fstream>
#include <memory>
#include <map>
#include <set>

typedef glm::vec4 Vec4;
typedef glm::vec3 Vec3;
typedef glm::vec3 Point;
typedef glm::vec2 Vec2;
typedef glm::mat4 Matrix;

typedef unsigned int UInt32;
typedef unsigned char UInt8;
typedef unsigned short UInt16;

typedef int SInt32;
typedef short SInt16;
typedef char SInt8;

#define EPSILON 0.0000001f
#define MAXF 1e10f