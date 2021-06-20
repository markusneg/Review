#include "tWindow.hpp"

#include <QVBoxLayout>
#include <QMdiSubWindow>

#include "../view/tView.hpp"
#include "../filter/tFilter.hpp"
#include "../filter/tObserver.hpp"
#include "../util/util.hpp"
#include "../util/Graph.hpp"        /// maybe can be removed
#include "../data/tMatrixData.hpp"


#define RESIZE() QWidget::resize(220,100);


std::vector<tWindow*> tWindow::m_windows;
QMutex tWindow::m_windowsMutex(QMutex::Recursive);


tWindow::tWindow()
{
    setMinimumSize(100, 0);

    auto verticalLayout = new QVBoxLayout(this);
    verticalLayout->setSpacing(0);
    verticalLayout->setContentsMargins(0, 0, 0, 0);

    RESIZE();

    m_windowsMutex.lock();
    m_windows.push_back(this);
    m_windowsMutex.unlock();
}

tWindow::~tWindow()
{
    doCallbacks(sigDecaying);

    // manage inpuut/output windows
    /*for(tWindow* inputWindow : m_inputWindows)
        removeFromVector(inputWindow->m_outputWindows, this);

    for(tWindow* outputWindow : m_outputWindows)
        removeFromVector(outputWindow->m_inputWindows, this);*/

    m_windowsMutex.lock();
    removeFromVector(m_windows, this);
    m_windowsMutex.unlock();

    /// generally, a bad idea! due to window at this point not completely destroyed it could be "re-owned"
    /// right after sigDecaying due to in-/output-detection mechanism of windows
    //updateWindowRelations();
}

/// [INFO] tWindow is not const because we may want to get non-const data-refs

tWindow::tWindow(tWindow& rhs) :
    QWidget(), tObject(rhs)
{
    assign(rhs);
}

tWindow& tWindow::operator=(tWindow& rhs)
{
    tObject::operator=(rhs);
    assign(rhs);
    return *this;
}

void tWindow::assign(tWindow& rhs)
{
    setName(rhs.getName());

    for(tData* rhsdata : rhs.m_data)
    {
        tData::Ownership newdata = rhsdata->copy();
        newdata->m_ownerWindow = this;
        m_dataOwned.push_back(newdata);
        m_data.push_back(newdata.get());
    }
}


tData::List& tWindow::getData()
{
    return m_data;
}

const tData::List& tWindow::getData() const
{
    return m_data;
}

tData::List tWindow::getSpecificData(tFlags flags)
{
    tData::List ret;

    m_data.lock();
    for(tData* d : m_data)
        if(d->checkFlags(flags))
            ret.push_back(d);
    m_data.unlock();

    return ret;
}

tData::ConstList tWindow::getSpecificData(tFlags flags) const
{
    tData::ConstList ret;

    m_data.lock();
    for(tData* d : m_data)
        if(d->checkFlags(flags))
            ret.push_back(d);
    m_data.unlock();

    return ret;
}

tView* tWindow::setView(const tView *prototype)
{
    if(prototype)   m_view = prototype->copy(this)->getOwnership();
    else            m_view = nullptr;

    if(m_view)      layout()->addWidget(m_view.get());

    RESIZE();
    return m_view.get();
}

tFilter* tWindow::setFilter(const tFilter* prototype)
{
    if(prototype)
    {
        m_filter = prototype->copy(this)->getOwnership();
        setName(m_filter->getName());
    }

    else m_filter = nullptr;

    return m_filter.get();
}


tView* tWindow::getView()
{
    return m_view.get();
}

const tView* tWindow::getView() const
{
    return m_view.get();
}

tFilter* tWindow::getFilter()
{
    return m_filter.get();
}

const tFilter* tWindow::getFilter() const
{
    return m_filter.get();
}

void tWindow::addData(tData* data)
{
    m_dataOwned.push_back(data->getOwnership());
    m_data.push_back(data);

    if(data->m_ownerWindow) T_COUT("[tWindow] Critical: added data was owned before. Ownership is now unhandled!");
    data->m_ownerWindow = this;

    // check for factor system
    /*if(m_dataOwned.size() == 2)
    {
        tMatrixData *data0 = dynamic_cast<tMatrixData*>(m_dataOwned.front().get()), *data1 = dynamic_cast<tMatrixData*>(data);
        assert(data0 && data1);

        if(data0->n_cols() == data1->n_cols() && data0->n_rows() != data1->n_rows())
        {
            if(data0->n_rows() > data1->n_rows())
                {data0->setFlags(fRotation); data1->setFlags(fRotation);}
            else
                {data0->setFlags(fRotation); data1->setFlags(fRotation);}

            T_COUT("[tWindow] factor system recognized..");
        }
    }*/
}

