#include "tRotationFilter.hpp"

#include <thread>
#include <limits>

#include <QLabel>
#include <QPushButton>
#include <QCoreApplication>

#include "../common/tWindow.hpp"
#include "../filter/tObserver.hpp"
#include "../view/tView.hpp"
#include "../util/Graph.hpp"

#include "helpers.hpp"

using namespace arma;

// number of pairwise rotational directions with n = # variables
#define NO_DIRS(n)  (n * (n + 1) / 2 - n) * 2

#define Rp          m_rot


/* for rotation, theoretically the input fields each have to be multiplicated with the full extent rotation matrix, which in turn
 * has to be rotated pairwisely each step. However, this should be way too slow! The pre-c645d33c versions only used the rows/cols needed, which is fast
 * but lacks absolute rotation information. A version should be implemented, in which the rotation matrix, initially eye, is treated as output field and thus
 * can be used to trace the rotation steps. On input-changes, this matrix is applied. Otherwise, differential rotation takes place.
 *
 */

/* current issues:
 *  - changed-callbacks do not regard dependencies of following filters. This is for both
 *      - callback ordering of one tObject (tData)
 *      - the order of sigChanged-emission within multi-output filters (e.g. PCAFilter)
 *  - input/output-window lists a) not updated and b) not compatible with multi-output filters
 *  - AR does not call prefilters in order (see 1)
 *
 *  for 1 and 3, topological sorting of DAGs could be useful
 *
 */

typedef std::array<uint,2> Pair;

struct RotationalDirection
{
    int j, k;
    double s, value;
    bool oblique;
    inline void set(int _j, int _k, double _s, double _value, double _oblique = false) {j = _j; k = _k; s = _s; value = _value; oblique = _oblique;}
    inline bool operator<(const RotationalDirection& rhs) const {return value > rhs.value; /*reverse sorting*/}
    inline RotationalDirection(int _j, int _k, double _s, double _value, bool _oblique = false) : j(_j), k(_k), s(_s), value(_value), oblique(_oblique) {}
};

typedef std::vector<RotationalDirection> RotationalPriority;

struct tRotationFilter::Private
{
    std::thread* rotationThread = nullptr;
    bool rotationRunning = false;
    bool rotationEnded = false;

    // for auto rotation

    typedef arma::vec Scores;
    arma::mat rotationBuffer;

    arma::mat pairwiseRotation = arma::mat(2, 2);

    std::vector<tFilter::Ownership> prefilters;
    std::vector<tObserver::Ownership> observers;
    bool getObservers(tWindow* this_window, Graph<tFilter>* prefilters = nullptr);
    void releaseOwnerships();
    void runPrefilters();
    void getScores(Scores& scores) const;
    void lockForAutoRotation(bool lock = true) const;

    inline void resetPriorities();
    std::vector<RotationalPriority> arPriorities;
    std::vector<RotationalDirection> arBestDirections;

    void recalculateStepVariations(uint variation, uint steps);
    inline double getCurrentStepVariation();
    arma::vec arStepVariations;
    uint arCurrentVariationStep = 0;
    uint arLastVariation = 0, arLastSteps = 0;

    QCheckBox* mrHighRes = new QCheckBox;
    QPushButton* arDebugging = new QPushButton("Debugging");
    QComboBox* rotStorageSlot = new QComboBox();

    // for rudimentary rotation storage
    arma::mat rotationStorage[6], dataStorage[6];

    static void getVariableComponents(std::vector<arma::uword>& comps, const tDiscreteSelection::Items& fixed, uint n_comps);
};


