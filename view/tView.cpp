#include "tView.hpp"
#include "../common/tWindow.hpp"

#include <QHBoxLayout>

/*tView::tView() :
    tView(nullptr)
{
}*/

tView::tView(tWindow *window) :
    m_window(window)
{

}

tView::~tView()
{
    if(m_inputSelect) delete m_inputSelect;
    if(m_settings) delete m_settings;
}

QString tView::getName() const
{
    return QObject::objectName();
}

void tView::setName(const QString& name)
{
    QObject::setObjectName(name);
}

/*bool tView::init(tData::List* data)
{
    if(data)
        return init_impl(*data);

    else if(m_window)
        return init_impl(m_window->getData());

    else return false;
}*/


/*bool tView::init(tData::Ref data)
{
    tData::List list;
    //list.push_back(data);
    return init(&list);
}*/

const QWidget* tView::getInputSelectWidget() const
{
    return m_inputSelect;
}

QWidget* tView::getInputSelectWidget()
{
    return m_inputSelect;
}

const QWidget* tView::getSettingsWidget() const
{
    return m_settings;
}

QWidget* tView::getSettingsWidget()
{
    return m_settings;
}

/*void tView::registerData(const tData::Ref& data)
{
    connect(&*data, SIGNAL(changed()), this, SLOT(update()));
}

void tView::unregisterData(const tData::Ref& data)
{
    disconnect(&*data, SIGNAL(changed()), this, SLOT(update()));
}*/
