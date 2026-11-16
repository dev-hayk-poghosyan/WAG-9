#include "AlignedBox2DToProto.h"

#include "neokon-shared/GlmUnigineInterop.h"
#include "neokon-shared/ProtobufUnigineInterop.h"

proto::AlignedBox2D
alignedBox2DToProto(AlignedBox2D const& box)
{
    proto::AlignedBox2D result;
    *result.mutable_max_coords() = unigineToProto(Unigine::Math::vec2(fromGlm(box.maxCoords())));
    *result.mutable_min_coords() = unigineToProto(Unigine::Math::vec2(fromGlm(box.minCoords())));
    return result;
}
