#include "Wag9Fs.h"
#include "Wag9CubiclePanel.h"
#include "Wag9PneumoPanel.h"
#include "Wag9PneumoE70Panel.h"

#include <QDebug>
#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QPushButton>
#include <QRegularExpression>
#include <QStringList>
#include <QButtonGroup>

#include <algorithm>

namespace
{

QStringList pantographFrames(const QString& direction)
{
    QStringList frames;
    for (int i = 1; i <= 10; ++i)
    {
        frames << QStringLiteral(":/Wag9/fs/panto/%1/%2.png").arg(direction).arg(i, 2, 10, QLatin1Char('0'));
    }
    return frames;
}

}

Wag9Fs::Wag9Fs(QWidget *parent)
    : QFrame(parent)
    , m_panelFactory(std::make_unique<Wag9::PanelFactory>())
{}

void Wag9Fs::init(std::shared_ptr<Wag9Configuration const> config)
{
    m_staticConfig = std::move(config);

    ui.setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint);
    setWindowFlag(Qt::WindowStaysOnTopHint, m_staticConfig->fsSettings().topMost);
    move(m_staticConfig->fsSettings().position);
    prepareZones();
    createPantographs();
    const QStringList zones = {
        QStringLiteral("SB1"),
        QStringLiteral("HB1"),
        QStringLiteral("SB2"),
        QStringLiteral("HB2")
    };
    m_popups.insert(QStringLiteral("PT"), new Wag9PneumoPanel(this));
    m_popups["PT"]->hide();
    m_popups.insert(QStringLiteral("E70"), new Wag9PneumoE70Panel(this));
    m_popups["E70"]->hide();
    for (const QString& zone : zones)
    {
        m_popups.insert(zone, new Wag9CubiclePanel(zone, this));
        m_popups[zone]->hide();
    }
    ui.background->lower();
    ui.pneumoCcb->raise();
    ui.pneumoE70->raise();
    ui.hintsButton->raise();

    createServer();
}

void Wag9Fs::prepareZones()
{
    m_linkButtons = findChildren<QPushButton*>(QRegularExpression(QStringLiteral("^link_")));
    for (QPushButton* button : m_linkButtons)
    {
        button->setFocusPolicy(Qt::NoFocus);
        button->setCursor(Qt::PointingHandCursor);
        connect(button, &QPushButton::clicked, this, [this, button](bool checked)
        {
            if (!checked)
            {
                closeFrame();
                return;
            }

            for (QPushButton* other : m_linkButtons)
            {
                if (other != button)
                {
                    other->setChecked(false);
                }
            }

            QString zone = button->property("zone").toString();
            if (zone == QLatin1String("PT") && ui.pneumoE70->isChecked())
            {
                zone = QStringLiteral("E70");
            }

            QWidget* panel = m_popups.value(zone);
            if (panel)
            {
                showPopup(panel);
            }
        });
    }

    ui.frameView->installEventFilter(this);

    const auto hintLabels = findChildren<QLabel*>(QRegularExpression(QStringLiteral("^hint_")));
    for (QLabel* label : hintLabels)
    {
        label->setAttribute(Qt::WA_TransparentForMouseEvents);
    }

    connect(ui.hintsButton, &QPushButton::toggled, this, [this, hintLabels](bool enabled)
    {
        for (QLabel* label : hintLabels)
        {
            label->setVisible(enabled);
            if (enabled)
            {
                label->raise();
            }
        }

        if (ui.frameView->isVisible())
        {
            ui.frameView->raise();
        }
        for (QWidget* popup : m_popups)
        {
            if (popup->isVisible())
            {
                popup->raise();
            }
        }
    });

    auto *pneumoModes = new QButtonGroup(this);
    pneumoModes->setExclusive(true);
    pneumoModes->addButton(ui.pneumoCcb);
    pneumoModes->addButton(ui.pneumoE70);

    const auto applyPneumBackground = [this]()
    {
        const bool e70 = ui.pneumoE70->isChecked();
        ui.background->setPixmap(e70
            ? QStringLiteral(":/Wag9/fs/WAG9_FS_Background_E70.png")
            : QStringLiteral(":/Wag9/fs/WAG9_FS_Background_CCB.png"));
    };

    const auto onModeToggled = [applyPneumBackground](bool checked)
    {
        if (checked)
        {
            applyPneumBackground();
        }
    };

    connect(ui.pneumoCcb, &QPushButton::toggled, this, onModeToggled);
    connect(ui.pneumoE70, &QPushButton::toggled, this, onModeToggled);
}

