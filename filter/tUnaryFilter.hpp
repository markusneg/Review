#pragma once

#include "tFilter.hpp"

#include "../common/tSetting.hpp"
#include "../data/tMatrixDataSelection.hpp"


// template for unary operations

// use these as T
namespace UnaryOperation
{
    struct Power;

    struct Sum;
    struct Mean;
    struct Variance; // and sd?
    struct Median;
    struct Min;
    struct Max;
    struct Abs;
    struct VectorLength;
    struct SmoothFourier;
    struct SmoothSG;
    struct Autoscale;
    struct Classes;
    struct Sort;
    struct Transpose;
}

template <typename T>
class tUnaryFilter : public tFilter
{

public:

    T_TYPEDEFS(tUnaryFilter)

    ~tUnaryFilter();

    void setInputWindow(tWindow* window) override;

protected:

    tUnaryFilter(tWindow* window = nullptr);

private:

    void run_impl(bool notify) override;

    tSetting<tMatrixDataSelection> m_input, m_classes;

    tMatrixData::Ownership m_output;

    T m_op;

    T_FILTER_COPYABLE(tUnaryFilter)

    friend class Review;
};




#include "tUnaryFilter.inl"
