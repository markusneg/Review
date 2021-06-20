#include "tVarianceFilter.hpp"

#include "../common/tWindow.hpp"


tVarianceFilter::tVarianceFilter(tWindow* window) :
    tFilter(window)
{
    setName(QObject::tr("Variance"));
    if(!window) return;

    m_output = tMatrixData::Create();
    m_output->setName(QObject::tr("Variance"));
    registerOwnedData(m_output.get());

    //m_input.addCallbackSafe([&]() {run();}, this, sigChanged | sigExtentChanged | sigContentChanged);
    m_input.getRefSetting()->setDefaultSelector(fRotation);
    registerInput(m_input);

    m_settings = m_input.widget();
}

tVarianceFilter::~tVarianceFilter()
{
    doCallbacks(sigDecaying);
}

void tVarianceFilter::setInputWindow(tWindow* window)
{
    m_input.setList(&window->getData());
}

void tVarianceFilter::run_impl(bool notify)
{
    if(!m_input) return;

    // lock
    tMatrixData::Ownership inputData = m_input->getData()->getOwnership();

    try {m_output->matrix() = arma::var(m_input->getSubmatrix(), 1);}
    catch (...) {T_COUT("[tVarianceFilter] Error running filter!");}

    m_output->dataChanged(notify);
}
