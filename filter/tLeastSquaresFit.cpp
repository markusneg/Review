#include "tLeastSquaresFit.hpp"

#include <QLabel>

#include "../common/tWindow.hpp"
#include "../view/tPlotView.hpp" // for policy

//#include "../../levmar-2.6/levmar.h"




tLeastSquaresFit::tLeastSquaresFit(tWindow* window) :
    tFilter(window)
{
    setName(QObject::tr("LS-Fit"));
    if(!window) return;

    m_output = tMatrixData::Create();
    m_output->setName(QObject::tr("Coeff"));
    registerOwnedData(m_output.get());

    //m_input.addCallbackSafe([&]() {run();}, this, sigChanged | sigExtentChanged | sigContentChanged);
    m_input->setSelectionPolicy(new tPlotSelectionPolicy(*m_input, 0));
    m_input.getRefSetting()->setDefaultSelector(fRotation);
    registerInput(m_input);

    m_settings = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_settings);
    layout->setContentsMargins(0, 0, 0, 0); layout->setSpacing(1);

    layout->addWidget(m_input.widget());

    layout->addWidget(new QLabel(QObject::tr("Support Pts")));
    m_b0 = 0; m_b1 = 1030;
    layout->addWidget(m_b0.widget());
    layout->addWidget(m_b1.widget());

    layout->addWidget(new QLabel(QObject::tr("Window")));
    m_win = 10;
    layout->addWidget(m_win.widget());
    layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));

    m_b0.addCallback([&]() {run();}, this, sigChanged); m_b1.addCallback([&]() {run();}, this, sigChanged);
    m_win.addCallback([&]() {run();}, this, sigChanged);
}

tLeastSquaresFit::~tLeastSquaresFit()
{
    doCallbacks(sigDecaying);
}


void tLeastSquaresFit::setInputWindow(tWindow* window)
{
    m_input.setList(&window->getData());
}

/*static void x(double *p, double *x, int m, int n, void *data)
{
  for(int z = 0; z < n; z++)			// fill x vector
    x[z] = f(z, p);
}*/

void tLeastSquaresFit::run_impl(bool notify)
{
    if(!m_input) return;

    //ret = dlevmar_bc_dif(x, p0, xm, NPARAM, n, lb, NULL, NULL, iterations, opts, info, NULL, NULL, NULL);


    // lock
    tMatrixData::Ownership inputData = m_input->getData()->getOwnership();
    arma::mat M = m_input->getSubmatrix();
    uint n = M.n_cols;

    if(*m_win >= n || *m_b0 >= *m_b1) return;

    m_output->matrix() = M;

    int hw = *m_win / 2; // half window

    /// [TODO] all the b0/b1 stuff should be handled by an appropriately coupled tDiscreteSelection!
    // "prefilter" the demanded Stützstellen
    uint b0 = *m_b0 >= n ? n-1 : *m_b0;
    if(*m_b1 <= b0) {T_COUT("tBaselineFilter: second pt smaller than first!"); return;}
    uint b1 = *m_b1;

    int b00 = b0 - hw, b01 = b0 + hw;
    int b10 = b1 - hw, b11 = b1 + hw;

    // adjust ranges if neccessary
    if(b00 < 0) {b01 -= b00; b00 = 0;}
    if(b11 >= (int)n) {b10 -= b11-n+1; b11 -= b11-n+1;}

    m_output->lockMutex();
    for(uint m = 0; m < M.n_rows; ++m)
    {
        double l0 = arma::mean(M(m, arma::span(b00, b01)));
        double l1 = arma::mean(M(m, arma::span(b10, b11)));

        double slope = (l1 - l0) / (b1 - b0);
        double offs = l0 - b0 * slope;

        for(uint i = 0; i < n; ++i)
            m_output->matrix()(m,i) = M(m,i) - (offs + i * slope);
    }
    m_output->unlockMutex();

    m_output->dataChanged(notify);
}
