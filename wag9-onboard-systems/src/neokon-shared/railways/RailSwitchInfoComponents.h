#pragma once

#include <UnigineComponentSystem.h>
#include <UnigineNodes.h>

class RailSwitchPointRail : public Unigine::ComponentBase
{
public:
    COMPONENT(RailSwitchPointRail, Unigine::ComponentBase);
    PROP_NAME("RailSwitchPointRail");

    /**
     * Угол в градусах на который надо повернуть вокруг оси Z
     * остряк, при переключении стрелочного перевода влево.
     */
    PROP_PARAM(Float, TurnToLeftDeg, 0.5f)
};

class RailSwitchRod : public Unigine::ComponentBase
{
public:
    COMPONENT(RailSwitchRod, Unigine::ComponentBase);
    PROP_NAME("RailSwitchRod");

    /**
     * Смешение в метрах на которое надо сместить
     * вдоль оси Y стержень тяги, при переключении
     * стрелочного перевода влево.
     */
    PROP_PARAM(Float, YOffsetToLeftM, -0.08f)
};

class RailSwitchPartTransforms : public Unigine::ComponentBase
{
public:
    COMPONENT(RailSwitchPartTransforms, Unigine::ComponentBase);
    PROP_NAME("RailSwitchPartTransforms");

    void setLeftTransform(const Unigine::Math::Mat4& transform);
    void setRightTransform(const Unigine::Math::Mat4& transform);

    Unigine::Math::Mat4 getLeftTransform();
    Unigine::Math::Mat4 getRightTransform();

    /**
     * Tranformation of railway junction part, intended to select left way of junction
     */
    PROP_ARRAY(Double, LeftTransform)

    /**
     * Tranformation of railway junction part, intended to select right way of junction
     */
    PROP_ARRAY(Double, RightTransform)
};

void railSwitchToRight(Unigine::NodePtr node, const char* suffix = "_Anim");
void railSwitchToLeft(Unigine::NodePtr node, const char* suffix = "_Anim");