tRotationFilter::tRotationFilter(tWindow* window) :
    tFilter(window), m_slRot(new QSlider(Qt::Horizontal)), m_private(new Private)
{
    setName(QObject::tr("Rotation"));

    if(!window) return;

    registerInput(m_inputData);
    registerInput(m_inputRotation);
    m_inputRotation.getRefSetting()->setEmptyTag(QObject::tr("Identity"));
    m_inputData.getRefSetting()->setDefaultSelector(!fRotation); /// does this work? (we do not want to select the rotation eg from pca)

    m_inputData.addCallback([&]() {inputChanged(true, false);}, sigChanged);
    m_inputRotation.addCallback([&]() {inputChanged(false, true);}, sigChanged);

    m_outputData = tMatrixData::Create();
    m_outputData->setName(QObject::tr("Data"));
    m_outputData->setFlags(fRotation);
    registerOwnedData(m_outputData.get());

    m_outputRotation = tMatrixData::Create();
    m_outputRotation->setName(QObject::tr("Rotation"));
    m_outputRotation->setFlags(fRotation);
    registerOwnedData(m_outputRotation.get());

    m_settings = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_settings);
    layout->setContentsMargins(0, 0, 0, 0); layout->setSpacing(1);

    layout->addWidget(new QLabel(QObject::tr("Input Data")));
    layout->addWidget(m_inputData.widget());

    layout->addWidget(new QLabel(QObject::tr("Rotation")));
    layout->addWidget(m_inputRotation.widget());

    layout->addWidget(new QLabel(QObject::tr("Storage")));
    QWidget* restoreWidget = new QWidget;
    QHBoxLayout* restoreLayout = new QHBoxLayout(restoreWidget);
    restoreLayout->setContentsMargins(0, 0, 0, 0); restoreLayout->setSpacing(1);
    m_private->rotStorageSlot->addItems({"1", "2", "3", "4", "5", "6", "7"}); m_private->rotStorageSlot->setCurrentIndex(0);
    restoreLayout->addWidget(m_private->rotStorageSlot);
    QPushButton* bSave = new QPushButton(QObject::tr("Save"));
    QObject::connect(bSave, &QPushButton::clicked, [&](){storeRotation();});
    restoreLayout->addWidget(bSave);
    QPushButton* bRestore = new QPushButton(QObject::tr("Restore"));
    QObject::connect(bRestore, &QPushButton::clicked, [&](){restoreRotation();});
    restoreLayout->addWidget(bRestore);
    layout->addWidget(restoreWidget);

    QWidget* resetWidget = new QWidget;
    QHBoxLayout* resetLayout = new QHBoxLayout(resetWidget);
    resetLayout->setContentsMargins(0, 0, 0, 0); resetLayout->setSpacing(1);
    auto btReset = new QPushButton("Reset");
    QObject::connect(btReset, &QPushButton::clicked, [&](){resetRotation();});
    resetLayout->addWidget(btReset);
    auto btRandom = new QPushButton("Random");
    QObject::connect(btRandom, &QPushButton::clicked, [&](){randomRotation();});
    resetLayout->addWidget(btRandom);
    layout->addWidget(resetWidget);

    layout->addWidget(new QLabel(QObject::tr("Swap Axes / Oblique Mode")));
    QWidget* ctrlWidget = new QWidget;
    QHBoxLayout* ctrlLayout = new QHBoxLayout(ctrlWidget);
    ctrlLayout->setContentsMargins(0, 0, 0, 0); ctrlLayout->setSpacing(1);
    ctrlLayout->addWidget(m_swapAxes.widget());
    m_swapAxes.addCallback([&]() {swapAxes(*m_swapAxes); outputChanged();}, sigChanged);
    ctrlLayout->addWidget(m_oblique.widget());
    layout->addWidget(ctrlWidget);

    /*QWidget* sourceWidget = new QWidget;
    QHBoxLayout* sourceLayout = new QHBoxLayout(sourceWidget);
    sourceLayout->setContentsMargins(0, 0, 0, 0); sourceLayout->setSpacing(1);
    sourceLayout->addWidget(m_comps.widget());
    sourceLayout->addWidget(m_observations.widget());
    layout->addWidget(sourceWidget);*/

    m_im = 1; m_in = 2;
    layout->addWidget(new QLabel(QObject::tr("Manual Rotation Indices")));
    QWidget* indexWidget = new QWidget;
    QHBoxLayout* indexLayout = new QHBoxLayout(indexWidget);
    indexLayout->setContentsMargins(0, 0, 0, 0); indexLayout->setSpacing(1);
    indexLayout->addWidget(m_im.widget());
    indexLayout->addWidget(m_in.widget());
    layout->addWidget(indexWidget);

    layout->addWidget(new QLabel(QObject::tr("Manual Rotation")));
    QWidget* slWidget = new QWidget;
    QHBoxLayout* slLayout = new QHBoxLayout(slWidget);
    slLayout->setContentsMargins(0, 0, 0, 0); slLayout->setSpacing(1);
    slLayout->addWidget(m_slRot);
    slLayout->addWidget(m_private->mrHighRes);
    layout->addWidget(slWidget);

    QObject::connect(m_slRot, &QSlider::valueChanged, [&]()
    {
        // assume, differential rotation has been initialized (buffers ready)

        if(!m_validRotation) return;

        uint im = *m_im - 1, in = *m_in - 1;
        uint n_cols = m_Vc->n_cols;

        if(im == in || im >= n_cols || in >= n_cols) return;

        // either rotate by 180° or take the ar-resolution
        double sliderValue = (double)m_slRot->value() - 49.0; // -50 to 50
        double angle = sliderValue / (m_private->mrHighRes->isChecked() ? *m_arResolution : (100.0 / M_PI));

        pairwiseRotation(im, in, angle, false, *m_oblique);

        outputChanged();
    });

    QPushButton* autoRot = new QPushButton(QObject::tr("Auto"));
    autoRot->setCheckable(true);
    QObject::connect(autoRot, &QPushButton::toggled, [&](bool checked)
    {
        if(checked) startAutoRotation();
        else        stopAutoRotation();
    });
    layout->addWidget(autoRot);

    QPushButton* varOpt = new QPushButton(QObject::tr("Variance"));
    QObject::connect(varOpt, &QPushButton::clicked, [&]() {realignForVariance();});
    layout->addWidget(varOpt);

    layout->addWidget(new QLabel(QObject::tr("Blocked Components")));
    layout->addWidget(m_fixed.widget());

    layout->addWidget(new QLabel(QObject::tr("Step Angle [1/pi]")));
    m_arResolution = 1000;
    layout->addWidget(m_arResolution.widget());

    layout->addWidget(new QLabel(QObject::tr("Bandwidth")));
    m_arBandwidth = 1;
    layout->addWidget(m_arBandwidth.widget());

    layout->addWidget(new QLabel(QObject::tr("Step Variation [1/pi] and period")));
    QWidget* stepVarWidget = new QWidget;
    QHBoxLayout* stepVarLayout = new QHBoxLayout(stepVarWidget);
    stepVarLayout->setContentsMargins(0, 0, 0, 0); stepVarLayout->setSpacing(1);
    stepVarLayout->addWidget(m_arResVariation.widget());
    stepVarLayout->addWidget(m_arResVariationSteps.widget());
    layout->addWidget(stepVarWidget);

    auto fnVariationUpdate = [&](){m_private->recalculateStepVariations(*m_arResVariation, *m_arResVariationSteps);};
    m_arResVariation.addCallback(fnVariationUpdate, sigChanged);
    m_arResVariationSteps.addCallback(fnVariationUpdate, sigChanged);

    layout->addWidget(new QLabel(QObject::tr("Min. oblq. angle")));
    layout->addWidget(m_betaMin.widget());

    layout->addWidget(new QLabel(QObject::tr("Fixed axes")));
    layout->addWidget(m_fixedAxis.widget());

    layout->addWidget(new QLabel(QObject::tr("Fixed coords")));
    layout->addWidget(m_fixedCoords.widget());

    layout->addWidget(new QLabel(QObject::tr("Target components")));
    layout->addWidget(m_targetComps.widget());

    m_im.addCallback([&](){prepareDifferentialRotation();}, sigChanged);
    m_in.addCallback([&](){prepareDifferentialRotation();}, sigChanged);
    m_comps.addCallback([&](){resetRotation();}, sigChanged);
    m_observations.addCallback([&](){resetRotation();}, sigChanged);

    m_iom = 0; m_ion = 0;
    /*layout->addWidget(new QLabel(QObject::tr("Oblique Rotation Pair")));
    QWidget* oblRotWidget = new QWidget;
    QHBoxLayout* oblRotLayout = new QHBoxLayout(oblRotWidget);
    oblRotLayout->setContentsMargins(0, 0, 0, 0); oblRotLayout->setSpacing(1);
    oblRotLayout->addWidget(m_iom.widget());
    oblRotLayout->addWidget(m_ion.widget());
    layout->addWidget(oblRotWidget);*/

    m_private->arDebugging->setCheckable(true);
    layout->addWidget(m_private->arDebugging);

    layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));

    swapAxes(false); // update U/V pointers
}

