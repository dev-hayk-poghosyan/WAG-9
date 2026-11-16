#include "AlignedBox2DFromProto.h"

AlignedBox2D
alignedBox2DFromProto(proto::AlignedBox2D const& protoBox)
{
    return AlignedBox<glm::dvec2>{
        glm::dvec2(protoBox.min_coords().x(), protoBox.min_coords().y()),
        glm::dvec2(protoBox.max_coords().x(), protoBox.max_coords().y())};
}
