#pragma once

#include "tObject.hpp"

#include <iostream>


namespace tObjectDetails
{
    //namespace std {template<typename _Tp, typename _Alloc>  class forward_list;}

    template <typename T, typename std::enable_if<std::is_base_of<tObject,T>::value>::type* = nullptr>
    size_t getHash(const T* obj)
    {
        std::hash<const tObject*> hash;
        return hash(static_cast<const tObject*>(obj));
    }

    template <typename T, typename std::enable_if<!std::is_base_of<tObject,T>::value>::type* = nullptr>
    size_t getHash(const T* obj)
    {
        std::hash<const T*> hash;
        return hash(obj);
    }

    template <typename T>
    inline size_t count(const T& list)
    {
        return std::distance(list.begin(), list.end());
    }
}

template <typename T>
tReference<const T> tObject::getRefOfType() const
{
    /*std::shared_ptr<const T> ptr;

    try {ptr = shared_from_this();} catch (...)
        {std::cout << "[tObject] Error: could not create Reference!" << std::endl << std::flush; return tReference<const T>();}

    std::shared_ptr<const T> casted_ptr = std::dynamic_pointer_cast<const T>(ptr);

    return tReference<const T>(casted_ptr);*/

    return std::static_pointer_cast<const T>(shared_from_this());
}

template <typename T>
tReference<T> tObject::getRefOfType()
{
    return std::static_pointer_cast<T>(shared_from_this());


    /*ConstRef const_ref = getRefOfType<T>();

    T* ptr = static_cast<T*>(const_cast<tObject*>(&(*const_ref.lock())));

    return tOwnership<T>(ptr);*/
}

template <typename T>
tOwnership<T> tObject::getOwnershipOfType()
{
    tOwnership<tObject> ret;

    try {ret = shared_from_this();} catch (...)
        {ret = tOwnership<tObject>(this);}

    return std::static_pointer_cast<T>(ret);
}

template <typename T>
tOwnership<const T> tObject::getOwnershipOfType() const
{
    tOwnership<const tObject> ret;

    try {ret = shared_from_this();} catch (...)
        {ret = tOwnership<const tObject>(this);}

    return std::static_pointer_cast<const T>(ret);
}


template <typename T>
void tObject::addCallback(const Callback& callback, const T* id, tSignals signal) const
{
    addCallback(callback, tObjectDetails::getHash(id), signal);
}

template <typename T>
void tObject::removeCallback(const T* id) const
{
    removeCallback(tObjectDetails::getHash(id));
}

inline bool tObject::checkFlags(tFlags flags) const
{
    return m_flags & flags;
}

inline tFlags tObject::getFlags() const
{
    return m_flags;
}

inline void tObject::setFlags(tFlags flags)
{
    m_flags |= flags;
}

inline void tObject::unsetFlags(tFlags flags)
{
    m_flags &= ~flags;
}

/// MISC FUNCTIONS

template <typename T>
inline bool operator==(const tReference<T>& lhs, const tReference<T>& rhs)
{
    if(lhs.expired() || rhs.expired()) return false;
    return PTR(lhs) == PTR(rhs);
}

template <typename T>
inline bool operator!=(const tReference<T>& lhs, const tReference<T>& rhs)
{
    if(lhs.expired() || rhs.expired()) return true;
    return PTR(lhs) != PTR(rhs);
}

template <typename T>
inline bool operator==(const tReference<T>& lhs, const T* rhs)
{
    if(lhs.expired()) return false;
    return PTR(lhs) == rhs;
}

template <typename T>
inline bool operator!=(const tReference<T>& lhs, const T* rhs)
{
    if(lhs.expired()) return true;
    return PTR(lhs) != rhs;
}