tRotationFilter::~tRotationFilter()
{
    stopAutoRotation();
    doCallbacks(sigDecaying);
    delete m_slRot;
    delete m_private;
}

void tRotationFilter::setInputWindow(tWindow* window)
{
    m_inputData.setList(&window->getData());

    /// look for existing rotations (e.g. from PCA)
    /*for(tData* data : (*m_inputWindow)->getData())
    {
        const tMatrixData* input = dynamic_cast<const tMatrixData*>(data);
        if(!input) continue;

    }*/

    // assume that everything changed
    inputChanged(true, true);

    run_impl(true);

}

bool tRotationFilter::updateInputWindow(const tList<tWindow*>& list)
{
    if(!tFilter::updateInputWindow(list) || !m_currentInputWindow) return false;
    return true;
}

#define PROVIDE_COMPS()     tDiscreteSelection::Items _comps0 = m_comps->getItems(), _obs0 = m_observations->getItems(); \
                            arma::uvec comps(_comps0.data(), _comps0.size(), false), obs(_obs0.data(), _obs0.size(), false);

/* What can happen?
 * - new input data field was set
 * - input data has changed
 * - new input rotation field was set
 * - input rotation has changed
 *
 *
 */


void tRotationFilter::inputChanged(bool dataChanged, bool rotationChanged)
{
    // completely dispatch from "rotation and data" and just regard U and V
    bool UChanged = dataChanged, VChanged = rotationChanged;
    if(isSwapped()) std::swap(UChanged, VChanged);

    // invalidate rotation
    m_validRotation = false;

    if(!*m_Ui) {T_COUT("[Rotation] no U, aborting!"); return;}

    if(UChanged)
    {
        *m_Uc = m_Ui->getSubmatrix();

        // update internal rotation
        if(!VChanged && !*m_Vi && m_Uc->n_cols != m_Vc->n_cols)
            VChanged = true;

    }

    const uint n_cols = m_Uc->n_cols, n_rows = m_Uc->n_rows;

    if(VChanged)
    {
        if(*m_Vi)   *m_Vc = m_Vi->getSubmatrix();
        else        m_Vc->eye(n_cols, n_cols);

        *m_Vo = *m_Vc;
    }

    // check validity
    if(!m_Uc->n_elem || !m_Vc->n_elem || !m_Vc->is_square() || m_Vc->n_cols != n_cols)
    {
        if(dataChanged || rotationChanged)
            {T_COUT("[Rotation] incompatible inputs!"); return;}

        // try to get valid fields
        else inputChanged(true, true);  /// [OPT] take only the relevant field
    }

    m_validRotation = true;
}

