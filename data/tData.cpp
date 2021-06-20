#include "tData.hpp"

#include <QDir>


tData::Ownership tData::Create()
{
    return Ownership(new tData);
}


tData::tData()
{
}

tData::~tData()
{
}


/*tData::Ref tData::getRef()
{
    return Ref(shared_from_this());
}

tData::ConstRef tData::getRef() const
{
    return shared_from_this();
}*/

QString tData::getName() const
{
    return QObject::objectName();
}

void tData::setName(const QString& name)
{
    QObject::setObjectName(name);
}

QString tData::getExtInfo() const {return QString();}

QString tData::getExtendedName() const
{
    return getName() + (getExtInfo().size() ? " " + getExtInfo() : QString());
}

bool tData::load(const QString&) {return false;}

bool tData::save(const QString&) const {return false;}

void tData::changed(bool delayed, bool wait) const
{
    doCallbacks(sigChanged, delayed, wait);
}

const tWindow* tData::getOwnerWindow() const
{
    return m_ownerWindow;
}

tWindow* tData::getOwnerWindow()
{
    return m_ownerWindow;
}

tData::Ownership tData::copy() const
{
    tData* copy = new tData(*this);
    return Ownership(copy);
}

tData::tData(const tData &rhs) :
    QObject(), tObject(rhs)
{
    assign(rhs);
}

tData& tData::operator=(const tData& rhs)
{
    tObject::operator=(rhs);
    return *this;
}

void tData::assign(const tData& rhs)
{
    setName(rhs.getName());
}

/*void tData::contentChanged(bool doCallbacks) const
{
    if(doCallbacks)
        doCallbacks(sigContentChanged | sigChanged);
}

void tData::extentChanged(bool doCallbacks) const
{
    if(doCallbacks)
        doCallbacks(sigExtentChanged | sigChanged);
}*/

