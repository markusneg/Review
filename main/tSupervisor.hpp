#pragma once

#include <functional>

#include <QObject>

//#include "../common/tObject.hpp"

class Review;
class tWindow;


// the only singleton in this program which does some supervising tasks
class tSupervisor : public QObject/*, public tObject*/
{

    Q_OBJECT

public:

    typedef std::function<void(tWindow*)> WindowCallback;

    //T_TYPEDEFS(tSupervisor)

    tSupervisor();
    ~tSupervisor();

    void getWindowFromUser(WindowCallback callback);

    tWindow* letUserSelectWindow();

protected:



private:

    friend class Review;
    Review* m_ReviewInstance = nullptr;

    struct Private;
    Private *m_private;


};


namespace tGlobal
{
    extern tSupervisor Supervisor;
}



