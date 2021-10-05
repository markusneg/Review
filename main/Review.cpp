#include "Review.hpp"
#include "ui_Review.h"

#include <QFileDialog>
#include <QMdiSubWindow>
#include <QTimer>
#include <QMdiArea>
#include <QScrollArea>
#include <QInputDialog>

//#include <QGraphicsScene>

#include "tSupervisor.hpp"
#include "../data/tMatrixData.hpp"

//#include "sdb/sdb.hpp"

#define REVIEW_VERSION      "v0.1.0"

#define HEARTBEAT_INTERVAL  2        // heartbeat interval for tObjects [ms]




struct Review::Private
{
    static uint instances;

    QScrollArea *saFilterOptions, *saViewOptions;
    tWindow* databaseWindow = nullptr;
};

Review::Review(QWidget *parent) :
    QMainWindow(parent), m_private(new Private), m_ui(new Ui::Review)
{
    m_ui->setupUi(this);

    setWindowTitle(windowTitle() + " - " + REVIEW_VERSION);

    /// TEST: experiments with graphicsView for MDIarea (zooming & panning)
    m_mdiArea = new QMdiArea;
    m_mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    /*QGraphicsScene* scene = new QGraphicsScene;
    scene->addWidget(m_mdiArea);
    m_ui->graphicsView->setScene(scene);*/
    setCentralWidget(m_mdiArea);

    connect(m_mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(subWindowActivated(QMdiSubWindow*)));

    resetViewPrototypes();
    resetFilterPrototypes();

    m_selectedViewPrototype.setList(&m_viewPrototypes);
    m_selectedFilterPrototype.setList(&m_filterPrototypes);

    m_ui->wMainTools->layout()->addWidget(m_selectedViewPrototype.widget());
    m_ui->wMainTools->layout()->addWidget(m_selectedFilterPrototype.widget());

    for(QScrollArea** sa : {&m_private->saFilterOptions, &m_private->saViewOptions})
    {
        *sa = new QScrollArea;
        (*sa)->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        (*sa)->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        (*sa)->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        (*sa)->setMaximumWidth(170);
    }

    m_private->saFilterOptions->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    m_ui->dockFilterOptions->setWidget(m_private->saFilterOptions);
    m_ui->dockViewOptions->setWidget(m_private->saViewOptions);


    // start the heartbeat timer for tObjects
    if(!Private::instances)
    {
        tGlobal::Supervisor.m_ReviewInstance = this;

        QTimer* heartbeatTimer = new QTimer(this);
        connect(heartbeatTimer, &QTimer::timeout, [](){tObject::heartbeat();});
        heartbeatTimer->start(HEARTBEAT_INTERVAL);        
    }
    else T_COUT("[Warning] More than one Review-Instance has been created!");

    Private::instances++;

    /*QLinearGradient grad(QPointF(0, 0), QPointF(0, 1));
    grad.setCoordinateMode(QGradient::ObjectBoundingMode);
    grad.setColorAt(0, QColor("#c6c6cf"));
    grad.setColorAt(1, QColor("#c4c4e5"));
    m_mdiArea->setBackground(QBrush(grad));*/
}

Review::~Review()
{
    // we must not destroy widgets owned by views or filters
    m_private->saFilterOptions->takeWidget(); m_private->saViewOptions->takeWidget();
    m_ui->dockViewInputSelect->widget()->setParent(nullptr);
    m_ui->dockMainTools->widget()->setParent(nullptr);

    delete m_ui;
    delete m_private;
    Private::instances--;
}

tWindow* Review::getActiveWindow()
{
    // instead of using:
    // assert(m_mdiArea->activeSubWindow());
    // static_cast<tWindow*>(m_mdiArea->activeSubWindow()->widget());

    return m_activeWindow;
}

const tWindow* Review::getActiveWindow() const
{
    return m_activeWindow;
}

tWindow* Review::letUserSelectWindow()
{
    m_mdiArea->clearFocus();

    tWindow* currentWindow = m_activeWindow;

    while(currentWindow == m_activeWindow)
        QCoreApplication::processEvents();

    return m_activeWindow;
}

