#pragma once

#include <functional>
#include <memory>
#include <list>

#include <QString>
#include <QMap>

#include "signals.hpp"
#include "tFlags.hpp"
//#include "tList.hpp"

// safe referencing is done using weak pointer
template <typename T>
using tReference = std::weak_ptr<T>;

// ownership controls lifetime
template <typename T>
using tOwnership = std::shared_ptr<T>;

// include tList.hpp to access this feature
template <typename T> class tList;


// to be placed in public area of derived classes
#define T_TYPEDEFS(type) \
    typedef tReference<type>        Ref;\
    typedef tReference<const type>  ConstRef;\
    typedef tList<type*>             List;\
    typedef tList<const type*>       ConstList;\
    typedef tOwnership<type>  Ownership;\
    typedef tOwnership<const type>  ConstOwnership;\
    typedef tList<tOwnership<type>> OwnershipList;\
    Ref         getRef() {return getRefOfType<type>();}\
    ConstRef    getRef() const {return getRefOfType<type>();}\
    Ownership   getOwnership() {return getOwnershipOfType<type>();}\
    ConstOwnership getOwnership() const {return getOwnershipOfType<type>();}


#define PTR(ref) &(*ref.lock())

// base class for all Review objects providing safe referencing and a basic callback mechanism
class tObject : public std::enable_shared_from_this<tObject>
{

public:

    T_TYPEDEFS(tObject)

    typedef std::function<void(void)> Callback;
    typedef Callback Task;

    static Ownership Create();
    virtual ~tObject();

    // to be overriden
    virtual QString getName() const;
    virtual void    setName(const QString&);

    // get an extended name. On default returns name only
    virtual QString getExtendedName() const;

    void addCallback(const Callback& callback, tSignals signal = sigAll) const;

    // id can be reused to remove callback again
    template <typename T>
    void addCallback(const Callback& callback, const T* id, tSignals signal) const;
    //void addCallback(const Callback& callback, const tObject* id, tSignal signal) const;

    /// [DBG] just virtual for testing
    // callback is automatically removed in when id is decaying
    virtual void addCallbackSafe(const Callback& callback, const tObject* id, tSignals signal) const;

    template <typename T>
    void removeCallback(const T* id) const;
    //void removeCallback(const tObject* id) const;   // this on is used as counterpart for addCallbackSafe, where always base ptr was used!

    void addSignalForwarding(const tObject* to) const;
    void removeSignalForwarding(const tObject *to) const;

    // run given task later and in main thread
    static void queueTask(const Task& task, bool wait = false);

    tObject(const tObject& rhs);
    tObject& operator=(const tObject& rhs);

    bool    checkFlags(tFlags flags) const;
    tFlags  getFlags() const;
    void    setFlags(tFlags flags);
    void    unsetFlags(tFlags flags);

    template <typename T>
    tReference<const T> getRefOfType() const;

    template <typename T>
    tReference<T>       getRefOfType();

    template <typename T>
    tOwnership<const T> getOwnershipOfType() const;

    template <typename T>
    tOwnership<T> getOwnershipOfType();

    // this is a general purpose non-recursive mutex, which usage can vary in derived classes
    void lockMutex() const;
    void unlockMutex() const;

    // check wether object is about to destruct
    bool isDecaying() const;

    // temporary block callback triggering. Works recursively!
    void blockCallbacks(bool block = true);

    // do all callbacks queued using doCallbacks(delayed = true)
    static void flushCallbacks();
    static uint getPendingCallbackCount();

    // comment about variables: the variables should be implemented for tSetting,
    // however, tSetting has no base class for the static lists.

    // make this object available as variable with getName() its variable name. After that available to getVariable()
    void exportAsVariable();

    // get an object registered as variable
    static tObject* getVariable(const QString& name);

    // get text representing value of object, e.g. for variable value
    virtual QString valueAsText() const;

protected:

    tObject();

    // do callbacks / emit signal. delayed will let it run later and by the main thread. wait will cause fn to return when job is finished
    void doCallbacks(tSignals signal, bool delayed = false, bool wait = false) const;

    // given signal is emitted on the next regular call of doCallbacks()
    void queueCallback(tSignals signal) const;

private:

    // internal methods for callback handling
    void addCallback(const Callback& callback, size_t id, tSignals signal) const;
    void removeCallback(size_t id) const;

    // will keep tObjects running (e.g. delayed signals)
    static void heartbeat();

    void assign(const tObject& rhs);

    struct Private;
    Private* m_private;

    tFlags m_flags = 0;

    QString m_name;

    mutable tSignals m_queuedSignals = 0;

    static QMap<QString, tObject*> m_variables;

    static std::list<tObject*> m_objectIndex;
    friend class Review;
};


#include "tObject.inl"
