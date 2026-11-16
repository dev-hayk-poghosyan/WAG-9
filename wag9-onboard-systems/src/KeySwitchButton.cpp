#include "KeySwitchButton.h"
#include <QSound>
#include <QVariant>
#include <QDebug>
#include <vector>
#include <algorithm>

const char* const KeySwitchButton::m_animationListOnProperty = "AnimationListOn";
const char* const KeySwitchButton::m_animationListOffProperty = "AnimationListOff";
const char* const KeySwitchButton::m_animationTimeoutProperty = "AnimationTimeout";
const char* const KeySwitchButton::m_controlGeometryRectProperty = "ControlButtonGeometry";

KeySwitchButton::KeySwitchButton(QWidget* parent)
    : QFrame(parent)
    , m_soundFileNameOn{ ":/sounds/ButtonBig_Press.wav" }
    , m_soundFileNameOff{ ":/sounds/ButtonBig_Press.wav" }
{
    m_timer.setInterval(60);
    connect(&m_timer, &QTimer::timeout, this, &KeySwitchButton::handleTimeout);
}

void
KeySwitchButton::init()
{
    m_animationListOn = property(m_animationListOnProperty).toStringList();
    m_animationListOff = property(m_animationListOffProperty).toStringList();
    const QVariant controlRect = property(m_controlGeometryRectProperty);

    m_controlButton = new QPushButton(this);
    if (!property(m_animationTimeoutProperty).isNull())
    {
        m_timer.setInterval(property(m_animationTimeoutProperty).toInt());
    }

    m_controlButton->setGeometry(!controlRect.isNull() ? controlRect.toRect() : rect());

    setPixmap(m_animationListOn.at(0));

    connect(m_controlButton, &QPushButton::clicked, this, [this]
    {
        if (m_state == ControlButtonState::Off)
        {
            startOnAnimation();
        }
        else if (m_state == ControlButtonState::On)
        {
            startOffAnimation();
        }
    });
}

void
KeySwitchButton::setState(ControlButtonState state)
{
    if (state == m_state)
        return;

    if (state == ControlButtonState::Off)
    {
        startOffAnimation();
    }
    else if (state == ControlButtonState::On)
    {
        startOnAnimation();
    }
}


void KeySwitchButton::setStateNoSignal(ControlButtonState state)
{
    m_noSignalState = true;
    setState(state);
}

void KeySwitchButton::setStateNoSignal(bool state)
{
    setStateNoSignal(state ? ControlButtonState::On : ControlButtonState::Off);
}

ControlButtonState KeySwitchButton::state() const
{
    return m_state;
}

void KeySwitchButton::setAnimationOnList(QStringList list)
{
    setProperty(m_animationListOnProperty, list);
}

void KeySwitchButton::setAnimationOffList(QStringList list)
{
    setProperty(m_animationListOffProperty, list);
}

void KeySwitchButton::setControlButtonGeometry(const QRect& rect)
{
    setProperty(m_controlGeometryRectProperty, rect);
}

void KeySwitchButton::setSilent(bool silent)
{
    m_silent = silent;
}

void KeySwitchButton::setClickable(bool clickable)
{
    m_controlButton->setEnabled(clickable);
}

void KeySwitchButton::setSoundFileName(const QString& on, const QString& off)
{
    m_soundFileNameOn = on;
    m_soundFileNameOff = off;
}

void KeySwitchButton::setAnimationInterval(int millseconds)
{
    m_timer.setInterval(millseconds);
}

KeySwitchButton::AnimationState
KeySwitchButton::animationState() const
{
    if (!m_timer.isActive())
    {
        return AnimationState::None;
    }

    if (m_currentAnimationList == &m_animationListOff)
    {
        return AnimationState::ToOff;
    }

    if (m_currentAnimationList == &m_animationListOn)
    {
        return AnimationState::ToOn;
    }

    return AnimationState::None;
}

void
KeySwitchButton::handleTimeout()
{
    setPixmap(m_currentAnimationList->at(m_animationIdx));

    ++m_animationIdx;

    if (!m_silent && (m_animationIdx == m_currentAnimationList->size() / 2 + 1))
    {
        QSound::play(m_currentAnimationList == &m_animationListOn
            ? m_soundFileNameOn
            : m_soundFileNameOff);
    }

    if (m_animationIdx == m_currentAnimationList->size())
    {
        m_timer.stop();
        setStateInternal(m_currentAnimationList == &m_animationListOn
            ? ControlButtonState::On
            : ControlButtonState::Off);
    }
}

