#pragma once

#include <mutex>
#include <memory>
#include <list>

#include "tObject.hpp"


// a smart list regarding object lifetime and threadsafety
template <typename T>
class tList : public tObject, public std::list<T>
{

public:

    tList();
    ~tList();
    tList(const tList& other);
    tList& operator=(const tList& other);

    using std::list<T>::list;

    // for these list-methods registering is already done
    void push_back(T o);
    void push_back(const tReference<T>& ref);

    void remove(T o);
    void clear();

    void lock() const {m_mutex.lock();}
    void unlock() const {m_mutex.unlock();}

    void registerObject(const T& o);
    void unregisterObject(const T& o);

private:

    mutable std::recursive_mutex m_mutex;

};


#include "tList.inl"