void Wag9Fs::createPantographs()
{
    const auto addView = [this](const QStringList& frames, QPushButton* clickArea)
    {
        QStringList framesDown = frames;
        std::reverse(framesDown.begin(), framesDown.end());

        auto view = new KeySwitchButton(this);
        view->setAnimationOnList(frames);
        view->setAnimationOffList(framesDown);
        view->setGeometry(clickArea->geometry());
        view->setControlButtonGeometry(view->rect());
        view->setSilent(true);
        view->init();
        view->setClickable(false);
        view->show();

        clickArea->setFocusPolicy(Qt::NoFocus);
        clickArea->setCursor(Qt::PointingHandCursor);
        clickArea->raise();
        connect(clickArea, &QPushButton::clicked, view, [view]
        {
            if (view->animationState() != KeySwitchButton::AnimationState::None)
            {
                return;
            }

            view->setState(view->state() == ControlButtonState::On
                ? ControlButtonState::Off
                : ControlButtonState::On);
        });
    };

    const QStringList pantograph1 = pantographFrames(QStringLiteral("left"));
    const QStringList pantograph2 = pantographFrames(QStringLiteral("right"));

    addView(pantograph1, ui.pan_right_1);
    addView(pantograph1, ui.pan_left_1);
    addView(pantograph2, ui.pan_right_2);
    addView(pantograph2, ui.pan_left_2);
}

void Wag9Fs::showPopup(QWidget* panel)
{
    hidePopups();

    m_frameRect = QRect(
        (width() - panel->width()) / 2,
        (height() - panel->height()) / 2,
        panel->width(),
        panel->height());

    ui.frameView->show();
    ui.frameView->raise();
    panel->setGeometry(m_frameRect);
    panel->show();
    panel->raise();
}

void Wag9Fs::hidePopups()
{
    for (QWidget* popup : m_popups)
    {
        popup->hide();
    }
}

void Wag9Fs::closeFrame()
{
    for (QPushButton* button : m_linkButtons)
    {
        button->setChecked(false);
    }

    hidePopups();
    ui.frameView->hide();
}

bool Wag9Fs::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == ui.frameView && event->type() == QEvent::MouseButtonPress)
    {
        const auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (!m_frameRect.contains(mouseEvent->pos()))
        {
            closeFrame();
            return true;
        }
    }

    return QFrame::eventFilter(watched, event);
}

void Wag9Fs::createServer()
{
    try
    {
        m_server = std::make_unique<Wag9FsServer>(m_staticConfig->fsSettings().bindAddress);
    }
    catch (const std::exception& ex)
    {
        qCritical() << "Failed to create server" << ex.what();
        return;
    }

    connect(m_server.get(), &Wag9FsServer::initMessageReceived, this, &Wag9Fs::handleInitMessage);
    connect(m_server.get(), &Wag9FsServer::messageReceived, this, &Wag9Fs::handleIncomingPackageFromSim);

    m_reportTimer.setInterval(100);
    connect(&m_reportTimer, &QTimer::timeout, this, &Wag9Fs::reportStationState);
    m_reportTimer.start();
}

void Wag9Fs::reportStationState()
{
}

void Wag9Fs::handleInitMessage(const proto::FaultStationInitMessage& msg)
{
    qDebug() << "Init from sim, vehicles:" << msg.vehicle_numbers_size();
}

void Wag9Fs::handleIncomingPackageFromSim(const proto::wag9::fs::FaultStationIncomingMessage&)
{
}