#pragma once

#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_LEFT_HANDED

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/matrix_major_storage.hpp>

typedef glm::ivec2 ivec2;
typedef glm::dvec2 dvec2;
typedef glm::vec2 vec2;

typedef glm::ivec3 ivec3;
typedef glm::dvec3 dvec3;
typedef glm::vec3 vec3;

typedef glm::ivec4 ivec4;
typedef glm::dvec4 dvec4;
typedef glm::vec4 vec4;

typedef glm::dquat dquat;
typedef glm::quat quat;

typedef glm::dmat2 dmat2;
typedef glm::mat2 mat2;

typedef glm::dmat3 dmat3;
typedef glm::mat3 mat3;

typedef glm::dmat4 dmat4;
typedef glm::mat4 mat4;