void tWindow::removeData(tData* data)
{
    m_data.remove(data);
    m_dataOwned.remove(data->getOwnership());
    data->m_ownerWindow = nullptr;
}


void tWindow::addDataRef(tData* data)
{
    m_data.push_back(data);
}

void tWindow::updateMdiSubWindow()
{
    if(!parent()) return;

    QMdiSubWindow* subWindow = dynamic_cast<QMdiSubWindow*>(parent());
    if(!subWindow) return;

    if(m_dataOwned.empty())
    {
        subWindow->setStyle(new tWindowStyle(tWindowStyle::Reference));
        subWindow->setWindowIcon(QIcon(":/img/ico_r.png"));
    }
    else if(m_filter)
    {
        if(dynamic_cast<const tObserver*>(m_filter.get()))
        {
            subWindow->setStyle(new tWindowStyle(tWindowStyle::Observer));
            subWindow->setWindowIcon(QIcon(":/img/ico_o.png"));
        }
        else
        {
            subWindow->setStyle(new tWindowStyle(tWindowStyle::Filter));
            subWindow->setWindowIcon(QIcon(":/img/ico_f.png"));
        }
    }
    else
    {
        subWindow->setStyle(new tWindowStyle(tWindowStyle::Data));
        subWindow->setWindowIcon(QIcon(":/img/ico_d.png"));
    }

    subWindow->resize(200,150);
}


#define EMIT_INOUT_CHANGED(in, out) in->doCallbacks(sigOutputChanged); out->doCallbacks(sigInputChanged);

void tWindow::addInputWindow(tWindow* input)
{
    if(isInContainer(m_inputWindows, input)) return;

    m_inputWindows.push_back(input);
    input->m_outputWindows.push_back(this);

    EMIT_INOUT_CHANGED(input, this)
}

void tWindow::removeInputWindow(tWindow* input)
{
    if(isInContainer(m_inputWindows, input))
    {
        m_inputWindows.remove(input);
        input->m_outputWindows.remove(this);

        EMIT_INOUT_CHANGED(input, this)
    }
}

void tWindow::addOutputWindow(tWindow* output)
{
    if(isInContainer(m_outputWindows, output)) return;

    m_outputWindows.push_back(output);
    output->m_inputWindows.push_back(this);

    EMIT_INOUT_CHANGED(this, output)
}

void tWindow::removeOutputWindow(tWindow* output)
{
    if(isInContainer(m_outputWindows, output))
    {
        m_outputWindows.remove(output);
        output->m_inputWindows.remove(this);

        EMIT_INOUT_CHANGED(this, output)
    }
}

const tWindow::List& tWindow::getInputWindows() const
{
    return m_inputWindows;
}

const tWindow::List& tWindow::getOutputWindows() const
{
    return m_outputWindows;
}

/*std::vector<tData*> tWindow::topologicalSortOfData()
{
    // be sure that in- and outputs are up to date
    // updateWindowRelations();

    // first, we have to acquire a complete linked-window list as continuous graph
    std::vector<tWindow*> windows;
    getWindowGroup(windows);

    // assemble graph. For every window, add the data and corresponding output data. The latter has to be found
    // manually by specificly searching the output windows either for filter-input (-or refs-)
    Graph<tData> graph;
    for(tWindow* currentWindow : windows)
    {
        for(tData* currentData : currentWindow->getData())
        {
            for(tWindow* outputWindow : currentWindow->getOutputWindows())
            {
                tFilter* outputFilter = outputWindow->getFilter();

                if(outputFilter)
                    for(const tData* inputData : outputFilter->getInputData())      // look if currentData is..
                        if(currentData == inputData)                                // .. associated especially with output filter..
                            for(tData* connectedData : outputWindow->getData())     // .. and if true associate all output data of the filter..
                                graph.addConnection(currentData, connectedData);    // .. with the current data

                // it should be OK to ignore refs
                // otherwise just associate all outpu
                //else for(tData* connectedData : outputWindow->getData())
            }
        }
    }

    graph.topologicalSort();
    return graph.getSorted();
}*/

void tWindow::lockWindows(bool lock)
{
    if(lock)    m_windowsMutex.lock();
    else        m_windowsMutex.unlock();
}

void tWindow::setName(const QString& name)
{
    setWindowTitle(name);
    tObject::setName(name);
}

void tWindow::getWindowGroup(std::vector<tWindow*>& windows, Direction dir)
{
    if(dir == Inputs || dir == Both)
        for(tWindow* w : m_inputWindows)
            w->getWindowGroup(windows, Inputs);

    if(dir == Outputs || dir == Both)
        for(tWindow* w : m_outputWindows)
            w->getWindowGroup(windows, Outputs);

    windows.push_back(this);
}

