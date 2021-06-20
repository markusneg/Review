#include "tObject.hpp"

//#include <unistd.h>
#include <forward_list>
//#include <thread>

// TEST for windows sleep
#include <chrono>
#include <thread>

#include <QObject>
#include <QMutex>

#include "../util/util.hpp"
#include "tTerm.hpp"

using namespace std;

static QString _QStringDummy = QObject::tr("Noname");
std::list<tObject*> tObject::m_objectIndex;
QMap<QString, tObject*> tObject::m_variables;



/// [TODO] add mutex to callback handling


struct CallbackEntry
{
    inline CallbackEntry(tSignals s, const tObject::Callback& cb, size_t i) :
        signal(s), callback(cb), id(i) {}

    tSignals signal;
    tObject::Callback callback;
    size_t id;
};

typedef forward_list<CallbackEntry> CallbackList;


struct tObject::Private
{
    Private() : callbackMutex(QMutex::Recursive) {}

    /// [OPT] make a vector of this due to signalling performance!
    mutable CallbackList registeredCallbacks;
    mutable std::list<const tObject*> forwardCallbacksTo;
    volatile int callbacksBlocked = 0; // counter
    bool callbacksRunning = false;

    mutable QMutex callbackMutex, generalMutex;
    mutable std::vector<size_t> callbacksToRemove;
    mutable std::vector<const tObject*> forwardsToRemove;
    mutable bool decaying = false;

    // apply *ToRemove for callbacks and forwards
    void cleanup();
    volatile bool cleaningUp = false;

    QMutex dependentObjectsMutex;
    std::vector<const tObject*> dependentObjects; // all objects registered a safe-callback with this one
    void addDependentObject(const tObject* o);
    void removeDependentObject(const tObject* o);

    static vector<tObject::Task> tasksPending;
    static QMutex taskMutex;
    //static thread taskThread;

    bool exported = false;
};

tObject::tObject() :
    m_private(new Private)
{
    lockMutex();
    m_objectIndex.push_back(this);
    unlockMutex();
}

tObject::Ownership tObject::Create()
{
    return Ownership(new tObject);
}

tObject::~tObject()
{
    if(!m_private->decaying)
        T_COUT("EXIT WITHOUT DECAY!");

    if(m_private->forwardsToRemove.size() || m_private->callbacksToRemove.size())
        T_COUT("I AM NOT FINISHED!");

    lockMutex();
    m_objectIndex.remove(this);
    unlockMutex();

    delete m_private;    
}

tObject::tObject(const tObject& rhs) :
    std::enable_shared_from_this<tObject>(), m_private(new Private(/* rhs.m_private ?!? */))
{
    assign(rhs);
}

tObject& tObject::operator=(const tObject& rhs)
{
    assign(rhs);
    return *this;
}

void tObject::assign(const tObject& rhs)
{
    //*m_private = *rhs.m_private; ?!?
    m_flags = rhs.m_flags;
    m_name = rhs.m_name;
}

QString tObject::getName() const
{
    //return _QStringDummy;
    return m_name;
}

void tObject::setName(const QString& name)
{
    if(m_private->exported)
    {
        m_variables.remove(getName());
        m_variables[name] = this;

        tTerm::findVariables();
    }

    m_name = name;
    doCallbacks(sigNameChanged);
}

QString tObject::getExtendedName() const {return getName();}

void tObject::addCallback(const Callback& callback, tSignals signal) const
{
    addCallback(callback, 0, signal);
}

/*void tObject::addCallback(const Callback& callback, const tObject* id, tSignal signal) const
{
    std::hash<const tObject*> hash;
    addCallback(callback, hash(id), signal);
}*/

void tObject::addCallbackSafe(const Callback& callback, const tObject* id, tSignals signal) const
{
    /// [OPT] only *one* decaying-callback per object!

    addCallback(callback, id, signal);

    // when object requiring a callback decays, remove this callback
    m_private->addDependentObject(id);
    id->m_private->addDependentObject(this);

    //id->addCallback([&, id]() {removeCallback(id);}, this, sigDecaying);
    //addCallback([=]() {id->removeCallback(this);}, id, sigDecaying);
}

/*void tObject::removeCallback(const tObject* id) const
{
    removeCallback(tObjectDetails::getHash(id));
}*/

void tObject::addSignalForwarding(const tObject* to) const
{
    //lockMutex();
    m_private->forwardCallbacksTo.push_back(to);

    m_private->addDependentObject(to);
    to->m_private->addDependentObject(this);
    //unlockMutex();
}

void tObject::removeSignalForwarding(const tObject* to) const
{
    m_private->callbackMutex.lock();

    m_private->forwardsToRemove.push_back(to);

    if(!m_private->callbacksRunning)
        m_private->cleanup();

    m_private->callbackMutex.unlock();
}

//using namespace std;
void tObject::queueTask(const Task& task, bool wait)
{
    tObject::Private::taskMutex.lock();
    tObject::Private::tasksPending.push_back(task);
    tObject::Private::taskMutex.unlock();

    /// [TODO] task-specific wait!!
    if(wait)
        while(!tObject::Private::tasksPending.empty())
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

}

void tObject::lockMutex() const
{
    m_private->generalMutex.lock();
}

