#include "tList.hpp"

template <typename T>
tList<T>::tList() :
    std::list<T>()
{
    setName("List");
}

template <typename T>
tList<T>::~tList()
{
    doCallbacks(sigDecaying);
}

template <typename T>
tList<T>::tList(const tList& other) :
    tObject(), std::list<T>(other)
{
    setName("List");
}

template <typename T>
tList<T>& tList<T>::operator=(const tList& other)
{
    std::list<T>::operator=(other);
    return *this;
}


template <typename T>
void tList<T>::push_back(T o)
{
    lock();
    registerObject(o);
    std::list<T>::push_back(o);
    unlock();

    doCallbacks(sigDataChanged);
}

template <typename T>
void tList<T>::push_back(const tReference<T>& ref)
{
    push_back(PTR(ref));
}

template <typename T>
void tList<T>::remove(T o)
{
    lock();
    unregisterObject(o);
    std::list<T>::remove(o);
    unlock();

    doCallbacks(sigDataChanged);
}

template <typename T>
void tList<T>::clear()
{
    lock();

    for(T o : *this)
        unregisterObject(o);

    std::list<T>::clear();

    unlock();

    doCallbacks(sigDataChanged);
}

namespace tListDetails
{
    /*template <typename T>
    T* getPtr(T* o) {return o;}

    template <typename T>
    T* getPtr(const std::shared_ptr<T>& o) {return o.get();}

    template <typename T>
    T* toOriginal(T* ptr) {return ptr;}

    template <typename T>
    T toOriginal(std::shared_ptr<T> ptr) {return ptr;}*/

    template<typename T>
    struct is_shared_ptr : std::false_type {};

    template<typename T>
    struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};
}


template <typename T>
void tList<T>::registerObject(const T& o)
{
    // we have to prevent storage of a shared_ptr in a lambda
    // ps: objects behind shared_ptr never decay when list is existing
    if(tListDetails::is_shared_ptr<T>::value) return;

    o->addCallbackSafe([&,o]()
    {
        lock();
        std::list<T>::remove(o);
        unlock();

    }, this, sigDecaying);
}

template <typename T>
void tList<T>::unregisterObject(const T& o)
{
    if(tListDetails::is_shared_ptr<T>::value) return;

    o->removeCallback(this);
}