void tWindow::updateWindowRelations()
{
    m_windowsMutex.lock();

    // wipe all previous relations
    for(tWindow* window : m_windows)
        {window->m_inputWindows.clear(); window->m_outputWindows.clear();}

    // derive window relations using filter inputs and..
    // .. also assemble graph to get a topological sort of windows
    //Graph<tWindow> graph;
    for(tWindow* window : m_windows)
    {
        if(tFilter* filter = window->getFilter())
            for(tData* inputData : filter->getInputData())
                window->addInputWindow(inputData->getOwnerWindow());

        // window is reference
        else if(window->m_dataOwned.empty() && !window->m_data.empty())
            window->addInputWindow(window->m_data.front()->getOwnerWindow());

        // also add to graph for topsorting
        //for(tWindow* inputWindow : window->getInputWindows())
        //    graph.addConnection(window, inputWindow);
    }

    // update filter-following lists regarding topsorted relations
    for(tWindow* window : m_windows)
    {
        if(tFilter* filter = window->getFilter())
        {
            filter->gatherFiltersFollowing();

            /*std::cout << filter->getName().toStdString() << ": ";

            for(tFilter* ff : filter->getFiltersFollowing())
                std::cout << ff->getName().toStdString() << ", ";

            std::cout << std::endl << std::flush;*/
        }
    }

    m_windowsMutex.unlock();


    /*graph.topologicalSort();

    std::cout << "Sort: ";
    std::vector<tFilter*> filters; auto& sorted = graph.getSorted();
    for(tWindow* window : sorted)
    {
        if(tFilter* filter = window->getFilter())
            filters.push_back(filter);

        std::cout << window->windowTitle().toStdString() << " ";
    }

    std::cout << std::endl << std::flush;

    std::vector<tFilter*>::iterator it1, it2;
    for(it1 = filters.begin(); it1 != filters.end(); ++it1)
    {
        (*it1)->m_filtersFollowing.clear();

        for(it2 = it1 + 1; it2 != filters.end(); ++it2)
            (*it1)->m_filtersFollowing.push_back(*it2);
    }*/
}


#define COLOR_DATA      "#c6856e"
#define COLOR_REF       "#b1b1b1"
#define COLOR_FILTER    "#a5cace"
#define COLOR_OBSERVER  "#b9b842"
#define COLOR_OBSERVERI "#abb985"
#define COLOR_HL_FACT   120

#define TITLE_FONTSIZE  11

#include <QPainter>

tWindowStyle::tWindowStyle(WindowType type) :
    QProxyStyle("fusion"), m_windowType(type)
{

}

void tWindowStyle::setActive(bool active) {m_active = active;}

void tWindowStyle::polish(QPalette &pal)
{
    pal.setColor(QPalette::Window, QColor(239, 239, 247));
}

void tWindowStyle::polish(QWidget*)
{
}

#include <qstyleoption.h>
void tWindowStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    if(control == CC_TitleBar)
    {
        if(const QStyleOptionTitleBar *titleBar = qstyleoption_cast<const QStyleOptionTitleBar *>(option))
        {
            QStyleOptionTitleBar optionCopy = *titleBar;
            QPalette& pal = optionCopy.palette;

            /// hightlighted text color cannot be adjusted due to bug in fusion style!
            //for(int z = 0; z < (int)QPalette::ToolTipText; ++z)
            //    pal.setBrush((QPalette::ColorRole)z, QBrush(Qt::red));

            QFont font = painter->font();
            font.setPointSize(TITLE_FONTSIZE);
            painter->setFont(font);

            switch(m_windowType)
            {
            case Data:
                pal.setColor(QPalette::Background, QColor(COLOR_DATA));
                pal.setColor(QPalette::Highlight, QColor(COLOR_DATA).lighter(COLOR_HL_FACT));
                break;
            case Reference:
                pal.setColor(QPalette::Background, QColor(COLOR_REF));
                pal.setColor(QPalette::Highlight, QColor(COLOR_REF).lighter(COLOR_HL_FACT));
                break;
            case Filter:
                pal.setColor(QPalette::Background, QColor(COLOR_FILTER));
                pal.setColor(QPalette::Highlight, QColor(COLOR_FILTER).lighter(COLOR_HL_FACT));
                break;
            case Observer: {
                const char* str = m_active ? COLOR_OBSERVER : COLOR_OBSERVERI;
                pal.setColor(QPalette::Background, QColor(str));
                pal.setColor(QPalette::Highlight, QColor(str).lighter(COLOR_HL_FACT));
                } break;
            }

            QProxyStyle::drawComplexControl(control, &optionCopy, painter, widget);
            return;
        }

    }

    QProxyStyle::drawComplexControl(control, option, painter, widget);

}

int tWindowStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    if(metric == PM_TitleBarHeight)
        return 21;
    else
        return QProxyStyle::pixelMetric(metric, option, widget);
}
