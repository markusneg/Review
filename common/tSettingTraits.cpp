#include "tSetting.hpp"

#include "../common/tWindow.hpp"
#include "../data/tMatrixData.hpp"
#include "../main/tSupervisor.hpp"

#include <QPushButton>


template <typename T>
void setList(T* setting)
{
    tWindow* window = tGlobal::Supervisor.letUserSelectWindow();
    if(window) setting->setList(&window->getData());
}

template <>
void tSetting<tData*>::getListFromUser() {::setList(this);}

template <>
void tSetting<tMatrixData*>::getListFromUser() {::setList(this);}



void tSetting<tWindow*>::init()
{
    m_checkBox = new QPushButton();

    this->setName("Setting (tWindow)");
    this->setFlags(fReseatable);

    // apply user choice to internal value
    QObject::connect(m_checkBox, &QPushButton::clicked, [&]() {set(tGlobal::Supervisor.letUserSelectWindow());});

    Base::m_widget = m_checkBox;

    this->setFlags(fAutoSet);
}

void tSetting<tWindow*>::valueToWidget()
{
    m_checkBox->setText(this->m_value ? this->m_value->windowTitle() : QObject::tr("No Input"));
}

void tSetting<tWindow*>::set(tWindow* const& value)
{
    /// this is almost the same as in T* tSetting!

    // cease forwarding previous values signals
    if(this->m_value)
    {
        this->m_value->removeSignalForwarding(this);
        this->m_value->removeCallback(this);
        this->removeCallback(this->m_value);
    }

    if(value)
    {
        value->addSignalForwarding(this);
        value->addCallbackSafe([&]() {this->set(nullptr);}, this, sigDecaying);
    }

    Base::set(value);
}

void tSetting<tWindow*>::destroy()
{
}

