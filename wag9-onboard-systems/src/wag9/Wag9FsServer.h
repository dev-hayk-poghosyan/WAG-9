#pragma once

#include "ZmqBasicServer.h"
#include "Wag9FaultStation.pb.h"
#include "FaultStationMessages.pb.h"

#include <QTimer>
#include <optional>
#include <chrono>

class Wag9FsServer : public QObject
{
    Q_OBJECT
public:
    explicit Wag9FsServer(
        std::string const& serverBindPoint, QObject* parent = nullptr);

    /**
     * Returns true on success, false on any kind of failure.
     */
    bool sendMessage(const proto::wag9::fs::FaultStationOutgoingMessage& msg);

    /**
     * Sends multiple messages as one atomic batch using ZMQ multipart.
     * Returns true on success, false on any kind of failure.
     */
    bool sendMessages(const std::vector<proto::wag9::fs::FaultStationOutgoingMessage>& messages);

    std::string const& bindPoint() const { return m_bindPoint; }
    bool hasClient() const { return m_routingId != 0; }

signals:
    void connected();
    void disconnected();
    void messageReceived(proto::wag9::fs::FaultStationIncomingMessage const& msg);

private:
    static std::string const& serverName();

    void processIncomingMessage(zmq::message_t const& zmqMsg, uint32_t routingId);

    ZmqBasicServer* m_instructorServer = nullptr;
    uint32_t m_routingId = 0;

    std::chrono::time_point<std::chrono::system_clock> m_lastMessageTs;
    QTimer m_timeoutWatcher;

    std::string const m_bindPoint;
    static const int m_fileTransferMaxChunkSize = 512;
};