void tRotationFilter::swapAxes(bool swap)
{
    /// use some Mutex here!

    if(!swap)   // has to be called first to bring in the object addresses!
    {
        m_Uc = &m_dataCache;
        m_Uo = &m_outputData->matrix();
        m_Ui = &(*m_inputData);

        m_Vc = &m_rotationCache;
        m_Vo = &m_outputRotation->matrix();
        m_Vi = &(*m_inputRotation);
    }

    else
    {
        // buffer U
        arma::mat *C = m_Uc, *O = m_Uo; const tMatrixDataSelection *I = m_Ui;

        m_Uc = m_Vc;
        m_Uo = m_Vo;
        m_Ui = m_Vi;

        m_Vc = C;
        m_Vo = O;
        m_Vi = I;
    }

    // Transfer variance to U

    const uint nc = m_Vc->n_cols;
    for(uint c = 0; c < nc; ++c)
    {
        auto cV = m_Vc->col(c), cU = m_Uc->col(c);

        double vl = sqrt(sum(square(cV))); // vector length

        cV /= vl;
        cU *= vl;
    }

    // apply
    *m_Uo = *m_Uc;
    *m_Vo = *m_Vc;
}

bool tRotationFilter::isSwapped() const {return m_swapped;}

bool tRotationFilter::isValidRotation() const {return m_swapped;}

void tRotationFilter::realignForVariance()
{
    if(!m_validRotation) return;

    const tDiscreteSelection::Items fixed = m_fixed->getItems();
    std::vector<arma::uword> comps;
    m_private->getVariableComponents(comps, fixed, m_Vc->n_cols);

    arma::uvec icomps(comps.data(), comps.size(), false);

    arma::mat X = m_Uc->cols(icomps) * m_Vc->cols(icomps).t();

    arma::mat U, V; arma::colvec s;
    const bool svd_ok = svd(U, s, V, X, "std");
    if(!svd_ok) return;

    //icomps.resize(V.n_cols);

    //for(uint c = 0; c < icomps.n_elem; ++c)
    //    U.col(c) *= s(c);

    arma::mat S = X * V;

    m_Uo->cols(icomps) = S.cols(0, icomps.n_elem-1);
    m_Vo->cols(icomps) = V.cols(0, icomps.n_elem-1);

    prepareDifferentialRotation();

    outputChanged();
}

void tRotationFilter::randomRotation()
{
    const tDiscreteSelection::Items fixed = m_fixed->getItems();
    std::vector<arma::uword> comps;
    m_private->getVariableComponents(comps, fixed, m_Vc->n_cols);

    auto comps2 = comps;
    srand(unsigned(time(NULL)));
    auto engine = std::default_random_engine{static_cast<long unsigned int>(time(0))};
    std::shuffle(comps2.begin(), comps2.end(), engine);

    for(uint i = 0; i < comps.size(); ++i)
        if(comps[i] != comps2[i])
        {
            pairwiseRotation(comps[i], comps2[i], (double)(rand() % 1000 - 500) / 1000.0 * 2.0 * M_PI, false, false);
            prepareDifferentialRotation();
        }


    outputChanged();
}


void tRotationFilter::resetRotation()
{
    if(!*m_Ui) return;

    const uint n_cols = m_Uc->n_cols;

    if(*m_Vi)   *m_Vc = m_Vi->getSubmatrix();
    else        m_Vc->eye(n_cols, n_cols);

    *m_Vo = *m_Vc;

    run_impl(true);
}


void tRotationFilter::prepareDifferentialRotation()
{
    /*m_buffer.resize(m_output.size());
    for(uint i = 0; i < m_output.size(); ++i)
        m_buffer[i] = m_output[i]->matrix();*/

    m_dataCache = m_outputData->matrix();
    m_rotationCache = m_outputRotation->matrix();

    m_slRot->setValue(49);
}


void tRotationFilter::run_impl(bool notify)
{
    using namespace arma;

    if(!m_window) {T_COUT("[tRotationFilter] Error: run() called without window!"); return;}

    m_rotationMutex.lock(); ON_LEAVE(m_rotationMutex.unlock();)

    /// regard swapped axes and identity rotation!
    // get the input data to be rotated
    if(*m_Ui) *m_Uc = m_Ui->getSubmatrix();

    // check validity
    inputChanged();

    if(!m_validRotation) {T_COUT("[tRotationFilter] Error: no valid rotation!"); return;}

    try
    {
        // when dealing with an existing factor system (both U and V given), we just take U as result
        if(*m_Vi && *m_Ui)  *m_Uo = *m_Uc;
        else                *m_Uo = *m_Uc * inv(*m_Vc).t();
    }

    catch(...) {T_COUT("[tRotationFilter] Error: no inverse for rotation matrix found!");}

    //if(m_private->rotationRunning)
    prepareDifferentialRotation();

    outputChanged(notify);
}