void
KeySwitchButton::startOffAnimation()
{
    m_currentAnimationList = &m_animationListOff;
    m_animationIdx = 0;

    m_timer.start();
}

void
KeySwitchButton::startOnAnimation()
{
    m_currentAnimationList = &m_animationListOn;
    m_animationIdx = 0;

    m_timer.start();
}

void
KeySwitchButton::setPixmap(QString const& pxUrl)
{
    static const auto templateStr =
        QStringLiteral(".KeySwitchButton{border-image: url(\"%1\") 0 0 0 0 no-repeat no-repeat;}");

    setStyleSheet(templateStr.arg(pxUrl));
}

void
KeySwitchButton::setStateInternal(ControlButtonState state)
{
    m_state = state;
    if (!m_noSignalState)
    {
        emit stateChanged(m_state);
    }

    m_noSignalState = false;
}


DeterminateStateSwitchButton::DeterminateStateSwitchButton(QWidget* parent)
    : QFrame(parent)
{
    m_controlButton = new QPushButton(this);
    connect(m_controlButton, &QPushButton::clicked,
        this, &DeterminateStateSwitchButton::transiteToNextState);
}

void
DeterminateStateSwitchButton::setStateNoSignal(int state)
{
    m_noSignalState = true;
    setStateInternal(state);
    m_noSignalState = false;
}

int DeterminateStateSwitchButton::state() const
{
    return m_state;
}

void
DeterminateStateSwitchButton::addState(int state, const QString& px)
{
    m_pixmapsByStates[state] = px;
}

void
DeterminateStateSwitchButton::setControlButtonGeometry(const QRect& rect)
{
    m_controlButton->setGeometry(rect);
}

void
DeterminateStateSwitchButton::setTransitionSound(const QString& url)
{
    m_transitionSound = url;
}

void
DeterminateStateSwitchButton::initFromProperties()
{
    const auto stateList = property("states").toStringList();
    const auto initState = property("initState").toInt();
    const auto controlRect = property("ctrlRect").toRect();

    const auto dragTransVector = property("transitionNextDragVec");
    const auto dragTransDelta = property("transitionDelta");
    if (!dragTransVector.isNull())
    {
        m_dragTransiteNextDirection = dragTransVector.toPoint();
    }
    if (!dragTransDelta.isNull())
    {
        m_dragTransitionDelta = dragTransDelta.toInt();
    }

    for (int idx = 0; idx < stateList.count(); ++idx)
    {
        addState(idx, stateList[idx]);
    }

    setStateNoSignal(initState);
    setControlButtonGeometry(controlRect);
}

void
DeterminateStateSwitchButton::mouseMoveEvent(QMouseEvent* event)
{
    QFrame::mouseMoveEvent(event);

    if (!m_dragStartPosition || !m_dragTransiteNextDirection || !m_dragTransitionDelta)
    {
        return;
    }

    const auto moveVector = *m_dragStartPosition - event->pos();

    if (moveVector.manhattanLength() < *m_dragTransitionDelta)
    {
        return;
    }

    if (QPoint::dotProduct(moveVector, *m_dragTransiteNextDirection) > 0)
    {
        tryTransiteToNextState();
    }
    else
    {
        tryTransiteToPrevState();
    }

    m_dragStartPosition = event->pos();
}

void
DeterminateStateSwitchButton::mousePressEvent(QMouseEvent* event)
{
    QFrame::mousePressEvent(event);
    if (m_dragTransiteNextDirection && event->button() == Qt::LeftButton)
    {
        m_dragStartPosition = event->pos();
    }
}

void
DeterminateStateSwitchButton::mouseReleaseEvent(QMouseEvent* event)
{
    QFrame::mouseReleaseEvent(event);
    m_dragStartPosition.reset();
}