bool Review::newDataWindow()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Load from Data"), QString());
    return path.size() ? newDataWindow(path) : false;
}

bool Review::newDataWindow(const QString& path)
{
    tMatrixData::Ownership newData = tMatrixData::Create();
    if(!newData->load(path))
        return false;

    return newDataWindow(newData);
}

bool Review::newDataWindow(const tWindow& window)
{

    return true;
}

#include "../filter/tObserver.hpp"

void Review::newFilterWindow()
{
    if(!m_selectedViewPrototype || !m_selectedFilterPrototype || !getActiveWindow()) return;

    tWindow* inputWindow = getActiveWindow();
    tWindow* newWindow = createWindow();

    tFilter* newFilter = newWindow->setFilter(*m_selectedFilterPrototype);
    newWindow->setView(*m_selectedViewPrototype);

    newFilter->setInputWindow(inputWindow);

    //newWindow->addInputWindow(inputWindow);

    m_mdiArea->addSubWindow(newWindow);
    newWindow->updateMdiSubWindow();

    newWindow->show();

    //newFilter->run();
    //inputWindow->doCallbacks(sigOutputChanged);
}

void Review::newReferenceWindow()
{
    tWindow* inputWindow = getActiveWindow(); if(!inputWindow) return;
    tWindow* newWindow = createWindow();

    for(tData* dat : inputWindow->m_data)
    {
        newWindow->m_data.push_back(dat);
    }

    newWindow->setName(inputWindow->windowTitle());
    newWindow->setView(*m_selectedViewPrototype);

    newWindow->addInputWindow(inputWindow);     // ref windows can't change their input window

    m_mdiArea->addSubWindow(newWindow);
    newWindow->updateMdiSubWindow();

    newWindow->show();    
}

bool Review::loadFromFile(const QString& path)
{
    tMatrixData::Ownership newData = tMatrixData::Create();
    if(!newData->load(path))
        return false;

    if(!m_activeWindow) return false;

    m_activeWindow->addData(newData.get());

    return true;
}

bool Review::loadFromFile()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Load from Data"), QString());
    return path.size() ? loadFromFile(path) : false;
}


bool Review::saveToFile(const QString& path)
{
    if(!m_activeWindow) return false;

    T_COUT("Saving to " + path.toStdString());

    m_activeWindow->m_data.lock();

    for(const tData* data : m_activeWindow->m_data)
    {
        data->save(path + "/" + data->getName());
    }

    m_activeWindow->m_data.unlock();

    return true;
}

bool Review::saveToFile()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Save Data"), QString());
    return path.size() ? saveToFile(path) : false;
}

void Review::copyWindow(tWindow* window)
{
    if(window->getData().empty()) return;
    if(window->getFilter() && dynamic_cast<const tObserver*>(window->getFilter())) return;  // certain filters cant be copied

    tWindow* newWindow = createWindow();
    *newWindow = *window;

    newWindow->setView(*m_selectedViewPrototype);

    m_mdiArea->addSubWindow(newWindow);
    newWindow->updateMdiSubWindow();

    newWindow->show();
}

void Review::copyWindow()
{
    if(m_activeWindow)
        copyWindow(m_activeWindow);
}

void Review::changeWindowName()
{
    if(!m_activeWindow) return;

    bool ok; QString newName;
    newName = QInputDialog::getText(this, tr("Rename Window"), tr("Please specify a new window name"), QLineEdit::Normal, m_activeWindow->getName(), &ok);

    if(ok) changeWindowName(newName);
}

void Review::changeWindowName(const QString& name)
{
    if(!m_activeWindow) return;

    m_activeWindow->setName(name);
}

void Review::newDatabaseInput()
{


    //updateFromSDBData
}

