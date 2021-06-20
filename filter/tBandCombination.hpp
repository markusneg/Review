#pragma once

#include "tFilter.hpp"

#include "../common/tSetting.hpp"
#include "../data/tMatrixDataSelection.hpp"



class tBandCombination : public tFilter
{

public:

    T_TYPEDEFS(tBandCombination)

    ~tBandCombination();

    void setInputWindow(tWindow* window) override;

protected:

    tBandCombination(tWindow* window = nullptr);

private:

    void run_impl(bool notify) override;

    void precompileBands();

    struct Private;
    Private* m_private;

    tSetting<tMatrixDataSelection> m_input;
    tSetting<tDiscreteSelection> m_bands;
    tSetting<uint> m_fwhm = 50;
    tSetting<uint> m_meanWindow = 10;

    tMatrixData::Ownership m_output;

    T_FILTER_COPYABLE(tBandCombination)

    friend class Review;
};
