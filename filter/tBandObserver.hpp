#pragma once

#include "tObserver.hpp"


class tBandObserver : public tObserver
{

public:

    T_TYPEDEFS(tBandObserver)

    tFilter* copy(tWindow*) const override;

    ~tBandObserver();

    double calculate() const override;

protected:

    tBandObserver(tWindow* window = nullptr);

private:    

    tSetting<tDiscreteSelection> m_bands;
    tSetting<uint> m_fwhm;

    tSetting<tMatrixDataSelection> m_scores;

    friend class Review;

};

