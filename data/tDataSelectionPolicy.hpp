#pragma once


//#include "../common/tObject.hpp"
//#include "../data/tDiscreteSelection.hpp"
#include "tData.hpp"

// to be given by specific DataSelection
//template <typename T>
//struct tDataSelectionPolicyTraits;


// task: data selection from list (tListSetting)


//template <typename T>
class tDataSelectionPolicy
{

public:

    tDataSelectionPolicy() {}
    virtual ~tDataSelectionPolicy() {}

    virtual void onExtentChange() = 0;
    virtual void onDataChange() = 0;
    virtual void onListChange() = 0;

    //tDataSelectionPolicyTraits<T>::Data* m_data = nullptr;

};





#include "tDataSelectionPolicy.inl"
