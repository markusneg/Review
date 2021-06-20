#pragma once

#include <QString>

#include "tObject.hpp"

// a text expression containing variables and operations (no '=')
// manages its variables
class tTerm : public tObject
{

public:

    T_TYPEDEFS(tTerm)

    tTerm();
    ~tTerm();

    //Ownership copy() const;

    tTerm(const tTerm &rhs);
    tTerm& operator=(const tTerm& rhs);
    tTerm& operator=(const QString& term);

    QString result() const;

protected:


private:

    // parse term and associate variables
    void updateVariables();

    // usually called when new variables were defined; globally connect variables with terms
    static void findVariables();

    struct Private;
    Private* m_private;

    //void assign(const tTerm& rhs);

    QString m_term;

    friend class tObject;

};
