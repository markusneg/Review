#include "tPlotView.hpp"

#include "qcustomplot.h"

#include <QLineEdit>

#include "../common/tWindow.hpp"
#include "../data/tMatrixData.hpp"
#include "../data/tMatrixDataSelection.hpp"
#include "tQCustomPlot.hpp"

/*tPlotView::tPlotView() :
    tView(nullptr), m_plot(nullptr)
{
    setName(tr("Plotview"));
}*/

enum Plotstyle {Line = 0, Points = 1};

struct tPlotView::Private
{
    std::vector<QCPAbstractItem*> additionalItems;

    tColorGradient* colorGradient = new tColorGradient; // lifetime later managed by Qt
    bool cenabled = false;  // coloring by data enabled
    double cmin, cmax;
    arma::vec cvec;
    QPushButton* intraPlotButton = new QPushButton(tr("Intraplot Coloring"));

    arma::mat colorData;
    //bool ipcDataValid = false;

    // rubberband stuff
    double m_newRange[4];
    QPoint m_rubberBandOrigin;

    void updateOffsetLines(QCustomPlot& qcp, double offset);
};


tPlotView::tPlotView(tWindow* window) :
    tView(window), m_plot(new QCustomPlot(window)), m_private(new Private)
{
    setName(tr("PlotView"));

    if(!window) return;

    QHBoxLayout* viewLayout = new QHBoxLayout();
    viewLayout->setContentsMargins(0, 0, 0, 0); viewLayout->setSpacing(0);
    setLayout(viewLayout);
    viewLayout->addWidget(m_plot);
    //m_plot->show();

    m_plotStyle.addItems({tr("Line"), tr("Points")});
    connect(&m_plotStyle, SIGNAL(activated(int)), this, SLOT(update()));

    m_xdata.getRefSetting()->unsetFlags(fAutoSet);
    m_cdata.getRefSetting()->unsetFlags(fAutoSet);
    m_cdata.getRefSetting()->setEmptyTag(tr("Color index"));

    m_ydata.getRefSetting()->setDefaultSelector(fRotation);
    m_ydata->setSelectionPolicy(new tPlotSelectionPolicy(*m_ydata));

    m_plot->addGraph();

    // prepare input-select
    m_inputSelect = new QWidget(); m_inputSelect->setMaximumSize(QSize(540, 70));
    QHBoxLayout* inpLayout = new QHBoxLayout(m_inputSelect); inpLayout->setContentsMargins(0, 0, 0, 0); inpLayout->setSpacing(0);

    inpLayout->addWidget(m_xdata.widget());
    inpLayout->addWidget(m_ydata.widget());
    //inpLayout->addWidget(m_invert.widget());

    m_xdata.addCallbackSafe([&](){update();}, this, sigChanged | sigExtentChanged | sigDataChanged);
    m_ydata.addCallbackSafe([&](){update();}, this, sigChanged | sigExtentChanged | sigDataChanged);    
    //m_invert.addCallbackSafe([&](){update();}, this, sigChanged);

    m_settings = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_settings);
    layout->setContentsMargins(0, 0, 0, 0); layout->setSpacing(1);

    layout->addWidget(new QLabel(QObject::tr("Type")));
    layout->addWidget(&m_plotStyle);

    //m_settings->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    layout->addWidget(new QLabel(QObject::tr("Spread")));
    layout->addWidget(m_spread.widget());

    QWidget* centerScaleWidget = new QWidget;
    QHBoxLayout* centerScalelayout = new QHBoxLayout(centerScaleWidget);
    centerScalelayout->setContentsMargins(0, 0, 0, 0); centerScalelayout->setSpacing(1);

    layout->addWidget(new QLabel(QObject::tr("Center / Scale")));
    centerScalelayout->addWidget(m_center.widget());
    centerScalelayout->addWidget(m_scale.widget());
    layout->addWidget(centerScaleWidget);

    layout->addWidget(new QLabel(QObject::tr("Color")));
    layout->addWidget(m_private->colorGradient);

    layout->addWidget(m_cdata.widget());

    m_private->intraPlotButton->setCheckable(true);
    layout->addWidget(m_private->intraPlotButton);

    layout->addWidget(new QLabel(QObject::tr("Tick Labels X/Y")));
    QWidget* labelsWidget = new QWidget;
    QHBoxLayout* labelsLayout = new QHBoxLayout(labelsWidget);
    labelsLayout->setContentsMargins(0, 0, 0, 0); labelsLayout->setSpacing(1);
    labelsLayout->addWidget(m_xLabels.widget()); labelsLayout->addWidget(m_yLabels.widget());
    m_xLabels.addCallback([&]() {m_plot->xAxis->setTickLabels(*m_xLabels); update();}, sigChanged);
    m_yLabels.addCallback([&]() {m_plot->yAxis->setTickLabels(*m_yLabels); update();}, sigChanged);
    layout->addWidget(labelsWidget);

    layout->addWidget(new QLabel(QObject::tr("Log X/Y")));
    QWidget* logWidget = new QWidget;
    QHBoxLayout* logLayout = new QHBoxLayout(logWidget);
    logLayout->setContentsMargins(0, 0, 0, 0); logLayout->setSpacing(1);
    logLayout->addWidget(m_logx.widget()); logLayout->addWidget(m_logy.widget());
    m_logx.addCallback([&]() {m_plot->xAxis->setScaleType((QCPAxis::ScaleType)*m_logx); update();}, sigChanged);
    m_logy.addCallback([&]() {m_plot->yAxis->setScaleType((QCPAxis::ScaleType)*m_logy); update();}, sigChanged);
    layout->addWidget(logWidget);

    layout->addWidget(new QLabel(QObject::tr("Axis Labels X/Y")));
    QWidget* alabWidget = new QWidget;
    QHBoxLayout* alabLayout = new QHBoxLayout(alabWidget);
    alabLayout->setContentsMargins(0, 0, 0, 0); alabLayout->setSpacing(1);
    alabLayout->addWidget(&m_xAxisLabel); alabLayout->addWidget(&m_yAxisLabel);
    layout->addWidget(alabWidget);

    layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));

    connect(m_private->colorGradient, SIGNAL(changed()), this, SLOT(update()));
    connect(m_private->intraPlotButton, SIGNAL(clicked()), this, SLOT(update()));

    m_spread.addCallback([&]()
    {
        //m_private->updateOffsetLines(m_plot, *m_spread);
        update();

    }, sigChanged);

    m_center.addCallback([&]() {update();}, sigChanged); m_scale.addCallback([&]() {update();}, sigChanged);


    // update indexed coloring data
    m_cdata.addCallback([&]()
    {
        if(!m_cdata) // invalidate
            m_private->colorData = arma::mat();

        m_private->colorData = m_cdata->getSubmatrix();

        if(m_private->colorData.n_elem <= 1) return;

        // used for graph-wise coloring
        arma::vec cvec = m_private->colorData.col(0);
        m_private->cmin = cvec.min();
        m_private->cmax = cvec.max();

        update();

    }, sigChanged);

    m_xAxisLabel.setObjectName("x");
    connect(&m_xAxisLabel, SIGNAL(editingFinished()), this, SLOT(axisLabelChanged()));
    connect(&m_yAxisLabel, SIGNAL(editingFinished()), this, SLOT(axisLabelChanged()));

    m_rubberBand = new QRubberBand(QRubberBand::Rectangle, m_plot);
    connect(m_plot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress(QMouseEvent*)));
    connect(m_plot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(mouseMove(QMouseEvent*)));
    connect(m_plot, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(mouseRelease(QMouseEvent*)));
    m_rubberBand->hide();

    m_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom /*| QCP::iSelectPlottables*/); // | QCP::iSelectAxes | QCP::iSelectLegend | QCP::iSelectPlottables

    m_xdata.setList(&m_window->getData());
    m_ydata.setList(&m_window->getData());
    m_cdata.setList(&m_window->getData());
}

