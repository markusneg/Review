#include "tObserver.hpp"

#include <QLabel>
#include <QPushButton>
#include <QLabel>
#include <QMdiSubWindow>

#include "../common/tWindow.hpp"

#define FLENTGH 100

#define tr(x) QObject::tr(x)

using namespace arma;

tObserver::tObserver(tWindow* window) :
    tFilter(window)
{
    setName(QObject::tr("Observer"));
    if(!window) return;

    m_fitness = tMatrixData::Create();
    m_fitness->setName(QObject::tr("Fitness"));
    registerOwnedData(m_fitness.get());

    m_fitness->matrix().resize(1, FLENTGH);

    //m_source.addCallbackSafe([&]() {run();}, this, sigChanged | sigContentChanged);
    m_source.getRefSetting()->setDefaultSelector(fRotation);

    registerInput(m_source);

    m_settings = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_settings);
    layout->setContentsMargins(0, 0, 0, 0); layout->setSpacing(1);

    QPushButton* enabled = new QPushButton(QObject::tr("Enabled"));
    enabled->setCheckable(true); enabled->setChecked(true);
    QObject::connect(enabled, &QPushButton::toggled, [&](bool checked) {setEnabled(checked);});
    layout->addWidget(enabled);

    m_evalType.addItems({tr("Sum"), tr("Max"), tr("Min"), tr("Mean"), tr("Median"), tr("Variance")});
    QObject::connect(&m_evalType, T_SIGNAL(QComboBox, activated, int), [&](){run();});
    layout->addWidget(&m_evalType);

    layout->addWidget(m_source.widget());
    layout->addWidget(new QLabel(QObject::tr("Minimize")));
    layout->addWidget(m_minimize.widget());
    layout->addWidget(new QLabel(QObject::tr("Absolute")));
    layout->addWidget(m_absolute.widget());
    layout->addWidget(new QLabel(QObject::tr("Negative")));
    layout->addWidget(m_negative.widget());

    m_sumWarning = new QLabel("<b><font color = 'red'>" + tr("Summing up!") + "</font></b>");
    //m_sumWarning->setStyleSheet("color: red;");
    layout->addWidget(m_sumWarning);
    m_sumWarning->hide();

    layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

tObserver::~tObserver()
{
    doCallbacks(sigDecaying);
    //delete m_sumWarning; // done by Qt!
}

double tObserver::calculate() const
{
    if(m_source->isNothingSelected()) return 0; /// [OPT] neccessary?

    auto M = m_source->getSubmatrix();

    if(*m_absolute) M = abs(M);
    else if(*m_negative)
    {
        for(double& v : M)
            if(v > 0) v = 0;
    }

    double result;

    // we have to show this warning because else user will make transposing errors
    auto maybeShowSumWarning = [&](){QMetaObject::invokeMethod(m_sumWarning, "setVisible", Qt::QueuedConnection, Q_ARG(bool, M.n_cols != 1));};

    try{ switch(m_evalType.currentIndex())
    {
    case 0: // sum
        m_sumWarning->hide();
        result = arma::accu(M);
        break;

    case 1: // max
        maybeShowSumWarning();
        result = arma::accu(arma::max(M));
        break;

    case 2: // min
        maybeShowSumWarning();
        result = arma::accu(arma::min(M));
        break;

    case 3: // mean
        maybeShowSumWarning();
        result = arma::accu(arma::mean(M));
        break;

    case 4: // mean
        maybeShowSumWarning();
        result = arma::accu(arma::median(M));
        break;
    case 5: // variance
        maybeShowSumWarning();
        result = arma::accu(arma::var(M));
        break;

    }} catch(...) {T_COUT("[tObserver] exception during calculate()!");}


    return *m_minimize ? -result : result;
}


void tObserver::run_impl(bool notify)
{
    // shift
    m_fitness->matrix()(0, span(0,FLENTGH-2)) = m_fitness->matrix()(0, span(1,FLENTGH-1));
    m_fitness->matrix()(0,FLENTGH-1) = calculate();

    m_fitness->dataChanged(notify);
}

void tObserver::setInputWindow(tWindow* window)
{
    m_source.setList(&window->getData());
}

void tObserver::setEnabled(bool enabled)
{
    if(!m_window) return;

    if(enabled) this->unsetFlags(fDisabled);
    else        this->setFlags(fDisabled);

    tWindowStyle* newStyle = new tWindowStyle(tWindowStyle::Observer);
    newStyle->setActive(enabled);
    dynamic_cast<QMdiSubWindow*>(m_window->parent())->setStyle(newStyle);

    tFilter::setEnabled(enabled);
}



tFilter *tObserver::copy(tWindow *window) const {return new tObserver(window);}
