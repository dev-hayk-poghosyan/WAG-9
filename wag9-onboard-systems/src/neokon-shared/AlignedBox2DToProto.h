#pragma once

#include "AlignedBox.h"
#include "AlignedBox2D.pb.h"

proto::AlignedBox2D alignedBox2DToProto(AlignedBox2D const& box);
