#pragma once

#include "DynamicElectrification.h"

#include "neokon-shared/BezierSegment.h"
#include "neokon-shared/electrification/ElectrificationAltMeshGeneratorConfig.h"
#include "neokon-shared/electrification/ElectrificationContextDataTypes.h"
#include "neokon-shared/electrification/utils/ElectrificationPoleHelperData.h"
#include "neokon-shared/TransformSpline.h"

#include <UnigineMathLib.h>

#include <boost/icl/split_interval_map.hpp>

using namespace Unigine;

class Insulator;
using InsulatorPtr = std::shared_ptr<Insulator>;

class DropperClamp;
using DropperClampPtr = std::shared_ptr<DropperClamp>;

class Wire;
using WirePtr = std::shared_ptr<Wire>;

/**
 * Класс провода для полвески.
 * Принимает на вход начало и конец провода, радиус, сдвиг по оси z, отвечающий за провисание.
 * Позволяет добавлять на провод любое количество изоляторов.
 * Рассчитывает сплайн с учётом изоляторов, позволяет найти точку на сплайне по параметру
 * и по расстоянию над проводом.
 * Хранит чистый провод (m_rawSegment), куда не включены изоляторы,
 * и сегменты провода между вставленными изоляторами (m_segments), по которым будет построен меш.
 */
class Wire
{
public:
    enum class WireType
    {
        Default,
        Nt,
        KpNear,
        KpFar
    };

    struct CreateArgs
    {
        WireType wireType{ WireType::Default };
        Unigine::Math::Vec3 startPos;
        Unigine::Math::Vec3 endPos;
        Unigine::Math::Vec3 offsetFromMiddle;
        double radius{0.0};
        double saggingOffset{0.0};
        bool straightenedWire{false};
    };

public:
    /**
     * Creates @meshDest for wire shaped by Bezier @segment with set @radius.
     */
    static void generateWireMeshFromSpline(BezierSegment const& segment, float radius,
        std::string surfaceName, MeshPtr meshDest);

    Wire(CreateArgs&& args);

    Wire::WireType getType() { return m_genParams.wireType; }

    /**
     * Calculates wire segments if update is needed.
     */
    void update();

    /**
     * Interpolate wire segments straight by doppers' and insulators' points.
     */
    void straighten();

    /**
     * Adds new insulator into the wire.
     * It's transform matrix will be calculated after calcSegment() method call.
     */
    void addInsulator(MorticeAttachablePtr attachable);
    void addDropperClamp(MorticeAttachablePtr attachable,
        Unigine::Math::Vec3 insertPos, double param);

    /**
     * @returns insulators info. After update() call InsulatorInfo contains
     * transformation for insulator.
     */
    std::map<double, MorticeAttachablePtr> const& getMorticeAttachables() const;

    /**
     * @return mesh for the wire with gaps for insulators insertion.
     */
    Unigine::MeshPtr getMesh();

    /**
     * @return position of point on the wire by parameter @t,
     * if @t is outside of any insulator gaps.
     */
    std::optional<Math::Vec3> eval(double t) const;
    /**
     * @return parameter t with @distance from pole over the wire;
     * sagging and other deformations are ignored,
     * and t may be inside of insulator gap.
     */
    double getParamByDistance(double distance) const;
    /**
     * @return parameter t with closest to the set @position,
     * and t may be inside of gap.
     */
    double getParamByAbsolutePosition(Math::Vec3 const& position) const;
    /**
     * @return parameter t with minimal distance between wire and vertical line
     * containing @position.
     */
    double getParamByPositionOnVerticalLine(Math::Vec3 const& position) const;

    /**
     * @return the length of the wire.
    */
    double length() const;
    /**
     * @return the distance between poles.
     */
    double airLength() const;

    Math::Vec3 startPosition() const { return m_genParams.startPos; }
    Math::Vec3 endPosition() const { return m_genParams.endPos; }

private:
    enum class ConnectorSide
    {
        In,
        Out
    };

    struct WireFragment
    {
        TransformSpline segmentSearch;
        BezierSegment segment;
        double paramStart{0.0};
        double paramEnd{0.0};

        WireFragment(BezierSegment wireSeg, double tStart, double tEnd)
            : segment(std::move(wireSeg))
            , paramStart(tStart)
            , paramEnd(tEnd)
        {
        }

        WireFragment(WireFragment const& wireSegment)
            : segment(wireSegment.segment)
            , paramStart(wireSegment.paramStart)
            , paramEnd(wireSegment.paramEnd)
        {
        }
    };

    std::optional<double> tFromRaw(WireFragment const& fragment, double t) const;
    std::optional<Unigine::Math::Vec3> getConnectorPosition(MorticeAttachablePtr attachable,
        ConnectorSide side);

private:
    void generateMesh();
    void calcRawSegment();
    void calcSegments();

    std::optional<TransformSpline> m_rawSegmentSearch;

    CreateArgs m_genParams;

    bool m_meshUpdated{false};
    bool m_splineUpdated{false};

    Unigine::MeshPtr m_mesh;
    std::map<double, MorticeAttachablePtr> m_attachables;
    std::map<double, Unigine::Math::Vec3> m_clampPositions;

    std::list<WireFragment> m_segments;
    std::optional<BezierSegment> m_rawSegment;

    boost::icl::split_interval_set<double> m_insulatorIntervals;
};
