#pragma once

#include <mutex>
#include <memory>


// a lockable std::list or std::vector container. Others may be possible
template <typename T, template <class, class> class Container>
class tLockableContainer : public Container<T, std::allocator<T>>, public std::recursive_mutex
{

public:

    using Container<T, std::allocator<T>>::Container;

    //void lock() {m_mutex.lock();}
    //void unlock() {m_mutex.unlock();}

private:

    //std::recursive_mutex m_mutex;

};


