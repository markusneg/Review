#pragma once

#include "tDiscreteSelection.hpp"


template <>
class tSetting<tDiscreteSelection> : public tSettingBase<tDiscreteSelection>
{

public:

    T_SETTING_DEFS(tSetting, tDiscreteSelection)

    void init()
    {
        setName("Setting (DiscreteSel)");

        QObject::connect(m_lineEdit, &QLineEdit::editingFinished, [&]()
        {
            QString input = m_lineEdit->text();
            this->premangleText(input);

            //if(input.size())
            //{
                setFromString(input);
                doCallbacks(sigChanged | sigByUser);        // why not use set() here?
            //}

            //valueToWidget(); // for user feedback
        });

        m_widget = m_lineEdit;
    }

    void valueToWidget() override
    {
        m_lineEdit->setText(Base::m_value.toString());
    }

    void setFromString(const QString& str)
    {
        m_value.fromString(str);
        doCallbacks(sigChanged);
        valueToWidget();
    }

    QString valueAsText() const override
    {
        return Base::m_value.toString();
    }


protected:

    QLineEdit* m_lineEdit = new QLineEdit;
};
