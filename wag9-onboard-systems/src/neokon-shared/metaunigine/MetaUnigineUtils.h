#pragma once

#include <UnigineXml.h>
#include <UnigineString.h>

namespace MetaUnigine
{

#define LOAD_CHILD_IF_EXISTS_INT(xml, param)                                                       \
    {                                                                                              \
        int paramIdx = xml->findChild(#param);                                                     \
        if (paramIdx != -1)                                                                        \
        {                                                                                          \
            m_##param = xml->getChild(paramIdx)->getIntData();                                     \
        }                                                                                          \
    }

#define LOAD_CHILD_IF_EXISTS_FLOAT(xml, param)                                                     \
    {                                                                                              \
        int paramIdx = xml->findChild(#param);                                                     \
        if (paramIdx != -1)                                                                        \
        {                                                                                          \
            m_##param = xml->getChild(paramIdx)->getFloatData();                                   \
        }                                                                                          \
    }

#define SAVE_CHILD_INT(xml, param)                                                                 \
    xml->addChild(Unigine::Xml::create(#param))->setIntData(m_##param);

#define SAVE_CHILD_FLOAT(xml, param)                                                               \
    xml->addChild(Unigine::Xml::create(#param))->setFloatData(m_##param);

Unigine::Math::Mat4 xmlToMat4(const Unigine::XmlPtr& xmlTransform);

template<typename MatType>
Unigine::String mat4ToString(const MatType& mat)
{
    Unigine::String transform;

    int precision = 12;
    int use_inf = 0;

    transform += Unigine::String::dtoa(mat.getColumn3(0).get()[0], precision, use_inf) + " ";
    transform += Unigine::String::dtoa(mat.getColumn3(0).get()[1], precision, use_inf) + " ";
    transform += Unigine::String::dtoa(mat.getColumn3(0).get()[2], precision, use_inf) + " ";
    if constexpr (std::is_same_v<MatType, Unigine::Math::Mat4>)
    {
        transform += "0.0 ";
    }
    else
    {
        transform += "0 ";
    }

    transform += Unigine::String::dtoa(mat.getColumn3(1).get()[0], precision, use_inf) + " ";
    transform += Unigine::String::dtoa(mat.getColumn3(1).get()[1], precision, use_inf) + " ";
    transform += Unigine::String::dtoa(mat.getColumn3(1).get()[2], precision, use_inf) + " ";
    if constexpr (std::is_same_v<MatType, Unigine::Math::Mat4>)
    {
        transform += "0.0 ";
    }
    else
    {
        transform += "0 ";
    }

    transform += Unigine::String::dtoa(mat.getColumn3(2).get()[0], precision, use_inf) + " ";
    transform += Unigine::String::dtoa(mat.getColumn3(2).get()[1], precision, use_inf) + " ";
    transform += Unigine::String::dtoa(mat.getColumn3(2).get()[2], precision, use_inf) + " ";
    if constexpr (std::is_same_v<MatType, Unigine::Math::Mat4>)
    {
        transform += "0.0 ";
    }
    else
    {
        transform += "0 ";
    }

    transform += Unigine::String::dtoa(mat.getTranslate().get()[0], precision, use_inf) + " ";
    transform += Unigine::String::dtoa(mat.getTranslate().get()[1], precision, use_inf) + " ";
    transform += Unigine::String::dtoa(mat.getTranslate().get()[2], precision, use_inf) + " ";
    if constexpr (std::is_same_v<MatType, Unigine::Math::Mat4>)
    {
        transform += "1.0";
    }
    else
    {
        transform += "1";
    }

    return transform;
}

}//namespace MetaUnigine
