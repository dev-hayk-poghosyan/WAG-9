#include "HitachiButton.h"
#include <QSound>
#include <QVariant>
#include <QDebug>

HitachiButton::HitachiButton(QWidget* parent)
    : QPushButton(parent)
{
    m_controlButton = new QPushButton(this);
    m_controlButton->setEnabled(false);
    m_controlButton->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_controlButton->setFocusPolicy(Qt::NoFocus);
    m_controlButton->setFlat(true);
    m_controlButton->setStyleSheet(QStringLiteral("background: transparent; border: none;"));

    setText(QString());
    setAttribute(Qt::WA_AcceptTouchEvents, true);
    setAttribute(Qt::WA_TouchPadAcceptSingleTouchEvents, true);
}

void HitachiButton::setState(ControlButtonState state)
{
    if (m_state == state)
        return;
    if (!m_initialGeometryCaptured)
    {
        m_initialGeometry = geometry();
        m_initialGeometryCaptured = true;
    }

    const std::string stateStr = controlButtonStateToString(state);
    const QVariant iconFileNameVariant = property(stateStr.c_str());
    const QVariant iconControlRectVariant = property("ctrlRect");

    if (iconFileNameVariant.isNull())
    {
        qWarning() << objectName() <<
            ": failed to get property icon:" << QString::fromStdString(stateStr);
        return;
    }

    setIcon(QPixmap(iconFileNameVariant.toString()));
    setIconSize(m_initialGeometryCaptured ? m_initialGeometry.size() : size());
    m_state = state;

    if (!m_blockSounds && !m_silent &&
        (m_state == ControlButtonState::PressedOff || m_state == ControlButtonState::PressedOn))
    {
        const QVariant soundPlayData = property("Sound");
        const QString sound = soundPlayData.isNull()
            ? QStringLiteral(":/sounds/ButtonPult_Press.wav")
            : soundPlayData.toString();

        QSound::play(sound);
    }

    if (!iconControlRectVariant.isNull())
    {   
        setControlButtonGeometry(iconControlRectVariant.toRect());
    }

    emit stateChanged(m_state);
}

void HitachiButton::setControlButtonGeometry(const QRect &rect)
{
    m_lastControlRect = rect;
    ensureHitArea();
    m_controlButton->setGeometry(rect.translated(m_childOffset));
}

void HitachiButton::setPressed(bool pressed)
{
    if (!pressed && (m_state == ControlButtonState::On || m_state == ControlButtonState::Off))
    {
        // Already unpressed.
        return;
    }

    if (pressed && (m_state == ControlButtonState::PressedOn || m_state == ControlButtonState::PressedOff))
    {
        // Already pressed.
        return;
    }

    setState(pressed
        ? (m_state == ControlButtonState::On ? ControlButtonState::PressedOn : ControlButtonState::PressedOff)
        : (m_state == ControlButtonState::PressedOn ? ControlButtonState::On : ControlButtonState::Off)
    );
}

ControlButtonState HitachiButton::state() const
{
    return m_state;
}

void HitachiButton::setLampState(bool on)
{
    blockSignals(true);
    m_blockSounds = true;
    if (m_state == ControlButtonState::On || m_state == ControlButtonState::Off)
    {
        setState(on ? ControlButtonState::On : ControlButtonState::Off);
    }
    else if (m_state == ControlButtonState::PressedOn || m_state == ControlButtonState::PressedOff)
    {
        setState(on ? ControlButtonState::PressedOn : ControlButtonState::PressedOff);
    }
    m_blockSounds = false;
    blockSignals(false);
}

void HitachiButton::setReadOnly(bool readOnly)
{
    m_readOnly = readOnly;
}

void HitachiButton::setSilent(bool silent)
{
    m_silent = silent;
}

void HitachiButton::mousePressEvent(QMouseEvent* event)
{
    if (event->source() == Qt::MouseEventSynthesizedBySystem)
    {
        event->accept();
        return;
    }

    if (m_readOnly)
    {
        return;
    }

    QPushButton::mousePressEvent(event);
    handlePressEvent();
}

void HitachiButton::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->source() == Qt::MouseEventSynthesizedBySystem)
    {
        event->accept();
        return;
    }

    if (m_readOnly)
    {
        return;
    }

    QPushButton::mouseReleaseEvent(event);
    handleReleaseEvent();
}

bool HitachiButton::event(QEvent* event)
{
    if (event->type() == QEvent::TouchBegin)
    {
        handlePressEvent();
        event->accept();

        return true;
    }
    else if (event->type() == QEvent::TouchEnd)
    {
        handleReleaseEvent();
        event->accept();

        return true;
    }

    return QPushButton::event(event);
}

void HitachiButton::handlePressEvent()
{
    if (m_readOnly)
    {
        return;
    }

    if (m_state == ControlButtonState::Off)
    {
        setState(ControlButtonState::PressedOff);
    }
    else if (m_state == ControlButtonState::On)
    {
        setState(ControlButtonState::PressedOn);
    }
    else if (m_state == ControlButtonState::PressedOff)
    {
        setState(ControlButtonState::Off);
    }
    else if (m_state == ControlButtonState::PressedOn)
    {
        setState(ControlButtonState::On);
    }
}

void HitachiButton::handleReleaseEvent()
{
    if (isCheckable())
    {
        return;
    }

    if (m_state == ControlButtonState::PressedOff)
    {
        setState(ControlButtonState::Off);
    }
    else if (m_state == ControlButtonState::PressedOn)
    {
        setState(ControlButtonState::On);
    }
}

void HitachiButton::ensureHitArea()
{
    if (!m_initialGeometryCaptured)
    {
        m_initialGeometry = geometry();
        m_initialGeometryCaptured = true;
    }

    QRect visualRect(QPoint(0, 0), m_initialGeometry.size());
    QRect desiredRect = visualRect;
    if (m_lastControlRect.isValid())
    {
        desiredRect = desiredRect.united(m_lastControlRect);
    }

    QSize newSize = desiredRect.size();

    QRect currentGeom = geometry();
    if (currentGeom.size() == newSize)
    {
        m_childOffset = m_initialGeometry.topLeft() - currentGeom.topLeft();
        return;
    }

    QRect newGeom(QPoint(0, 0), newSize);
    newGeom.moveCenter(m_initialGeometry.center());
    setGeometry(newGeom);

    m_childOffset = m_initialGeometry.topLeft() - newGeom.topLeft();
}
