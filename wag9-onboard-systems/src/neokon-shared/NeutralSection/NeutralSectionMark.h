#pragma once

#include "NeutralSectionMarkType.h"

#include "glm/glm.hpp"

#include <string>

/**
 * Neutral electrification section located between two marks of a "Zone" type on the path.
 * The marks can also has "Warning" type, such marks are placed before the marks of the "Zone" type
 * and warn the train about approaching the neutral section.
 * Axes Y of the marks should be directed inside the neutral zone.
 */
class NeutralSectionMark
{
public:
    /**
     * @p id is an ID of the mark in the layer file.
     * @p rootName is the name of the mark group node in layer file.
     * @p type is a mark type @see NeutralSectionMarkType.
     * @p name is the name of the mark in layer file.
     * @p transform is the mark node transform.
     * @p isActive indicates if mark should be handled by logic.
     */
    NeutralSectionMark(int id, std::string const& rootName, NeutralSectionMarkType type,
        std::string const& name, glm::dmat4 const& transform, bool isActive);

    void setActive(bool active);

    glm::dmat4 const& worldTransform() const;
    glm::dvec3 worldPosition() const;

    /**
     * The name of the mark in layer file.
     */
    std::string const& name() const;

    /**
     * The id of the mark in layer file.
     */
    int id() const;

    /**
     * @see m_rootName.
     */
    const std::string& rootName() const;

    NeutralSectionMarkType type() const;

    bool isActive() const;

private:
    const glm::dmat4 m_transform;

    /**
     * The name of the mark in layer file.
     */
    const std::string m_name;

    /**
     * The name of the mark group node in layer file.
     */
    const std::string m_rootName;
    const NeutralSectionMarkType m_type;

    /**
     * The id of the mark in layer file.
     */
    const int m_id;

    /**
     * If mark is inactive it will be ignored by processing logic.
     */
    bool m_active = true;
};
