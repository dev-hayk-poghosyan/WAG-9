#pragma once

#include "AlignedBox.h"
#include "AlignedBox2D.pb.h"

AlignedBox2D alignedBox2DFromProto(proto::AlignedBox2D const& protoBox);
