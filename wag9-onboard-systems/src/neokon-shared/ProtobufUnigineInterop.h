#pragma once

#include "Mat4d.pb.h"
#include "Mat4f.pb.h"
#include "Vec2f.pb.h"
#include "Vec3f.pb.h"
#include "Vec3d.pb.h"
#include "Vec4d.pb.h"
#include "Vec4f.pb.h"

#include <UnigineMathLib.h>

proto::Vec2f unigineToProto(Unigine::Math::vec2 const& v);

Unigine::Math::vec2 protoToUnigine(proto::Vec2f const& v);

proto::Vec3f unigineToProto(Unigine::Math::vec3 const& v);

Unigine::Math::vec3 protoToUnigine(proto::Vec3f const& v);

proto::Vec3d unigineToProto(Unigine::Math::dvec3 const& v);

Unigine::Math::dvec3 protoToUnigine(proto::Vec3d const& v);

proto::Vec4f unigineToProto(Unigine::Math::vec4 const& v);

Unigine::Math::vec4 protoToUnigine(proto::Vec4f const& v);

proto::Vec4d unigineToProto(Unigine::Math::dvec4 const& v);

Unigine::Math::dvec4 protoToUnigine(proto::Vec4d const& v);

proto::Mat4f unigineToProto(Unigine::Math::mat4 const& m);

Unigine::Math::mat4 protoToUnigine(proto::Mat4f const& m);

proto::Mat4d unigineToProto(Unigine::Math::dmat4 const& m);

Unigine::Math::dmat4 protoToUnigine(proto::Mat4d const& m);

Unigine::Math::quat protoQuatToUnigine(proto::Vec4f const& v);

proto::Vec4f unigineQuatToProto(Unigine::Math::quat const& q);
