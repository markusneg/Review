#include "tDerivationFilter.hpp"

#include <QLabel>

#include "../common/tWindow.hpp"
#include "../view/tPlotView.hpp"


tDerivationFilter::tDerivationFilter(tWindow* window) :
    tFilter(window)
{
    setName(QObject::tr("Derivation"));
    if(!window) return;

    m_output = tMatrixData::Create();
    m_output->setName(QObject::tr("Derived"));
    registerOwnedData(m_output.get());

    //m_input.addCallbackSafe([&]() {run();}, this, sigChanged | sigExtentChanged | sigContentChanged);
    m_input->setSelectionPolicy(new tPlotSelectionPolicy(*m_input, 0));
    m_input.getRefSetting()->setDefaultSelector(fRotation);

    registerInput(m_input);

    m_settings = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_settings);
    layout->setContentsMargins(0, 0, 0, 0); layout->setSpacing(1);

    layout->addWidget(m_input.widget());

    layout->addWidget(new QLabel(QObject::tr("Take every")));
    m_nth = 1;
    layout->addWidget(m_nth.widget());

    layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));

    m_nth.addCallback([&]() {run();}, this, sigChanged);
}

tDerivationFilter::~tDerivationFilter()
{
    doCallbacks(sigDecaying);
}


void tDerivationFilter::setInputWindow(tWindow* window)
{
    m_input.setList(&window->getData());
}


void tDerivationFilter::run_impl(bool notify)
{
    if(!m_input) return;

    // lock
    tMatrixData::Ownership inputData = m_input->getData()->getOwnership();
    arma::mat M = m_input->getSubmatrix();
    int m = M.n_rows, n = M.n_cols, nth = *m_nth;
    int n_out = n / nth - 1; if(n < 0) n = 0;

    if(!m || !n || !n_out || !nth) return;

    /// [OPT] maybe use matrix operation (-)

    m_output->lockMutex();
    m_output->matrix().set_size(m,n_out);

    for(int i = 0; i < m; ++i)
        for(int j = 1; j < n_out; ++j)
            m_output->matrix()(i,j) = M(i, j * nth) - M(i, (j-1) * nth);

    m_output->unlockMutex();

    m_output->dataChanged(notify);
}
