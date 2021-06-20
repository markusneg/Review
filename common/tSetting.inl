#include "tSetting.hpp"

namespace tSettingDetails
{
    // setting should forward signals of its content when it is an tObject
    template <typename T, typename U, typename std::enable_if<!std::is_base_of<tObject,T>::value>::type* = nullptr>
    void forwardSignals(const T&, tSettingBase<U>*, bool = true) {}

    template <typename T, typename U, typename std::enable_if<std::is_base_of<tObject,T>::value>::type* = nullptr>
    void forwardSignals(const T& from, tSettingBase<U>* to, bool forward = true)
    {
        if(forward) from.addSignalForwarding(to);
        else        from.removeSignalForwarding(to);
    }
}


template <typename T>
tSettingBase<T>::tSettingBase() :
    m_value(*new T())
{
    tSettingDetails::forwardSignals(m_value, this);

}

template <typename T>
tSettingBase<T>::tSettingBase(const T& copyFrom) :
    tSettingBase()
{
    //*this = copyFrom; // causes virtual dispatch (valueToWidget)
    m_value = copyFrom;
}

template <typename T>
tSettingBase<T>::tSettingBase(T& existingValue) :
    m_value(existingValue)
{
    setFlags(fJustBorrowed);
    tSettingDetails::forwardSignals(m_value, this);
}

template <typename T>
tSettingBase<T>::tSettingBase(const tSettingBase<T>& rhs) :
    tSettingBase()
{
}

template <typename T>
tSettingBase<T>::~tSettingBase()
{    
    /// [CRIT] remove forwards again
    tSettingDetails::forwardSignals(m_value, this, false);

    if(m_term)      delete m_term;
    if(m_widget)    delete m_widget;

    if(!checkFlags(fJustBorrowed))
        delete &m_value;
}

template <typename T>
inline const T& tSettingBase<T>::value() const
{
    return m_value;
}

template <typename T>
inline T& tSettingBase<T>::value()
{
    return m_value;
}

template <typename T>
void tSettingBase<T>::set(const T& value)
{
    //tSettingDetails::forwardSignals(value, this);
    doCallbacks(sigBeforeChange | sigSettingChanged);
    m_value = value;
    valueChanged();
}

template <typename T>
inline QWidget* tSettingBase<T>::widget()
{
    return m_widget;
}

template <typename T>
inline const QWidget* tSettingBase<T>::widget() const
{
    return m_widget;
}

template <typename T>
inline tSettingBase<T>::operator const T&() const
{
    return m_value;
}

template <typename T>
inline tSettingBase<T>::operator T&()
{
    return m_value;
}

template <typename T>
inline const T& tSettingBase<T>::operator*() const
{
    return m_value;
}

template <typename T>
inline T& tSettingBase<T>::operator*()
{
    return m_value;
}

template <typename T>
inline const T* tSettingBase<T>::operator->() const
{
    return &m_value;
}

template <typename T>
inline T* tSettingBase<T>::operator->()
{
    return &m_value;
}

template <typename T>
inline tSettingBase<T>& tSettingBase<T>::operator=(const T& rhs)
{
    set(rhs);
    return *this;
}

template <typename T>
tSettingBase<T>& tSettingBase<T>::operator=(const tSettingBase<T>& rhs)
{
    set(rhs.m_value);
    return *this;
}

template <typename T>
bool tSettingBase<T>::operator==(const tSettingBase<T>& rhs) const
{
    return m_value == rhs.m_value;
}

template <typename T>
bool tSettingBase<T>::operator!=(const tSettingBase<T>& rhs) const
{
    return !(m_value == rhs.m_value);
}

template <typename T>
tSettingBase<T>::operator bool() const
{
    return m_value;
}

template <typename T>
void tSettingBase<T>::valueChanged()
{
    valueToWidget();
    doCallbacks(sigChanged | sigSettingChanged);
}

template <typename T>
void tSettingBase<T>::premangleText(QString& entry)
{
    // variable definition
    if(entry.startsWith("->"))
    {
        entry.remove(0, 2);
        if(!entry.contains(QRegExp("[\\W]")))
        {
            setName(entry);
            exportAsVariable();
            entry.clear();

            //m_variable->addCallback([&]() {m_variable->doCallbacks(sigChanged);}, sigChanged);

        }
    }

    else if(entry.startsWith("="))
    {
        initTerm();
        entry.remove(0, 1);
        *m_term = entry;
        entry.clear();
    }
}

template <typename T>
void tSettingBase<T>::initTerm()
{
    if(!m_term)
    {
        m_term = new tTerm;
        m_term->addCallback([&]() {this->setFromString(this->m_term->result());}, sigChanged);
    }
}

// regard the default traits
#include "tSettingTraits.inl"

