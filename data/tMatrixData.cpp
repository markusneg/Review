#include "tMatrixData.hpp"

#include <QDir>

//#include "sdb/sdb.hpp"
//#include "sdb/Matrix.hpp"

tMatrixData::tMatrixData(uint n_rows, uint n_cols) :
    m_matrix(n_rows, n_cols) /*: m_lastExt{0,0}*/
{
}

tMatrixData::~tMatrixData()
{
    doCallbacks(sigDecaying);
    //std::cout << ("MatrixData was destroyed!") << std::endl << std::flush;
}

bool tMatrixData::load(const QString& path)
{
    if(m_matrix.load(path.toStdString()))
    {
        setName(QFileInfo(path).fileName());
        doCallbacks(T_SIG(sigChanged | sigExtentChanged | sigDataChanged));
        return true;
    }

    else return false;
}

bool tMatrixData::save(const QString& path) const
{
    return m_matrix.save(path.toStdString(), arma::raw_ascii);
}


tMatrixData::Ownership tMatrixData::Create(uint n_rows, uint n_cols)
{
    return Ownership(new tMatrixData);
}

QString tMatrixData::getExtInfo() const
{
    return QString("[%1x%2]").arg(n_rows()).arg(n_cols());
}

void tMatrixData::changed(bool delayed, bool wait) const
{
    tSignals toEmit = sigChanged;

    // check change of extent
    if(m_matrix.n_rows != m_lastExt[0] || m_matrix.n_cols != m_lastExt[1])
        {m_lastExt[0] = m_matrix.n_rows; m_lastExt[1] = m_matrix.n_cols; toEmit |= sigExtentChanged | sigNameChanged;}

    if(m_dataModified) {toEmit |= sigDataChanged; m_dataModified = false;};

    doCallbacks(toEmit, delayed, wait);
}

void tMatrixData::dataChanged(bool signal, bool delayed, bool wait) const
{
    m_dataModified = true;
    if(signal) changed(delayed, wait);
}

tData::Ownership tMatrixData::copy() const
{


    //return Ownership(new tMatrixData);

    return Ownership(new tMatrixData(*this));
}

tMatrixData::tMatrixData(const tMatrixData& rhs) :
    tData(rhs)
{
    assign(rhs);
}

tMatrixData& tMatrixData::operator=(const tMatrixData& rhs)
{
    tData::operator=(rhs);
    assign(rhs);
    return *this;
}

/*void tMatrixData::updateFromSDBData(const DataList& dataList)
{
    if(dataList.empty() || !dataList[0]->matrix())
        return;

    m_matrix.resize(dataList.size(), dataList[0]->matrix()->ext(0));

    for(int i = 0; i < dataList.size(); ++i)
    {
        const Matrix* const m = dataList[i]->matrix();
        if(!m) return;

        for(int j = 0; j < m->ext(0); ++j)
            m_matrix(i,j) = m->at(j);
    }

    dataChanged();
}*/

void tMatrixData::assign(const tMatrixData& rhs)
{
    m_matrix = rhs.m_matrix;
    m_lastExt[0] = rhs.m_lastExt[0]; m_lastExt[1] = rhs.m_lastExt[1];
    m_dataModified = rhs.m_dataModified;
}

