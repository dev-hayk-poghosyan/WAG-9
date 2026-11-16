#pragma once

#include <vector>
#include <boost/uuid/uuid.hpp>

#include "neokon-shared/BezierSpline.h"

/**
 * This class is used by 'DynamicRailways' parts provider to create adjunct parts.
 */
class DynamicRailwayPartWithPaths
{
public:
    struct Path
    {
        /**
         * Path uuid.
         */
        boost::uuids::uuid uuid{};
        /**
         * Path part start distance.
         */
        double startDistance = 0.0;
        /**
         * Path part end distance.
         */
        double endDistance = 0.0;
        /**
         * Spline part from 'startDistance' to 'endDistance', neccessary for branch segments,
         * which are split to multiple parts, this spline will be used to visibility checking.
         */
        BezierSpline worldSpline;

        std::weak_ptr<DynamicRailwayPartWithPaths> parent;
        int pathIdx = 0;
    };

public:
    virtual ~DynamicRailwayPartWithPaths() = default;

    template<typename ParentType>
    void setPaths(std::vector<Path>&& paths, const ParentType& parent)
    {
        std::weak_ptr<DynamicRailwayPartWithPaths> pathsParent =
            std::static_pointer_cast<DynamicRailwayPartWithPaths>(parent);

        m_paths = std::move(paths);

        for (int pathIdx = 0; pathIdx < m_paths.size(); ++pathIdx)
        {
            m_paths[pathIdx].pathIdx = pathIdx;
            m_paths[pathIdx].parent = pathsParent;
        }
    }

    void setPaths(std::vector<Path>&& paths)
    {
        assert(paths.empty() || !paths[0].parent.expired());
        m_paths = std::move(paths);
    }

    virtual const std::vector<Path>& getPaths() const { return m_paths; }

private:
    std::vector<Path> m_paths;
};
