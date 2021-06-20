#include "Review.hpp"
#include <QApplication>
#include <QStyleFactory>


//#define BOOST_ICL_USE_STATIC_BOUNDED_INTERVALS
//#define BOOST_ICL_DISCRETE_STATIC_INTERVAL_DEFAULT closed_interval
//#include <boost/icl/interval_map.hpp>

//using namespace boost::icl;
using namespace std;


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setStyle(QStyleFactory::create("fusion"));

    std::cout << a.style()->objectName().toStdString() << std::endl << std::flush;
    //std::cout << a.style() << std::endl << std::flush;

    Review w;
    w.show();

    return a.exec();
}
