#pragma once

#include <QObject>
#include <QAbstractButton>

#include <initializer_list>

/**
 * Lightweight simplified version of QButtonGroup unlike of which allows 
 * the uncheckable state of all items
 */
class UncheckableButtonGroup : public QObject
{
public:
    explicit UncheckableButtonGroup(QObject* parent = nullptr);

    void add(std::initializer_list<QAbstractButton*> const& items);
    void add(QAbstractButton* item);

private:
    QAbstractButton* m_selectedElement = nullptr;
};