tPlotView::~tPlotView()
{
    doCallbacks(sigDecaying);
    delete m_plot;
    delete m_private;
}

void tPlotView::update()
{
    // probably a prototype
    if(!m_plot) return;

    // no data selected
    if(!m_xdata && !m_ydata) return;

    // prevent double plots
    lockMutex(); ON_LEAVE(unlockMutex();)

    // lock the available data refs
    //if(m_xdata) m_xdata->getData()->lockMutex(); if(m_ydata) m_ydata->getData()->lockMutex();
    //ON_LEAVE2(if(m_xdata) m_xdata->getData()->unlockMutex(); if(m_ydata) m_ydata->getData()->unlockMutex();)

    arma::mat Mx = m_xdata ? m_xdata->getSubmatrix() : arma::mat();
    arma::mat My = m_ydata ? m_ydata->getSubmatrix() : arma::mat();

    if(!My.n_elem) {/*T_COUT("[PlotView] no elements in Y!");*/ return;}
    if(!Mx.n_elem) Mx.clear(); // do not regard ncols or nrows

    uint im = My.n_rows; uint in = My.n_cols;

    bool transposed = false; // helper for row-reduction

    if(Mx.n_cols != in)
    {
        if(Mx.n_rows == in && Mx.n_elem)
        {
            // x would fit transposed: do so automatically
            m_xdata.getTransSetting()->blockCallbacks();
            m_xdata->toggleTranspose();
            m_xdata.getTransSetting()->valueChanged();
            m_xdata.getTransSetting()->blockCallbacks(false);

            //T_COUT("Auto-transposing Mx!");
            Mx = Mx.t();
            transposed = true;
        }

        else
        {
            //T_COUT("No x-axis found. Using indices!");

            Mx.set_size(1, in);
            for(uint i = 0; i < in; ++i)
                Mx(0,i) = i + 1;
        }
    }

    // currently, we only need one row for all y-data
    if(Mx.n_rows > 1)
    {
        tDiscreteSelection& sel = m_xdata->getDiscreteSelections()[transposed];

        m_xdata.blockCallbacks();
        uint lowest = sel.getLowestItem();
        sel.reset();
        sel.select(lowest);
        m_xdata.getSelectionSettings()[transposed]->valueChanged();
        m_xdata.blockCallbacks(false);
    }

    const arma::mat& cdata = m_private->colorData;
    bool ipcDataValid = (cdata.n_rows == in && cdata.n_cols);

    QPen offsetLinesPen(QBrush(Qt::lightGray), 1);

    for(uint i = 0; i < im; ++i)
    {
        QCPGraph* currentGraph;
        if(i >= (uint)m_plot->graphCount()) currentGraph = m_plot->addGraph();
        else                                currentGraph = m_plot->graph(i);

        updatePlotstyle(currentGraph, i, im);

        QCPDataMap* data = currentGraph->data();
        data->clear();

        double mean = 0;
        if(*m_center)
            {for(uint j = 0; j < in; ++j) mean += My(i,j); mean /= in;}

        double vlength = 1;
        if(*m_scale)
        {
            vlength = 0;
            for(uint j = 0; j < in; ++j) vlength += pow(My(i,j) - mean, 2);
            vlength = sqrt(vlength);
        }

        QCPData newData;
        for(uint j = 0; j < in; ++j)
        {
            newData.key = Mx(0,j);
            newData.value = (My(i,j) - mean) / vlength + m_spread * i;
            newData.color = ipcDataValid ? cdata(j,0) : j;
            data->insertMulti(newData.key, newData);
        }

        /*if(*m_spread)
        {
            QCPItemLine *line = new QCPItemLine(m_plot);
            m_private->additionalItems.push_back(line);

            line->start->setCoords(data->first().key, m_spread * i);
            line->end->setCoords(data->last().key, m_spread * i);

            line->setPen(offsetLinesPen);

            m_plot->addItem(line);
        }*/

    }

    if(*m_spread)
    {
        QVector<double> tickPositions;
        for(uint i = 0; i < im; ++i)
            tickPositions.push_back(m_spread * i);

        m_plot->yAxis->grid()->setPen(QPen(Qt::lightGray));
        m_plot->yAxis->setAutoTicks(false);
        m_plot->yAxis->setAutoSubTicks(false);
        //m_plot->yAxis->setAutoTickLabels(false);

        //ui->chart->xAxis->setAutoTickStep(false);
        //ui->chart->xAxis->setAutoTickLabels(false);
        /* ID is a QVector<double> which stored numbers from 1 to 99. */

        m_plot->yAxis->setTickVector(tickPositions);


    }

    else
    {
        m_plot->yAxis->grid()->setPen(QPen(Qt::lightGray, 1, Qt::DotLine));

        m_plot->yAxis->setAutoTicks(true);
        m_plot->yAxis->setAutoSubTicks(true);
    }



    // match number of graphs to m
    while((uint)m_plot->graphCount() > im)
        m_plot->removeGraph(m_plot->graphCount() - 1);

    m_plot->rescaleAxes();
    m_plot->update();
    m_plot->replot();
}

