#include "tImageView.hpp"

#include <QMouseEvent>
#include <QLineEdit>
#include <QLabel>
#include <QPixmap>
#include <QImage>

#include "../common/tWindow.hpp"
#include "../data/tMatrixData.hpp"
#include "../data/tMatrixDataSelection.hpp"

#include "tQCustomPlot.hpp"
#include "qcustomplot.h"


#define P   (*m_private)

struct tImageView::Private
{
    QLabel* label = nullptr;
    QImage* image = new QImage();

    tColorGradient* colorGradient = new tColorGradient;

};


tImageView::tImageView(tWindow* window) :
    tView(window), m_private(new Private)
{
    setName(tr("ImageView"));
    if(!window) return;

    P.label = new QLabel(window);
    P.label->setScaledContents(true);
    P.colorGradient->getColorGradient().setColorInterpolation(QCPColorGradient::ciHSV);

    QHBoxLayout* viewLayout = new QHBoxLayout();
    viewLayout->setContentsMargins(0, 0, 0, 0); viewLayout->setSpacing(0);
    setLayout(viewLayout);
    viewLayout->addWidget(P.label);

    //m_plotStyle.addItems({tr("Line"), tr("Points")});
    //connect(&m_plotStyle, SIGNAL(activated(int)), this, SLOT(update()));

    //m_data.getRefSetting()->setDefaultSelector(fRotation);

    m_data->setSelectionPolicy(new tImageSelectionPolicy(*m_data));

    m_settings = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_settings);
    layout->setContentsMargins(0, 0, 0, 0); layout->setSpacing(1);

    layout->addWidget(new QLabel(QObject::tr("Folding")));
    layout->addWidget(m_folding.widget());

    layout->addWidget(new QLabel(QObject::tr("Color")));
    layout->addWidget(m_private->colorGradient);


    //m_settings->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    //layout->addWidget(new QLabel(QObject::tr("Spread")));
    //layout->addWidget(m_spread.widget());

    // prepare input-select
    m_inputSelect = new QWidget(); m_inputSelect->setMaximumSize(QSize(540/2, 70));
    QHBoxLayout* inpLayout = new QHBoxLayout(m_inputSelect); inpLayout->setContentsMargins(0, 0, 0, 0); inpLayout->setSpacing(0);

    inpLayout->addWidget(m_data.widget());
    //inpLayout->addWidget(m_invert.widget());

    connect(P.colorGradient, SIGNAL(changed()), this, SLOT(update()));
    m_folding.addCallback([&]() {update();}, sigChanged);

    m_data.addCallbackSafe([&](){update();}, this, sigChanged | sigExtentChanged | sigDataChanged);
    //m_invert.addCallbackSafe([&](){update();}, this, sigChanged);

    m_data.setList(&m_window->getData());
}


tImageView::~tImageView()
{
    doCallbacks(sigDecaying);
    delete P.label;
    delete P.image;
    delete m_private;
}

void tImageView::update()
{
    // probably a prototype
    if(!P.label) return;

    // no data selected
    if(!m_data) return;

    // prevent double plots
    lockMutex(); ON_LEAVE(unlockMutex();)

    // lock the available data refs
    //if(m_xdata) m_xdata->getData()->lockMutex(); if(m_ydata) m_ydata->getData()->lockMutex();
    //ON_LEAVE2(if(m_xdata) m_xdata->getData()->unlockMutex(); if(m_ydata) m_ydata->getData()->unlockMutex();)

    arma::mat M = m_data ? m_data->getSubmatrix() : arma::mat();

    if(!M.n_elem) {/*T_COUT("[PlotView] no elements in Y!");*/ return;}

    if((M.n_rows == 1 || M.n_cols == 1) && m_folding.value() != 0)
        M.reshape(M.n_elem / m_folding, m_folding);

    const uint im = M.n_rows, in = M.n_cols;
    const double v0 = M.min(), v1 = M.max(), rng = v1 - v0;

    if(P.image->size() != QSize{im, in})
    {
        delete P.image;
        P.image = new QImage(QSize{im, in}, QImage::Format_RGB32);
    }

    const QCPColorGradient& cg = P.colorGradient->getColorGradient();

    for(uint x = 0; x < im; ++x) for(uint y = 0; y < in; ++y)
    {
        const double scale = (M(x,y) - v0) / rng;
        //P.image->setPixel(x, y, qRgb(scale * 0xFF, scale * 0xFF, scale * 0xFF));
        P.image->setPixel(x, y, P.colorGradient->isEnabled() ? cg.color(M(x,y), {v0, v1}) :
                                                               qRgb(scale * 0xFF, scale * 0xFF, scale * 0xFF));
    }

    P.label->setPixmap(QPixmap::fromImage(*P.image));
}

bool tImageView::isApplicable(const tData* data) const
{
    const tMatrixData* mdata = dynamic_cast<const tMatrixData*>(data);
    return mdata && mdata->n_cols() && mdata->n_rows();
}

tView* tImageView::copy(tWindow* window) const
{
    tImageView& ret = *new tImageView(window);

    //ret.m_ydata = m_ydata;
    ret.setName(getName());

    return &ret;
}

void tImageView::mousePress(QMouseEvent* mevent)
{

}

void tImageView::mouseMove(QMouseEvent *mevent)
{
    const double    x = mevent->pos().x(),
                    y = mevent->pos().y();

    QString statusInfo = QString("x: ") + QString::number(x) + " y: " + QString::number(y);
    emit newStatusInfo(statusInfo);
}

void tImageView::mouseRelease(QMouseEvent *mevent)
{

}

void tImageView::leaveEvent(QEvent *event)
{
    emit newStatusInfo("");
    QWidget::leaveEvent(event);
}

void tImageView::axisLabelChanged()
{

}


tImageSelectionPolicy::tImageSelectionPolicy(tMatrixDataSelection& sel, uint graphCount) : m_sel(sel), m_graphCount(graphCount) {}

void tImageSelectionPolicy::setGraphCountHint(uint graphCount)
{
}

void tImageSelectionPolicy::onExtentChange()
{

}

void tImageSelectionPolicy::onDataChange()
{

}

void tImageSelectionPolicy::onListChange()
{

}
