#include "tQCustomPlot.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QColorDialog>
#include <QComboBox>
#include <QPixmap>
#include <QImage>

#include "qcustomplot.h"

#include "../common/tWindow.hpp"
#include "../common/tSetting.hpp"
#include "../data/tMatrixDataSelection.hpp"
#include "../util/util.hpp"

#define P (*m_private)

struct tColorGradient::Private
{
    QPushButton *btEnabled, *btColor0, *btColor1;
    QComboBox* cbGradientSelect;
};

tColorGradient::tColorGradient() : m_private(new Private), m_colorGradient(new QCPColorGradient(QCPColorGradient::gpIon))
{
    QVBoxLayout* vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0); vLayout->setSpacing(1);

    QWidget* hWidget = new QWidget;
    vLayout->addWidget(hWidget);
    QHBoxLayout* hLayout = new QHBoxLayout(hWidget);
    hLayout->setContentsMargins(0, 0, 0, 0); hLayout->setSpacing(0);

    m_private->btEnabled = new QPushButton(tr("On"));
    m_private->btEnabled->setCheckable(true);
    m_private->btEnabled->setMaximumWidth(34);
    hLayout->addWidget(m_private->btEnabled);
    connect(m_private->btEnabled, T_SIGNAL(QPushButton, clicked, bool), [=](bool checked){emit changed();});

    m_private->btColor0 = new QPushButton(tr("First"));
    hLayout->addWidget(m_private->btColor0);
    connect(m_private->btColor0, &QPushButton::clicked, [=](){setColor(0);});

    m_private->btColor1 = new QPushButton(tr("Last"));
    hLayout->addWidget(m_private->btColor1);
    connect(m_private->btColor1, &QPushButton::clicked, [=](){setColor(1);});

    vLayout->addWidget(new QLabel(QObject::tr("Color Preset")));
    P.cbGradientSelect = new QComboBox;
    QMetaEnum gp = QMetaEnum::fromType<QCPColorGradient::GradientPreset>();
    for(int i = 0; i < gp.keyCount(); ++i)
        P.cbGradientSelect->addItem(gp.key(i));

    vLayout->addWidget(P.cbGradientSelect);
    connect(P.cbGradientSelect, T_SIGNAL(QComboBox, activated, int), [&, gp](int index)
    {
        m_colorGradient->loadPreset(static_cast<QCPColorGradient::GradientPreset>(gp.value(index)));
        emit changed();
    });

    updateWidget();
}

tColorGradient::~tColorGradient()
{
    delete m_colorGradient;
    delete m_private;
}

const QCPColorGradient& tColorGradient::getColorGradient() const
{
    return *m_colorGradient;
}

QCPColorGradient& tColorGradient::getColorGradient()
{
    return *m_colorGradient;
}

bool tColorGradient::isEnabled() const
{
    return m_private->btEnabled->isChecked();
}

void tColorGradient::setColor(double position)
{
    QColor currentColor = m_colorGradient->color(position, {0,1});
    QColor newColor = QColorDialog::getColor(currentColor, this, tr("Choose color"));

    if(newColor.isValid())
    {
        m_colorGradient->setColorStopAt(position, newColor);
        updateWidget();
        emit changed();
    }
}

void tColorGradient::updateWidget()
{
    QPixmap colorIndicator(15, 15);

    QColor color = m_colorGradient->color(0, {0,1});
    colorIndicator.fill(color);
    m_private->btColor0->setIcon(QIcon(colorIndicator));
    //m_private->btColor0->setStyleSheet(QString("background-color: ") + color.name() + ";");

    color = m_colorGradient->color(1, {0,1});
    colorIndicator.fill(color);
    m_private->btColor1->setIcon(QIcon(colorIndicator));
    //m_private->btColor1->setStyleSheet(QString("background-color: ") + color.name() + ";");
}