void tPlotView::updatePlotstyle(QCPGraph* graph, uint graphIndex, uint totalGraphs)
{
    bool colorEnabled = m_private->colorGradient->isEnabled();
    bool ipcEnabled = colorEnabled && m_private->intraPlotButton->isChecked() && m_plotStyle.currentIndex() == Points; // different rules for intraplot coloring
    QColor graphColor = Qt::black;

    const arma::mat& cdata = m_private->colorData;
    bool cdataValid = (cdata.n_rows == totalGraphs && cdata.n_cols);

    if(colorEnabled)
    {
        double position, cmin, cmax;

        // valid for ipc and graph coloring
        if(cdataValid)
        {
            position = graphIndex < cdata.n_rows ? cdata(graphIndex,0) : m_private->cmax;
            cmin = m_private->cmin; cmax = m_private->cmax;
        }

        else {cmin = 0; cmax = totalGraphs ? totalGraphs - 1 : 0; position = graphIndex;}

        graphColor = m_private->colorGradient->getColorGradient().color(position, {cmin, cmax});
    }

    QCPScatterStyle& scatterStyle = graph->scatterStyleRef();

    switch(m_plotStyle.currentIndex())
    {

    case Line:

        graph->setLineStyle(QCPGraph::LineStyle::lsLine);
        scatterStyle.setPen(Qt::NoPen);

        graph->setPen(QPen(graphColor));

        break;

    case Points:

        graph->setLineStyle(QCPGraph::LineStyle::lsNone);
        scatterStyle.setShape(QCPScatterStyle::ssCircle);
        scatterStyle.setSize(6);

        if(ipcEnabled)
            scatterStyle.setIntraPlotColors(m_private->colorGradient->getColorGradient()); /// [OPT] do this once for every graph and when master gradient changes
        else
            scatterStyle.setPen(QPen(graphColor));

        break;
    }

    scatterStyle.activateIntraPlotColoring(ipcEnabled);
}


