#pragma once

#include "tFilter.hpp"

#include "../common/tSetting.hpp"
#include "../data/tMatrixDataSelection.hpp"


// calculate row or colwise variance

class tVarianceFilter : public tFilter
{

public:

    T_TYPEDEFS(tVarianceFilter)

    ~tVarianceFilter();

    void setInputWindow(tWindow* window) override;

protected:

    tVarianceFilter(tWindow* window = nullptr);

private:

    void run_impl(bool notify) override;

    tSetting<tMatrixDataSelection> m_input;
    tMatrixData::Ownership m_output;

    T_FILTER_COPYABLE(tVarianceFilter)

    friend class Review;
};
