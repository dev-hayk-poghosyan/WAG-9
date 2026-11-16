#pragma once

#include <string>
#include <list>
#include <mutex>

/**
 * Класс для группировки и отложенного вывода диагностических сообщений,
 * связанных с определенным объектом, заданным текстовым идентификатором token 
 * Предназначен для записи сообщений, сгенерированных в рабочем потоке с последующим
 * выводом их в главном потоке приложения, например при параллельных вычислениях.
 * ObjectMessageLogger предотвращает перемешивание сообщений от разных объектов
 * в общем логе вывода, объединяя их в секции вида:
 * 
 * ***********begin section of <#token>**************
 * processing stage...
 * warning: incorrect input data...
 * stage complete
 * **********end section of <#token>*****************
 */
class ObjectMessageLogger
{
public:
    ObjectMessageLogger() = default;
    ObjectMessageLogger(std::string const& token);

    /**
     * Методы для вывода (а точнее записи в буфер) сообщений,
     * вызываются в рабочем потоке.
     */
    void debug(const char* format, ...);
    void message(const char* format, ...);
    void warning(const char* format, ...);
    void error(const char* format, ...);

    void debug(const char* format, va_list& argptr);
    void message(const char* format, va_list& argptr);
    void warning(const char* format, va_list& argptr);
    void error(const char* format, va_list& argptr);

    /**
     * Очищает буфер сообщений.
     */
    void clear();

    /**
     * Вывод сообщений в лог из буфера. Нужно вызывать из главного потока.
     */
    void flushSync();

    /**
     * Вывод сообщений в лог из буфера. Нужно вызывать из рабочего потока.
     */
    void flushAsync();

    /**
     * Задать токен- строку для идетификации потока или объекта, для которого
     * происходит группировка сообщений.
     */
    void setToken(std::string const& token);

private:
    enum class MessageType
    {
        Debug,
        Message,
        Warning,
        Error
    };

    struct Message
    {
        std::string message;
        MessageType type;
    };

    std::string m_token;
    std::list<Message> m_messages;
    std::mutex m_messagesMutex;
};

using ObjectMessageLoggerPtr = std::shared_ptr<ObjectMessageLogger>;

/**
 * Класс-надстройка для буфера сообщений, реализующий стратегию RAII.
 * Экземпляр класса должен быть объявлен вначале функции рабочего потока.
 * Данный класс гарантирует вывод диагностических сообщений в случае выброса исключения
 * или аварийного завершения функции.
 */
class ObjectMessageLoggerGuard
{
public:
    ObjectMessageLoggerGuard(ObjectMessageLogger& logger)
        : m_logger(logger)
    {
        m_logger.clear();
    }

    ~ObjectMessageLoggerGuard()
    {
        m_logger.flushAsync();
    }

private:
    ObjectMessageLogger& m_logger;
};
