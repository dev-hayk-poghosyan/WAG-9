#pragma once

#include "ui_Wag9Fs.h"
#include "Wag9Configuration.h"
#include "Wag9FsServer.h"
#include "KeySwitchButton.h"
#include "FaultStationMessages.pb.h"

#include <QFrame>
#include <QHash>
#include <QList>
#include <QTimer>
#include <QWidget>

#include <memory>

class QAbstractButton;
class QLabel;
class QPushButton;
class Wag9PneumoPanel;
class Wag9PneumoE70Panel;

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
        void onLinkClicked(QAbstractButton* button);
        void setHintsVisible(bool enabled);
        void onPneumoModeToggled(bool checked);
        void createPantographs();
        KeySwitchButton* addPantographView(const QStringList& frames, QPushButton* clickArea);
        static void togglePantographPair(KeySwitchButton* a, KeySwitchButton* b);
        void showPopup(QWidget* panel);
        void hidePopups();
        void closeFrame();
        void createServer();
        void reportStationState();
        void handleInitMessage(const proto::FaultStationInitMessage& msg);
        void handleIncomingPackageFromSim(const proto::wag9::fs::FaultStationIncomingMessage& msg);

        Ui::Wag9Fs ui;
        std::shared_ptr<Wag9Configuration const> m_staticConfig;
        std::unique_ptr<Wag9FsServer> m_server;
        QRect m_frameRect;
        QList<QPushButton*> m_linkButtons;
        QList<QLabel*> m_hintLabels;
        QHash<QString, QWidget*> m_popups;
        Wag9PneumoPanel* m_ccbPanel = nullptr;
        Wag9PneumoE70Panel* m_e70Panel = nullptr;
        QTimer m_reportTimer;
        KeySwitchButton* m_pantograph1[2] = {};
        KeySwitchButton* m_pantograph2[2] = {};
};
