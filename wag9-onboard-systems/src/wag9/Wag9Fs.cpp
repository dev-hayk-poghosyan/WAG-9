#include "Wag9Fs.h"
#include "Wag9PneumoPanel.h"

#include <QDebug>
#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QPushButton>
#include <QRegularExpression>

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

QList<QPushButton*> linkButtons(const QWidget* root)
{
    return root->findChildren<QPushButton*>(QRegularExpression(QStringLiteral("^link_")));
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
    m_pneumoPanel = new Wag9PneumoPanel(this);
    m_pneumoPanel->hide();
    ui.background->lower();
    ui.hintsButton->raise();

    createServer();
}

void Wag9Fs::prepareZones()
{
    for (QPushButton* button : linkButtons(this))
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

            for (QPushButton* other : linkButtons(this))
            {
                if (other != button)
                {
                    other->setChecked(false);
                }
            }
            if (button == ui.link_PT)
            {
                showPneumo();
            }
            else
            {
                showFrame(button->property("frame").toString());
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
        if (m_pneumoPanel && m_pneumoPanel->isVisible())
        {
            m_pneumoPanel->raise();
        }
    });
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

void Wag9Fs::showFrame(const QString& resourcePath)
{
    m_pneumoPanel->hide();

    QPixmap pix(resourcePath);
    const QSize maxSize = size() * 0.9;
    if (pix.width() > maxSize.width() || pix.height() > maxSize.height())
    {
        pix = pix.scaled(maxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    m_frameRect = QRect(QPoint((width() - pix.width()) / 2, (height() - pix.height()) / 2), pix.size());

    ui.frameView->setPixmap(pix);
    ui.frameView->show();
    ui.frameView->raise();
}

void Wag9Fs::showPneumo()
{
    m_frameRect = QRect(
        (width() - m_pneumoPanel->width()) / 2,
        (height() - m_pneumoPanel->height()) / 2,
        m_pneumoPanel->width(),
        m_pneumoPanel->height());

    ui.frameView->setPixmap(QPixmap());
    ui.frameView->show();
    ui.frameView->raise();
    m_pneumoPanel->setGeometry(m_frameRect);
    m_pneumoPanel->show();
    m_pneumoPanel->raise();
}

void Wag9Fs::closeFrame()
{
    for (QPushButton* button : linkButtons(this))
    {
        button->setChecked(false);
    }

    m_pneumoPanel->hide();
    ui.frameView->hide();
    ui.frameView->setPixmap(QPixmap());
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