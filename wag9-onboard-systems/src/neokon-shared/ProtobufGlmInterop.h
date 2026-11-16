#pragma once

#include "Vec3d.pb.h"
#include <glm/glm.hpp>

proto::Vec3d glmToProto(glm::dvec3 const& v);

glm::dvec3 protoToGlm(proto::Vec3d const& v);