void
DeterminateStateSwitchButton::setPixmap(QString const& pxUrl)
{
    static const auto templateStr =
        QStringLiteral(".DeterminateStateSwitchButton{border-image: url(\"%1\") 0 0 0 0 no-repeat no-repeat;}");

    setStyleSheet(templateStr.arg(pxUrl));
}


void
DeterminateStateSwitchButton::setStateInternal(int state)
{
    auto itState = m_pixmapsByStates.find(state);
    if (itState == m_pixmapsByStates.end())
    {
        qWarning() << "Unknown state requested:" << state
            << " in DeterminateStateSwitchButton " << objectName();

        return;
    }

    setPixmap(itState->second);
    m_state = state;

    if (!m_transitionSound.isEmpty())
    {
        QSound::play(m_transitionSound);
    }

    if (!m_noSignalState)
    {
        emit stateChanged(m_state);
    }
}


void
DeterminateStateSwitchButton::transiteToNextState()
{
    auto itState = m_pixmapsByStates.find(m_state);
    if (itState == m_pixmapsByStates.end())
    {
        qWarning() << "Can't transiteToNextState requested:" << m_state
            << " in DeterminateStateSwitchButton " << objectName();

        return;
    }

    if (itState == std::prev(m_pixmapsByStates.end()))
    {
        itState = m_pixmapsByStates.begin();
    }
    else
    {
        ++itState;
    }

    setStateInternal(itState->first);
}

void DeterminateStateSwitchButton::tryTransiteToNextState()
{
    auto itState = m_pixmapsByStates.find(m_state);
    if (itState == m_pixmapsByStates.end())
    {
        qWarning() << "Can't tryTransiteToNextState requested:" << m_state
            << " in DeterminateStateSwitchButton " << objectName();

        return;
    }

    if (itState == std::prev(m_pixmapsByStates.end()))
    {
        return;
    }

    setStateInternal(std::next(itState)->first);
}

void DeterminateStateSwitchButton::tryTransiteToPrevState()
{   
    auto itState = m_pixmapsByStates.find(m_state);
    if (itState == m_pixmapsByStates.end())
    {
        qWarning() << "Can't tryTransiteToPrevState requested:" << m_state
            << " in DeterminateStateSwitchButton " << objectName();

        return;
    }

    if (itState == m_pixmapsByStates.begin())
    {
        return;
    }

    setStateInternal(std::prev(itState)->first);
}


DeterminedDualStateSwitchButton::DeterminedDualStateSwitchButton(QWidget* parent)
    : QFrame(parent)
{
    m_controlButton = new QPushButton(this);
    connect(m_controlButton, &QPushButton::clicked,
        this, &DeterminedDualStateSwitchButton::transiteToNextState);
}

int DeterminedDualStateSwitchButton::state() const
{
    return m_state;
}

int DeterminedDualStateSwitchButton::secondaryState() const
{
    return m_secondary;
}

void DeterminedDualStateSwitchButton::addState(int state, const QString& freedPx)
{
    m_pixmapsByStates[{state, 0}] = freedPx;
}

void DeterminedDualStateSwitchButton::addPressedPixmap(int state, const QString& pressedPx)
{
    m_pixmapsByStates[{state, 1}] = pressedPx;
}

void DeterminedDualStateSwitchButton::setControlButtonGeometry(const QRect& rect)
{
    m_controlButton->setGeometry(rect);
}

void DeterminedDualStateSwitchButton::setTransitionSound(const QString& url)
{
    m_transitionSound = url;
}

void DeterminedDualStateSwitchButton::setState(int state)
{
    setStateInternal(state);
}

void DeterminedDualStateSwitchButton::setStateNoSignal(int state)
{
    m_noSignalState = true;
    setStateInternal(state);
    m_noSignalState = false;
}

void DeterminedDualStateSwitchButton::setSecondaryState(int secondary)
{
    m_secondary = secondary;
    updatePixmap();
}

void DeterminedDualStateSwitchButton::tryTransiteToNextState()
{
    std::vector<int> states;
    for (const auto& [key, value] : m_pixmapsByStates)
    {
        int state = key.first;
        if (key.second == 0)
        {
            states.push_back(state);
        }
    }
    
    if (states.empty())
    {
        return;
    }
    
    auto it = std::find(states.begin(), states.end(), m_state);
    if (it == states.end() || it == std::prev(states.end()))
    {
        return;
    }
    
    setStateInternal(*std::next(it));
}

