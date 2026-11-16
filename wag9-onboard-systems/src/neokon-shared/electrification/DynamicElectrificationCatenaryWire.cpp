#include "DynamicElectrificationCatenaryGenerator.h"

#include "DynamicElectrificationCatenaryWire.h"

#include "neokon-shared/BezierSegmentHelpers.h"
#include "neokon-shared/electrification/CatenaryGenerationConstants.h"
#include "neokon-shared/electrification/DynamicElectrification.h"
#include "neokon-shared/electrification/DynamicElectrificationCatenaryAttachable.h"
#include "neokon-shared/FloatingPointCompare.h"
#include "neokon-shared/GenerateMeshFromSplineList.h"
#include "neokon-shared/GlmUnigineInterop.h"

#include <random>

using namespace electrification;

namespace
{
constexpr double SplineSearchStep = 1.0;
constexpr double MaximumWireLengthSqr = (300 * 300);
} // anonymous namespace.

//static
void
Wire::generateWireMeshFromSpline(BezierSegment const& segment, float radius,
    std::string surfaceName, MeshPtr meshDest)
{
    std::vector<Math::Vec3> pointsTop;
    pointsTop.push_back(Math::Vec3{0., radius, 0.});
    pointsTop.push_back(Math::Vec3{0., 0., radius});
    pointsTop.push_back(Math::Vec3{0., -radius, 0.});

    std::vector<Math::vec3> normalsTop;
    normalsTop.push_back(Math::vec3{0., 1., 1.}.normalize());
    normalsTop.push_back(Math::vec3{0., -1., 1.}.normalize());
    normalsTop.push_back(Math::vec3{0., 0., -1.}.normalize());

    std::vector<Math::Vec3> pointsBottom;
    pointsBottom.push_back(Math::Vec3{0., -radius, 0.});
    pointsBottom.push_back(Math::Vec3{0., 0., -radius});
    pointsBottom.push_back(Math::Vec3{0., radius, 0.});

    std::vector<Math::vec3> normalsBottom;
    normalsBottom.push_back(Math::vec3{0., -1., -1.}.normalize());
    normalsBottom.push_back(Math::vec3{0., 1., -1.}.normalize());
    normalsBottom.push_back(Math::vec3{0., 0., 1.}.normalize());

    GenerateMeshFromSplineArgs genArgs;

    genArgs.dstMesh = meshDest;
    genArgs.profilePointPositions = pointsTop;
    genArgs.profilePointNormals = normalsTop;
    genArgs.numSegmentsPerUnit = 2;
    genArgs.splineList.push_back(segment);
    genArgs.adaptiveDividing = true;
    genArgs.postCap = genArgs.preCap = true;
    genArgs.profileCapTris.push_back({1, 2, 0});
    genArgs.uCoordStepPerUnit = 3.;
    genArgs.surfaceIndex = meshDest->findSurface(surfaceName.c_str());
    genArgs.surfaceName = surfaceName;

    genArgs.profilePointUVCoords.push_back(Math::vec2{0., 1.});
    genArgs.profilePointUVCoords.push_back(Math::vec2{0., 0.5});
    genArgs.profilePointUVCoords.push_back(Math::vec2{0., 0.});

    generateMeshFromSpline(genArgs);

    genArgs.surfaceIndex = meshDest->findSurface(surfaceName.c_str());
    genArgs.profilePointPositions = pointsBottom;
    genArgs.profilePointNormals = normalsBottom;

    generateMeshFromSpline(genArgs);
}

Wire::Wire(CreateArgs&& args)
    : m_genParams(std::move(args))
{
    if ((m_genParams.startPos - m_genParams.endPos).length2() > MaximumWireLengthSqr)
    {
        throw std::runtime_error("Cannot create Wire: wire length is too big.");
    }
}

void
Wire::update()
{
    if (!m_splineUpdated)
    {
        calcSegments();
        m_splineUpdated = true;
    }
}

