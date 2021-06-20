#pragma once

#include "tFilter.hpp"

#include "../common/tSetting.hpp"
#include "../data/tMatrixDataSelection.hpp"


// calculate row or colwise variance

class tBaselineFilter : public tFilter
{

public:

    T_TYPEDEFS(tBaselineFilter)

    ~tBaselineFilter();

    void setInputWindow(tWindow* window) override;

protected:

    tBaselineFilter(tWindow* window = nullptr);

private:

    void run_impl(bool notify) override;

    tSetting<tMatrixDataSelection> m_input;
    tMatrixData::Ownership m_output;

    tSetting<uint> m_b0, m_b1;
    tSetting<uint> m_win;

    T_FILTER_COPYABLE(tBaselineFilter)

    friend class Review;
};
