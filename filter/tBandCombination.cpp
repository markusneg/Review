#include "tBandCombination.hpp"

#include <QLabel>

#include "../common/tWindow.hpp"
#include "../view/tPlotView.hpp" // for policy


struct tBandCombination::Private
{
    struct Band
    {
        uint left[2], center[2], right[2];
        double result;
    };

    std::vector<Band> bands;

    QMutex bandMutex;
};


tBandCombination::tBandCombination(tWindow* window) :
    tFilter(window), m_private(new Private)
{
    setName(QObject::tr("Bands"));
    if(!window) return;

    m_output = tMatrixData::Create();
    m_output->setName(QObject::tr("Feature"));
    registerOwnedData(m_output.get());

    m_input->setSelectionPolicy(new tPlotSelectionPolicy(*m_input, 0));
    m_input.getRefSetting()->setDefaultSelector(fRotation);
    registerInput(m_input);

    m_bands.addCallback([&]() {precompileBands(); run();}, this, sigChanged);
    m_fwhm.addCallback([&]() {precompileBands(); run();}, this, sigChanged);
    m_meanWindow.addCallback([&]() {precompileBands(); run();}, this, sigChanged);

    m_settings = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_settings);
    layout->setContentsMargins(0, 0, 0, 0); layout->setSpacing(1);

    layout->addWidget(new QLabel(QObject::tr("Input")));
    layout->addWidget(m_input.widget());

    layout->addWidget(new QLabel(QObject::tr("Band-positions")));
    layout->addWidget(m_bands.widget());

    layout->addWidget(new QLabel(QObject::tr("FWHM")));
    layout->addWidget(m_fwhm.widget());

    layout->addWidget(new QLabel(QObject::tr("Mean Window")));
    layout->addWidget(m_meanWindow.widget());
}

tBandCombination::~tBandCombination()
{
    doCallbacks(sigDecaying);
    delete m_private;
}


void tBandCombination::setInputWindow(tWindow* window)
{
    m_input.setList(&window->getData());
}


void tBandCombination::run_impl(bool notify)
{
    if(!m_input) return;

    // lock
    tMatrixData::Ownership inputData = m_input->getData()->getOwnership();
    m_private->bandMutex.lock(); ON_LEAVE(m_private->bandMutex.unlock();)

    arma::mat M = m_input->getSubmatrix();


    m_output->matrix().set_size(M.n_rows, m_private->bands.size());


    try
    {

        for(uint m = 0; m < M.n_rows; ++m)
        {

            //for(uint b = 0; b < m_private->bands.size(); ++b)
            //    const Private::Band& band = m_private->bands[b];

            uint nb = 0;
            for(const Private::Band& band : m_private->bands)
            {

                if(band.right[1] > M.n_cols)
                    {T_COUT("[Warning] tBandCombination: OOR band ignored"); continue;}

                //double left = 0; center = 0; right = 0;

                //for(uint n = band.left[0]; n < band.left[1]; ++n)
                //    left +=

                arma::mat out = arma::mean(M.submat(m, band.left[0], m, band.left[1]), 1);
                double left = out(0,0);

                out = arma::mean(M.submat(m, band.center[0], m, band.center[1]), 1);
                double center = out(0,0);

                out = arma::mean(M.submat(m, band.right[0], m, band.right[1]), 1);
                double right = out(0,0);

                m_output->matrix()(m, nb) = center - left + (right - left) / 2.0;

                nb++;
            }

        }

    }

    catch(...)
        {T_COUT("Error in BandCombination!"); return;}

    m_output->dataChanged(notify);

}


void tBandCombination::precompileBands()
{
    m_private->bandMutex.lock();

    m_private->bands.clear();

    const tDiscreteSelection::Items bands = m_bands->getItems();
    const uint hwhm = *m_fwhm / 2, hwin = *m_meanWindow / 2;

    // get the column indices of each band (left slope, center, rights slope: mean interval begin and end each)
    for(uint center : bands)
    {
        auto newBand = Private::Band{
                center - hwhm - hwin,
                center - hwhm + hwin,
                center - hwin,
                center + hwin,
                center + hwhm - hwin,
                center + hwhm + hwin, 0};

        m_private->bands.push_back(newBand);
    }

    m_private->bandMutex.unlock();
}
