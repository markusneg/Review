#pragma once

#include "tMatrixDataSelection.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>

#include "../common/tSetting.hpp"
#include "tMatrixDataSelection.hpp"

/*
this->setName("Setting (DataRef)");

this->m_value.addSignalForwarding(this);

Base::m_widget = new QWidget;
QVBoxLayout* layout = new QVBoxLayout(Base::m_widget);
layout->setContentsMargins(1, 1, 1, 1); layout->setSpacing(1);

// create setting for data reference
m_setRef = new tSetting<T*>(Base::m_value.m_data);
m_setRef->addSignalForwarding(this);

// setting for selector
m_setSel = new tSetting<typename T::Selector>(Base::m_value.getSelector());
m_setSel->addSignalForwarding(this);

// child settings value change also has to be regarded
m_setRef->addCallback([&]()
{
    this->m_value.getSelector().setData(**m_setRef);
}, sigChanged);

layout->addWidget(m_setRef->widget()); layout->addWidget(m_setSel->widget());
*/


template <>
class tSetting<tMatrixDataSelection> : public tSettingBase<tMatrixDataSelection>
{

public:

    T_SETTING_DEFS(tSetting, tMatrixDataSelection)

    void destroy() {delete m_selections[0]; delete m_selections[1]; delete m_setRef; delete m_setTrans;}

    void init()
    {
        setName("Setting (MatrixDataSel)");

        m_widget = new QWidget;
        m_widget->setMaximumSize(QSize(270, 70));

        // align dataSelection and refSelection vertically
        QVBoxLayout* vLayout = new QVBoxLayout(m_widget);
        vLayout->setContentsMargins(1, 1, 1, 1); vLayout->setSpacing(1);

        // create setting for data selection
        m_setRef = new tSetting<tMatrixData*>(m_value.m_data);
        //m_setRef->addSignalForwarding(this);

        // instead of setting pointer, use the right matrixDataSelection method
        m_setRef->setSetter([&](tMatrixData* data)
        {
            this->m_value.queueCallback(sigSettingChanged);
            this->m_value.setData(data);

            m_setRef->valueToWidget();      /// [TODO] we should have an own valueToWidget for this setting
            m_setTrans->valueToWidget();

            this->onNewRef();

            //this->doCallbacks(sigChanged | sigSettingChanged);
        });

        vLayout->addWidget(m_setRef->widget());

        // align the two discreteSelections horizontally
        QWidget* selWidget = new QWidget;
        QHBoxLayout* hLayout = new QHBoxLayout(selWidget);
        hLayout->setContentsMargins(0, 0, 0, 0); hLayout->setSpacing(1);

        // add the settings for m- and n-selections
        for(uint mn = 0; mn < 2; ++mn)
        {
            m_selections[mn] = new tSetting<tDiscreteSelection>(m_value.getDiscreteSelections()[mn]);
            hLayout->addWidget(m_selections[mn]->widget());

            // for every edit of a discrete selection, check validity
            m_selections[mn]->addCallbackSafe([&, mn]()
            {
                m_value.validate(false);
                m_value.markAsUserTouched((tMatrixDataSelection::Margin)mn);
                doCallbacks(sigChanged | sigSettingChanged);
            }, this, sigChanged);

            // also, when tMatrixDataSelection itself reports change, transfer it to discrete selection widgets
            m_value.addCallbackSafe([&, mn]()
            {
                m_selections[mn]->valueToWidget();
            }, this, sigChanged);
        }

        m_setTrans = new tSetting<bool>(m_value.m_transpose);
        m_setTrans->addSignalForwarding(this);
        hLayout->addWidget(m_setTrans->widget());

        vLayout->addWidget(selWidget);

        onNewRef();
    }

    template <typename U>
    void setList(const U* list)
    {
        m_setRef->setList(list);
    }

    auto getSelectionSettings() {return m_selections;}
    auto getRefSetting() {return m_setRef;}
    auto getTransSetting() {return m_setTrans;}

private:

    void onNewRef()
    {
        for(uint i : {0,1})
            m_selections[i]->widget()->setEnabled(*m_setRef);
    }

    tSetting<tDiscreteSelection>* m_selections[2];
    tSetting<tMatrixData*>* m_setRef;
    tSetting<bool>* m_setTrans;
};