bool tPlotView::isApplicable(const tData* data) const
{
    const tMatrixData* mdata = dynamic_cast<const tMatrixData*>(data);
    return mdata && mdata->n_cols() && mdata->n_rows();
}

tView* tPlotView::copy(tWindow* window) const
{
    tPlotView& ret = *new tPlotView(window);

    //ret.m_xdata = m_xdata;
    //ret.m_ydata = m_ydata;
    ret.setName(getName());

    return &ret;
}

void tPlotView::mousePress(QMouseEvent* mevent)
{
    if(mevent->button() == Qt::RightButton)
    {
        m_private->m_rubberBandOrigin = mevent->pos();
        m_private->m_newRange[0] = m_plot->xAxis->pixelToCoord(m_private->m_rubberBandOrigin.x());
        m_private->m_newRange[1] = m_plot->yAxis->pixelToCoord(m_private->m_rubberBandOrigin.y());

        m_rubberBand->setGeometry(QRect(m_private->m_rubberBandOrigin, m_private->m_rubberBandOrigin));
        m_rubberBand->show();
    }
}

void tPlotView::mouseMove(QMouseEvent *mevent)
{
    double x = m_plot->xAxis->pixelToCoord(mevent->pos().x());
    double y = m_plot->yAxis->pixelToCoord(mevent->pos().y());

    //std::cout << x << " " << y << std::endl << std::flush;

    QString statusInfo = QString("x: ") + QString::number(x) + " y: " + QString::number(y);
    emit newStatusInfo(statusInfo);

    if(m_rubberBand->isHidden()) return;
    m_rubberBand->setGeometry(QRect(m_private->m_rubberBandOrigin, mevent->pos()).normalized());
}