void Review::subWindowActivated(QMdiSubWindow* window)
{
    // the last window was closed
    if(!window)
    {
        m_ui->dockViewInputSelect->hide();
        return;
    }

    m_activeWindow = static_cast<tWindow*>(window->widget());

    tView* view = m_activeWindow->getView(); assert(view);
    if(view->getInputSelectWidget()) m_ui->dockViewInputSelect->setWidget(view->getInputSelectWidget());
    //if(view->getSettingsWidget()) m_ui->dockViewOptions->setWidget(view->getSettingsWidget());
    view->getSettingsWidget()->setMaximumWidth(150);
    m_private->saViewOptions->takeWidget();
    if(view->getSettingsWidget()) m_private->saViewOptions->setWidget(view->getSettingsWidget());

    connect(view, SIGNAL(newStatusInfo(QString)), this, SLOT(newViewStatusInfo(QString)));

    tFilter* filter = static_cast<tWindow*>(window->widget())->getFilter();
    if(filter && filter->getSettingsWidget())
    {
        filter->getSettingsWidget()->setMaximumWidth(150);
        m_private->saFilterOptions->takeWidget();
        m_private->saFilterOptions->setWidget(filter->getSettingsWidget());
        //m_ui->dockFilterOptions->setWidget(filter->getSettingsWidget());
    }

    m_ui->dockViewInputSelect->setVisible(view->getInputSelectWidget());
    m_ui->dockViewOptions->setVisible(view->getSettingsWidget());
    m_ui->dockFilterOptions->setVisible(filter && filter->getSettingsWidget());

    /// [TODO] must be where new window is created / old one is deleted
    tWindow::updateWindowRelations();

    std::cout << "In: ";
    for(tWindow* w : m_activeWindow->getInputWindows())
        std::cout << w->windowTitle().toStdString() << " ";
    std::cout << std::endl;

    std::cout << "Out: ";
    for(tWindow* w : m_activeWindow->getOutputWindows())
        std::cout << w->windowTitle().toStdString() << " ";
    std::cout << std::endl;

    if(m_activeWindow->getFilter())
    {
        std::cout << "Following Filters: ";
        for(tFilter* f : m_activeWindow->getFilter()->getFiltersFollowing())
            std::cout << f->getName().toStdString() << " ";
    }

    /*std::cout << "Sort: ";
    tWindow::updateWindowRelations();
    std::vector<tData*> sorted = m_activeWindow->topologicalSortOfData();
    for(tData* d : sorted)
        std::cout << d->getName().toStdString() << " ";*/
    std::cout << std::endl << std::flush;
}

tWindow* Review::createWindow()
{
    tWindow* ret = new tWindow();

    //ret->addCallback([&, ret]() {removeFromVector(m_windows, ret);}, this, sigDecaying);

    //m_windows.push_back(ret);

    return ret;
}

tWindow* Review::newDataWindow(tData::Ownership data)
{
    /// [TODO] clear naming conventions!
    tWindow* newWindow = createWindow();
    newWindow->addData(data.get());
    newWindow->setName(data->getExtendedName());
    //newWindow->setName(newData->getExtendedName());

    newWindow->setView(*m_selectedViewPrototype);

    m_mdiArea->addSubWindow(newWindow);
    newWindow->updateMdiSubWindow();

    newWindow->show();

    return newWindow;
}


void Review::newViewStatusInfo(QString viewStatusInfo)
{
    m_ui->statusBar->showMessage(viewStatusInfo);
}


uint Review::Private::instances = 0;


// Here, the view prototype list is created

#include "../view/tPlotView.hpp"
#include "../view/tImageView.hpp"

void Review::resetViewPrototypes()
{
    m_viewPrototypes.clear();
    m_viewPrototypes.push_back((new tPlotView())->getOwnership());
    m_viewPrototypes.push_back((new tImageView())->getOwnership());

}


// Here, the view prototype list is created

#include "../filter/tPCAFilter.hpp"
#include "../filter/tRotationFilter.hpp"
#include "../filter/tVarianceFilter.hpp"
#include "../filter/tObserver.hpp"
#include "../filter/tBaselineFilter.hpp"
#include "../filter/tDerivationFilter.hpp"
#include "../filter/tArithmeticFilter.hpp"
#include "../filter/tUnaryFilter.hpp"
#include "../filter/tBinaryFilter.hpp"
#include "../filter/tFourierFilter.hpp"
#include "../filter/tBandObserver.hpp"
#include "../filter/tBandCombination.hpp"



