#pragma once

#include <list>
#include <array>

#include <QObject>

#include "../common/tObject.hpp"
//#include "tDataReference.hpp"
//#include "tLockableContainer.hpp"

class tWindow;
class Review;

/*  Why tData-Base?
 *  - Ref-Interface
 *  - Access
 *  - common bases (QObject..)
 *  - Signalling (Qt + tObject)
 */

/// [Design] why derive from QObject?

class tData : public QObject, public tObject
{

    Q_OBJECT

public:

    T_TYPEDEFS(tData)

    static Ownership Create();
    ~tData();

    //template <typename T>
    //tDataReference<T> getRef();


    // gain a secure reference to the data also providing selection mechanism
    //virtual Ref         getRef();
    //virtual ConstRef    getRef() const;

    // for data name, qobject caps are used
    QString         getName() const override;
    void            setName(const QString& name) override;

    // dimension info
    virtual QString getExtInfo() const;

    // returns name + dimInfo
    virtual QString getExtendedName() const;

    virtual bool load(const QString&);
    virtual bool save(const QString&) const;

    // derived have a chance to keep track of changes and emit the right signals. Defaultly emits sigChanged on call
    virtual void changed(bool delayed = false, bool wait = false) const;

    const tWindow*  getOwnerWindow() const;
    tWindow*        getOwnerWindow();

    // to be called when data has been changed. Will do callbacks
    //void contentChanged(bool doCallbacks = false) const;

    // mark changed extents of matrix
    //void extentChanged(bool doCallbacks = false) const;

    virtual Ownership copy() const;

    tData(const tData &rhs);
    tData& operator=(const tData& rhs);

    // used by tDataReference to instantiate the data selectors. To be customized by concrete class
    struct Selector {};

protected:

    tData();

    //bool m_contendModified = false, m_extentModified = false;

private:

    void assign(const tData& rhs);

    tWindow* m_ownerWindow = nullptr;

    // can gain ownership over tData
    friend class tWindow;
    friend class Review;
};


#include "tData.inl"
