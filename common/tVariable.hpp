#pragma once

#include <QString>
#include <QMap>

#include "tObject.hpp"

class tVariable : public tObject
{

public:

    T_TYPEDEFS(tVariable)

    //static Ownership Create();
    tVariable(QString name);
    ~tVariable();

    tVariable(const tVariable &rhs);
    tVariable& operator=(const tVariable& rhs);

    static tVariable*       get(QString name);

    void setName(const QString&) override;

    //void doExport();
    //void doUnexport();

protected:


private:

    void assign(const tVariable& rhs);

    /// Should use tObject:name here!
    //const QString m_name;

    static QMap<QString, tVariable*> m_exported;
};
