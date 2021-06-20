#pragma once

#include <QComboBox>

// this special combobox can emit a Show signal
class tQComboBox : public QComboBox
{

    Q_OBJECT

public:

    using QComboBox::QComboBox;

    void showPopup() override;

signals:

    void boxOpened();

protected:

    bool event(QEvent* e) override;
};


