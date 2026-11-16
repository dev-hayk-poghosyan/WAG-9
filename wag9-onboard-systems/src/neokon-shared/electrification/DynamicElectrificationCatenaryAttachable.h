#pragma once

#include "neokon-shared/electrification/ElectrificationAltMeshGeneratorConfig.h"
#include "neokon-shared/electrification/ElectrificationContextDataTypes.h"
#include "neokon-shared/electrification/utils/ElectrificationPoleHelperData.h"

#include <Unigine.h>

class MorticeAttachable;
using MorticeAttachablePtr = std::shared_ptr<MorticeAttachable>;

class Insulator;
using InsulatorPtr = std::shared_ptr<Insulator>;

class DropperClamp;
using DropperClampPtr = std::shared_ptr<DropperClamp>;

enum class AttachableConnectorType
{
    NT_IN,
    NT_OUT,
    KP_NEAR_IN,
    KP_NEAR_OUT,
    KP_FAR_IN,
    KP_FAR_OUT,
    KP_DROPPER_IN,
    KP_DROPPER_OUT,
    DROPPER,
};

using ConnectorPosByType = std::unordered_map<AttachableConnectorType, Unigine::Math::Vec3>;

/**
 * Class for catenary mortice attachable, which is an object having in and out wire connectors.
 * Keeps data about insulator and calculates connector positions.
 */
class MorticeAttachable
{
public:
    MorticeAttachable(std::string nodeName, double distance, bool useDroppers, bool isFlipped);

    /**
     * @return the local position of a connector with set @type
     * if such connector exists.
     */
    std::optional<Unigine::Math::Vec3> getConnectorPosition(AttachableConnectorType type) const;

    std::string nodeName() const { return m_nodeName; }
    double distance() const { return m_distance; }
    bool useDroppers() const { return m_useDroppers; }
    bool isFlipped() const { return m_flipped; }

    /**
     * Connected is supposed to be upper part of composite section insulator.
     * This method is used to provide information about upper part in generator
     * so droppers could be correctly placed.
     */
    void connectOtherAttachable(MorticeAttachablePtr other) { m_connectedAttachable = other; }
    /**
     * @return connected insulator to provide information for dropper construction.
     */
    MorticeAttachablePtr getConnectedInsulator() const { return m_connectedAttachable; }

    Unigine::Math::Mat4 getWorldTransform() const { return m_worldTransform; }
    Unigine::Math::Mat4 getWorldWithNoInitialTransform() const;

    void setWorldTransform(Unigine::Math::Mat4 const& transform);

protected:
    /**
     * Analyses insulator node and retrieves any connector information it contains.
     */
    virtual void fillConnectorsInfo() = 0;

    ConnectorPosByType m_connectors;

private:
    std::string m_nodeName;
    double m_distance{ 0.0 };
    bool m_useDroppers{ false };
    bool m_flipped{ false };

    Unigine::Math::Mat4 m_worldTransform;
    Unigine::Math::Mat4 m_internalMeshTransform{Unigine::Math::Mat4::IDENTITY};
    Unigine::Math::Mat4 m_worldWithNoInitialTransform;

    Unigine::ObjectMeshStaticPtr m_mesh;

    MorticeAttachablePtr m_connectedAttachable;
};

/**
 * Class for catenary insulator, an attachable where in and out connectors differ
 */
class Insulator : public MorticeAttachable
{
public:
    Insulator(std::string nodeName, double distance, bool useDroppers, bool isFlipped);

private:
    virtual void fillConnectorsInfo() override;
};

/**
 * Class for dropper clamp, an attachable where in and out connectors are the same
 */
class DropperClamp : public MorticeAttachable
{
public:
    DropperClamp(std::string nodeName, double distance);

private:
    virtual void fillConnectorsInfo() override;
};
