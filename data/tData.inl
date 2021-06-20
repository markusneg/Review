#pragma once

#include "tData.hpp"

/*template <typename T>
tDataReference<T> tData::getRef()
{
    std::shared_ptr<tData> ptr;

    try {ptr = shared_from_this();} catch (...)
        {std::cout << "[tData] Error: could not create Reference!" << std::endl << std::flush; return tDataReference<T>();}

    std::shared_ptr<T> casted_ptr = std::dynamic_pointer_cast<T>(ptr);

    return tDataReference<T>(casted_ptr);
}*/