void
Wire::straighten()
{
    const double normalizedTangentLength = 0.3;

    m_meshUpdated = false;

    auto oldSegments = m_segments;
    m_segments.clear();

    auto currentIt = m_clampPositions.begin();
    for (auto const& fragment : oldSegments)
    {
        double startParam = fragment.paramStart;
        Math::Vec3 startPos = fromGlm(fragment.segment.p0());

        double endParam{0.0};
        Math::Vec3 endPos;

        if (currentIt != m_clampPositions.end())
        {
            endParam = currentIt->first;
            while (endParam <= startParam)
            {
                ++currentIt;
                if (currentIt == m_clampPositions.end())
                {
                    break;
                }

                endParam = currentIt->first;
            }

            while (endParam < fragment.paramEnd)
            {
                Math::Vec3 endPos = currentIt->second;

                Math::Vec3 rightTangent = endPos - startPos;
                rightTangent = rightTangent * normalizedTangentLength;
                Math::Vec3 leftTangent = -rightTangent;

                BezierSegment segment(toGlm(startPos), toGlm(startPos + rightTangent),
                    toGlm(endPos + leftTangent), toGlm(endPos));
                m_segments.emplace_back(segment, startParam, endParam);
                //TODO: replace segmentSearch with something more effective
                m_segments.back().segmentSearch.build(
                    m_segments.back().segment, SplineSearchStep);

                startParam = endParam;
                startPos = endPos;

                ++currentIt;
                if (currentIt == m_clampPositions.end())
                {
                    break;
                }

                endParam = currentIt->first;
            }
        }

        endParam = fragment.paramEnd;
        endPos = fromGlm(fragment.segment.p3());

        Math::Vec3 rightTangent = endPos - startPos;
        rightTangent = rightTangent * normalizedTangentLength;
        Math::Vec3 leftTangent = -rightTangent;

        BezierSegment segment(toGlm(startPos), toGlm(startPos + rightTangent),
            toGlm(endPos + leftTangent), toGlm(endPos));
        m_segments.emplace_back(segment, startParam, endParam);
        m_segments.back().segmentSearch.build(
            m_segments.back().segment, SplineSearchStep);
    }

    m_splineUpdated = true;
    generateMesh();
}

void
Wire::addInsulator(MorticeAttachablePtr attachable)
{
    InsulatorPtr insulator = std::dynamic_pointer_cast<Insulator>(attachable);
    if (!insulator)
    {
        return;
    }

    m_attachables.emplace(insulator->distance(), insulator);
    m_meshUpdated = false;
    m_splineUpdated = false;
}

void
Wire::addDropperClamp(MorticeAttachablePtr attachable, Unigine::Math::Vec3 insertPos, double param)
{
    DropperClampPtr dropperClamp = std::dynamic_pointer_cast<DropperClamp>(attachable);
    if (!dropperClamp)
    {
        return;
    }

    auto ntConnectorPos = dropperClamp->getConnectorPosition(AttachableConnectorType::NT_IN);
    if (!ntConnectorPos)
    {
        return;
    }

    const Math::Vec3 insertTangent = fromGlm(m_rawSegment->unitTangentAt(param)); // axisY
    const Math::Vec3 axisZ = Math::cross(insertTangent, Math::Vec3::RIGHT);
    Math::Mat4 insertTransform{insertTangent, Math::Vec3::RIGHT, axisZ, insertPos};
    dropperClamp->setWorldTransform(Math::translate(insertPos));

    m_clampPositions.emplace(param, dropperClamp->getWorldTransform() * *ntConnectorPos);
    m_attachables.emplace(dropperClamp->distance(), dropperClamp);

    m_meshUpdated = false;
    m_splineUpdated = false;
}

std::map<double, MorticeAttachablePtr> const&
Wire::getMorticeAttachables() const
{
    return m_attachables;
}

