#include "tDiscreteSelection.hpp"

#include <QDir>

// we use static closed intervals for convenience
#define BOOST_ICL_USE_STATIC_BOUNDED_INTERVALS
#define BOOST_ICL_DISCRETE_STATIC_INTERVAL_DEFAULT closed_interval
#include <boost/icl/interval_set.hpp>

typedef boost::icl::interval_set<uint>  BIntervals;
typedef BIntervals::interval_type       BInterval;

struct tDiscreteSelection::Private
{
    BIntervals intervals;
};

tDiscreteSelection::tDiscreteSelection() :
    m_private(new Private)
{

}

tDiscreteSelection::tDiscreteSelection(const tDiscreteSelection& rhs) :
    m_private(new Private)
{
    operator=(rhs);
}

tDiscreteSelection& tDiscreteSelection::operator=(const tDiscreteSelection& rhs)
{
    m_private->intervals = rhs.m_private->intervals;
    return *this;
}


tDiscreteSelection::~tDiscreteSelection()
{
    delete m_private;
}

void tDiscreteSelection::getIntervals(Intervals& out) const
{
    out.clear();

    for(auto& i : m_private->intervals)
        out.push_back({i.lower(), i.upper()});
}

tDiscreteSelection::Items tDiscreteSelection::getItems() const
{
    Items ret;

    for(auto& i : m_private->intervals)
        for(uint z = i.lower(); z <= i.upper(); ++z)
            ret.push_back(z);

    return ret;
}

uint tDiscreteSelection::getLowestItem() const
{
    return m_private->intervals.empty() ?
                0 :
                m_private->intervals.begin()->lower();
}

uint tDiscreteSelection::getHighestItem() const
{
    return m_private->intervals.empty() ?
                0 :
                m_private->intervals.rbegin()->upper();
}


void tDiscreteSelection::select(uint i)
{
    if(m_mode == Selected)  m_private->intervals.insert(i);
    else                    m_private->intervals.erase(i);
}

void tDiscreteSelection::select(uint from, uint to)
{
    BInterval newInterval = BInterval(from, to);

    if(m_mode == Selected)  m_private->intervals += newInterval;
    else                    m_private->intervals -= newInterval;
}

void tDiscreteSelection::unselect(uint i)
{
    if(m_mode == Selected)      m_private->intervals.erase(i);
    else                        m_private->intervals.insert(i);
}

void tDiscreteSelection::unselect(uint from, uint to)
{
    BInterval newInterval = BInterval(from, to);

    if(m_mode == Selected)  m_private->intervals -= newInterval;
    else                    m_private->intervals += newInterval;
}

uint tDiscreteSelection::count() const
{
    uint count = 0;
    for(auto& i : m_private->intervals)
        count += i.upper() - i.lower() + 1;

    return count;
}

void tDiscreteSelection::fromString(const QString& str)
{
    QStringList terms = str.split(',', QString::SkipEmptyParts);

    reset();

    for(const QString& term : terms)
    {
        if(term.contains(':'))
        {
            QStringList lowerUpper = term.split(':', QString::SkipEmptyParts);
            if(lowerUpper.size() != 2) continue;

            bool ok;
            uint from = lowerUpper[0].toUInt(&ok); if(!ok) continue;
            uint to =   lowerUpper[1].toUInt(&ok); if(!ok) continue;

            // internal representation starts counting from 0. When user enters 0, 1 is taken
            if(from) from -= 1; if(to) to -= 1;

            m_private->intervals += BInterval(from, to);
        }

        else
        {
            bool ok;
            uint newValue = term.toUInt(&ok); if(!ok) continue;

            if(newValue) newValue -= 1;

            m_private->intervals.insert(newValue);
        }
    }
}

QString tDiscreteSelection::toString() const
{
    QString ret;

    for(auto& i : m_private->intervals)
    {
        if(i.lower() == i.upper())
            ret += QString::number(i.lower() + 1);
        else
            ret += QString::number(i.lower() + 1) + ":" + QString::number(i.upper() + 1);

        ret += ", ";
    }

    // truncate last comma..
    if(ret.size()) ret.chop(2);

    return ret;
}

void tDiscreteSelection::reset()
{
    m_private->intervals.clear();
}

tDiscreteSelection::Mode tDiscreteSelection::getMode() const
{
    return m_mode;
}

void tDiscreteSelection::setMode(tDiscreteSelection::Mode newMode)
{
    if(newMode == m_mode) return;

    // if selections exist, apply the complement
    if(!m_private->intervals.empty())
    {
        uint min = m_private->intervals.begin()->lower();
        uint max = m_private->intervals.rbegin()->upper();

        BIntervals complement;
        complement += BInterval(min, max);

        for(auto& i : m_private->intervals)
            complement -= i;

        m_private->intervals = complement;
    }

    m_mode = newMode;
}

bool tDiscreteSelection::isContinuous() const
{
    /// [TBD] "Unselected" mode has to be regarded here

    return m_private->intervals.iterative_size() == 1;
}
