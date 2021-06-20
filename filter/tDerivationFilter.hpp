#pragma once

#include "tFilter.hpp"

#include "../common/tSetting.hpp"
#include "../data/tMatrixDataSelection.hpp"


// calculate row or colwise variance

class tDerivationFilter : public tFilter
{

public:

    T_TYPEDEFS(tDerivationFilter)

    ~tDerivationFilter();

    void setInputWindow(tWindow* window) override;

protected:

    tDerivationFilter(tWindow* window = nullptr);

private:

    void run_impl(bool notify) override;

    tSetting<tMatrixDataSelection> m_input;
    tMatrixData::Ownership m_output;

    tSetting<uint> m_nth;

    T_FILTER_COPYABLE(tDerivationFilter)

    friend class Review;
};