void tRotationFilter::startAutoRotation()
{
    if(m_private->rotationThread)
        {T_COUT("[tRotationFilter] Auto-rotation already running"); return;}

    if(!initAutoRotation()) return;

    m_private->rotationRunning = true;
    m_private->rotationEnded = false;

    m_private->rotationThread = new std::thread([&]()
    {
        while(m_private->rotationRunning)
            autoRotate();

        m_private->rotationEnded = true;
    });
}

void tRotationFilter::stopAutoRotation()
{
    if(!m_private->rotationThread) return;

    m_private->rotationRunning = false; //usleep(1000); /// [PB] when AR-thread sends delayed/waited signal after flush, we have a deadlock!

    // we probably have a delayed and waited for changed() in queue and have to run it first
    while(!m_private->rotationEnded)
        QCoreApplication::processEvents();

    m_private->rotationThread->join();

    delete m_private->rotationThread;
    m_private->rotationThread = nullptr;

    m_private->releaseOwnerships();
}

inline void init2DRotationMatrix(arma::mat& rot, double rad)
{
    double s = sin(rad), c = cos(rad);
    rot(0,0) = c;  rot(0,1) = -s;
    rot(1,0) = s;  rot(1,1) = c;
}

inline void rotateScores(arma::mat& buf, arma::mat& out, uint i1, uint i2, double alpha, double beta, bool oblique)
{
    double delta = beta - alpha;

    if(oblique)
    {
        out.col(i1) = buf.col(i1) * sin(beta) / sin(delta);
        out.col(i2) = buf.col(i2) - buf.col(i1) * sin(alpha) / sin(delta);
    }

    else
    {
        out.col(i2) = buf.col(i2) * sin(delta) / sin(beta) - buf.col(i1) * sin(alpha) / sin(beta);
        out.col(i1) = buf.col(i1) * sin(beta) / sin(delta) + out.col(i2) * sin(alpha) / sin(delta);
    }
}

inline void rotateAxes(arma::mat& buf, arma::mat& out, uint i1, uint i2, double alpha, double beta, bool oblique)
{
    /// [OPT] maybe find a vector operation here instead of orthogonalization
    arma::colvec L2_orth = buf.col(i2) - arma::dot(buf.col(i2), buf.col(i1)) * buf.col(i1);
    L2_orth /= vectorLength(L2_orth); // unit vector length

    out.col(i1) = cos(alpha) * buf.col(i1) + sin(alpha) * L2_orth;
    if(!oblique) out.col(i2) = cos(alpha + beta) * buf.col(i1) + sin(alpha + beta) * L2_orth;
}


inline void tRotationFilter::pairwiseRotation(uint i1, uint i2, double alpha, bool apply, bool oblique)
{
    init2DRotationMatrix(Rp, alpha);  /// [OPT] have different 2d-rot-mat cached!
    uvec indices{i1, i2};

    //assert(m_buffer.size() == m_output.size());
    if(!m_Uo) return;

    // get current angle between axes. when no axes were found, we assume "identity loadings"
    double beta = acos(arma::dot(m_Vc->col(i1), m_Vc->col(i2)));

    if(!arma::is_finite(beta))
        {T_COUT("tRotationFilter: error calculating beta!"); return;}

    if(oblique && fabs(beta - alpha) < (*m_betaMin * M_PI / 180.0) ) // prev: < M_PI / 100
        {T_COUT("tRotationFilter: resulting angle too small!"); return;}

    rotateAxes(*m_Vc, *m_Vo, i1, i2, alpha, beta, oblique);
    rotateScores(*m_Uc, *m_Uo, i1, i2, alpha, beta, oblique);

    if(apply)
    {
        m_Vc->cols(indices) = m_Vo->cols(indices);
        m_Uc->cols(indices) = m_Uo->cols(indices);
    }

    // is not registered as public data
    //rotateScores(m_coordinatesBuffer, m_coordinates, i1, i2, alpha, beta, oblique);
    //if(apply) m_coordinatesBuffer.cols(indices) = m_coordinates.cols(indices);

    //if(oblique && apply) T_COUT("Beta(" << i1 << "," << i2 << ") " << (beta - alpha) * 180.0 / M_PI);
}

inline void tRotationFilter::resetRotation(uint i0, uint i1)
{
    if(!m_Uo) return;

    uvec i{i0, i1};

    m_Uo->cols(i) = m_Uc->cols(i);
    m_Vo->cols(i) = m_Vc->cols(i);
}


bool tRotationFilter::initAutoRotation()
{
    m_private->releaseOwnerships();

    if(!m_private->getObservers(m_window))
        {T_COUT("[tRotationFilter] No Observers found!"); return false;}

    int n = m_sbuf.n_cols;
    int n_o = m_private->observers.size();
    uint n_dirs = NO_DIRS(n); // number of directions

    m_private->arPriorities.clear();
    m_private->arPriorities.resize(n_o);

    m_private->resetPriorities();

    prepareDifferentialRotation();

    return true;
}

