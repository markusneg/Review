#include "tBandObserver.hpp"

#include <QLabel>
#include <QPushButton>
#include <QLabel>

#include "../common/tWindow.hpp"

#define tr(x) QObject::tr(x)

using namespace arma;

tBandObserver::tBandObserver(tWindow* window) :
    tObserver(window)
{
    setName(QObject::tr("Band-Observer"));
    if(!window) return;

    QLayout* layout = m_settings->layout();

    layout->addWidget(new QLabel(QObject::tr("Band-positions")));
    layout->addWidget(m_bands.widget());

    layout->addWidget(new QLabel(QObject::tr("FWHM")));
    layout->addWidget(m_fwhm.widget());

    layout->addWidget(new QLabel(QObject::tr("Scores")));
    layout->addWidget(m_scores.widget());

}

tBandObserver::~tBandObserver()
{
    doCallbacks(sigDecaying);
}

double tBandObserver::calculate() const
{
    if(m_source->isNothingSelected()) return 0;

    auto M = m_source->getSubmatrix();
    //auto S = m_scores->getSubmatrix();

    try
    {
        /// [OPT] precompile band pos..
        tDiscreteSelection::Items bands = m_bands->getItems();

        uint f2 = *m_fwhm / 2;

        double value = 0;

        for(uint m = 0; m < M.n_cols; ++m)
        {
            for(uint pos : bands)
            {
                //double v = arma::stddev(S.col(m), 1);
                double p0 = M(pos - f2, m); double p1 = M(pos + f2, m);
                value += (M(pos, m) - p0 + (p1 - p0) / 2.0);
            }
        }


        value /= bands.size();

        return value;
    }

    catch(...)
        {T_COUT("Error in Bandobserver!"); return 0;}
}


/*void tBandObserver::run(bool notify)
{
    // shift
    m_fitness->matrix()(0, span(0,FLENTGH-2)) = m_fitness->matrix()(0, span(1,FLENTGH-1));
    m_fitness->matrix()(0,FLENTGH-1) = calculate();

    m_fitness->contentChanged(notify);
}*/

/*bool tBandObserver::updateInputWindow(const tList<tWindow*>& list)
{
    if(tFilter::updateInputWindow(list) && m_currentInputWindow)
        {m_source.setList(&m_currentInputWindow->getData()); return true;}

    else return false;
}*/



tFilter *tBandObserver::copy(tWindow *window) const {return new tBandObserver(window);}
