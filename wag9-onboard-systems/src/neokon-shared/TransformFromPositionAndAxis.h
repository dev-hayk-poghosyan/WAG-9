#include <UnigineMathLib.h>

/**
 * Deprecated (legacy). This function will keep the Up, changing the tangent.
 */
Unigine::Math::Mat4 transformFromPositionAndTangent(Unigine::Math::Vec3 const& position,
    Unigine::Math::Vec3 const& tangent);

/**
 * Deprecated (legacy). This function will keep the Up, changing the normal.
 */
Unigine::Math::Mat4 transformFromPositionAndNormal(Unigine::Math::Vec3 const& position,
    Unigine::Math::Vec3 const& normal);

/**
 * This function will keep the direction, changing Up.
 */
Unigine::Math::Mat4 transformFromTranslateAndDirectionDirYUpZ(
    const Unigine::Math::Vec3& translate, const Unigine::Math::Vec3& direction);

/**
 * This function will keep the direction, changing Up.
 */
Unigine::Math::Mat4 transformFromTranslateAndDirectionDirXUpZ(
    const Unigine::Math::Vec3& translate, const Unigine::Math::Vec3& direction);
