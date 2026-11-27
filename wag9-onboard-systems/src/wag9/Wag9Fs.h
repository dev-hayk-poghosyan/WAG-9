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
class Wag9StatusSprite;

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
        void setBrakeSystem(BrakeSystem system);
        void setBrakeSystemLocked(bool locked);
        QWidget *getPneumoPanel() const;
        void createPantographs();
        KeySwitchButton* addPantographView(const QStringList& frames, QPushButton* clickArea);
        static void setPantographPair(KeySwitchButton* a, KeySwitchButton* b, bool raised);
        void createBlowersAndVcb();
        void setVcbFailure(bool failure);
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
        BrakeSystem m_brakeSystem = BrakeSystem::Ccb;
        QTimer m_reportTimer;
        KeySwitchButton* m_pantograph1[2] = {};
        KeySwitchButton* m_pantograph2[2] = {};
        // Index 0 is blower 1, index 1 is blower 2 (TSD: unit 1 is fed from HB1, unit 2 from HB2).
        Wag9StatusSprite* m_tractionMotorBlower[2] = {};
        Wag9StatusSprite* m_machineRoomBlower[2] = {};
        // One VCB drawn on both views: right view, left view.
        Wag9StatusSprite* m_vcb[2] = {};
};
