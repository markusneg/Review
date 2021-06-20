#include "tFilter.hpp"

#include <QObject>

#include "../common/tWindow.hpp"
#include "../common/tSetting.hpp"
#include "../util/Graph.hpp"


struct tFilter::Private
{
    tFilter::List m_filtersFollowing;
};


tFilter::tFilter(tWindow* window) :
    m_window(window), m_private(new Private), m_name(QObject::tr("Unnamed"))
{
    //if(window) window->addCallback([&, window]()
    //    {updateInputWindow(window->getInputWindows());}, sigInputChanged);
}

tFilter::~tFilter()
{
    delete m_private;
}

QString tFilter::getName() const
{
    return m_name;
}

void tFilter::setName(const QString& name)
{
    m_name = name;
}

void tFilter::run(bool notify, bool runFollowing)
{
    run_impl(notify);

    if(runFollowing)
        runFiltersFollowing(notify);
}

void tFilter::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

/*const tWindow* tFilter::getInputWindow() const
{
    return m_inputWindow;
}

void tFilter::setInputWindow(const tWindow* input)
{
    m_inputWindow = input;
    doCallbacks(sigInputChanged);
}*/


const QWidget* tFilter::getSettingsWidget() const
{
    return m_settings;
}

QWidget* tFilter::getSettingsWidget()
{
    return m_settings;
}

std::vector<tData*> tFilter::getInputData()
{
    std::vector<tData*> inputs;

    // convert "virtual ptrs" to data
    for(auto& virtualPtr : m_inputs)
        if(tData* ptr = virtualPtr())
            inputs.push_back(ptr);

    return inputs;
}

void tFilter::setInputWindow(tWindow*)
{
    std::cout << "[tFilter] unimplemented setInputWindow() called!\n" << std::flush;
}

bool tFilter::updateInputWindow(const tList<tWindow*>& list)
{
    if(list.empty())
        {m_currentInputWindow = nullptr; return false;}

    if(list.front() == m_currentInputWindow)
        return false;

    m_currentInputWindow = list.front();
    return true;
}

void tFilter::registerOwnedData(tData* data)
{
    if(m_window) m_window->addData(data); /// [ToDo] addData was meant to be used with pre-loaded data from HD
}

void tFilter::unregisterOwnedData(tData* data)
{
    if(m_window) m_window->removeData(data);
}

void tFilter::registerInput(tSetting<tWindow*, void>& inputWindow)
{
    inputWindow.addCallbackSafe([&]()
    {
        if(inputWindow)
            (*inputWindow)->removeOutputWindow(m_window);

    }, this, sigAND | sigSettingChanged | sigBeforeChange);

    inputWindow.addCallbackSafe([&]()
    {
        if(inputWindow)
            (*inputWindow)->addOutputWindow(m_window);

        run(); // user changed input parameters

    }, this, sigSettingChanged);
}

void tFilter::unregisterInput(tSetting<tWindow*, void>& inputWindow)
{

}

const std::list<tFilter*>& tFilter::getFiltersFollowing()
{
    return m_private->m_filtersFollowing;
}

void tFilter::runFiltersFollowing(bool notify)
{
    for(tFilter* nextFilter : m_private->m_filtersFollowing)
        nextFilter->run_impl(notify);
}

void tFilter::gatherFiltersFollowing(Graph<tFilter>* graph)
{
    bool firstCall = false;

    if(!graph)
    {
        m_private->m_filtersFollowing.clear();
        tWindow::lockWindows();
        graph = new Graph<tFilter>;
        firstCall = true;
    }

    for(tWindow* w : m_window->getOutputWindows())
        if(tFilter* f = w->getFilter())
        {
            graph->addConnection(f, this);
            f->gatherFiltersFollowing(graph);
        }

    if(firstCall)
    {
        tWindow::lockWindows(false);

        graph->topologicalSort();

        if(!graph->getSorted().empty())
            for(auto it = ++(graph->getSorted().begin()); it != graph->getSorted().end(); ++it) // leave first out: is this filter
                m_private->m_filtersFollowing.push_back(*it);

        delete graph;
    }
}
