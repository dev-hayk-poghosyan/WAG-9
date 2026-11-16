#include "Wag9Configuration.h"
#include "Wag9Fs.h"
#include "FileNames.h"
#include "LogMessageHandler.h"

#include <QApplication>
#include <QDir>
#include <QFontDatabase>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(Wag9);

    QApplication app(argc, argv);
    app.setOrganizationName("S.M.A.R.T");
    app.setApplicationDisplayName("Wag9 FaultStation");
    app.setApplicationVersion("0.1");
    QDir const appDir(QCoreApplication::applicationDirPath());

    std::optional<LogMessageHandler> handler;
    try
    {
        handler.emplace(appDir.absoluteFilePath(FileNames::Wag9LogFileName()));
    }
    catch (std::exception const& e)
    {
        QMessageBox::critical(nullptr, qApp->applicationDisplayName(), QStringLiteral(
            "Failed to install the logging service: %1.").arg(QString::fromLocal8Bit(e.what())));

        return EXIT_FAILURE;
    }
    
    std::shared_ptr<Wag9Configuration const> staticConfig;
    const QString configPath = appDir.filePath(FileNames::Wag9Settings());
    qDebug() << "Loading settings from " << configPath;

    try
    {
        staticConfig = loadWag9Configuration(configPath);
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(nullptr, app.applicationDisplayName(),
            QStringLiteral("Failed to load config from %1 : %2")
            .arg(configPath, QString::fromLocal8Bit(e.what())));

        return 1;
    }
    static const auto fontName = QStringLiteral(":/fonts/Oswald-VariableFont_wght.ttf");

    const int dottedFontId = QFontDatabase::addApplicationFont(fontName);

    if (dottedFontId == -1)
    {
        qWarning() << "Failed to load custom font " << fontName;
    }
    else
    {
        qDebug() << "Added custom font:" << QFontDatabase::applicationFontFamilies(dottedFontId);
    }

    Wag9Fs fs{};
    fs.init(staticConfig);
    fs.show();

    return app.exec();
}