void tObject::unlockMutex() const
{
    m_private->generalMutex.unlock();
}

bool tObject::isDecaying() const
{
    return m_private->decaying;
}


void tObject::addCallback(const Callback& callback, size_t id, tSignals signal) const
{
    m_private->callbackMutex.lock();
    m_private->registeredCallbacks.emplace_front(signal, callback, id);
    //std::cout << "Added CB. Now have " << tObjectDetails::count(m_private->registeredCallbacks) << "! (" << getExtendedName().toUtf8().data() << ", " << id << ")\n" << std::flush;
    m_private->callbackMutex.unlock();
}

void tObject::removeCallback(size_t id) const
{       
    m_private->callbackMutex.lock();

    m_private->callbacksToRemove.push_back(id);

    if(!m_private->callbacksRunning)
        m_private->cleanup();

    m_private->callbackMutex.unlock();
}


void tObject::doCallbacks(tSignals signal, bool delayed, bool wait) const
{    
    signal |= m_queuedSignals;
    m_queuedSignals = 0;

    if(delayed)
    {
        queueTask([&, signal](){doCallbacks(signal);}, wait);
        return;
    }

    if(m_private->callbacksBlocked) return;

    // this signal indicates a decaying object.
    if(signal == sigDecaying)
    {
        m_private->decaying = true;

        // resolve all dependencies by removing safe callbacks
        for(const tObject* o : m_private->dependentObjects)
        {
            o->removeCallback(this);
            o->removeSignalForwarding(this);
            o->m_private->removeDependentObject(this);
        }

        m_private->dependentObjects.clear();
    }

    m_private->callbackMutex.lock(); ON_LEAVE(m_private->callbackMutex.unlock();)

    // prevent doing callbacks twice
    if(m_private->callbacksRunning) return;
    else m_private->callbacksRunning = true;

    for(const CallbackEntry& entry : m_private->registeredCallbacks)
    {
        if((entry.signal & sigAND && entry.signal == (signal & entry.signal)) || entry.signal & signal)
            entry.callback();
    }

    if(signal != sigDecaying)
        for(const tObject* o : m_private->forwardCallbacksTo)
            o->doCallbacks(signal);

    // maybe we got cleanup tasks while processing the callbacks
    m_private->cleanup();

    m_private->callbacksRunning = false;
}

void tObject::queueCallback(tSignals signal) const
{
    m_queuedSignals |= signal;
}


void tObject::blockCallbacks(bool block)
{
    m_private->callbacksBlocked += block ? 1 : -1;
    if(m_private->callbacksBlocked < 0) m_private->callbacksBlocked = 0;
}


void tObject::flushCallbacks()
{
    // recursive check to also get incoming tasks waiting for unlock
    while(!Private::tasksPending.empty())
    {
        tObject::Private::taskMutex.lock();

        for(const tObject::Task& task : tObject::Private::tasksPending)
            task();

        tObject::Private::tasksPending.clear();

        tObject::Private::taskMutex.unlock();

        std::this_thread::sleep_for(std::chrono::microseconds(10));
        //usleep(10);
    }
}

uint tObject::getPendingCallbackCount()
{
    return tObject::Private::tasksPending.size();
}

void tObject::exportAsVariable()
{
    if(!m_private->exported)
    {
        m_variables[getName()] = this;
        m_private->exported = true;

        tTerm::findVariables();
    }
}

tObject* tObject::getVariable(const QString& name)
{
    return m_variables.value(name, nullptr);
}

QString tObject::valueAsText() const
{
    return QString();
}


void tObject::heartbeat()
{
    flushCallbacks();
}


vector<tObject::Task> tObject::Private::tasksPending;
QMutex tObject::Private::taskMutex;
/*thread tObject::Private::taskThread = thread([&]()
{
    while(!m_objectIndex.empty())
    {
        taskMutex.lock();

        for(const tObject::Task& task : tasksPending)
            task();

        taskMutex.unlock();

        usleep(1000);
    }
});*/


void tObject::Private::cleanup()
{
    if(!cleaningUp) cleaningUp = true; else return;

    //int i0 = tObjectDetails::count(registeredCallbacks);

    for(size_t id : callbacksToRemove)
        registeredCallbacks.remove_if([=](const CallbackEntry& cb)
            {return cb.id == id;});

    callbacksToRemove.clear();

    //std::cout << "Removed " << i0 - tObjectDetails::count(registeredCallbacks) << "CB. Now have " << tObjectDetails::count(registeredCallbacks) << "\n" << std::flush;

    for(const tObject* fw : forwardsToRemove)
        forwardCallbacksTo.remove(fw);

    forwardsToRemove.clear();

    cleaningUp = false;
}

void tObject::Private::addDependentObject(const tObject* o)
{
    dependentObjectsMutex.lock();
    if(!isInContainer(dependentObjects, o))
        dependentObjects.push_back(o);
    dependentObjectsMutex.unlock();
}

void tObject::Private::removeDependentObject(const tObject* o)
{
    dependentObjectsMutex.lock();
    removeFromVector(dependentObjects, o);
    dependentObjectsMutex.unlock();
}

/*void tObject::decaying()
{
    doCallbacks(sigDecaying);
}*/
