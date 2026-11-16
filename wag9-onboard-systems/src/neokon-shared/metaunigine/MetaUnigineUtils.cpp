#include "MetaUnigineUtils.h"

using namespace Unigine;

namespace MetaUnigine
{

Math::Mat4 xmlToMat4(const XmlPtr& xmlTransform)
{
    Math::Mat4 transform;

    double number[16];
    if (xmlTransform && xmlTransform->getDoubleArrayData(number, 16))
    {
        transform.m00 = number[0];
        transform.m01 = number[4];
        transform.m02 = number[8];
        transform.m03 = number[12];
        transform.m10 = number[1];
        transform.m11 = number[5];
        transform.m12 = number[9];
        transform.m13 = number[13];
        transform.m20 = number[2];
        transform.m21 = number[6];
        transform.m22 = number[10];
        transform.m23 = number[14];
    }

    return transform;
}

}//namespace MetaUnigine