void DeterminedDualStateSwitchButton::tryTransiteToPrevState()
{
    std::vector<int> states;
    for (const auto& [key, value] : m_pixmapsByStates)
    {
        int state = key.first;
        if (key.second == 0)
        {
            states.push_back(state);
        }
    }
    
    if (states.empty())
    {
        return;
    }
    
    auto it = std::find(states.begin(), states.end(), m_state);
    if (it == states.end() || it == states.begin())
    {
        return;
    }
    
    setStateInternal(*std::prev(it));
}

void DeterminedDualStateSwitchButton::mouseMoveEvent(QMouseEvent* event)
{
    QFrame::mouseMoveEvent(event);

    if (!m_dragStartPosition || !m_dragTransiteNextDirection || !m_dragTransitionDelta)
    {
        return;
    }

    const auto moveVector = *m_dragStartPosition - event->pos();

    if (moveVector.manhattanLength() < *m_dragTransitionDelta)
    {
        return;
    }

    if (QPoint::dotProduct(moveVector, *m_dragTransiteNextDirection) > 0)
    {
        tryTransiteToNextState();
    }
    else
    {
        tryTransiteToPrevState();
    }

    m_dragStartPosition = event->pos();
}

void DeterminedDualStateSwitchButton::mousePressEvent(QMouseEvent* event)
{
    QFrame::mousePressEvent(event);
    if (m_dragTransiteNextDirection && event->button() == Qt::LeftButton)
    {
        m_dragStartPosition = event->pos();
    }
}

void DeterminedDualStateSwitchButton::mouseReleaseEvent(QMouseEvent* event)
{
    QFrame::mouseReleaseEvent(event);
    m_dragStartPosition.reset();
}

void DeterminedDualStateSwitchButton::setPixmap(QString const& pxUrl)
{
    static const auto templateStr =
        QStringLiteral(".DeterminedDualStateSwitchButton{border-image: url(\"%1\") 0 0 0 0 no-repeat no-repeat;}");
    setStyleSheet(templateStr.arg(pxUrl));
}

void DeterminedDualStateSwitchButton::updatePixmap()
{
    auto itState = m_pixmapsByStates.find({m_state, m_secondary});
    if (itState != m_pixmapsByStates.end())
    {
        setPixmap(itState->second);
        return;
    }
    
    if (m_secondary != 0)
    {
        auto itFreed = m_pixmapsByStates.find({m_state, 0});
        if (itFreed != m_pixmapsByStates.end())
        {
            setPixmap(itFreed->second);
            return;
        }
    }
    
    qWarning() << "Unknown state/secondary combination requested:" << m_state << "/" << m_secondary
        << " in DeterminedDualStateSwitchButton " << objectName();
}

void DeterminedDualStateSwitchButton::setStateInternal(int state)
{
    auto itState = m_pixmapsByStates.find({state, 0});
    if (itState == m_pixmapsByStates.end())
    {
        qWarning() << "Unknown state requested:" << state
            << " in DeterminedDualStateSwitchButton " << objectName();
        return;
    }

    m_state = state;
    updatePixmap();

    if (!m_transitionSound.isEmpty())
    {
        QSound::play(m_transitionSound);
    }

    if (!m_noSignalState)
    {
        emit stateChanged(m_state);
    }
}

void DeterminedDualStateSwitchButton::transiteToNextState()
{
    std::vector<int> states;
    for (const auto& [key, value] : m_pixmapsByStates)
    {
        int state = key.first;
        if (key.second == 0)
        {
            states.push_back(state);
        }
    }
    
    if (states.empty())
    {
        qWarning() << "Can't transiteToNextState: no states defined"
            << " in DeterminedDualStateSwitchButton " << objectName();
        return;
    }
    
    auto it = std::find(states.begin(), states.end(), m_state);
    if (it == states.end())
    {
        qWarning() << "Can't transiteToNextState requested:" << m_state
            << " in DeterminedDualStateSwitchButton " << objectName();
        return;
    }
    
    if (it == std::prev(states.end()))
    {
        setStateInternal(states[0]);
    }
    else
    {
        setStateInternal(*std::next(it));
    }
}

