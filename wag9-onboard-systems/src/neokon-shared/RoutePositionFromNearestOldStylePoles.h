#pragma once

#include "ObjectsOfInterest.pb.h"
#include "OldStylePole.pb.h"

#include "neokon-shared/RoutePosition.h"

#include <glm/glm.hpp>
#include <nanoflann.hpp>
#include <optional>
#include <array>
#include <cstdint>
#include <memory>
#include <vector>

/**
 * This class takes the arbitrary position in the world and resolves it into a RoutePosition.
 *
 * This class was developed before we added support for multiple rail lines, so it always
 * assumes line 0.
 *
 * While in theory we could have converted old-style poles into new-style ones and get rid of this
 * class altogether, in practice that wouldn't work, as the new RoutePositionResolver class
 * demands absolute correctness in pole data (no duplicate or missing poles), unlike this one.
 */
class RoutePositionFromNearestOldStylePoles
{
public:
    explicit RoutePositionFromNearestOldStylePoles(
        proto::ObjectsOfInterest const& objectsOfInterest);

    /**
     * Produces the position along the route corresponding to a position in the world.
     *
     * In case it's not possible to compute the position along the route, returns boost::none.
     * This can happen if less than two poles were loaded or if two poles were positioned on the
     * same spot.
     *
     * If desired, it also returns a vector of arbitrary length pointing in the direction
     * of increasing kilometers (or miles).
     */
    std::optional<RoutePosition> resolvePosition(
        glm::dvec3 const& location, glm::dvec3* routeTangentVector = nullptr) const;

private:
    struct Pole
    {
        /**
         * Note that even though we store 3D points, when it comes to finding
         * nearest neighbours, we perform 2D matching on them. That's done because
         * we fake the Z coordinate of the points on the RoadSet in RailroadControl.
         */
        glm::dvec3 location;

        double distanceOnRoute;

        explicit Pole(proto::OldStylePole const& pole);
    };

    struct Storage
    {
        std::vector<Pole> poles;

        size_t kdtree_get_point_count() const { return poles.size(); }

        double kdtree_get_pt(size_t idx, int dim) const { return poles[idx].location[dim]; }

        template<class BBOX>
        bool kdtree_get_bbox(BBOX &bb) const { return false; }
    };

    static bool discontinuityDetected(Pole const& pole0, Pole const& pole1);

    RoutePosition buildRoutePosition(
        Pole const& pole0, Pole const& pole1, double projScalar,
        glm::dvec3* routeTangentVector) const;

    Storage m_storage;

    nanoflann::KDTreeSingleIndexAdaptor<
        nanoflann::L2_Simple_Adaptor<double, Storage>, Storage, 2, size_t> m_index;
};
