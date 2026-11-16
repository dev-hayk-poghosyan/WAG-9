#include "LogMessageHandler.h"
#include "GitVersion.h"
#include "Wag9Version.h"

#ifdef  Q_OS_WINDOWS
#include <windows.h>
#include <debugapi.h>
#endif

#include <QApplication>
#include <QAudioDeviceInfo>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <stdexcept>

LogMessageHandler* LogMessageHandler::m_instance = nullptr;
QMutex LogMessageHandler::m_mutex;

LogMessageHandler::LogMessageHandler(QString const& logFileName, bool backupPrevLogFile)
    : m_outStream(&m_file)
{
    assert(!m_instance && "Only one instance of the log is allowed");

    if (backupPrevLogFile && QFile::exists(logFileName))
    {
        QString const backupFileName = generateBackupFileName(logFileName);

        if (QFile::exists(backupFileName) && !QFile::remove(backupFileName))
        {
            throw std::runtime_error("Failed to remove backup log:" + backupFileName.toStdString());
        }

        if (!QFile::rename(logFileName, backupFileName))
        {
            throw std::runtime_error("Failed to backup prev log:" + logFileName.toStdString());
        }
    }

    m_file.setFileName(logFileName);

    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Text))
        throw std::runtime_error("Failed to open the log file:" + logFileName.toStdString());

    m_instance = this;
    qInstallMessageHandler(handleLog);

    logStartupCommonInfo();
}

LogMessageHandler::~LogMessageHandler()
{
    QMutexLocker lock(&m_mutex);
    qInstallMessageHandler(nullptr);
    m_instance = nullptr;
}

void
LogMessageHandler::handleLog(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    QMutexLocker lock(&m_mutex);

    if (!m_instance)
        return;

    m_instance->m_outStream << "["
        << QDateTime::currentDateTime().toString("dd.MM.yy hh:mm:ss.zzz") << "] ";

    switch (type)
    {
    case QtDebugMsg:
        m_instance->m_outStream << "[D]";
        break;
    case QtInfoMsg:
        m_instance->m_outStream << "[I]";
        break;
    case QtWarningMsg:
        m_instance->m_outStream << "[W]";
        break;
    case QtCriticalMsg:
    {
        QMessageBox::critical(nullptr, qApp->applicationDisplayName(),
            tr("Critical event while executing this program:\n") + msg);

        m_instance->m_outStream << "[C]";

        break;
    }
    case QtFatalMsg:
        m_instance->m_outStream << "[F]";
        break;
    }

    m_instance->m_outStream << QStringLiteral("> %1").arg(msg) << nl();
    m_instance->m_outStream.flush();

#ifdef  Q_OS_WINDOWS
    {
        const auto debugText = msg.toStdWString() + L"\n";
        ::OutputDebugStringW(debugText.c_str());
    }
#else
#warning "Don't know how to write to IDE output window"
#endif
}

QString
LogMessageHandler::generateBackupFileName(QString const& logFileName)
{
    QFileInfo const fi(logFileName);

    return fi.absoluteDir().absoluteFilePath
    (
        fi.completeBaseName() +
        bakupFileNameSuffix() +
        (!fi.suffix().isEmpty() ? "." + fi.suffix() : QString())
    );
}

char
LogMessageHandler::nl()
{
    return '\n';
}

char
LogMessageHandler::tab()
{
    return '\t';
}

QString
LogMessageHandler::bakupFileNameSuffix()
{
    return QStringLiteral("-bak");
}

void
LogMessageHandler::logStartupCommonInfo()
{
    m_instance->m_outStream << QStringLiteral("%1 - %2 v%3- git/%4")
        .arg(QApplication::organizationName(),
             QApplication::applicationName(),
             QApplication::applicationVersion(),
             QString::fromLocal8Bit(Git::buildHash))
        << nl() << nl();

    m_instance->m_outStream << "Available Output devices:" << nl();
    for (auto const& info : QAudioDeviceInfo::availableDevices(QAudio::Mode::AudioOutput))
        m_instance->m_outStream << tab() << "\"" << info.deviceName() << "\"" << nl();

    m_instance->m_outStream << "Available Input devices:" << nl();
    for (auto const& info : QAudioDeviceInfo::availableDevices(QAudio::Mode::AudioInput))
        m_instance->m_outStream << tab() << "\"" << info.deviceName() << "\"" << nl();

    m_instance->m_outStream << nl() << nl();
    m_instance->m_outStream.flush();
}
