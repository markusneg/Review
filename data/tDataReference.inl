#include "tDataReference.hpp"

#include <iostream>

#include <QVBoxLayout>

#include "../util/util.hpp"
#include "../util/tLockableContainer.hpp"
#include "../common/tSetting.hpp"
#include "tMatrixDataSelection.hpp"


// combine the data-ref and selectors
template <typename T>
class tSetting<tDataReference<T>> : public tSettingBase<tDataReference<T>>
{

public:

    T_SETTING_DEFS(tSetting, tDataReference<T>)

    void destroy() {delete m_setRef; delete m_setSel;}

    void init()
    {
        //if(!set.m_aux) return;

        this->setName("Setting (DataRef)");

        this->m_value.addSignalForwarding(this);

        Base::m_widget = new QWidget;
        QVBoxLayout* layout = new QVBoxLayout(Base::m_widget);
        layout->setContentsMargins(1, 1, 1, 1); layout->setSpacing(1);

        // create setting for data reference
        m_setRef = new tSetting<T*>(Base::m_value.m_data);
        m_setRef->addSignalForwarding(this);

        // setting for selector
        m_setSel = new tSetting<typename T::Selector>(Base::m_value.getSelector());
        m_setSel->addSignalForwarding(this);

        // child settings value change also has to be regarded
        m_setRef->addCallback([&]()
        {
            this->m_value.getSelector().setData(**m_setRef);
        }, sigChanged);

        //setSel.addCallback([&](){this->doCallbacks(sigChanged);}, sigChanged);

        layout->addWidget(m_setRef->widget()); layout->addWidget(m_setSel->widget());
        //repopulateItems(box, set.m_value, set.m_aux);
    }

    template <typename U>
    void setList(const U* list)
    {
        m_setRef->setList(list);
    }

protected:

    tSetting<T*>* m_setRef;
    tSetting<typename T::Selector>* m_setSel;

};

/*template <typename T, typename U>
void setList(tSetting<tDataReference<T>>& set, const U& list)
{
    set.setList(new tListProxyImpl<T*,U>(list));
}*/


template <typename T>
tDataReference<T>::tDataReference() :
    tDataReference(nullptr)
{

}

template <typename T>
tDataReference<T>::tDataReference(T* data) :
    m_data(data)
{

}

template <typename T>
tDataReference<T>::tDataReference(Ref& data) :
   tDataReference(PTR(data))
{

}

template <typename T>
tDataReference<T>::tDataReference(Ownership& data) :
    tDataReference(data.get())
{

}

template <typename T>
tDataReference<T>::~tDataReference()
{
    doCallbacks(sigDecaying);
}

template <typename T>
T* tDataReference<T>::ptr()
{
    return m_data;
}

template <typename T>
const T* tDataReference<T>::ptr() const
{
    return m_data;
}

/*tDataReference::operator T*()
{
    return &(*m_ptr);
}*/

template <typename T>
T& tDataReference<T>::operator*()
{
    return *m_data;
}

template <typename T>
const T& tDataReference<T>::operator*() const
{
    return *m_data;
}

template <typename T>
T* tDataReference<T>::operator->()
{
    return m_data;
}

template <typename T>
const T* tDataReference<T>::operator->() const
{
    return m_data;
}

template <typename T>
inline tDataReference<T>::operator bool() const
{
    return m_data;
}

template <typename T>
typename T::Selector& tDataReference<T>::getSelector()
{
    return m_selector;
}

template <typename T>
const typename T::Selector& tDataReference<T>::getSelector() const
{
    return m_selector;
}

template <typename T>
void tDataReference<T>::invalidate()
{
    m_data.reset();
}

template <typename T>
inline bool tDataReference<T>::operator==(tDataReference<T>& rhs) const
{
    return m_data == rhs.m_data;
}

template <typename T>
tDataReference<T>::tDataReference(const tDataReference<T>& rhs)
{
    operator=(rhs);
}


template <typename T>
tDataReference<T>& tDataReference<T>::operator=(const tDataReference<T>& rhs)
{
    m_data = rhs.m_data;
    m_selector = rhs.m_selector;
    doCallbacks(sigChanged);
    return *this;
}

template <typename T>
tDataReference<T>::operator const Ref&() const
{
    return m_data;
}

/*template <typename T>
tDataReference<T>::operator Ref&()
{
    return m_data;
}*/

/*template <typename T>
tDataReference<T>::operator tDataReference::ConstOwnership() const
{
    return m_data.lock();
}

template <typename T>
tDataReference<T>::operator tDataReference::Ownership()
{
    return m_data.lock();
}*/
