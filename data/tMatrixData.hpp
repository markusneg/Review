#pragma once

#include <memory>
#include <list>
#include <array>

#include <QObject>
#include <armadillo>

#include "tData.hpp"
//#include "tMatrixDataSelection.hpp"
//#include "tLockableContainer.hpp"

class tWindow;

class tMatrixDataSelection;
class Review;

template <typename T>
class tDataReference;

class tMatrixData;
class DataList;

typedef tDataReference<tMatrixData> tMatrixDataReference;


class tMatrixData : public tData
{

public:

    T_TYPEDEFS(tMatrixData)

    static Ownership Create(uint n_rows = 0, uint n_cols = 0);

    // used by tDataReference to instantiate the data selectors. Denotes the selected rows and columns of the matrix
    typedef tMatrixDataSelection Selector;

    ~tMatrixData();

    bool load(const QString& path) override;
    bool save(const QString& path) const override;

    uint n_rows() const;
    uint n_cols() const;

    // direct access to armadillo matrix
    arma::Mat<double>&          matrix();
    const arma::Mat<double>&    matrix() const;

    // get string info about extents
    QString getExtInfo() const override;

    void changed(bool delayed = false, bool wait = false) const override;

    void dataChanged(bool signal = true, bool delayed = false, bool wait = false) const;

    tData::Ownership copy() const override;

    tMatrixData(const tMatrixData &rhs);
    tMatrixData& operator=(const tMatrixData& rhs);

    //void updateFromSDBData(const DataList& dataList);

signals:

    // emitted when data has been changed
    //void changed();

protected:

    tMatrixData(uint n_rows = 0, uint n_cols = 0);

private:    

    void assign(const tMatrixData& rhs);

    // can gain ownership over tMatrixData
    friend class tWindow;
    friend class Review;

    arma::mat m_matrix;
    mutable uint m_lastExt[2] = {0,0};
    mutable bool m_dataModified = false;
};


#include "tMatrixData.inl"
