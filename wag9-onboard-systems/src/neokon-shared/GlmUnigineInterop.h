#pragma once

#include <UnigineMathLib.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#define DECLARE_GLM_UNIGINE_VEC2_CONV(retType, fn, argType) \
inline retType fn(argType const& v) { return retType(v.x, v.y); }

#define DECLARE_GLM_UNIGINE_VEC3_CONV(retType, fn, argType) \
inline retType fn(argType const& v) { return retType(v.x, v.y, v.z); }

#define DECLARE_GLM_UNIGINE_VEC4_CONV(retType, fn, argType) \
inline retType fn(argType const& v) { return retType(v.x, v.y, v.z, v.w); }

DECLARE_GLM_UNIGINE_VEC2_CONV(glm::vec2, toGlm, Unigine::Math::vec2)
DECLARE_GLM_UNIGINE_VEC2_CONV(Unigine::Math::vec2, fromGlm, glm::vec2)
DECLARE_GLM_UNIGINE_VEC2_CONV(glm::dvec2, toGlm, Unigine::Math::dvec2)
DECLARE_GLM_UNIGINE_VEC2_CONV(Unigine::Math::dvec2, fromGlm, glm::dvec2)

DECLARE_GLM_UNIGINE_VEC3_CONV(glm::vec3, toGlm, Unigine::Math::vec3)
DECLARE_GLM_UNIGINE_VEC3_CONV(Unigine::Math::vec3, fromGlm, glm::vec3)
DECLARE_GLM_UNIGINE_VEC3_CONV(glm::dvec3, toGlm, Unigine::Math::dvec3)
DECLARE_GLM_UNIGINE_VEC3_CONV(Unigine::Math::dvec3, fromGlm, glm::dvec3)

DECLARE_GLM_UNIGINE_VEC4_CONV(glm::vec4, toGlm, Unigine::Math::vec4)
DECLARE_GLM_UNIGINE_VEC4_CONV(Unigine::Math::vec4, fromGlm, glm::vec4)
DECLARE_GLM_UNIGINE_VEC4_CONV(glm::dvec4, toGlm, Unigine::Math::dvec4)
DECLARE_GLM_UNIGINE_VEC4_CONV(Unigine::Math::dvec4, fromGlm, glm::dvec4)


#define DECLARE_TO_GLM_MAT2_CONV(retType, argType) \
inline retType toGlm(argType const& m) \
{ return retType(toGlm(m.getColumn(0)), toGlm(m.getColumn(1))); }

#define DECLARE_TO_GLM_MAT3_CONV(retType, argType) \
inline retType toGlm(argType const& m) \
{ return retType(toGlm(m.getColumn(0)), toGlm(m.getColumn(1)), toGlm(m.getColumn(2))); }

#define DECLARE_TO_GLM_MAT4_CONV(retType, argType) \
inline retType toGlm(argType const& m) \
{ return retType(toGlm(m.getColumn(0)), toGlm(m.getColumn(1)), \
    toGlm(m.getColumn(2)), toGlm(m.getColumn(3))); }

DECLARE_TO_GLM_MAT2_CONV(glm::mat2, Unigine::Math::mat2)
//DECLARE_TO_GLM_MAT2_CONV(glm::dmat2, Unigine::Math::dmat2) // Unigine doesn't have dmat2.

DECLARE_TO_GLM_MAT3_CONV(glm::mat3, Unigine::Math::mat3)
//DECLARE_TO_GLM_MAT3_CONV(glm::dmat3, Unigine::Math::dmat3) // Unigine doesn't have dmat3.

DECLARE_TO_GLM_MAT4_CONV(glm::mat4, Unigine::Math::mat4)
DECLARE_TO_GLM_MAT4_CONV(glm::dmat4, Unigine::Math::dmat4)


#define DECLARE_FROM_GLM_MAT2_CONV(retType, argType) \
inline retType fromGlm(argType const& m) \
{ return retType(fromGlm(m[0]), fromGlm(m[1])); }

#define DECLARE_FROM_GLM_MAT3_CONV(retType, argType) \
inline retType fromGlm(argType const& m) \
{ return retType(fromGlm(m[0]), fromGlm(m[1]), fromGlm(m[2])); }

#define DECLARE_FROM_GLM_MAT4_CONV(retType, argType) \
inline retType fromGlm(argType const& m) \
{ return retType(fromGlm(m[0]), fromGlm(m[1]), fromGlm(m[2]), fromGlm(m[3])); }

DECLARE_FROM_GLM_MAT2_CONV(Unigine::Math::mat2, glm::mat2)
//DECLARE_FROM_GLM_MAT2_CONV(Unigine::Math::dmat2, glm::dmat2) // Unigine doesn't have dmat2.

DECLARE_FROM_GLM_MAT3_CONV(Unigine::Math::mat3, glm::mat3)
//DECLARE_FROM_GLM_MAT3_CONV(Unigine::Math::dmat3, glm::dmat3) // Unigine doesn't have dmat3.

DECLARE_FROM_GLM_MAT4_CONV(Unigine::Math::mat4, glm::mat4)

inline Unigine::Math::dmat4 fromGlm(glm::dmat4 const& m)
{
    // Unfortunately there is no Unigine::Math::dmat4 constructor taking 4 dvec4's,
    // otherwise we could have used the macro above.
    Unigine::Math::dmat4 ret;

    ret.setColumn(0, fromGlm(m[0]));
    ret.setColumn(1, fromGlm(m[1]));
    ret.setColumn(2, fromGlm(m[2]));
    ret.setColumn(3, fromGlm(m[3]));

    return ret;
}


inline glm::quat toGlm(Unigine::Math::quat const& q)
{
    return glm::quat(q.w, q.x, q.y, q.z);
}

inline Unigine::Math::quat fromGlm(glm::quat const& q)
{
    return Unigine::Math::quat(Unigine::Math::vec4(q.x, q.y, q.z, q.w));
}
