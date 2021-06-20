#pragma once

#include "tFilter.hpp"

#include "../common/tSetting.hpp"
#include "../data/tMatrixDataSelection.hpp"


// template for operations with two inputs.

// use these as T
namespace BinaryOperation
{
    struct Bind;

}

template <typename T>
class tBinaryFilter : public tFilter
{

public:

    T_TYPEDEFS(tBinaryFilter)

    ~tBinaryFilter();

    void setInputWindow(tWindow* window) override;

protected:

    tBinaryFilter(tWindow* window = nullptr);

private:

    void run_impl(bool notify) override;

    tSetting<tMatrixDataSelection> m_inputA, m_inputB;

    tMatrixData::Ownership m_output;

    T_FILTER_COPYABLE(tBinaryFilter)

    T m_fn;

    friend class Review;
};




#include "tBinaryFilter.inl"
