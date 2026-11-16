#pragma once

#include "EventExecutor.h"
#include <QObject>
#include <zmq.hpp>
#include <cstdint>
#include <thread>

class ZmqBasicServer : public QObject
{
    Q_OBJECT

public:
    /**
     * @throw std::exception If anything goes wrong.
     */
    ZmqBasicServer(std::string const& serverBindPoint, std::string const& serverName,
        QObject* parent = nullptr);

    ~ZmqBasicServer();

    /**
     * @note For more information on routing id, see http://api.zeromq.org/4-2:zmq-socket#toc5
     *
     * @throw std::exception if anything goes wrong. In particular, the client identified by
     *        @p routingId may have disconnected.
     */
    void sendMessage(zmq::message_t&& zmqMsg);

    /**
     * Sends multiple messages as one atomic multipart message.
     * All messages must have the same routing_id set.
     * @throw std::exception if anything goes wrong.
     */
    void sendMessages(std::vector<zmq::message_t>&& messages);

signals:
    /**
     * @note For more information on routing id, see http://api.zeromq.org/4-2:zmq-socket#toc5
     */
    void messageReceived(zmq::message_t const& zmqMsg, uint32_t routingId);

private:
    void runIoThread();

    EventExecutor m_eventExecutor;

    zmq::context_t m_context;
    zmq::socket_t m_sock;

    std::thread m_thread;
};