void tRotationFilter::autoRotate()
{
    if(!m_Uc) return;

    m_rotationMutex.lock();

    std::vector<tObserver::Ownership>& observers = m_private->observers;
    std::vector<RotationalPriority>& priorities = m_private->arPriorities;

    m_private->resetPriorities();

    int n = m_rotationCache.n_cols;
    int n_o = observers.size();
    //uint n_dirs = NO_DIRS(n); // number of directions. last *2 is sign!
    uint bandwidth = *m_arBandwidth; // >= n_dirs ? n_dirs - 1 : *m_arBandwidth;
    double s_diff[n_o * 2];

    const double alpha = M_PI / (*m_arResolution + m_private->getCurrentStepVariation());

    m_private->lockForAutoRotation();

    /// we should update every step like this. However, doing this lead to drifting rotation (89e5125df2)
    /// was output "dirty" due to gradient calculation? I tested reversing each probe rotation but it didnt help?!
    //prepareDifferentialRotation();

    Private::Scores s0(n_o); //, s1(n_o);
    m_private->getScores(s0);

    tDiscreteSelection::Items fixed = m_fixed->getItems();
    tDiscreteSelection::Items fixedAxis = m_fixedAxis->getItems(), fixedCoords = m_fixedCoords->getItems();
    tDiscreteSelection::Items targetComps = m_targetComps->getItems();

    std::vector<uint> comps0, comps1;

    /// [OPT] we should use the caps of tDiscreteSelection here!
    // now index the components used in comps1. exclude blocked components
    comps1.reserve(n - fixed.size());
    for(int i = 0; i < n; ++i) comps1.push_back(i);
    for(uint toBeRemoved : fixed) removeFromVector(comps1, toBeRemoved);

    // special case: "projection pursuit": comps0 only has the target components
    if(targetComps.size())
    {
        // be sure that target comps are not fixed (would be users fault)
        for(uint toBeRemoved : fixed) removeFromVector(targetComps, toBeRemoved);

        for(uint tc : targetComps)
        {
            comps0.push_back(tc);

            // have to reorder target components in second index
            removeFromVector(comps1, tc);
        }

        comps1.insert(comps1.begin(), comps0.begin(), comps0.end());
    }

    // when not in PP-mode, try every combination
    else comps0 = comps1;

    uint n_j = comps0.size(), n_k = comps1.size();

    bool ard = m_private->arDebugging->isChecked(); std::ofstream ardfile;
    if(ard) ardfile.open("ard.dat", std::ios_base::out | std::ios_base::app);

    const bool oblq = *m_oblique;

    uint n_comb = 0;
    for(uint j = 0; j < n_j; ++j)
    {
        for(uint k = j + 1; k < n_k; ++k)
        {
            // assemble index helper for axis-wise rotation and
            // regard the rotated-component list
            const uint i_cross[3] = {comps0[j], comps1[k], comps0[j]};
            //std::cout << j << " " << k << " (" << s0[0] << ") : ";

            for(int axis = 0; axis <= oblq; ++axis) // when in oblique mode, rotate each axis using i_cross helper
            {
                const uint c_j = i_cross[axis], c_k = i_cross[axis+1];

                const bool c_j_fixed = isInContainer(fixedAxis, c_j), c_k_fixed = isInContainer(fixedAxis, c_k);
                const bool c_j_cfixed = isInContainer(fixedCoords, c_j), c_k_cfixed = isInContainer(fixedCoords, c_k);

                // do not move a fixed axis
                if((oblq && c_j_fixed) || (!oblq && (c_j_fixed || c_k_fixed))) continue;

                // do not changed fixed coordinates / scores
                if((oblq && c_k_cfixed) || (!oblq && (c_j_cfixed || c_k_cfixed))) continue;

                for(int s = -1; s <= 1; s += 2)
                {
                    const bool positive = (s > 0);

                    pairwiseRotation(c_j, c_k, (double)s * alpha, false, oblq);
                    m_private->runPrefilters();

                    if(ard) ardfile << "0 " << " " << c_j << " " << c_k << " " << s;

                    // treat each observer independently and remember results signwise
                    for(int i = 0; i < n_o; ++i)
                    {
                        double s1 = observers[i]->calculate();
                        s_diff[i + n_o * positive] = s1 - s0[i];

                        if(ard) ardfile << " " << s1 - s0[i];
                    }

                    if(ard) ardfile << "\n";

                    //std::cout << diff << " ";

                    n_comb++;
                }

                /// [OPT] the sign mechanism is too complex and slow
                // now chose which sign to use
                for(int i = 0; i < n_o; ++i)
                {
                    bool choose_positive = s_diff[i] < s_diff[i + n_o];
                    if(s_diff[i + choose_positive * n_o] > 0)
                        priorities[i].emplace_back(c_j, c_k, (double)(choose_positive * 2 - 1) * alpha, s_diff[i + choose_positive * n_o], oblq);
                }

                // when finished with one pair, reset to original position before proceeding (output is still evaluated by observers)
                resetRotation(c_j, c_k);
            }
        }
    }

    if(ard) T_COUT("[ARD] tried " << n_comb << "combinations.");

    // score the oblique pair
    if(*m_iom && *m_ion && *m_iom != *m_ion)
    {
        uint im = *m_iom - 1, in = *m_ion - 1;
        uint i_cross[3] = {im, in, im}; // helper for switching indices

        // each axis
        for(uint* i = i_cross; i != i_cross + 2; ++i)
        {
            // each direction (sign)
            for(int s = -1; s <= 1; s += 2)
            {
                bool positive = (s > 0);

                // each observer
                for(int o = 0; o < n_o; ++o)
                {
                    pairwiseRotation(*i, *(i+1), (double)s * alpha, false, true);
                    m_private->runPrefilters();

                    double s1 = observers[o]->calculate();
                    s_diff[o + n_o * positive] = s1 - s0[o];
                }

                /// [OPT] same as above!!!
                // now chose which sign to use
                for(int o = 0; o < n_o; ++o)
                {
                    bool choose_positive = s_diff[o] < s_diff[o + n_o];
                    if(s_diff[o + choose_positive * n_o] > 0)
                        priorities[o].emplace_back(*i, *(i+1), (double)(choose_positive * 2 - 1) * alpha, s_diff[o + choose_positive * n_o], true);
                }
            }
        }
    }

    bool rotation_happened = false; // detect stationary state for cpu time management

    // find optimal rotations for each observer by sorting and apply according to bandwidth
    for(int i = 0; i < n_o; ++i)
    {
        if(observers[i]->checkFlags(fDisabled) || priorities[i].empty()) continue;

        uint bandwidth_used = bandwidth <= priorities[i].size() ? bandwidth : priorities[i].size();
        rotation_happened |= bandwidth_used;

        if(!bandwidth_used) continue;

        std::sort(priorities[i].begin(), priorities[i].end());

        for(uint p = 0; p < bandwidth_used; ++p)
        {
            if(ard) ardfile << i+1 << " " << priorities[i][p].j << " " << priorities[i][p].k << " " << priorities[i][p].s << priorities[i][p].value << "\n";

            pairwiseRotation(priorities[i][p].j, priorities[i][p].k, priorities[i][p].s, true, priorities[i][p].oblique);
        }
    }

    if(ard && !rotation_happened)
        ardfile << 0 << " " << 0 << " " << 0 << " " << 0 << "\n";

    //m_scores->unlockMutex(); m_loadings->unlockMutex();

    m_private->lockForAutoRotation(false);
    m_rotationMutex.unlock();

    // check for disabled observers and update list of observers and prefilters when needed
    bool observerDisabled = false;
    for(tObserver::Ownership& o : m_private->observers)
        observerDisabled |= o->checkFlags(fDisabled);

    if(observerDisabled)
    {
        m_private->releaseOwnerships();
        m_private->getObservers(m_window);
    }

    // only wait when rotation didn't stop yet
    if(!rotation_happened) 
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    else queueTask([&]()
    {
        //m_window->getView()->update();

        outputChanged();
        //runFiltersFollowing();

    }, true);

        //outputChanged(true, true, true/*m_private->rotationRunning*/);
}

