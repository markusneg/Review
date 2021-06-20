#include "tMatrixDataSelection.hpp"

#include "tDataSelectionPolicy.hpp"


//#include <QMutex>

#define NO_SIGNAL false

struct tMatrixDataSelection::Private
{
    mutable bool userTouched[2];
};

tMatrixDataSelection::tMatrixDataSelection() :
    m_private(new Private)
{

}

tMatrixDataSelection::tMatrixDataSelection(const tMatrixDataSelection& rhs) :
    tObject(rhs), m_private(new Private)
{
    operator=(rhs);
}

tMatrixDataSelection::~tMatrixDataSelection()
{
    doCallbacks(sigDecaying);

    if(m_selectionPolicy) delete m_selectionPolicy;
    delete m_private;
}

tMatrixDataSelection& tMatrixDataSelection::operator=(const tMatrixDataSelection& rhs)
{
    m_mn[0] = rhs.m_mn[0]; m_mn[1] = rhs.m_mn[1];
    m_transpose = rhs.m_transpose;
    setData(rhs.m_data);

    return *this;
}

void tMatrixDataSelection::setData(tMatrixData* data)
{
    //lockMutex();    // be sure, this method is not run twice simultaneously

    if(m_data)
        {m_data->removeCallback(this); m_data->removeSignalForwarding(this);}

    //blockCallbacks();   // for reset()

    m_data = data;
    reset(false);

    //blockCallbacks(false);

    if(m_data)
    {
        // forget about user selection
        clearUserTouched(All);

        // let data unregister itself when decaying
        m_data->addCallbackSafe([&]() {setData(nullptr);}, this, sigDecaying);

        // validate when dimensions changed. If nothing is selected, reset, to show previously empty data
        m_data->addCallbackSafe([&]()
        {
            // lets adapt selected ranges when not explicitly given by user
            conditionalReset(false);

            if(m_selectionPolicy) m_selectionPolicy->onExtentChange();

            validate(false);

        }, this, sigExtentChanged);

        m_data->addSignalForwarding(this);

        if(m_selectionPolicy) m_selectionPolicy->onDataChange();
        validate(false);
    }

    //unlockMutex();

    doCallbacks(sigChanged);
}

const tMatrixData* tMatrixDataSelection::getData() const
{
    return m_data;
}

tMatrixData* tMatrixDataSelection::getData()
{
    return m_data;
}

void tMatrixDataSelection::select(Margin margin, uint i)
{
    m_mn[(uint)margin].select(i);
    markAsUserTouched(margin);
}

void tMatrixDataSelection::select(Margin margin, uint from, uint to)
{
    m_mn[(uint)margin].select(from, to);
    markAsUserTouched(margin);
}

void tMatrixDataSelection::unselect(Margin margin, uint i)
{
    m_mn[(uint)margin].unselect(i);
    markAsUserTouched(margin);
}

void tMatrixDataSelection::unselect(Margin margin, uint from, uint to)
{
    m_mn[(uint)margin].unselect(from, to);
    markAsUserTouched(margin);
}

void tMatrixDataSelection::markAsUserTouched(Margin margin) const
{
    m_private->userTouched[(uint)margin] = true;
}

bool tMatrixDataSelection::isUserTouched(Margin margin) const
{
    return m_private->userTouched[(uint)margin];
}


/*void tMatrixDataSelection::setData(const tMatrixData::ConstRef& data)
{
    blockCallbacks();
    m_data = data;
    reset();
    blockCallbacks(false);
    doCallbacks(sigChanged);
}*/

bool tMatrixDataSelection::validate(bool emitSignal)
{
    if(!m_data) return false;

    bool changed = false;
    for(uint mn : {0, 1})
    {
        tDiscreteSelection& sel = m_mn[mn];

        uint maxData = mn ? m_data->n_cols() : m_data->n_rows();
        uint maxSelection = sel.getHighestItem();

        if(maxSelection >= maxData)
            {sel.unselect(maxData, maxSelection); changed |= true;}
    }

    if(changed && emitSignal) doCallbacks(sigChanged);

    return changed;
}

inline void tMatrixDataSelection::reset(Margin margin, bool emitSignal)
{
    uint dim = (uint)margin;

    m_mn[dim].reset();
    m_mn[dim].setMode(tDiscreteSelection::Selected);

    uint size = m_data ? (dim ? m_data->n_cols() : m_data->n_rows()) : 0;

    if(size)
        m_mn[dim].select(0, size-1);

    if(emitSignal)
        doCallbacks(sigChanged);
}

void tMatrixDataSelection::reset(bool emitSignal)
{
    for(uint mn : {0, 1})
        reset((Margin)mn, false);

    if(emitSignal)
        doCallbacks(sigChanged);
}

void tMatrixDataSelection::conditionalReset(bool emitSignal)
{
    for(uint dim = 0; dim < 2; ++dim)
        if(!m_private->userTouched[dim])
            reset((Margin)dim, emitSignal);
}


const tDiscreteSelection* tMatrixDataSelection::getDiscreteSelections() const
{
    return m_mn;
}

tDiscreteSelection* tMatrixDataSelection::getDiscreteSelections()
{
    return m_mn;
}

void tMatrixDataSelection::setTranspose(bool on)
{
    m_transpose = on;
}

void tMatrixDataSelection::toggleTranspose()
{
    m_transpose = !m_transpose;
}

tMatrixDataSelection::operator bool() const
{
    return m_data;
}

bool tMatrixDataSelection::isContinuous() const
{
    return m_mn[0].isContinuous() && m_mn[1].isContinuous();
}

arma::mat tMatrixDataSelection::getSubmatrix() const
{
    if(!m_data) return arma::mat();

    //validate(NO_SIGNAL); // would be nice but not possible due to constness

    //tDiscreteSelection::Items mIt = m_mn[m_transpose].getItems();
    //tDiscreteSelection::Items nIt = m_mn[!m_transpose].getItems();

    /// [OPT] regard continuous views!

    tDiscreteSelection::Items mIt = m_mn[0].getItems();
    tDiscreteSelection::Items nIt = m_mn[1].getItems();

    arma::uvec im(mIt.data(), mIt.size(), false), in(nIt.data(), nIt.size(), false);

    auto ret = m_data->matrix()(im, in);

    try {
        if(m_transpose) return ret.t();
        else            return ret;}
    catch (...) {T_COUT("[tMatrixDataSelection] Invalid selection during getSubmatrix()! Returning zero-matrix"); return arma::mat();}
}

bool tMatrixDataSelection::isNothingSelected() const
{
    return !(m_mn[0].count() && m_mn[1].count()) || !m_data;
}

void tMatrixDataSelection::setSelectionPolicy(tDataSelectionPolicy* selectionPolicy)
{
    m_selectionPolicy = selectionPolicy;
}


void tMatrixDataSelection::clearUserTouched(Margin margin) const
{
    if(margin == All)   for(uint i : {0, 1}) m_private->userTouched[i] = false;
    else                m_private->userTouched[(uint)margin] = false;

}
