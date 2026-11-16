#include "Wag9FsServer.h"
#include <zmq.hpp>
#include <QDebug>
#include <vector>

Wag9FsServer::Wag9FsServer(std::string const& serverBindPoint, QObject* parent)
    : QObject(parent)
    , m_instructorServer(new ZmqBasicServer(serverBindPoint, serverName(), this))
    , m_lastMessageTs(std::chrono::system_clock::now())
    , m_bindPoint(serverBindPoint)
{
    using namespace std::chrono_literals;
    m_timeoutWatcher.setInterval(1s);

    connect(m_instructorServer, &ZmqBasicServer::messageReceived,
            this, &Wag9FsServer::processIncomingMessage);

    connect(&m_timeoutWatcher, &QTimer::timeout, this, [this]
    {
        static const auto timeout = 10s;
        if (std::chrono::system_clock::now() - m_lastMessageTs > timeout)
        {
            qWarning() << "Wag9FsServer::Timeout via watchdog. Client haven't sent any messages for a long time."
                << " RouteID=" << m_routingId;

            m_routingId = 0;
            m_timeoutWatcher.stop();
            emit disconnected();
        }
    });
}

bool
Wag9FsServer::sendMessage(const proto::wag9::fs::FaultStationOutgoingMessage& msg)
{
    if (m_routingId == 0)
    {
        qWarning() << "Wag9FsServer::Unable to send a message: no routeId,i.e. no client at this time";
        return false;
    }

    try
    {
        proto::TrainSimFromFaultStationMessage wrapper;
        wrapper.set_vehicle_number("WAG9");
        wrapper.set_from_fault_station_bytes(msg.SerializeAsString());

        zmq::message_t zmqMsg(wrapper.ByteSizeLong());
        zmqMsg.set_routing_id(m_routingId);

        wrapper.SerializePartialToArray(zmqMsg.data(), zmqMsg.size());

        m_instructorServer->sendMessage(std::move(zmqMsg));
        return true;
    }
    catch (zmq::error_t const& e)
    {
        if (e.num() == EHOSTUNREACH)
        {
            qWarning() << "Wag9FsServer::Network failure: host unreach" << e.what();

            m_routingId = 0;
            emit disconnected();
        }

        return false;
    }
}

bool
Wag9FsServer::sendMessages(const std::vector<proto::wag9::fs::FaultStationOutgoingMessage>& messages)
{
    if (m_routingId == 0)
    {
        qWarning() << "Wag9FsServer::Unable to send messages: no routeId,i.e. no client at this time";
        return false;
    }

    if (messages.empty())
    {
        return true;
    }

    qDebug() << "Wag9FsServer:sending batch of " << messages.size() << " messages to client " << m_routingId;

    try
    {
        std::vector<zmq::message_t> zmqMessages;
        zmqMessages.reserve(messages.size());

        for (const auto& msg : messages)
        {
            proto::TrainSimFromFaultStationMessage wrapper;
            wrapper.set_vehicle_number("WAG9");
            wrapper.set_from_fault_station_bytes(msg.SerializeAsString());

            zmq::message_t zmqMsg(wrapper.ByteSizeLong());
            zmqMsg.set_routing_id(m_routingId);
            wrapper.SerializePartialToArray(zmqMsg.data(), zmqMsg.size());
            zmqMessages.push_back(std::move(zmqMsg));
        }

        m_instructorServer->sendMessages(std::move(zmqMessages));
        return true;
    }
    catch (zmq::error_t const& e)
    {
        if (e.num() == EHOSTUNREACH)
        {
            qWarning() << "Wag9FsServer::Network failure: host unreach" << e.what();

            m_routingId = 0;
            emit disconnected();
        }

        return false;
    }
}

std::string const&
Wag9FsServer::serverName()
{
    static const std::string name{ "Wag9FsServer" };
    return name;
}

void
Wag9FsServer::processIncomingMessage(zmq::message_t const& zmqMsg, uint32_t routingId)
{
    const bool isNewClient = (m_routingId == 0);
    if (isNewClient)
    {
        m_routingId = routingId;
        qWarning() << "Wag9FsServer::Client connected. RouteID = " << m_routingId;
    }

    proto::FaultStationInitMessage initMsg;
    if (initMsg.ParseFromArray(zmqMsg.data(), zmqMsg.size())
        && initMsg.vehicle_numbers_size() > 0)
    {
        emit initMessageReceived(initMsg);
    }
    else
    {
        proto::TrainSimToFaultStationMessage wrapper;
        wrapper.ParseFromArray(zmqMsg.data(), zmqMsg.size());

        proto::wag9::fs::FaultStationIncomingMessage msg;
        msg.ParseFromString(wrapper.to_fault_station_bytes());

        emit messageReceived(msg);
    }

    m_lastMessageTs = std::chrono::system_clock::now();
    if (!m_timeoutWatcher.isActive())
    {
        m_timeoutWatcher.start();
    }

    if (isNewClient)
    {
        emit connected();
    }
}
