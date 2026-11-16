#pragma once

#include "ControlButtonState.h"

#include <QMouseEvent>
#include <QFrame>
#include <QPushButton>

/**
 * Common button with a lamp on the 80x panel.
 */
class HitachiButton : public QPushButton
{
    Q_OBJECT
public:
    HitachiButton(QWidget* parent = nullptr);

    void setState(ControlButtonState state);
    void setPressed(bool pressed);

    ControlButtonState state() const;

    void setControlButtonGeometry(const QRect &rect);
    void setLampState(bool on);
    void setReadOnly(bool readOnly);
    void setSilent(bool silent);

signals:
    void stateChanged(ControlButtonState state);

protected:
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual bool event(QEvent* event) override;

private:
    void handlePressEvent();
    void handleReleaseEvent();
    void ensureHitArea();

    bool m_readOnly = false;
    bool m_blockSounds = false;
    bool m_silent = false;
    ControlButtonState m_state = ControlButtonState::Indeterminate;

    QPushButton *m_controlButton = nullptr;
    QRect m_initialGeometry;
    bool m_initialGeometryCaptured = false;
    QPoint m_childOffset = QPoint(0, 0);
    QRect m_lastControlRect;
};
