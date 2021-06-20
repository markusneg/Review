#pragma once

#include "tFilter.hpp"
#include "../common/tSetting.hpp"
#include "../data/tDataReference.hpp"


class tPCAFilter : public tFilter
{

public:

    T_TYPEDEFS(tPCAFilter)

    ~tPCAFilter();

    void setInputWindow(tWindow* window) override;

protected:

    tPCAFilter(tWindow* window = nullptr);

private:

    void run_impl(bool notify) override;

    //bool updateInputWindow(const tList<tWindow*>& list) override;

    tSetting<tMatrixDataSelection> m_inputData;

    tMatrixData::Ownership m_scores, m_loadings;

    tSetting<bool> m_centering = false;
    tSetting<bool> m_includeCenter = false;


    friend class Review;

    T_FILTER_COPYABLE(tPCAFilter)

};


