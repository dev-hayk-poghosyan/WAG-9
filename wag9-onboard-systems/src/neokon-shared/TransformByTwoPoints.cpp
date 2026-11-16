#include "TransformByTwoPoints.h"

Unigine::Math::Mat4
transformByTwoPoints(Unigine::Math::Vec3 const& point0,
    Unigine::Math::Vec3 const& point1)
{
    using namespace Unigine;

    Math::Vec3 direction = point1 - point0;
    Math::Vec3 directionFlat = direction;
    directionFlat.z = 0;
    direction.normalize();
    directionFlat.normalize();

    Math::Vec3 upVector = Math::Vec3_up;
    Math::Vec3 binormal = Math::cross(directionFlat, upVector);
    upVector = Math::cross(binormal, direction);

    upVector.normalize();
    binormal.normalize();

    Math::Mat4 basis = Math::Mat4::IDENTITY;
    basis.setColumn(0, Math::Vec4(binormal, 0.0));
    basis.setColumn(1, Math::Vec4(direction, 0.0));
    basis.setColumn(2, Math::Vec4(upVector, 0.0));
    basis.setColumn(3, Math::Vec4(point0, 0.0));

    return basis;
}

Unigine::Math::Mat4
transformByThreePoints(Unigine::Math::Vec3 const& point0, Unigine::Math::Vec3 const& point1,
    Unigine::Math::Vec3 const& point2)
{
    using namespace Unigine;

    Math::Vec3 direction1 = point1 - point0;
    Math::Vec3 direction2 = point2 - point0;
    direction1.normalize();
    direction2.normalize();

    Math::Vec3 upVector = Math::cross(direction1, direction2);
    Math::Vec3 binormal = Math::cross(direction1, upVector);

    upVector.normalize();
    binormal.normalize();

    Math::Mat4 basis = Math::Mat4::IDENTITY;
    basis.setColumn(0, Math::Vec4(binormal, 0.0));
    basis.setColumn(1, Math::Vec4(direction1, 0.0));
    basis.setColumn(2, Math::Vec4(upVector, 0.0));
    basis.setColumn(3, Math::Vec4(point0, 0.0));

    return basis;
}
