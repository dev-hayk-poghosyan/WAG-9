#include "ObjectMessageLogger.h"

#include "neokon-shared/StringFormat.h"

#include <UnigineLog.h>

#include <future>

ObjectMessageLogger::ObjectMessageLogger(std::string const& token)
    : m_token(token)
{
}

void ObjectMessageLogger::debug(const char* format, ...)
{
    va_list argptr;
    va_start(argptr, format);
    debug(format, argptr);
    va_end(argptr);
}

void ObjectMessageLogger::message(const char* format, ...)
{
    va_list argptr;
    va_start(argptr, format);
    message(format, argptr);
    va_end(argptr);
}

void ObjectMessageLogger::warning(const char* format, ...)
{
    va_list argptr;
    va_start(argptr, format);
    warning(format, argptr);
    va_end(argptr);
}

void ObjectMessageLogger::error(const char* format, ...)
{
    va_list argptr;
    va_start(argptr, format);
    error(format, argptr);
    va_end(argptr);
}

void ObjectMessageLogger::debug(const char* format, va_list& argptr)
{
    std::string message = stringFormatVaList(format, argptr);

    std::lock_guard<std::mutex> lock(m_messagesMutex);
    m_messages.emplace_back(Message{ message, MessageType::Debug });
}

void ObjectMessageLogger::message(const char* format, va_list& argptr)
{
    std::string message = stringFormatVaList(format, argptr);

    std::lock_guard<std::mutex> lock(m_messagesMutex);
    m_messages.emplace_back(Message{ message, MessageType::Message });
}

void ObjectMessageLogger::warning(const char* format, va_list& argptr)
{
    std::string message = stringFormatVaList(format, argptr);

    std::lock_guard<std::mutex> lock(m_messagesMutex);
    m_messages.emplace_back(Message{ message, MessageType::Warning });
}

void ObjectMessageLogger::error(const char* format, va_list& argptr)
{
    std::string message = stringFormatVaList(format, argptr);

    std::lock_guard<std::mutex> lock(m_messagesMutex);
    m_messages.emplace_back(Message{ message, MessageType::Error });
}

void ObjectMessageLogger::clear()
{
    std::lock_guard<std::mutex> lock(m_messagesMutex);
    m_messages.clear();
}

void ObjectMessageLogger::flushSync()
{
    std::lock_guard<std::mutex> lock(m_messagesMutex);

    Unigine::Log::message("begin of %s messages*************************\n", m_token.c_str());

    for (Message const& item : m_messages)
    {
        switch (item.type)
        {
        case MessageType::Message:
            Unigine::Log::message(item.message.data());
            break;
        case MessageType::Warning:
            Unigine::Log::warning(item.message.data());
            break;
        case MessageType::Error:
            Unigine::Log::error(item.message.data());
            break;
        default:
            break;
        }
    }
    m_messages.clear();

    Unigine::Log::message("end of %s messages*************************\n", m_token.c_str());
}

void ObjectMessageLogger::flushAsync()
{
    auto callPtr = [this]()->bool
    {
        flushSync();
        return false;
    };

    static_cast<void>(std::async(std::launch::async, callPtr));
}

void ObjectMessageLogger::setToken(std::string const& token)
{
    m_token = token;
}
