#include "TransformFromPositionAndAxis.h"

using namespace Unigine;

Math::Mat4 transformFromPositionAndTangent(Math::Vec3 const& position,
    Math::Vec3 const& tangent)
{
    Math::Vec3 axisX = tangent;
    axisX.z = 0.0;
    axisX.normalize();

    const Math::Vec3 axisZ = Math::Vec3_up;
    const Math::Vec3 axisY = Math::cross(axisZ, axisX);

    return Math::Mat4(axisX, axisY, axisZ, position);
}

Math::Mat4 transformFromPositionAndNormal(Math::Vec3 const& position,
    Math::Vec3 const& normal)
{
    Math::Vec3 axisY = normal;
    axisY.z = 0.0;
    axisY.normalize();

    Math::Vec3 axisZ = Math::Vec3_up;
    Math::Vec3 axisX = Math::cross(axisY, axisZ);

    return Math::Mat4(axisX, axisY, axisZ, position);
}

Math::Mat4
transformFromTranslateAndDirectionDirYUpZ(const Math::Vec3& translate,
    const Math::Vec3& direction)
{
    Math::Vec3 unitDir(direction);
    unitDir.normalize();

    // Think Gram-Schmidt orthogonalization.
    Math::Vec3 finalUp = Math::Vec3_up - unitDir * Math::dot(unitDir, Math::Vec3_up);
    finalUp.normalize();

    Math::Vec3 unitRight = Math::cross(unitDir, finalUp);
    return Unigine::Math::Mat4(unitRight, unitDir, finalUp, translate);
}

Math::Mat4
transformFromTranslateAndDirectionDirXUpZ(const Math::Vec3& translate,
    const Math::Vec3& direction)
{
    Math::Vec3 unitDir(direction);
    unitDir.normalize();

    // Think Gram-Schmidt orthogonalization.
    Math::Vec3 finalUp = Math::Vec3_up - unitDir * Math::dot(unitDir, Math::Vec3_up);
    finalUp.normalize();

    Math::Vec3 unitRight = Math::cross(unitDir, finalUp);
    return Math::Mat4(unitDir, -unitRight, finalUp, translate);
}
