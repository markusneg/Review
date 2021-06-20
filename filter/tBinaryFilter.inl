#include "tBinaryFilter.hpp"

// little helper for simple operators
#define FUNCTION(FN) \
    template <typename V, typename W> \
    inline auto fn(const V& lhs, const W& rhs) \
    { \
        FN \
    } \

#define NAME(NM) static QString name() {return QObject::tr(NM);}


namespace BinaryOperation
{
    struct Bind
    {
        NAME("Bind")
        FUNCTION(return arma::join_horiz(lhs, rhs);)

        tSetting<bool> joinCols = false;
    };

}


#undef FUNCTION
#undef NAME
