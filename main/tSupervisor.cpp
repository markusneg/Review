#include "tSupervisor.hpp"

#include <queue>

#include "Review.hpp"

//using namespace tGlobal;

tSupervisor tGlobal::Supervisor;


struct tSupervisor::Private
{
    std::queue<tSupervisor::WindowCallback> m_windowEvents;
};

tSupervisor::tSupervisor() :
    m_private(new Private)
{

}

tSupervisor::~tSupervisor()
{
    delete m_private;
}

tWindow* tSupervisor::letUserSelectWindow()
{
    if(m_ReviewInstance)
        return m_ReviewInstance->letUserSelectWindow();
    else
        return nullptr;
}