Unigine::MeshPtr
Wire::getMesh()
{
    if (!m_meshUpdated)
    {
        generateMesh();
        m_meshUpdated = true;
    }

    return m_mesh;
}

std::optional<Math::Vec3>
Wire::eval(double t) const
{
    auto poleFoundIt = m_insulatorIntervals.find(t);
    if (poleFoundIt != m_insulatorIntervals.end())
    {
        return std::nullopt;
    }

    if (!m_rawSegment)
    {
        return std::nullopt;
    }

    for (auto const& fragment : m_segments)
    {
        if (auto param = tFromRaw(fragment, t))
        {
            return fromGlm(fragment.segment.eval(*param));
        }
    }

    return std::nullopt;
}

double
Wire::length() const
{
    if (m_rawSegment)
    {
        return m_rawSegment->arcLength();
    }

    return 0.0;
}

double
Wire::airLength() const
{
    return (m_genParams.endPos - m_genParams.startPos).length();
}

double
Wire::getParamByDistance(double distance) const
{
    const double searchHeight = m_genParams.saggingOffset + 0.5;

    assert(m_rawSegment);
    if (!m_rawSegment)
    {
        return 0.0;
    }

    glm::dvec3 direction = m_rawSegment->p3() - m_rawSegment->p0();
    direction = glm::normalize(direction);

    glm::dvec3 const pointOnDirection = direction * distance;
    glm::dvec3 const offset = glm::dvec3{0.0, 0.0, -searchHeight};

    assert(m_rawSegmentSearch);
    auto const closestPoint = m_rawSegmentSearch->findClosestPoints(
        pointOnDirection - offset, pointOnDirection + offset);
    if (!closestPoint)
    {
        return 0.0;
    }

    TransformSpline::Point thisPoint = closestPoint->first;
    return m_rawSegment->tAtDistance(thisPoint.distanceOffset);
}

double
Wire::getParamByAbsolutePosition(Math::Vec3 const& position) const
{
    assert(m_rawSegment);
    if (!m_rawSegment)
    {
        return 0.0;
    }

    assert(m_rawSegmentSearch);
    auto const closestPoint = m_rawSegmentSearch->findClosestPoint(toGlm(position));
    if (!closestPoint)
    {
        return 0.0;
    }

    return m_rawSegment->tAtDistance(closestPoint->distanceOffset);
}

double
Wire::getParamByPositionOnVerticalLine(Math::Vec3 const& position) const
{
    double distance = std::abs(position.y - m_genParams.startPos.y);
    return getParamByDistance(distance);
}

std::optional<double>
Wire::tFromRaw(WireFragment const& fragment, double t) const
{
    if (!m_rawSegment)
    {
        return std::nullopt;
    }

    if (t < fragment.paramStart || t > fragment.paramEnd)
    {
        return std::nullopt;
    }

    glm::dvec3 const rawPoint = m_rawSegment->eval(t);

    auto fragmentPoint = fragment.segmentSearch.findClosestPoint(rawPoint);
    if (!fragmentPoint)
    {
        return std::nullopt;
    }

    return fragment.segment.tAtDistance(fragmentPoint->distanceOffset);
}

void
Wire::generateMesh()
{
    update();

    m_mesh.clear();
    m_mesh = Mesh::create();

    for (auto const& fragment : m_segments)
    {
        generateWireMeshFromSpline(fragment.segment, m_genParams.radius, "wire", m_mesh);
    }
}

void
Wire::calcRawSegment()
{
    Math::Vec3 middlePos = (m_genParams.startPos + m_genParams.endPos) * 0.5;
    middlePos.z = middlePos.z - m_genParams.saggingOffset;
    m_rawSegment = bezierSegmentByThreePoints(m_genParams.startPos, middlePos, m_genParams.endPos);

    m_rawSegmentSearch.emplace();
    m_rawSegmentSearch->build(BezierSpline(*m_rawSegment), SplineSearchStep);
}

