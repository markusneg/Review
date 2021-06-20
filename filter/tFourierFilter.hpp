#pragma once

#include "tFilter.hpp"

#include "../common/tSetting.hpp"
#include "../data/tMatrixDataSelection.hpp"


// calculate row or colwise variance

class tFourierFilter : public tFilter
{

public:

    T_TYPEDEFS(tFourierFilter)

    ~tFourierFilter();

    void setInputWindow(tWindow* window) override;

protected:

    tFourierFilter(tWindow* window = nullptr);

private:

    void run_impl(bool notify) override;

    tSetting<tMatrixDataSelection> m_input;
    tMatrixData::Ownership m_mod, m_arg;

    //tSetting<uint> m_b0, m_b1;
    //tSetting<uint> m_win;

    T_FILTER_COPYABLE(tFourierFilter)

    friend class Review;
};