/*inline void tRotationFilter::lockInputData()
{
    if(m_lockedInput.size()) return;

    for(auto& input : m_input)
    {
        m_lockedInput.push_back(input.lock());
        m_lockedInput.back()->lockMutex();
    }
}

inline void tRotationFilter::unlockInputData()
{
    for(auto& input : m_lockedInput)
        input->unlockMutex();

    m_lockedInput.clear();
}*/

inline void tRotationFilter::outputChanged(bool signal , bool delayed , bool wait)
{
    /*for(auto& output : m_output)
        output->dataChanged(signal, delayed, wait);*/

    m_outputData->dataChanged(signal, delayed, wait);
    m_outputRotation->dataChanged(signal, delayed, wait);

    runFiltersFollowing(signal);
}

void tRotationFilter::storeRotation()
{
    m_private->rotationStorage[m_private->rotStorageSlot->currentIndex()] = *m_Vo;
    m_private->dataStorage[m_private->rotStorageSlot->currentIndex()] = *m_Uo;
}

void tRotationFilter::restoreRotation()
{
    arma::mat &R = m_private->rotationStorage[m_private->rotStorageSlot->currentIndex()];
    arma::mat &D = m_private->dataStorage[m_private->rotStorageSlot->currentIndex()];


    if(m_Vo->n_rows != R.n_rows || m_Vo->n_cols != R.n_cols) {T_COUT("[Rotation] restore: incompatible extents!"); return;}

    *m_Vc = R;
    *m_Vo = R;

    *m_Uc = D;
    *m_Uo = D;

    //run_impl(true);
    outputChanged();

    T_COUT("Successfully restored!");
}

