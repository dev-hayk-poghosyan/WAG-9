#pragma once

#include <QApplication>
#include <QFile>
#include <QMutex>
#include <QString>
#include <QTextStream>

#include <chrono>

/**
 * Custom message handler for Qt loggin system. Only one instance is allowed.
 */
class LogMessageHandler
{
    Q_DECLARE_TR_FUNCTIONS(LogMessageHandler)
public:
    /**
     * Installs this handler and dumps incoming messages into @p fileNameTemplate.
     * @p backupPrevLogFile if @p fileNameTemplate exists it will be renamed by adding a suffix.
     * Throws std::exception.
     */
    LogMessageHandler(QString const& fileNameTemplate, bool backupPrevLogFile = true);

    /**
     * Restores the default qt's message handler.
     */
    virtual ~LogMessageHandler();

    /**
     * Used as a handler for Qt's logging system.
     */
    static void handleLog(QtMsgType type, const QMessageLogContext& context, const QString& msg);

private:
    static QString generateBackupFileName(QString const& logFileName);
    static char nl();
    static char tab();
    static QString bakupFileNameSuffix();
    static void logStartupCommonInfo();

    static LogMessageHandler* m_instance;
    static QMutex m_mutex;
    QFile m_file;
    QTextStream m_outStream;
};

#define SMC_PROFILING_START \
    auto smc_profiling_start = std::chrono::steady_clock::now(); \

#define SMC_PROFILING_RESET {\
    smc_profiling_start = std::chrono::steady_clock::now(); }\

#define SMC_PROFILING_ELAPSED_MS (\
    std::chrono::duration_cast<std::chrono::milliseconds>(\
        std::chrono::steady_clock::now() - smc_profiling_start).count()) \
