#include "TrainFeatures.h"

TrainFeatures::TrainFeatures(std::string const& type, std::string const& name,
    std::string const& heading)
    : m_type(type)
    , m_name(name)
    , m_heading(heading)
{
}

TrainFeatures::TrainFeatures(proto::TrainFeatures const& protoMessage)
    : m_type(protoMessage.type())
    , m_name(protoMessage.name())
    , m_heading(protoMessage.heading())
{
}

proto::TrainFeatures TrainFeatures::toProtoMessage() const
{
    proto::TrainFeatures features;
    features.set_type(m_type);
    features.set_name(m_name);
    features.set_heading(m_heading);
    return features;
}
