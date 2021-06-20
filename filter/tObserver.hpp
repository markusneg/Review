#pragma once

#include "tFilter.hpp"

#include "../common/tSetting.hpp"
#include "../data/tMatrixDataSelection.hpp"

class QLabel;

// observer class used for iterative rotational optimization

class tObserver : public tFilter
{

public:

    T_TYPEDEFS(tObserver)

    tFilter* copy(tWindow*) const override;

    ~tObserver();

    // calculate the current fitness value
    virtual double calculate() const;

    void setInputWindow(tWindow* window) override;

    void setEnabled(bool enabled = true) override;

protected:

    void run_impl(bool notify) override;

    tObserver(tWindow* window = nullptr);

    tMatrixData::Ownership m_fitness;

    tSetting<tMatrixDataSelection> m_source;
    tSetting<bool> m_minimize = false;
    tSetting<bool> m_absolute = false;
    tSetting<bool> m_negative = false;

    QLabel* m_sumWarning;

private:    

    QComboBox m_evalType;

    friend class Review;

};

