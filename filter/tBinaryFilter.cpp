#include "tBinaryFilter.hpp"

#include <QLabel>

#include "../common/tWindow.hpp"

template <typename T>
tBinaryFilter<T>::tBinaryFilter(tWindow* window) :
    tFilter(window)
{
    setName(T::name());
    if(!window) return;

    m_output = tMatrixData::Create();
    m_output->setName(T::name());
    registerOwnedData(m_output.get());

    //m_inputA.addCallbackSafe([&]() {run();}, this, sigChanged | sigExtentChanged | sigContentChanged);
    //m_inputB.addCallbackSafe([&]() {run();}, this, sigChanged | sigExtentChanged | sigContentChanged);

    m_settings = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_settings);
    layout->setContentsMargins(0, 0, 0, 0); layout->setSpacing(1);

    registerInput(m_inputA); registerInput(m_inputB);

    layout->addWidget(m_inputA.widget());
    layout->addWidget(m_inputB.widget());

    layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));

    //m_b0.addCallback([&]() {run();}, this, sigChanged); m_b1.addCallback([&]() {run();}, this, sigChanged);
    //m_win.addCallback([&]() {run();}, this, sigChanged);
}

template <typename T>
tBinaryFilter<T>::~tBinaryFilter()
{
    doCallbacks(sigDecaying);
}

template <typename T>
void tBinaryFilter<T>::setInputWindow(tWindow* window)
{
    m_inputA.setList(&window->getData());
    m_inputB.setList(&window->getData());
}

template <typename T>
void tBinaryFilter<T>::run_impl(bool notify)
{
    if(!m_inputA || !m_inputB) return;

    // lock
    tMatrixData::Ownership inputDataA = m_inputA->getData()->getOwnership();
    tMatrixData::Ownership inputDataB = m_inputB->getData()->getOwnership();

    try {m_output->matrix() = m_fn.fn(m_inputA->getSubmatrix(), m_inputB->getSubmatrix());}
    catch (...) {T_COUT("tBinaryFilter: op exception");}

    m_output->dataChanged(notify);
}

template class tBinaryFilter<BinaryOperation::Bind>;
