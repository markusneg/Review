#pragma once

#include "tFilter.hpp"

#include "../common/tSetting.hpp"
#include "../data/tMatrixDataSelection.hpp"


// template for arithmetic operations (+ - * /) with two inputs.
// suited for element-wise matrix operation, row-/colwise operation or single-element operation

// use these as T
namespace ArithmeticOperation
{
    struct Plus;
    struct Minus;
    struct Multiply;
    struct Divide;
}

template <typename T>
class tArithmeticFilter : public tFilter
{

public:

    T_TYPEDEFS(tArithmeticFilter)

    ~tArithmeticFilter();

    void setInputWindow(tWindow* window) override;


protected:

    tArithmeticFilter(tWindow* window = nullptr);

private:

    void run_impl(bool notify) override;

    tSetting<tMatrixDataSelection> m_inputA, m_inputB;

    tMatrixData::Ownership m_output;

    T_FILTER_COPYABLE(tArithmeticFilter)

    friend class Review;
};




#include "tArithmeticFilter.inl"
