#include "ZmqBasicServer.h"
#include "EventExecutor.h"
#include "ExecEvent.h"
#include "ScopeGuard.h"
#include <QCoreApplication>

#include <boost/format.hpp>

#include <QDebug>
#include <exception>
#include <memory>
#include <utility>
#include <vector>


ZmqBasicServer::ZmqBasicServer(std::string const& serverBindPoint,
    std::string const& serverName, QObject* parent)
try : QObject(parent)
    ,  m_sock(m_context, zmq::socket_type::server)
{
    // Don't hang on shutdown, trying to deliver pending messages.
    m_sock.setsockopt<int>(ZMQ_LINGER, 0);

    m_sock.bind(serverBindPoint); // May throw.

    m_thread = std::thread(&ZmqBasicServer::runIoThread, this);
}
catch (const std::exception& e)
{
    throw std::runtime_error((boost::format("Unable to create <%1%>(%2%): %3%")
        % serverName
        % serverBindPoint
        % e.what()).str());
}

ZmqBasicServer::~ZmqBasicServer()
{
    // This will cause blocking socket operations running in the I/O thread to throw an exception.
    m_context.close();

    m_thread.join();
}

void
ZmqBasicServer::sendMessage(zmq::message_t&& zmqMsg)
{
    try
    {
        m_sock.send(std::move(zmqMsg), ZMQ_DONTWAIT);
    }
    catch (std::exception const& e)
    {
        qWarning() << "ZmqBasicServer: failed to send a message:" << e.what();
    }
}

void
ZmqBasicServer::sendMessages(std::vector<zmq::message_t>&& messages)
{
    if (messages.empty())
    {
        return;
    }

    try
    {
        // Send all messages as multipart (all but the last have ZMQ_SNDMORE flag)
        for (size_t i = 0; i < messages.size(); ++i)
        {
            int flags = ZMQ_DONTWAIT;
            if (i < messages.size() - 1)
            {
                flags |= ZMQ_SNDMORE;
            }
            m_sock.send(std::move(messages[i]), flags);
        }
    }
    catch (std::exception const& e)
    {
        qWarning() << "ZmqBasicServer: failed to send batch messages:" << e.what();
    }
}

void
ZmqBasicServer::runIoThread()
{
    // m_context.close() won't return until we close this socket.
    auto const socketClose = createScopeGuard([this] { m_sock.close(); });

    try
    {
        for (;;)
        {
            zmq::message_t zmqMsg;
            m_sock.recv(&zmqMsg);

            ExecEvent* evt = createExecEvent(
            [this, msg = std::move(zmqMsg), routingId = zmqMsg.routing_id()]
            {
                emit messageReceived(msg, routingId);
            });

            QCoreApplication::postEvent(&m_eventExecutor, evt);
        }
    }
    catch (zmq::error_t const& zmqEx)
    {
        if (zmqEx.num() == ETERM)
        {
            qInfo() << "ZmqBasicServer: ETERM error occured."
                " Context is closing at the moment now? " << zmqEx.what();
        }
        else
        {
            qCritical() << "ZmqBasicServer: ZMQ related error occred: " << zmqEx.what();
        }
    }
    catch (std::exception const& e)
    {
        qCritical() << "ZmqBasicServer: unexpected system error occred: " << e.what();
    }
}
