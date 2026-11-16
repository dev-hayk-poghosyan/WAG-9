#include "UncheckableButtonGroup.h"
#include <functional>

UncheckableButtonGroup::UncheckableButtonGroup(QObject *parent) :
    QObject(parent)
{}

void
UncheckableButtonGroup::add(const std::initializer_list<QAbstractButton*>& items)
{
    for (auto const& item : items)
    {
        add(item);
    }
}

void
UncheckableButtonGroup::add(QAbstractButton* item)
{
    if (item->isChecked())
    {
        if (m_selectedElement)
        {
            // Already have a checked item, so unchecking this one.
            item->setChecked(false);
        }
        else
        {
            m_selectedElement = item;
        }
    }

    auto const handlerToggle = [this](bool checked, QAbstractButton* myself)
    {
        if (!checked)
        {
            m_selectedElement = nullptr;
            return;
        }

        if (m_selectedElement)
            m_selectedElement->setChecked(false);

        m_selectedElement = myself;
    };

    connect(item, &QAbstractButton::toggled,
        std::bind(handlerToggle, std::placeholders::_1, item));
}