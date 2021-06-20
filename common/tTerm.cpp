#include "tTerm.hpp"

#include <forward_list>
#include <QMap>
#include <QStringList>
#include <QRegExp>

#define p   (*m_private)

struct tTerm::Private
{
    QMap<QString, tObject*> m_variables;

    // when new variables are exported, all terms are asked to look for missing variables
    static std::forward_list<tTerm*> m_termIndex;
};

std::forward_list<tTerm*> tTerm::Private::m_termIndex;

/*tTerm::Ownership tTerm::Create()
{
    return Ownership(new tTerm);
}*/


tTerm::tTerm() :
    m_private(new Private)
{
    p.m_termIndex.push_front(this);
    findVariables();
}

tTerm::~tTerm()
{
    doCallbacks(sigDecaying);
    std::remove(p.m_termIndex.begin(), p.m_termIndex.end(), this);
    delete m_private;
}

tTerm::tTerm(const tTerm &rhs) :
    tObject(rhs), m_private(new Private)
{
    operator=(rhs.m_term);
    //m_term = rhs.m_term;

}

tTerm& tTerm::operator=(const tTerm& rhs)
{
    tObject::operator=(rhs);
    return *this;
}

tTerm& tTerm::operator=(const QString& term)
{
    m_term = term;
    updateVariables();
    findVariables();

    return *this;
}

QString tTerm::result() const
{
    QString result = m_term;

    for(QMap<QString, tObject*>::Iterator it = p.m_variables.begin(); it != p.m_variables.end(); ++it)
        if(it.value())
            result.replace(it.key(), it.value()->valueAsText());

    return result;
}


/*void tTerm::assign(const tTerm& rhs)
{
    m_term = rhs.m_term;
}*/

void tTerm::updateVariables()
{
    //QString name = m_term.replace(QRegExp("[\W]"), "");
    QStringList variables = m_term.split(QRegExp("(,|:)"), QString::SkipEmptyParts);

    // Regex.Replace(stringToCleanUp, @"[\W]", "");
    // [^a-zA-Z0-9]
    //str.remove(QRegExp("[^a-zA-Z\\d\\s]"));
    //QRegExp rx("[A-Z]|[a-z]*"); if( rx.exactMatch(Name) ) {....}'t

    p.m_variables.clear();

    for(const QString& variable : variables)
    {
        if(variable.contains(QRegExp("[\\W]")))
            continue;

        tObject* var = tObject::getVariable(variable);

        p.m_variables[variable] = var;
    }
}

void tTerm::findVariables()
{
    for(tTerm* term : Private::m_termIndex)
    {
        for(QMap<QString, tObject*>::Iterator var = term->m_private->m_variables.begin(); var != term->m_private->m_variables.end(); ++var)
        {
            var.value() = tObject::getVariable(var.key());

            if(var.value())
                var.value()->addCallbackSafe([term]() {term->doCallbacks(sigChanged);}, term, sigChanged);
        }
    }
}
