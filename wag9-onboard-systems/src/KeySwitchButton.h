#pragma once

#include "ControlButtonState.h"

#include <QFrame>
#include <QPushButton>
#include <QMouseEvent>
#include <QTimer>
#include <optional>
#include <unordered_map>

/**
 * Represents a switcher with the key on the virtual panels.
 * Allows animated transition between states.
 */
class KeySwitchButton : public QFrame
{
    Q_OBJECT
public:
    explicit KeySwitchButton(QWidget* parent = nullptr);

    /**
     * Extracts the various properties given via the qt designer.
     */
    void init();

    void setState(ControlButtonState state);

    /**
     * false - ControlButtonState::Off; true - ControlButtonState::On.
     */
    void setStateNoSignal(bool state);
    void setStateNoSignal(ControlButtonState state);

    ControlButtonState state() const;

    void setAnimationOnList(QStringList list);
    void setAnimationOffList(QStringList list);
    void setControlButtonGeometry(const QRect& rect);
    void setSilent(bool silent);
    void setClickable(bool clickable);
    void setSoundFileName(const QString& soundFileNameOn, const QString& soundFileNameOff);
    void setAnimationInterval(int millseconds);

    enum class AnimationState
    {
        None = 0,
        ToOn = 1,
        ToOff = 2,
    };

    AnimationState animationState() const;

signals:
    void stateChanged(ControlButtonState state);

private: 
    void handleTimeout();

    void startOffAnimation();
    void startOnAnimation();
    void setPixmap(QString const& pxUrl);
    void setStateInternal(ControlButtonState state);

    ControlButtonState m_state = ControlButtonState::Off;
    int m_animationIdx = 0;
    bool m_silent = false;
    bool m_noSignalState = false;

    QTimer m_timer;
    QStringList m_animationListOn;
    QStringList m_animationListOff;
    QStringList* m_currentAnimationList = nullptr;
    QPushButton* m_controlButton = nullptr;
    QString m_soundFileNameOn;
    QString m_soundFileNameOff;

    static const char* const m_animationListOnProperty;
    static const char* const m_animationListOffProperty;
    static const char* const m_animationTimeoutProperty;
    static const char* const m_controlGeometryRectProperty;
};

/**
 * A simple cyclic state switch without animation.
 */
class DeterminateStateSwitchButton : public QFrame
{
    Q_OBJECT
public:
    explicit DeterminateStateSwitchButton(QWidget* parent = nullptr);

    void setStateNoSignal(int state);
    int state() const;

    /**
     * Adds a new state and corresponding pixmap. @p state must be positive.
     */
    void addState(int state, const QString& px);
    void setControlButtonGeometry(const QRect& rect);
    void setTransitionSound(const QString& url);

    void initFromProperties();

    void tryTransiteToNextState();
    void tryTransiteToPrevState();

signals:
    void stateChanged(int state);

protected:
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void setPixmap(QString const& pxUrl);
    void setStateInternal(int state);
    void transiteToNextState();

    int m_state = -1;
    QString m_transitionSound;
    bool m_noSignalState = false;

    QPushButton* m_controlButton = nullptr;

    std::unordered_map<int, QString> m_pixmapsByStates;

    std::optional<QPoint> m_dragTransiteNextDirection;
    std::optional<int> m_dragTransitionDelta;
    std::optional<QPoint> m_dragStartPosition;
};


/**
 * A dual state switch button with pressed/freed states for each main state.
 * Similar to DeterminateStateSwitchButton but with secondary pressed state.
 */
class DeterminedDualStateSwitchButton : public QFrame
{
    Q_OBJECT
public:
    explicit DeterminedDualStateSwitchButton(QWidget* parent = nullptr);

    int state() const;
    int secondaryState() const; // Returns 0 for freed, 1 for pressed

    /**
     * Adds a new state with freed pixmap. State is stored as int.
     * @p state is the state number (can be any int)
     * @p freedPx is the pixmap when button is not pressed
     */
    void addState(int state, const QString& freedPx);
    
    /**
     * Adds pressed pixmap for an existing state.
     * @p state is the state number
     * @p pressedPx is the pixmap when button is pressed
     */
    void addPressedPixmap(int state, const QString& pressedPx);
    void setControlButtonGeometry(const QRect& rect);
    void setTransitionSound(const QString& url);

    void setState(int state);
    void setStateNoSignal(int state);
    void setSecondaryState(int secondary);
    void tryTransiteToNextState();
    void tryTransiteToPrevState();

signals:
    void stateChanged(int state);

protected:
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void setPixmap(QString const& pxUrl);
    void setStateInternal(int state);
    void updatePixmap();
    void transiteToNextState();

    int m_state = -1;
    int m_secondary = 0; // 0 = freed, 1 = pressed
    QString m_transitionSound;
    bool m_noSignalState = false;

    QPushButton* m_controlButton = nullptr;

    // Hash for std::pair<int,int>
    struct PairHash
    {
        std::size_t operator()(std::pair<int, int> p) const
        {
            return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
        }
    };
    std::unordered_map<std::pair<int, int>, QString, PairHash> m_pixmapsByStates; // {state, secondary} -> pixmap

    std::optional<QPoint> m_dragTransiteNextDirection;
    std::optional<int> m_dragTransitionDelta;
    std::optional<QPoint> m_dragStartPosition;
};
