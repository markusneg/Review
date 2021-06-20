#include "tFourierFilter.hpp"

#include <QLabel>

#include "../common/tWindow.hpp"


tFourierFilter::tFourierFilter(tWindow* window) :
    tFilter(window)
{
    setName(QObject::tr("Fourier Transform"));
    if(!window) return;

    m_mod = tMatrixData::Create();
    m_mod->setName(QObject::tr("Modulus"));
    registerOwnedData(m_mod.get());

    m_arg = tMatrixData::Create();
    m_arg->setName(QObject::tr("Argument"));
    registerOwnedData(m_arg.get());

    registerInput(m_input);

    //m_input.addCallbackSafe([&]() {run();}, this, sigChanged | sigExtentChanged | sigContentChanged);

    m_settings = m_input.widget();

    /*m_settings = new QWidget();
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
    m_win.addCallback([&]() {run();}, this, sigChanged);*/
}

tFourierFilter::~tFourierFilter()
{
    doCallbacks(sigDecaying);
}

void tFourierFilter::setInputWindow(tWindow* window)
{
    m_input.setList(&window->getData());
}


void tFourierFilter::run_impl(bool notify)
{
    if(!m_input) return;

    // lock
    tMatrixData::Ownership inputData = m_input->getData()->getOwnership();
    arma::mat M = m_input->getSubmatrix();

    if(!M.n_cols || !M.n_rows) return;

    //arma::cx_mat F = arma::fft(X);
    arma::cx_mat F = arma::fft(M.t());

    uint used = ceil((double)F.n_rows / 2.0) - 1;
    auto F_eff = F.rows(0, used);

    m_mod->matrix() = arma::abs(F_eff);
    m_arg->matrix() = arma::atan(arma::imag(F_eff) / arma::real(F_eff));

    m_mod->dataChanged(notify);
}
