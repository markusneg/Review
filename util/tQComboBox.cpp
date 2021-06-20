#include "tQComboBox.hpp"

#include <QEvent>


void tQComboBox::showPopup()
{
    emit boxOpened();
    QComboBox::showPopup();
}


bool tQComboBox::event(QEvent* e)
{
    //if(e->type() == QEvent::Show)
    //    emit boxOpened();

    return QComboBox::event(e);
}
