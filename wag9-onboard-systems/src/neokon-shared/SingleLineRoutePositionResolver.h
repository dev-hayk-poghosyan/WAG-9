#pragma once

#include "Pole.pb.h"
#include "RoutePosition.h"

#include <nanoflann.hpp>
#include <glm/glm.hpp>

#include <optional>
#include <tuple>
#include <vector>

class SingleLineRoutePositionResolver
{
public:
    SingleLineRoutePositionResolver(
        unsigned lineIndex, std::vector<proto::Pole> const& poles);

    /**
     * Produces the position along the route corresponding to a position in the world.
     * In case it's not possible to compute the position along the route, returns std::nullopt.
     *
     * If desired, it also returns a vector of arbitrary length pointing in the direction
     * of increasing kilometers (or miles).
     */
    std::optional<RoutePosition> resolve(
        glm::dvec2 const& location, glm::dvec2* routeTangentVector = nullptr) const;

    /**
     * Each instance of SingleLineRoutePositionResolver works with only one rail line.
     * When we don't know for sure which line a given point in the world belongs to,
     * we can try calling this method on every RoutePositionResolver we've got.
     *
     * @note This method is unreliable. It's only meant to be used in the Line Editor.
     */
    bool likelyBelongsToThisLine(glm::dvec2 const& location) const;

private:
    struct Pole
    {
        glm::dvec2  location;
        proto::Pole protoPole;

        explicit Pole(proto::Pole const& pole);
        std::tuple<int, int> orderToken() const;
    };

    struct Storage
    {
        std::vector<Pole> poles;

        size_t kdtree_get_point_count() const { return poles.size(); }

        double kdtree_get_pt(size_t idx, int dim) const { return poles[idx].location[dim]; }

        template<class BBOX>
        bool kdtree_get_bbox(BBOX &bb) const { return false; }
    };

    RoutePosition buildRoutePosition(
        Pole const& prevPole, Pole const& nextPole,
        double interpolationFactor, glm::dvec2* routeTangentVector) const;

    unsigned m_lineIndex = 0;

    Storage m_storage;

    nanoflann::KDTreeSingleIndexAdaptor<
        nanoflann::L2_Simple_Adaptor<double, Storage>, Storage, 2, size_t> m_index;
};
