#include "NeutralSectionMark.h"

NeutralSectionMark::NeutralSectionMark(int id, std::string const& rootName,
    NeutralSectionMarkType type, std::string const &name, glm::dmat4 const &transform, bool isActive)
    : m_transform(transform)
    , m_name(name)
    , m_type(type)
    , m_id(id)
    , m_rootName(rootName)
    , m_active(isActive)
{
}

void NeutralSectionMark::setActive(bool active)
{
    m_active = active;
}

glm::dmat4 const&
NeutralSectionMark::worldTransform() const
{
    return m_transform;
}

glm::dvec3
NeutralSectionMark::worldPosition() const
{
    return m_transform[3];
}

std::string const&
NeutralSectionMark::name() const
{
    return m_name;
}

int
NeutralSectionMark::id() const
{
    return m_id;
}

const std::string&
NeutralSectionMark::rootName() const
{
    return m_rootName;
}

NeutralSectionMarkType
NeutralSectionMark::type() const
{
    return m_type;
}

bool NeutralSectionMark::isActive() const
{
    return m_active;
}
