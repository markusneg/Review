#pragma once

#include <functional>
#include <vector>

//#include <QWidget>
class QWidget;

#include "tObject.hpp"
#include "tTerm.hpp"
#include "tVariable.hpp"

// can be provided by user to enable custom tSettings
/*template <typename T>
struct tSettingTraits
{
    typedef QWidget WidgetType;
    typedef void* AuxiliaryType;
    static void setup(tSetting<T>&) {}
    static void valueToWidget(T&, WidgetType*, AuxiliaryType = nullptr) {}
    static void setAuxiliary(AuxiliaryType aux, tSetting<T>& set) {}            // called before aux is set using tSetting::setAuxiliary
};*/

// a template class managing correspondence between a value T and an associated Qt widget

// the base for tSetting
template <typename T>
class tSettingBase : public tObject
{

public:

    tSettingBase();
    tSettingBase(const T& copyFrom);
    tSettingBase(T& existingValue);
    tSettingBase(const tSettingBase<T>& rhs);
    ~tSettingBase();

    const T&    value() const;
    T&          value();

    virtual void set(const T& value);

    const QWidget*   widget() const;
    QWidget*         widget();

    operator const T&() const;
    operator T&();

    const T&    operator*() const;
    T&          operator*();

    const T*    operator->() const;    // want to have "drill-down behavior" here?
    T*          operator->();

    // same as set()
    tSettingBase& operator=(const T& rhs);
    tSettingBase& operator=(const tSettingBase<T>& rhs);

    // delegate to value type
    bool operator==(const tSettingBase<T>& rhs) const;
    bool operator!=(const tSettingBase<T>& rhs) const;
    operator bool() const;

    //void setName(const QString& name) override;

    // needed to be called by user when value was modified
    virtual void valueChanged();

    // defines how value is rendered to a QWidget. Normally has not to be run manually
    virtual void valueToWidget() {}
    virtual void widgetToValue() {}

    virtual void setFromString(const QString&) {}


protected:

    // to be overriden for use in derived destructor
    void destroy() {}

    // used to check some tSetting-wide commands in text entered by user (e.g. variable export by text command)
    // when special control strings are found, they are removed from string or empty string is returned
    void premangleText(QString& entry);

    // create term on this setting
    void initTerm();

    T& m_value;

    // may be set by derived. When set, lifetime his handled by this base
    QWidget* m_widget = nullptr;

    // every setting may be provided with a term containing variables and operations for extended control.
    // in textfields, term may be entered directly, others may be accessed via context menu
    tTerm* m_term = nullptr;

    // every setting may also export itself as variable which can be used by terms
    //tVariable* m_variable = nullptr;

private:



};


template <typename T, typename Enable = void>
class tSetting;



#include "tSetting.inl"
