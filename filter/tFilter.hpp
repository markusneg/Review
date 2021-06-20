#pragma once

#include <vector>

#include <functional>

#include "../common/tObject.hpp"

class QWidget;
class tWindow;
class tData;
template <typename, typename> class tSetting;
template <typename> class Graph;

class tFilter : public tObject
{

public:

    T_TYPEDEFS(tFilter)

    // get a copy of the prototyped filter associated with a window
    virtual tFilter* copy(tWindow*) const = 0;

    ~tFilter();

    QString getName() const override;
    void    setName(const QString& name) override;

    //const tWindow*  getInputWindow() const;

    // run the filter action (relies on run_impl()). Also runs dependant filters. When notify is true, changed-signals of data are emitted
    void    run(bool notify = true, bool runFollowing = true);

    // enable or disable the filter. Dependent on implementation, filter could e.g. only bypass
    bool            isEnabled() const;
    virtual void    setEnabled(bool enabled = true);

    // returns the settings widget provided by derived class. May be nullptr!
    const QWidget*  getSettingsWidget() const;
    QWidget*        getSettingsWidget();

    // can be reimplemented and report all data, this filter currently operates on. Needed for determination of in/output (filter-)windows
    // per default, reports data registered using registerInput()
    virtual std::vector<tData*> getInputData();

    // filter should react by introducing window and/or its data to its input fields
    virtual void setInputWindow(tWindow* window);

    /// should be protected!
    const std::list<tFilter*>&   getFiltersFollowing();

protected:

    typedef std::function<tData*(void)> VirtualDataPtr;

    /// [DEPRECATED] (use setInputWindow()!)
    // is called when input windows changed. Reimplement to let filter prepare for new input and return true if smth changed
    // default: get first window from list and return true if m_currentInputWindow changed
    virtual bool updateInputWindow(const tList<tWindow*>& list);

    // interface for derived filters to create "owned data".
    // Filters themselfes are not expected to own data: they are always destroyed before the owning window
    void registerOwnedData(tData* data);
    void unregisterOwnedData(tData* data);

    // for determination of in- and output relations of windows, inputs of filters have to be registered
    template <typename T>
    void registerInput(T& input);
    void registerInput(tSetting<tWindow*, void>& inputWindow);

    template <typename T>
    void unregisterInput(T& input);
    void unregisterInput(tSetting<tWindow*, void>& inputWindow);

    void runFiltersFollowing(bool notify = true);
    void gatherFiltersFollowing(Graph<tFilter>* graph = nullptr); // gather all following filters and deploy them topsorted in a local list for rapid access

    tFilter(tWindow* window = nullptr);

    tWindow* const m_window;

    QWidget* m_inputSelect = nullptr;
    QWidget* m_settings = nullptr;

    const tWindow* m_currentInputWindow = nullptr;

private:

    struct Private;
    Private* m_private;

    // here the actual filter action has to be implemented. Should not trigger signals when notify is false!
    virtual void run_impl(bool notify) = 0;

    friend class tWindow; // can equip filters following - list

    QString m_name;
    QString m_type;

    std::list<VirtualDataPtr> m_inputs;

    bool m_enabled = true;

    //tFilter::List m_filtersFollowing;
    //std::vector<tFilter*> m_filtersFollowing;
    //QMutex m_filterMutex;
};


// eqip derived views with this macro to enably copying
#define T_FILTER_COPYABLE(this_type) private: tFilter* copy(tWindow* window) const override {return new this_type(window);}


#include "tFilter.inl"
