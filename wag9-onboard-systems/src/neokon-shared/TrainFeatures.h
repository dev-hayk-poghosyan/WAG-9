#pragma once

#include "TrainFeatures.pb.h"

#include <string>

/**
 * Keeps data from 'Features' section from the train in the Trains.xml, like this:
 * <Features>
 *   <Type>Passenger Train</Type>
 *   <Name>Stadler</Name>
 *   <Heading>Tbilisi - Zestafoni</Heading>
 * </Features>
 */
class TrainFeatures
{
public:
    TrainFeatures(std::string const& type, std::string const& name, std::string const& heading);

    TrainFeatures(proto::TrainFeatures const& protoMessage);

    proto::TrainFeatures toProtoMessage() const;

    std::string const& type() const { return m_type; }
    std::string const& name() const { return m_name; }
    std::string const& heading() const { return m_heading; }

private:
    std::string m_type;
    std::string m_name;
    std::string m_heading;
};
