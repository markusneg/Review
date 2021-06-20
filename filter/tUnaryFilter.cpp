#include "tUnaryFilter.hpp"

#include <QLabel>

#include "../common/tWindow.hpp"
#include "../view/tPlotView.hpp"


template <typename T>
tUnaryFilter<T>::tUnaryFilter(tWindow* window) :
    tFilter(window)
{
    setName(m_op.name());
    if(!window) return;

    m_op.changed = [&](){run();};

    m_output = tMatrixData::Create();
    m_output->setName(QObject::tr("Result"));
    registerOwnedData(m_output.get());

    //m_input.addCallbackSafe([&]() {run();}, this, sigChanged | sigExtentChanged | sigContentChanged);
    m_input->setSelectionPolicy(new tPlotSelectionPolicy(*m_input, 0));
    m_input.getRefSetting()->setDefaultSelector(fRotation);
    registerInput(m_input);

    m_classes.addCallbackSafe([&]() {run();}, this, sigChanged | sigExtentChanged | sigDataChanged);

    m_settings = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_settings);
    layout->setContentsMargins(0, 0, 0, 0); layout->setSpacing(1);

    layout->addWidget(m_input.widget());

    layout->addWidget(new QLabel(QObject::tr("Classes")));
    layout->addWidget(m_classes.widget());

    m_op.setupUI(layout);

    layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));

    //m_b0.addCallback([&]() {run();}, this, sigChanged); m_b1.addCallback([&]() {run();}, this, sigChanged);
    //m_win.addCallback([&]() {run();}, this, sigChanged);
}

template <typename T>
tUnaryFilter<T>::~tUnaryFilter()
{
    doCallbacks(sigDecaying);
}

template <typename T>
void tUnaryFilter<T>::setInputWindow(tWindow* window)
{
    m_input.setList(&window->getData());
}

template <typename T>
void tUnaryFilter<T>::run_impl(bool notify)
{
    if(!m_input) return;

    // lock
    tMatrixData::Ownership inputDataA = m_input->getData()->getOwnership();

    arma::mat input = m_input->getSubmatrix();
    arma::mat& output = m_output->matrix();

    if(m_classes)
    {
        arma::mat classes = m_classes->getSubmatrix();

        if(classes.n_rows == input.n_rows)
        {
            uint n_classes = classes.max();

            for(uint c = 1; c <= n_classes; ++c)
            {
                arma::uvec members = arma::find(classes == c);

                arma::mat subview = input.rows(members);
                auto result = m_op.fn(subview);

                if(c == 1)  output = result;
                else
                {
                    output.insert_rows(output.n_rows, result);
                }
            }

        }

        else T_COUT("tUnaryFilter: class vector unsuited for data!");

    }

    else
    {
        try {output = m_op.fn(input);}
        catch (...) {T_COUT("tUnaryFilter: op exception");}
    }

    m_output->dataChanged(notify);
}

template class tUnaryFilter<UnaryOperation::Power>;
template class tUnaryFilter<UnaryOperation::Variance>;
template class tUnaryFilter<UnaryOperation::Sum>;
template class tUnaryFilter<UnaryOperation::Mean>;
template class tUnaryFilter<UnaryOperation::Median>;
template class tUnaryFilter<UnaryOperation::Min>;
template class tUnaryFilter<UnaryOperation::Max>;
template class tUnaryFilter<UnaryOperation::Abs>;
template class tUnaryFilter<UnaryOperation::VectorLength>;
template class tUnaryFilter<UnaryOperation::SmoothFourier>;
//template class tUnaryFilter<UnaryOperation::SmoothSG>;
template class tUnaryFilter<UnaryOperation::Autoscale>;
template class tUnaryFilter<UnaryOperation::Classes>;
template class tUnaryFilter<UnaryOperation::Sort>;
template class tUnaryFilter<UnaryOperation::Transpose>;






