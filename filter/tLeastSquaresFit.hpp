#pragma once

#include "tFilter.hpp"

#include "../common/tSetting.hpp"
#include "../data/tMatrixDataSelection.hpp"


// calculate row or colwise variance

class tLeastSquaresFit : public tFilter
{

public:

    T_TYPEDEFS(tLeastSquaresFit)

    ~tLeastSquaresFit();

    void setInputWindow(tWindow* window) override;

protected:

    tLeastSquaresFit(tWindow* window = nullptr);

private:

    void run_impl(bool notify) override;

    tSetting<tMatrixDataSelection> m_input;
    tMatrixData::Ownership m_output;

    tSetting<uint> m_b0, m_b1;
    tSetting<uint> m_win;

    T_FILTER_COPYABLE(tLeastSquaresFit)

    friend class Review;
};
