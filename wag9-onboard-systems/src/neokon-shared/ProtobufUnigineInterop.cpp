#include "ProtobufUnigineInterop.h"

proto::Vec2f unigineToProto(Unigine::Math::vec2 const& v)
{
    proto::Vec2f out;

    out.set_x(v.x);
    out.set_y(v.y);

    return out;
}

Unigine::Math::vec2 protoToUnigine(proto::Vec2f const& v)
{
    return Unigine::Math::vec2(v.x(), v.y());
}

proto::Vec3f unigineToProto(Unigine::Math::vec3 const& v)
{
    proto::Vec3f out;
    
    out.set_x(v.x);
    out.set_y(v.y);
    out.set_z(v.z);

    return out;
}

Unigine::Math::vec3 protoToUnigine(proto::Vec3f const& v)
{
    return Unigine::Math::vec3(v.x(), v.y(), v.z());
}

proto::Vec3d unigineToProto(Unigine::Math::dvec3 const& v)
{
    proto::Vec3d out;

    out.set_x(v.x);
    out.set_y(v.y);
    out.set_z(v.z);

    return out;
}

Unigine::Math::dvec3 protoToUnigine(proto::Vec3d const& v)
{
    return Unigine::Math::dvec3(v.x(), v.y(), v.z());
}

proto::Vec4f unigineToProto(Unigine::Math::vec4 const& v)
{
    proto::Vec4f out;

    out.set_x(v.x);
    out.set_y(v.y);
    out.set_z(v.z);
    out.set_w(v.w);

    return out;
}

Unigine::Math::vec4 protoToUnigine(proto::Vec4f const& v)
{
    return Unigine::Math::vec4(v.x(), v.y(), v.z(), v.w());
}

proto::Vec4d unigineToProto(Unigine::Math::dvec4 const& v)
{
    proto::Vec4d out;

    out.set_x(v.x);
    out.set_y(v.y);
    out.set_z(v.z);
    out.set_w(v.w);

    return out;
}

Unigine::Math::dvec4 protoToUnigine(proto::Vec4d const& v)
{
    return Unigine::Math::dvec4(v.x(), v.y(), v.z(), v.w());
}

proto::Mat4f unigineToProto(Unigine::Math::mat4 const& m)
{
    proto::Mat4f out;

    *out.mutable_col0() = unigineToProto(m.getColumn(0));
    *out.mutable_col1() = unigineToProto(m.getColumn(1));
    *out.mutable_col2() = unigineToProto(m.getColumn(2));
    *out.mutable_col3() = unigineToProto(m.getColumn(3));

    return out;
}

Unigine::Math::mat4 protoToUnigine(proto::Mat4f const& m)
{
    return Unigine::Math::mat4(
        protoToUnigine(m.col0()),
        protoToUnigine(m.col1()),
        protoToUnigine(m.col2()),
        protoToUnigine(m.col3())
    );
}

proto::Mat4d unigineToProto(Unigine::Math::dmat4 const& m)
{
    proto::Mat4d out;

    *out.mutable_col0() = unigineToProto(m.getColumn(0));
    *out.mutable_col1() = unigineToProto(m.getColumn(1));
    *out.mutable_col2() = unigineToProto(m.getColumn(2));
    *out.mutable_col3() = unigineToProto(m.getColumn(3));

    return out;
}

Unigine::Math::dmat4 protoToUnigine(proto::Mat4d const& m)
{
    // Unigine::Math::dmat4 doesn't have a constructor taking column vectors.
    Unigine::Math::dmat4 out;

    out.setColumn(0, protoToUnigine(m.col0()));
    out.setColumn(1, protoToUnigine(m.col1()));
    out.setColumn(2, protoToUnigine(m.col2()));
    out.setColumn(3, protoToUnigine(m.col3()));
    
    return out;
}

Unigine::Math::quat protoQuatToUnigine(proto::Vec4f const& v)
{
    return Unigine::Math::quat(
        v.x(),
        v.y(),
        v.z(),
        v.w(),
        Unigine::Math::quat::RawValuesTag{});
}

proto::Vec4f unigineQuatToProto(Unigine::Math::quat const& q)
{
    proto::Vec4f out;
    out.set_x(q.x);
    out.set_y(q.y);
    out.set_z(q.z);
    out.set_w(q.w);
    return out;
}