void
Wire::calcSegments()
{
    const double eps = 1e-6;
    // несмотря на то, что провод присутствует,
    // на небольшом расстоянии от промежутка считаем его недоступным,
    // чтобы не добавлялись струны вплотную к изолятору
    const double deadZone = 0.005;

    m_segments.clear();
    m_insulatorIntervals.clear();
    m_rawSegment.reset();

    // если сплайн будет слишком большой, TransformSpline зависнет при построении
    if ((m_genParams.startPos - m_genParams.endPos).length2() > MaximumWireLengthSqr)
    {
        return;
    }

    calcRawSegment();
    assert(m_rawSegment);

    if (m_attachables.empty())
    {
        // Init only the first segment.
        // TODO: clone() the m_rawSegmentSearch here.
        m_segments.emplace_back(*m_rawSegment, 0.0, 1.0);
        m_segments.back().segmentSearch.build(m_segments.back().segment, SplineSearchStep);
        return;
    }

    double prevParam = 0.0;
    BezierSegment leftoverSegment = *m_rawSegment;
    for (auto& [connectorId, insulator] : m_attachables)
    {
        assert(CmDoubleCompare::moreLess(prevParam, 1.0));
        if (CmDoubleCompare::greaterEqual(prevParam, 1.0))
        {
            break;
        }

        auto connectorIn = getConnectorPosition(insulator, ConnectorSide::In);
        auto connectorOut = getConnectorPosition(insulator, ConnectorSide::Out);
        if (!connectorIn || !connectorOut)
        {
            continue;
        }

        const double wireLength = (endPosition() - startPosition()).length();
        double correctedDistance = insulator->distance();

        double insulatorLengthCorrection{0.0};
        NodePtr insulatorNode = World::loadNode(insulator->nodeName().c_str());
        if (insulatorNode)
        {
            auto boundBox = insulatorNode->getBoundBox();
            insulatorLengthCorrection = boundBox.getMin().x - connectorIn->x;
        }

        if (correctedDistance < insulatorLengthCorrection)
        {
            correctedDistance = insulatorLengthCorrection;
        }
        else if (correctedDistance > (wireLength - insulatorLengthCorrection))
        {
            correctedDistance = wireLength - insulatorLengthCorrection;
        }

        double param = getParamByDistance(correctedDistance);

        const Math::Vec3 insertPoint = fromGlm(m_rawSegment->eval(param));
        const Math::Vec3 insertTangent = fromGlm(m_rawSegment->unitTangentAt(param)); // axisY
        const Math::Vec3 axisZ = Math::cross(insertTangent, Math::Vec3::RIGHT);

        //трансформация поворачивает изолятор, потому что
        //провод идёт по оси Oy, а изолятор ориентирован по оси Oy
        Math::Mat4 insertTransform{insertTangent, Math::Vec3::RIGHT, axisZ, insertPoint};
        insertTransform = Math::translate(m_genParams.offsetFromMiddle)  * insertTransform;
        insulator->setWorldTransform(insertTransform);
        insertTransform = insulator->getWorldTransform();

        connectorIn = insertTransform * *connectorIn;
        connectorOut = insertTransform * *connectorOut;

        auto const pointIn = m_rawSegmentSearch->findClosestPoint(toGlm(*connectorIn));
        if (!pointIn)
        {
            continue;
        }
        auto const pointOut = m_rawSegmentSearch->findClosestPoint(toGlm(*connectorOut));
        if (!pointOut)
        {
            continue;
        }

        double paramIn = m_rawSegment->tAtDistance(pointIn->distanceOffset);
        double paramOut = m_rawSegment->tAtDistance(pointOut->distanceOffset);

        if (CmDoubleCompare::equal(m_genParams.saggingOffset, 0.0))
        {
            {
                Math::Vec3 startPos = fromGlm(leftoverSegment.p0());
                Math::Vec3 endPos = *connectorIn;
                Math::Vec3 rightTangent = endPos - startPos;
                rightTangent = rightTangent * 0.3;
                Math::Vec3 leftTangent = -rightTangent;

                BezierSegment segmentIn(toGlm(startPos), toGlm(startPos + rightTangent),
                    toGlm(endPos + leftTangent), toGlm(endPos));
                m_segments.emplace_back(segmentIn, prevParam, paramIn);
                prevParam = paramOut;
            }

            {
                Math::Vec3 startPos = *connectorOut;
                Math::Vec3 endPos = fromGlm(leftoverSegment.p3());
                Math::Vec3 rightTangent = endPos - startPos;
                rightTangent = rightTangent * 0.3;
                Math::Vec3 leftTangent = -rightTangent;

                BezierSegment segmentOut(toGlm(startPos), toGlm(startPos + rightTangent),
                    toGlm(endPos + leftTangent), toGlm(endPos));
                leftoverSegment = segmentOut;
            }
        }
        else
        {
            double coef = 1. / (1.0 - prevParam);
            double paramInLeftover = (paramIn - prevParam) * coef;
            double paramOutLeftover = (paramOut - prevParam) * coef;

            auto pointInL = fromGlm(leftoverSegment.eval(paramInLeftover));
            auto pointInO = fromGlm(leftoverSegment.eval(paramOutLeftover));

            auto subdividedIn = leftoverSegment.subdivide(paramInLeftover);
            auto subdividedOut = leftoverSegment.subdivide(paramOutLeftover);

            BezierSegment& subdividedInSegment = subdividedIn.front();
            BezierSegment& subdividedOutSegment = subdividedOut.back();

            Math::Vec3 inOffset = *connectorIn - pointIn->transform.getTranslate();
            Math::Vec3 outOffset = *connectorOut - pointOut->transform.getTranslate();

            BezierSegment segmentIn(subdividedInSegment.p0(), subdividedInSegment.p1(),
                subdividedInSegment.p2() + toGlm(inOffset), toGlm(*connectorIn));
            BezierSegment segmentOut(toGlm(*connectorOut), subdividedOutSegment.p1(),
                subdividedOutSegment.p2() + toGlm(outOffset), subdividedOutSegment.p3());

            m_segments.emplace_back(segmentIn, prevParam, paramIn);
            prevParam = paramOut;

            leftoverSegment = segmentOut;
        }
    }

    m_segments.emplace_back(leftoverSegment, prevParam, 1.0);

    // Build segment TransformSpline instances.
    for (auto& segment : m_segments)
    {
        segment.segmentSearch.build(segment.segment, SplineSearchStep);
    }
}

std::optional<Unigine::Math::Vec3>
Wire::getConnectorPosition(MorticeAttachablePtr attachable, ConnectorSide side)
{
    if (side == ConnectorSide::In)
    {
        switch (m_genParams.wireType)
        {
        case WireType::Nt:
            return attachable->getConnectorPosition(AttachableConnectorType::NT_IN);
            break;
        case WireType::KpNear:
            return attachable->getConnectorPosition(AttachableConnectorType::KP_NEAR_IN);
            break;
        case WireType::KpFar:
            return attachable->getConnectorPosition(AttachableConnectorType::KP_FAR_IN);
        case WireType::Default:
            assert(false);
        }
    }
    else if (side == ConnectorSide::Out)
    {
        switch (m_genParams.wireType)
        {
        case WireType::Nt:
            return attachable->getConnectorPosition(AttachableConnectorType::NT_OUT);
            break;
        case WireType::KpNear:
            return attachable->getConnectorPosition(AttachableConnectorType::KP_NEAR_OUT);
            break;
        case WireType::KpFar:
            return attachable->getConnectorPosition(AttachableConnectorType::KP_FAR_OUT);
        case WireType::Default: assert(false);
        }
    }

    return std::nullopt;
}
