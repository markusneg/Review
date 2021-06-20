#pragma once


#include "../common/tObject.hpp"
#include "../data/tDiscreteSelection.hpp"
#include "tMatrixData.hpp"

class tDataSelectionPolicy;

// denotes the selected rows and columns of tData
class tMatrixDataSelection : public tObject
{

public:

    enum Margin {Rows = 0, Columns, All};

    tMatrixDataSelection();
    tMatrixDataSelection(const tMatrixDataSelection& rhs);
    ~tMatrixDataSelection();

    tMatrixDataSelection& operator=(const tMatrixDataSelection& rhs);

    // set the data, this selection checks with
    void setData(tMatrixData* data);
    //void setData(const tMatrixData::ConstRef& data);

    const tMatrixData*  getData() const;
    tMatrixData*        getData();

    // same as in discreteSelection
    void select(Margin margin, uint i);
    void select(Margin margin, uint from, uint to);
    void unselect(Margin margin, uint i);
    void unselect(Margin margin, uint from, uint to);

    // remember that user edited the given margin. Useful to prevent automatic resets
    void markAsUserTouched(Margin margin) const;
    bool isUserTouched(Margin margin) const;


    // check bounds of selection. Returns 1 if bounds were corrected
    bool validate(bool emitSignal = true);

    // select to all available data
    void reset(Margin margin, bool emitSignal = true);
    void reset(bool emitSignal = true);

    // just reset selections not explicitly given by user
    void conditionalReset(bool emitSignal = true);

    // get the 2-array of the row/column selectors
    const tDiscreteSelection*   getDiscreteSelections() const;
    tDiscreteSelection*         getDiscreteSelections();

    // if set, submatrix output is transposed
    void setTranspose(bool on = true);
    void toggleTranspose();

    // check if ptr is set
    operator bool() const;

    tMatrixData* operator->() {return m_data;}
    const tMatrixData* operator->() const {return m_data;}


    // check wether a continuous submatrix is possible (i.e. both discrete selections continuous)
    bool isContinuous() const;

    /// [OPT] return submatrix, which is optimized for performance (no copy?!)
    // get a matrix according to the current selection
    arma::mat getSubmatrix() const;

    // check wether anything is selected
    bool isNothingSelected() const;

    // lifetime is managed by this class!
    void setSelectionPolicy(tDataSelectionPolicy* selectionPolicy);

protected:


private:    

    void clearUserTouched(Margin margin) const;

    struct Private;
    Private* m_private;

    tDiscreteSelection m_mn[2];

    tMatrixData* m_data = nullptr;
    //tMatrixData::ConstRef m_data;

    bool m_transpose = false;

    tDataSelectionPolicy* m_selectionPolicy = nullptr;

    friend class tSetting<tMatrixDataSelection>;
};


#include "tMatrixDataSelection.inl"
