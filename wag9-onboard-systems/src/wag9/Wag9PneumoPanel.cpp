#include "Wag9PneumoPanel.h"

#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QStringList>

namespace
{

class PneumoSwitch final : public QLabel
{
public:
    PneumoSwitch(const QStringList& frames, int initial, QWidget* parent)
        : QLabel(parent)
        , m_frames(frames)
        , m_index(initial)
    {
        setCursor(Qt::PointingHandCursor);
        const QPixmap pix(m_frames.at(m_index));
        setPixmap(pix);
        setFixedSize(pix.size());
    }

protected:
    void mousePressEvent(QMouseEvent* event) override
    {
        m_index = (m_index + 1) % m_frames.size();
        setPixmap(QPixmap(m_frames.at(m_index)));
        event->accept();
    }

private:
    QStringList m_frames;
    int m_index = 0;
};

void addSwitch(QWidget* parent, const QString& sprite, int frameCount, int initial, int x, int y)
{
    QStringList frames;
    for (int frame = 1; frame <= frameCount; ++frame)
    {
        frames << QStringLiteral(":/Wag9/fs/pneumo/%1_%2.png")
            .arg(sprite)
            .arg(frame, 2, 10, QLatin1Char('0'));
    }

    auto* control = new PneumoSwitch(frames, initial, parent);
    control->move(x, y);
}

}

Wag9PneumoPanel::Wag9PneumoPanel(QWidget* parent)
    : QWidget(parent)
{
    setFixedSize(1000, 1000);

    auto* background = new QLabel(this);
    background->setPixmap(QPixmap(QStringLiteral(":/Wag9/fs/pneumo/Background.png")));
    background->setGeometry(0, 0, 1000, 1000);

    auto* needle = new QLabel(this);
    const QPixmap needlePix(QStringLiteral(":/Wag9/fs/pneumo/ArrowBlack1.png"));
    needle->setPixmap(needlePix);
    needle->setFixedSize(needlePix.size());
    needle->move(770, 238);
    needle->setAttribute(Qt::WA_TransparentForMouseEvents);

    // EC-COC, TC2-COC, TC1-COC.
    addSwitch(this, QStringLiteral("SwitchBlack2"), 2, 0, 47, 82);
    addSwitch(this, QStringLiteral("SwitchBlack2"), 2, 0, 47, 147);
    addSwitch(this, QStringLiteral("SwitchBlack2"), 2, 0, 47, 211);

    // 21(SIFA)-COC, FP-COC.
    addSwitch(this, QStringLiteral("SwitchRed1"), 2, 0, 383, 126);
    addSwitch(this, QStringLiteral("SwitchYellow1"), 2, 0, 364, 236);

    // SAND1-COC, SAND2-COC.
    addSwitch(this, QStringLiteral("SwitchBlack3"), 2, 0, 551, 133);
    addSwitch(this, QStringLiteral("SwitchBlack3"), 2, 0, 617, 133);

    // UL-COC, CB-COC. The second frame is the resting position.
    addSwitch(this, QStringLiteral("SwitchBlack2"), 2, 1, 683, 136);
    addSwitch(this, QStringLiteral("SwitchBlack2"), 2, 1, 745, 136);

    // PANS COC, PAN1.COC, PAN2.COC.
    addSwitch(this, QStringLiteral("SwitchKey"), 2, 0, 818, 149);
    addSwitch(this, QStringLiteral("SwitchBlack2"), 2, 1, 869, 136);
    addSwitch(this, QStringLiteral("SwitchBlack2"), 2, 1, 931, 136);

    // PAN-SEL-SW has three positions.
    addSwitch(this, QStringLiteral("SwitchBlack1"), 3, 0, 860, 82);

    // PASS/GOODS, PER-COC.
    addSwitch(this, QStringLiteral("SwitchBlack4"), 2, 0, 241, 612);
    addSwitch(this, QStringLiteral("SwitchRed1"), 2, 0, 681, 677);

    // DEAD ENGINE. The second frame is the resting position.
    addSwitch(this, QStringLiteral("SwitchYellow2"), 2, 1, 831, 696);

    // G/P.
    addSwitch(this, QStringLiteral("SwitchYellow3"), 2, 0, 204, 878);
}
