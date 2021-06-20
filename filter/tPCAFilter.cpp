#include "tPCAFilter.hpp"

#include "../common/tWindow.hpp"

#include <QLabel>

tPCAFilter::tPCAFilter(tWindow* window) :
    tFilter(window)
{
    setName(QObject::tr("PCA"));

    if(!window) return;

    // create the output fields
    m_loadings = tMatrixData::Create();
    m_loadings->setName(QObject::tr("Loadings"));
    m_loadings->setFlags(fRotation);
    registerOwnedData(m_loadings.get());

    m_scores = tMatrixData::Create();
    m_scores->setName(QObject::tr("Scores"));
    registerOwnedData(m_scores.get());

    registerInput(m_inputData);

    m_settings = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_settings);
    layout->setContentsMargins(0, 0, 0, 0); layout->setSpacing(1);

    layout->addWidget(new QLabel(QObject::tr("Input")));
    layout->addWidget(m_inputData.widget());

    layout->addWidget(new QLabel(QObject::tr("Centering")));
    layout->addWidget(m_centering.widget());

    layout->addWidget(new QLabel(QObject::tr("Include Center")));
    layout->addWidget(m_includeCenter.widget());

    m_includeCenter.widget()->setEnabled(false);
    m_centering.addCallbackSafe([&]()
    {
        if(*m_centering)    m_includeCenter.widget()->setEnabled(true);
        else                m_includeCenter.widget()->setEnabled(false);
        run();

    }, this, sigChanged);

    m_includeCenter.addCallbackSafe([&](){run();}, this, sigChanged);

}

tPCAFilter::~tPCAFilter()
{
    doCallbacks(sigDecaying);
}



void tPCAFilter::run_impl(bool notify)
{
    //princomp( mat coeff, mat score, mat X )

    if(!m_window) {T_COUT("[tPCAFilter] Error: run() called without window!"); return;}
    if(!m_loadings || !m_scores) {T_COUT("[tPCAFilter] Error: missing output data fields!"); return;}
    if(!m_inputData) {T_COUT("[tPCAFilter] Error: no input data!"); return;}

    // lock
    tMatrixData::Ownership inputData = m_inputData->getData()->getOwnership();

    arma::mat X = m_inputData->getSubmatrix();
    arma::rowvec center;

    const uint n_rows = X.n_rows; const uint n_cols = X.n_cols;

    arma::mat& score_out = m_scores->matrix();
    arma::mat& coeff_out = m_loadings->matrix();

    if(n_rows > 1) // more than one sample
    {        
        // subtract the mean - use score_out as temporary matrix
        if(*m_centering)
        {
            center = mean(X); // used later for integration of center factor
            X.each_row() -= center;
        }

        // singular value decomposition
        arma::mat U; arma::colvec s;
        const bool svd_ok = svd(U, s, coeff_out, X, "std");

        if(svd_ok == false) return;

        // normalize the eigenvalues
        s /= std::sqrt(double(n_rows - 1));

        // project the samples to the principals
        score_out = X * coeff_out;

        if(n_rows <= n_cols) // number of samples is less than their dimensionality
        {
            score_out.cols(n_rows-1,n_cols-1).zeros();

            arma::colvec s_tmp(n_cols);
            s_tmp.zeros();

            s_tmp.rows(0,n_rows-2) = s.rows(0,n_rows-2);
            s = s_tmp;
        }

        if(*m_centering && *m_includeCenter)
        {
            double vl = arma::norm(center);
            coeff_out.insert_cols(0, center.t() / vl);

            arma::vec scores(X.n_rows);
            scores.fill(vl);
            score_out.insert_cols(0, scores);
        }
    }
    else // 0 or 1 samples
    {
        coeff_out.eye(n_cols, n_cols);

        //score_out.copy_size(in);
        //score_out.zeros();
    }

    //arma::princomp(m_loadings->matrix(), m_scores->matrix(), m_inputData->getSubmatrix());

    m_loadings->dataChanged(notify);
    m_scores->dataChanged(notify);
}

void tPCAFilter::setInputWindow(tWindow* window)
{
    m_inputData.setList(&window->getData());
}

