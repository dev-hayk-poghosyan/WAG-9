#pragma once

#include "ui_Wag9FsInfoPanel.h"
#include "AutomatonButton.h"
#include "Wag9FaultStation.pb.h"
#include "KeySwitchButton.h"
#include "HitachiButton.h"
#include "Wag9Vehicle.h"
#include <QFrame>
#include <QAudioDeviceInfo>
#include <QSoundEffect>

struct PanelsHasher
{
    using argument_type = std::tuple<Wag9Zone, int, Wag9Side>;
    size_t operator()(argument_type const& key) const;
};

class Wag9FsInfoPanel : public QFrame
{
    Q_OBJECT
public:
    Wag9FsInfoPanel(QWidget* parent = nullptr);

    void initialize(Wag9Zone zone, int refNumber, Wag9Side side, QAudioDeviceInfo* soundEffects);

    void setNumberTitle(const QString& title);
    void setTitleText(const QString& text);
    void setDebugPanelVisible(bool visible);
    PanelsHasher::argument_type key() const;

    Wag9Zone zone() const;
    Wag9Side side() const;
    QString vehicleId() const;
    void setVehicleId(QString const& vehicleId);
    int refNumber() const;

    virtual void processIncomingMessage(const proto::wag9::fs::FaultStationIncomingMessage&) {};
    virtual void writeStateToProto(proto::wag9::fs::FaultStationOutgoingMessage&) const {};

signals:
    void closeRequested();
    void someStateChanged();

protected:
    virtual void buildUi(QWidget& container) = 0;
    void setBackground(QWidget& container, const QString& url);
    QWidget* changeSourceControl() const { return m_changeSourceControl; }

    void handleSwitchClicked(QWidget* button)
    {
        m_changeSourceControl = button;
        emit someStateChanged();
    }

    std::unique_ptr<KeySwitchButton> createSwitchButtonSymmetry(QStringList animationOnList,
        const QRect& controlButtonGeometry);
    std::unique_ptr<KeySwitchButton> createSwitchButton(QStringList animationOnList,
        QStringList animationOffList, const QRect& controlButtonGeometry);

    std::unique_ptr<QPushButton> createLamp(const QRect& geometry,
        const QString& offPixmap, const QString& onPixmap);

    std::unique_ptr<QPushButton> createLamp(const QRect& geometry, const QIcon& icon);

    std::unique_ptr<QPushButton> createSimpleButton(const QRect& geometry, const QIcon& icon, bool checked = false);
    std::unique_ptr<QPushButton> createSimpleButton(const QRect& geometry,
        const QString& offPixmap, const QString& onPixmap, bool checked = false);

    std::unique_ptr<QLabel> createStaticLabel(const QString&text, const QRect& geometry);

    std::unique_ptr<HitachiButton> createPressButton(const QRect& geometry,
        const QString& releasedPixmap, const QString& pressedPixmap);

    std::unique_ptr<HitachiButton> createButton(const QRect& geometry,
        const QString& offPixmap, const QString& onPixmap,
        const QString& pressedOffPixmap, const QString& pressedOnPixmap);

    std::unique_ptr<AutomatonButton> createAutomatonButton(const QRect& geometry,
        const QIcon& icon, const QString& name);

    void togglePrimarySoundEffect(const QString& source, bool play = true) const;
    void toggleSecondarySoundEffect(const QString& source, bool play = true) const;

    void playAirDropSound(bool play) const;
    void playValveToggleSound(bool opened) const;

    void playPressSound(bool down) const;

private:
    Ui::Wag9FsInfoPanel ui;

    QString m_vehicleId{};
    int m_refNumber{};
    Wag9Zone m_zone{};
    Wag9Side m_side{};
    QAudioDeviceInfo* m_soundEffectsOutputDevice = nullptr;

    QWidget* m_changeSourceControl = nullptr;

    QSoundEffect* m_soundEffectsPrimary  = nullptr;
    QSoundEffect* m_soundEffectsSecondary = nullptr;
};

QDebug operator<< (QDebug d, const PanelsHasher::argument_type& key);