void tPlotView::mouseRelease(QMouseEvent *mevent)
{
    if(!m_rubberBand->isHidden() && mevent->button() == Qt::RightButton)
    {
        m_rubberBand->hide();

        m_private->m_newRange[2] = m_plot->xAxis->pixelToCoord(mevent->pos().x());
        m_private->m_newRange[3] = m_plot->yAxis->pixelToCoord(mevent->pos().y());

        m_plot->xAxis->setRange(m_private->m_newRange[0], m_private->m_newRange[2]);
        m_plot->yAxis->setRange(m_private->m_newRange[1], m_private->m_newRange[3]);
        m_plot->replot();
    }
}

void tPlotView::leaveEvent(QEvent *event)
{
    emit newStatusInfo("");
    QWidget::leaveEvent(event);
}

void tPlotView::axisLabelChanged()
{
    if(sender()->objectName() == "x")
        m_plot->xAxis->setLabel(m_xAxisLabel.text());
    else
        m_plot->yAxis->setLabel(m_yAxisLabel.text());

    m_plot->replot();
}

tPlotSelectionPolicy::tPlotSelectionPolicy(tMatrixDataSelection& sel, uint graphCount) : m_sel(sel), m_graphCount(graphCount) {}

void tPlotSelectionPolicy::setGraphCountHint(uint graphCount)
{
    m_graphCount = graphCount;
}

void tPlotSelectionPolicy::onExtentChange()
{
    tMatrixData* data = m_sel.getData();

    if(data->checkFlags(fRotation /*| fRotation*/))
    {
        if(!m_sel.isUserTouched(tMatrixDataSelection::Columns) && m_graphCount)
        {
            m_sel.getDiscreteSelections()[1].reset();        // "select" does no unselection so reset before
            m_sel.getDiscreteSelections()[1].select(0, m_graphCount-1);

            //m_sel.markAsUserTouched(tMatrixDataSelection::Columns);
            //m_sel->validate(false); // done by sel!
        }
    }
}

void tPlotSelectionPolicy::onDataChange()
{
    if(m_sel.getData()->checkFlags(fRotation /*| fRotation*/))
        m_sel.setTranspose();

    onExtentChange(); // controversial. DataChange is potentially accompanied by ExtentChange. Should be regarded by MatrixDataSelection
}

void tPlotSelectionPolicy::onListChange()
{

}

void tPlotView::Private::updateOffsetLines(QCustomPlot& qcp, double offset)
{
    /*for(QCPAbstractItem* line : m_private->additionalItems)
        m_plot->removeItem(line);
    m_private->additionalItems.clear();*/

    /*QCPItemLine *line = new QCPItemLine(m_plot);
    m_private->additionalItems.push_back(line);

    line->start->setCoords(data->first().key, m_spread * i);
    line->end->setCoords(data->last().key, m_spread * i);

    line->setPen(offsetLinesPen);

    m_plot->addItem(line);*/
}
