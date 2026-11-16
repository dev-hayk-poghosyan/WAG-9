#pragma once

#include "ui_Wag9Fs.h"
#include "Wag9Configuration.h"
#include "Wag9FsServer.h"
#include "Wag9PanelFactory.h"
#include "KeySwitchButton.h"
#include "FaultStationMessages.pb.h"

#include <QFrame>
#include <QTimer>

#include <memory>

class Wag9PneumoPanel;

class Wag9Fs : public QFrame
{
    Q_OBJECT

    public:
        explicit Wag9Fs(QWidget *parent = nullptr);
        void init(std::shared_ptr<Wag9Configuration const> config);

    protected:
        bool eventFilter(QObject* watched, QEvent* event) override;

    private:
        void prepareZones();
        void createPantographs();
        void showFrame(const QString& resourcePath);
        void showPneumo();
        void closeFrame();
        void createServer();
        void reportStationState();
        void handleInitMessage(const proto::FaultStationInitMessage& msg);
        void handleIncomingPackageFromSim(const proto::wag9::fs::FaultStationIncomingMessage& msg);

        Ui::Wag9Fs ui;
        std::shared_ptr<Wag9Configuration const> m_staticConfig;
        std::unique_ptr<Wag9FsServer> m_server;
        std::unique_ptr<Wag9::PanelFactory> m_panelFactory;
        QRect m_frameRect;
        Wag9PneumoPanel* m_pneumoPanel = nullptr;
        QTimer m_reportTimer;
};