void Review::resetFilterPrototypes()
{
    m_filterPrototypes.clear();
    m_filterPrototypes.push_back((new tPCAFilter())->getOwnership());
    m_filterPrototypes.push_back((new tRotationFilter())->getOwnership());
    //m_filterPrototypes.push_back((new tVarianceFilter())->getOwnership());
    m_filterPrototypes.push_back((new tBaselineFilter())->getOwnership());
    m_filterPrototypes.push_back((new tDerivationFilter())->getOwnership());
    m_filterPrototypes.push_back((new tObserver())->getOwnership());
    m_filterPrototypes.push_back((new tBandObserver())->getOwnership());
    m_filterPrototypes.push_back((new tArithmeticFilter<ArithmeticOperation::Plus>())->getOwnership());
    m_filterPrototypes.push_back((new tArithmeticFilter<ArithmeticOperation::Minus>())->getOwnership());
    m_filterPrototypes.push_back((new tArithmeticFilter<ArithmeticOperation::Divide>())->getOwnership());
    m_filterPrototypes.push_back((new tArithmeticFilter<ArithmeticOperation::Multiply>())->getOwnership());
    m_filterPrototypes.push_back((new tUnaryFilter<UnaryOperation::Power>())->getOwnership());
    m_filterPrototypes.push_back((new tUnaryFilter<UnaryOperation::Sum>())->getOwnership());
    m_filterPrototypes.push_back((new tUnaryFilter<UnaryOperation::Mean>())->getOwnership());
    m_filterPrototypes.push_back((new tUnaryFilter<UnaryOperation::Median>())->getOwnership());
    m_filterPrototypes.push_back((new tUnaryFilter<UnaryOperation::Min>())->getOwnership());
    m_filterPrototypes.push_back((new tUnaryFilter<UnaryOperation::Max>())->getOwnership());
    m_filterPrototypes.push_back((new tUnaryFilter<UnaryOperation::Abs>())->getOwnership());
    m_filterPrototypes.push_back((new tUnaryFilter<UnaryOperation::VectorLength>())->getOwnership());
    m_filterPrototypes.push_back((new tUnaryFilter<UnaryOperation::Log>())->getOwnership());
    m_filterPrototypes.push_back((new tUnaryFilter<UnaryOperation::Variance>())->getOwnership());
    m_filterPrototypes.push_back((new tUnaryFilter<UnaryOperation::SmoothFourier>())->getOwnership());
    //m_filterPrototypes.push_back((new tUnaryFilter<UnaryOperation::SmoothSG>())->getOwnership());
    m_filterPrototypes.push_back((new tUnaryFilter<UnaryOperation::Autoscale>())->getOwnership());
    m_filterPrototypes.push_back((new tUnaryFilter<UnaryOperation::Classes>())->getOwnership());
    m_filterPrototypes.push_back((new tUnaryFilter<UnaryOperation::Sort>())->getOwnership());
    m_filterPrototypes.push_back((new tUnaryFilter<UnaryOperation::Transpose>())->getOwnership());
    m_filterPrototypes.push_back((new tFourierFilter())->getOwnership());
    m_filterPrototypes.push_back((new tBinaryFilter<BinaryOperation::Bind>())->getOwnership());
    m_filterPrototypes.push_back((new tBandCombination())->getOwnership());


}


void tQMdiSubWindow::paintEvent(QPaintEvent* paintEvent)
{
    QMdiSubWindow::paintEvent(paintEvent);

    //d->cachedStyleOptions.rect = QRect(border, border, width() - 2 * border, titleBarHeight);
    /*QPainter p(this);

    p.setPen( Qt::black );
    p.drawLine( 0,0, size().width(), size().height() );
    p.drawRect(5, 5, size().width() - 10, size().height() - 10);*/
}