bool tRotationFilter::Private::getObservers(tWindow* this_window, Graph<tFilter>* prefilters)
{
    tFilter* 	this_filter = this_window->getFilter();
    tObserver* 	this_observer = this_filter ? dynamic_cast<tObserver*>(this_window->getFilter()) : nullptr;

    bool first;
    if(!prefilters) {first = true; prefilters = new Graph<tFilter>;}
    else            first = false;

    // observer was found! close this leaf
    if(this_observer)
    {
        if(!this_observer->checkFlags(fDisabled))
        {
            //T_COUT("Found observer!");
            observers.push_back(this_observer->getOwnership());
            return true;
        }

        // cancel search
        else return false;
    }

    // look if any following window contains an observer
    bool observerFound = false;
    for(tWindow* outputWindow : this_window->getOutputWindows())
    {
        tFilter* f;
        if(observerFound |= getObservers(outputWindow, prefilters) && (f = outputWindow->getFilter()))
            prefilters->addConnection(f, this_filter);

        //observerFound |= getObservers(outputWindow);
    }

    /*if(observerFound && this_filter && !first) // do not take first rotation filter
    {
        for(tWindow* w : this_window->getOutputWindows())
            if(tFilter* f = w->getFilter())
                prefilters->addConnection(f, this);

        T_COUT("Found required filter!");
        prefilters.push_back(this_filter->getOwnership());
    }*/

    // topological sort of prefilters
    if(first)
    {
        prefilters->topologicalSort();

        if(!prefilters->getSorted().empty())
            for(auto it = ++(prefilters->getSorted().begin()); it != prefilters->getSorted().end(); ++it) // leave first out: is the rotation filter
                if(!dynamic_cast<tObserver*>(*it)) // do not transfer observers
                    this->prefilters.push_back((*it)->getOwnership());

        delete prefilters;

        observers.erase(std::unique(observers.begin(), observers.end()), observers.end());

        std::cout << "[AR] Prefilters: ";
        for(auto& f : this->prefilters)
                std::cout << f->getName().toStdString() << ", ";
        std::cout << std::endl << std::flush;
    }


    if(first && arDebugging->isChecked())
    {
        std::ofstream ardinfo("ardinfo.txt");

        ardinfo << "[ARD] found " << observers.size() << " observer and " << this->prefilters.size() << " filters:\n";
        for(auto& f : this->prefilters)
                ardinfo << f->getName().toStdString() << ", ";
        ardinfo << std::endl << std::flush;
    }

    return observerFound;
}


void tRotationFilter::Private::releaseOwnerships() {prefilters.clear(); observers.clear();}

void tRotationFilter::Private::runPrefilters()
{
    for(tFilter::Ownership& filter : prefilters)
        filter->run(false, false);

    // must be run reversely due to nature of getObservers()
    /*for(auto it = prefilters.rbegin(); it != prefilters.rend(); ++it)
    {
        //if(arDebugging->isChecked())
        //    T_COUT("[ARD] running prefilter " << (*it)->getName());

        (*it)->run(false, false);
    }*/
}

inline void tRotationFilter::Private::getScores(tRotationFilter::Private::Scores& scores) const
{
    for(uint i = 0; i < observers.size(); ++i)
        scores(i) = observers[i]->calculate();
}

static inline void _lock(const tObject* obj, bool lock)
{
    if(lock)    obj->lockMutex();
    else        obj->unlockMutex();
}

inline void tRotationFilter::Private::lockForAutoRotation(bool lock) const
{
    //_lock(m_scores.get(), lock); _lock(m_loadings.get(), lock);

    for(const auto& filter : prefilters)
        _lock(filter.get(), lock);

    for(const auto& observer : observers)
        _lock(observer.get(), lock);
}

void tRotationFilter::Private::resetPriorities()
{
    for(auto& p : arPriorities)  /// [OPT] does "clear" also clear reserved capacity?
        p.clear();
}

void tRotationFilter::Private::recalculateStepVariations(uint variation, uint steps)
{
    arCurrentVariationStep = 0; arLastVariation = variation; arLastSteps = steps;

    if(!arLastVariation || !arLastSteps) return;

    arma_rng::set_seed_random();
    arStepVariations = (arma::randu(steps) - 0.5) * variation / steps;

    for(int step = 0; step < steps; ++step)
        arStepVariations(step) += -(double)variation/2.0 + (double)step * (double)variation / (double)steps;

    //arStepVariations = arma::shuffle(arStepVariations);
}

inline double tRotationFilter::Private::getCurrentStepVariation()
{
    if(!arLastVariation || !arLastSteps) return 0;

    arCurrentVariationStep++;

    if(arCurrentVariationStep >= arStepVariations.n_elem)
        recalculateStepVariations(arLastVariation, arLastSteps);

    //std::cout << arStepVariations(arCurrentVariationStep) << " " << std::flush;

    return arStepVariations(arCurrentVariationStep);
}

void tRotationFilter::Private::getVariableComponents(std::vector<arma::uword>& comps, const tDiscreteSelection::Items& fixed, uint n_comps)
{
    comps.clear();
    comps.reserve(n_comps - fixed.size());

    auto it = fixed.begin();
    for(uint i = 0; i < n_comps; ++i)
    {
        if(it != fixed.end() && i == *it) ++it;
        else comps.push_back(i);
    }
}


arma::vec arStepVariations;
uint arCurrentVariationStep = 0;
