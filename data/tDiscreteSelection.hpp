#pragma once

#include <vector>
#include <array>

#include <armadillo>
#include <QString>

#include "../common/tSetting.hpp"


// representing a discrete selection of 1-dimensional data supporting intervals (e.g. "1:3, 5, 10")
class tDiscreteSelection
{

public:

    typedef std::array<uint, 2>         Interval;
    typedef std::vector<Interval>       Intervals;
    typedef std::vector<arma::uword>    Items;

    enum Mode {Selected, Unselected};

    tDiscreteSelection();
    ~tDiscreteSelection();

    // get the chosen intervals for both selection and unselection
    /// [OPT] consider direct access on interval
    void getIntervals(Intervals& out) const;

    // get single items for both selection and unselection
    Items getItems() const;

    // get highest and lowest items within intervals
    uint getLowestItem() const;
    uint getHighestItem() const;

    void select(uint i);
    void select(uint from, uint to);

    void unselect(uint i);
    void unselect(uint from, uint to);

    // total count of (un)selected values
    uint count() const;

    // parses (un)selection from string and back. Expected in the form of ranges (1:3) or single values, separated by comma
    void    fromString(const QString& str);
    QString toString() const;

    void reset();

    Mode getMode() const;
    void setMode(Mode newMode);

    // check if there is only one (selected) interval
    bool isContinuous() const;

    tDiscreteSelection(const tDiscreteSelection& rhs);
    tDiscreteSelection& operator=(const tDiscreteSelection& rhs);

private:

    struct Private;
    Private* m_private;

    Mode m_mode = Selected;
};


#include "tDiscreteSelection.inl"
