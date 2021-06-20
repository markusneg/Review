#pragma once

//#include <memory>
//#include <list>
//#include <functional>

//#include <QObject>

#include "../common/tObject.hpp"


// a special kind of reference also providing a datatype-specific selector, tSetting integration and signal propagation
template <typename T>
class tDataReference : public tObject
{

public:

    // use the refs of tData type
    typedef typename T::Selector Selector;
    typedef typename T::ConstOwnership ConstOwnership;
    typedef typename T::Ownership Ownership;
    typedef typename T::ConstRef ConstRef;
    typedef typename T::Ref Ref;

    tDataReference();
    tDataReference(T* data);
    tDataReference(Ref& data);
    tDataReference(Ownership& data);

    ~tDataReference();

    void setData(T* data);

    T*          ptr();
    const T*    ptr() const;

    T&          operator*();
    const T&    operator*() const;

    T*          operator->();
    const T*    operator->() const;

    operator bool() const;

    Selector&       getSelector();
    const Selector& getSelector() const;

    void invalidate();

    bool operator==(tDataReference<T>& rhs) const;

    tDataReference(const tDataReference<T>& rhs);
    tDataReference& operator=(const tDataReference<T>& rhs);

    // can be treated as ref
    operator const Ref&() const;
    //operator Ref&();

    // convenience casts to temporarily gain ownership (= deletion lock)
    //operator ConstOwnership() const;
    //operator Ownership();

private:

    T* m_data;
    Selector m_selector;

    template <typename U, typename V>
    friend class tSetting;
};


#include "tDataReference.inl"
