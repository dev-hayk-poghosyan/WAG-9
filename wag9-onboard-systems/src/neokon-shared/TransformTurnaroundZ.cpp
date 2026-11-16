#include "TransformTurnaroundZ.h"

Unigine::Math::Mat4
transformTurnaroundZ(const Unigine::Math::Mat4& transform)
{
    // The matrix that rotates by 180 degrees around the Z axis is the same
    // as the identity matrix, except its first two columns are negated.

    Unigine::Math::Mat4 newTransform = transform;

    newTransform.setColumn(0, -newTransform.getColumn(0));
    newTransform.setColumn(1, -newTransform.getColumn(1));

    return newTransform;
}
