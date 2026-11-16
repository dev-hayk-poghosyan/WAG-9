#include "TransformFromTwoPointPairs.h"

using namespace Unigine;

Math::Mat4
transformFromTwoPointPairs(const Math::Vec3& from0, const Math::Vec3& to0, const Math::Vec3& from1,
    const Math::Vec3& to1)
{
    Math::Vec3 fromVec = from1 - from0;
    Math::Vec3 toVec = to1 - to0;

    Math::Mat4 rotSrcToX;
    {
        Math::vec3 dirX(Math::Vec3(fromVec).normalize());
        Math::vec3 dirY = Math::cross(Math::vec3::UP, dirX);
        Math::vec3 dirZ = Math::cross(dirX, dirY);
        rotSrcToX = Math::inverse(Math::Mat4(Math::mat3(dirX, dirY, dirZ)));
    }

    Math::Mat4 rotXToDst;
    {
        Math::vec3 dirX(Math::Vec3(toVec).normalize());
        Math::vec3 dirY = Math::cross(Math::vec3::UP, dirX);
        Math::vec3 dirZ = Math::cross(dirX, dirY);
        rotXToDst = Math::Mat4(Math::mat3(dirX, dirY, dirZ));
    }

    Math::Mat4 scaleXMat = Math::Mat4::IDENTITY;
    scaleXMat.setScale(Math::Vec3(toVec.length() / fromVec.length(), 1.0, 1.0));

    Math::Mat4 rotScaleMat = rotSrcToX * scaleXMat * rotXToDst;

    Math::Mat4 translateMat = Math::Mat4::IDENTITY;
    translateMat.setTranslate(to0 - rotScaleMat * from0);

    return translateMat * rotScaleMat;
}
