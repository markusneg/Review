#include "tArithmeticFilter.hpp"

// little helper for simple operators. Here, row- or columnwise operations are defined
#define OPERATION(OP) \
    template <typename V, typename W> \
    inline static auto op(const V& lhs, const W& rhs) /*-> decltype(lhs OP rhs)*/ \
    { \
        if(rhs.n_elem == 1) return arma::mat(lhs OP rhs(0,0));\
        else \
        { \
            arma::mat ret(lhs.n_rows, lhs.n_cols); \
            if(rhs.n_rows == lhs.n_rows && rhs.n_cols == 1) {\
                for(uint c = 0; c < lhs.n_cols; ++c) \
                    ret.col(c) = lhs.col(c) OP rhs; return ret; } \
            else if(rhs.n_cols == lhs.n_cols && rhs.n_rows == 1) { \
                for(uint r = 0; r < lhs.n_rows; ++r) \
                    ret.row(r) = lhs.row(r) OP rhs; return ret; } \
        } \
        return arma::mat(lhs OP rhs); \
    } \

#define NAME(NM) static QString name() {return QObject::tr(NM);}


namespace ArithmeticOperation
{
    struct Plus
    {
        NAME("Addition")
        OPERATION(+)
    };

    struct Minus
    {
        NAME("Subtraction")
        OPERATION(-)
    };

    struct Multiply
    {
        NAME("Multiplication")
        //OPERATION(*)

        template <typename V, typename W>
        inline static auto op(const V& lhs, const W& rhs)
        {
            try
            {
                if(lhs.n_rows == rhs.n_rows && rhs.n_cols == 1)
                {
                    //T_COUT("Arithm.-Mult.: doing it element-wise..");
                    auto ret = lhs;
                    for(uint c = 0; c < lhs.n_cols; ++c)
                        ret.col(c) = ret.col(c) % rhs;

                    return ret;
                }

                //if(lhs.n_rows == rhs.n_rows && lhs.n_cols == rhs.n_cols)
                else return arma::mat(lhs * rhs);

            }
            catch(...)
            {
                //return lhs.each_col() % rhs; // why is it not working?

                return arma::mat();
            }

        }

    };

    struct Divide
    {
        NAME("Division")
        OPERATION(/)
    };
}


#undef OPERATION
#undef NAME
