#include "ProtobufGlmInterop.h"

proto::Vec3d glmToProto(glm::dvec3 const& v)
{
    proto::Vec3d out;
    
    out.set_x(v.x);
    out.set_y(v.y);
    out.set_z(v.z);

    return out;
}

glm::dvec3 protoToGlm(proto::Vec3d const& v)
{
    return glm::dvec3(v.x(), v.y(), v.z());
}
