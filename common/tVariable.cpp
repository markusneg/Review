#include "tVariable.hpp"

QMap<QString, tVariable*> tVariable::m_exported;


/*tVariable::Ownership tVariable::Create()
{
    return Ownership(new tVariable);
}*/


tVariable::tVariable(QString name)
{
    setName(name);
}

tVariable::~tVariable()
{
    m_exported.remove(getName());
}

tVariable::tVariable(const tVariable &rhs) :
    tObject(rhs)
{
    assign(rhs);
}

tVariable& tVariable::operator=(const tVariable& rhs)
{
    tObject::operator=(rhs);
    return *this;
}


void tVariable::setName(const QString& name)
{
    // update index
    m_exported.remove(getName());
    m_exported[name] = this;

    tObject::setName(name);
}

/*void tVariable::doExport()
{
    m_exported[m_name] = this;
}

void tVariable::doUnexport()
{
    m_exported.
}*/

void tVariable::assign(const tVariable &rhs)
{
    setName(rhs.getName());
}

