#pragma once

#include <vector>

#include <QWidget>
#include <QProxyStyle>
#include <QMutex>

class Review; class tView; class tFilter; class QMdiSubWindow;

#include "tObject.hpp"
#include "../data/tData.hpp"
#include "tList.hpp"


class tWindow : public QWidget, public tObject
{

    Q_OBJECT

public:

    T_TYPEDEFS(tWindow)

    tData::List&         getData();
    const tData::List&   getData() const;

    tData::List         getSpecificData(tFlags flags);
    tData::ConstList    getSpecificData(tFlags flags) const;

    tView*          getView();
    const tView*    getView() const;

    tFilter*        getFilter();
    const tFilter*  getFilter() const;

    void addDataRef(tData* data);

    void addInputWindow(tWindow* input);
    void removeInputWindow(tWindow* input);

    void addOutputWindow(tWindow* output);
    void removeOutputWindow(tWindow* output);

    const tWindow::List& getInputWindows() const;
    const tWindow::List& getOutputWindows() const;

    // assemble window- and data graph, conduct a topsort and return an *inverted* priority list of data
    // for this method to work, updateWindowRelations() has to be called before
    //std::vector<tData*> topologicalSortOfData();

    // maintain window structures (new, delete) for short time periods
    static void lockWindows(bool lock = true);

    void setName(const QString&) override;

protected:

private:

    // lifetime completely managed by Review
    tWindow();
    ~tWindow();

    // data is copied
    tWindow(tWindow& rhs);
    tWindow& operator=(tWindow& rhs);
    void assign(tWindow& rhs);

    // set view or filter by copying it from existing ones. Returns newly created objects
    tView*      setView(const tView* prototype);
    tFilter*    setFilter(const tFilter* prototype);

    // ownership management of data happens here! only attached filters and Review may add ownership
    void addData(tData* data);
    void removeData(tData* data);

    // dependent on its type (ducktyped by filter and data), update the appearance of the parent QMdiSubWindow
    void updateMdiSubWindow();

    // recursive method for gathering a complete, *continuous* graph of windows for this window (independent graphs are not included)
    enum Direction {Inputs, Outputs, Both};
    void getWindowGroup(std::vector<tWindow*>& windows, Direction dir = Both);

    // recalculate input/output relations between all windows. Also, prepare the topsorted filter lists
    static void updateWindowRelations();

    tOwnership<tView>   m_view;
    tOwnership<tFilter> m_filter;

    tData::OwnershipList    m_dataOwned; // only contains owned data
    tData::List             m_data;      // contains owned data + referenced data

    tWindow::List m_inputWindows;
    tWindow::List m_outputWindows;

    static std::vector<tWindow*> m_windows;
    static QMutex m_windowsMutex;

    friend class Review; // may create windows, equip views and filter etc
    friend class tFilter; // can create new data
};



// used for coloring of windows
class tWindowStyle : public QProxyStyle
{
    Q_OBJECT

public:

    enum WindowType {Data, Reference, Filter, Observer};

    tWindowStyle(WindowType type);

    void setActive(bool active = true);

    void polish(QPalette &pal) override;    /// a pity this one does not seem to work for mdiSubwindows..
    void polish(QWidget *pal) override;

    void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget = 0) const override;

    int pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const override;

private:

    WindowType m_windowType;
    bool m_active = true;

};


