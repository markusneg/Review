#include "tUnaryFilter.hpp"

#include <algorithm>

#include <QLabel>

//#include "savgol.h"

// little helpers
#define NAME(NM) static QString name() {return QObject::tr(NM);}
#define FUNCTION(FN) template <typename U> inline auto fn(const U& rhs) {FN}
#define UI(FN) std::function<void(void)> changed; void setupUI(QBoxLayout* layout) {FN}
#define SETTING(member) member.addCallback([&](){changed();}, sigChanged);


namespace UnaryOperation
{
    struct Power
    {
        NAME("Power")
        FUNCTION(return arma::pow(rhs, power);)
        UI(layout->addWidget(power.widget()); SETTING(power))

        tSetting<double> power = 2;
    };

    struct Sum
    {
        NAME("Sum")
        FUNCTION
        (
                arma::mat tmp = *abs ? arma::abs(rhs) : rhs;
                if(*accum) {arma::mat ret(1,1); ret(0,0) = arma::accu(tmp); return ret;} else return arma::mat(arma::sum(tmp));
        )
        UI(layout->addWidget(accum.widget()); layout->addWidget(abs.widget()); SETTING(accum) SETTING(abs))

        tSetting<bool> accum = true, abs = false;
    };

    struct Mean
    {
        NAME("Mean")
        FUNCTION(return arma::mean(rhs);)
        UI()
    };

    struct Variance
    {
        NAME("Variance")
        FUNCTION(return arma::var(rhs);)
        UI()
    };

    struct Median
    {
        NAME("Median")
        FUNCTION(return arma::median(rhs);)
        UI()
    };

    struct Min
    {
        NAME("Minimum")
        FUNCTION(return arma::min(rhs);)
        UI()
    };

    struct Max
    {
        NAME("Maximum")
        FUNCTION(return arma::max(rhs);)
        UI()
    };

    struct Abs
    {
        NAME("Absolute")
        FUNCTION(return arma::abs(rhs);)
        UI()
    };

    struct VectorLength
    {
        NAME("Vector Length")
        FUNCTION
        (
            arma::mat ret(rhs.n_rows, 1);

            for(uint r = 0; r < rhs.n_rows; ++r)
                ret.row(r) = arma::sqrt(arma::sum(arma::pow(rhs.row(r), 2), 1)); /// [OPT] operate rather on columns (more streamlined) (a lot of filters should do that!)

            return ret;
        )
        UI()
    };

    struct SmoothFourier
    {
        NAME("Smooth (Fourier)")

        //FUNCTION
        //(
        template <typename U>
        inline auto fn(const U& rhs)
        {            
                // info: armadillo does the fft rowwise

                int n = rhs.n_cols;
                int n_pad = padLength * n; // the pad length on each side of the matrix
                int n_total = n + 2 * n_pad;

                double freqFactor = n / 200.0 * n_total / n;

                int n0 = round(blockFrom * freqFactor);
                int n1 = round(blockTo * freqFactor);

                if(n0 < 0) n0 = 0;
                if(n1 >= n_total) n1 = n_total - 1;
                if(n1 < n0) n1 = n0;

                arma::mat X = rhs.t();

                // fft wraps negative values -> shift variables of each observation so that there are no neg. values
                arma::rowvec minValues = arma::min(X);
                X.each_row() -= minValues;

                X.insert_rows(0, n_pad, false);
                X.insert_rows(n + n_pad, n_pad, false);

                for(int i = 0; i < n_pad; ++i)
                    X.row(i) = X.row(n_pad);

                for(int i = n + n_pad; i < n_total; ++i)
                    X.row(i) = X.row(n + n_pad - 1);

                //X.save("X.dat", arma::raw_ascii);

                arma::cx_mat F = arma::fft(X);

                // block freqs
                if(!*bandpass)
                {
                    F.rows(n0, n1).zeros();
                    F.rows(n_total - n1 - 1, n_total - n0 - 1).zeros();
                }

                else
                {
                    int n_end = round(freqFactor * 100.0);

                    F.rows(0, n0-1).zeros(); F.rows(n1, n_end-1).zeros();
                    F.rows(n_total - n0 - 2, n_total - 1).zeros();
                    F.rows(n_total - n_end - 2, n_total - n1 - 1).zeros();
                }

                arma::cx_mat RF = arma::ifft(F);
                arma::mat R = arma::abs(RF.rows(n_pad, n_pad + n - 1));

                R.each_row() += minValues;

                //arma::mat R = arma::abs(F);
                return R;
        }

        UI(layout->addWidget(bandpass.widget()); layout->addWidget(blockFrom.widget()); layout->addWidget(blockTo.widget()); layout->addWidget(padLength.widget()); SETTING(bandpass) SETTING(blockFrom) SETTING(blockTo) SETTING(padLength))

        tSetting<bool> bandpass = false;
        tSetting<double> blockFrom = 2, blockTo = 100;        
        tSetting<double> padLength = 0.5;
    };

    struct Autoscale
    {
        NAME("Autoscale")

        template <typename U>
        inline auto fn(const U& rhs)
        {
            if(*center || *scale)
            {
                auto ret = arma::mat(rhs.n_rows, rhs.n_cols);

                for(uint i = 0; i < rhs.n_rows; ++i)
                {
                    double mean = 0, vlength = 0;

                    if(*center)
                    {
                        for(uint j = 0; j < rhs.n_cols; ++j)
                            mean += rhs(i,j);
                        mean /= rhs.n_cols;

                        ret.row(i) = rhs.row(i) - mean;
                    }

                    else ret.row(i) = rhs.row(i);

                    for(uint j = 0; j < rhs.n_cols; ++j)
                        vlength += pow(ret(i,j), 2);
                    vlength = sqrt(vlength);

                    if(*scale)
                        ret.row(i) /= vlength;
                }

                return ret;
            }

            else return rhs;

            /*arma::vec means = arma::mean(rhs);
            auto centered = rhs;rhs.each_col() - means;

            auto vectorLengths = arma::sqrt(arma::pow(centered).each_col().sum());

            return centered.each_col() / vectorLengths;*/
        }

        UI(layout->addWidget(new QLabel(QObject::tr("Center"))); layout->addWidget(center.widget()); layout->addWidget(new QLabel(QObject::tr("Scale"))); layout->addWidget(scale.widget()); SETTING(center) SETTING(scale))

        tSetting<bool> center = true, scale = true;
    };

    struct Classes
    {
        NAME("Classes")

        template <typename U>
        inline auto fn(const U& rhs)
        {
            arma::mat classes(rhs.n_rows, 1), occurencies(rhs.n_rows, rhs.n_cols);
            uint n_classes = 0;

            // assign a class to every row
            for(uint i = 0; i < rhs.n_rows; ++i)
            {
                // for every row, search the class list for existing classes
                uint c;
                for(c = 0; c < n_classes; ++c)
                {
                    // rhs.row(i) == occurencies.row(c)
                    bool equal = true;
                    for(uint j = 0; j < rhs.n_cols; ++j)
                        equal &= rhs(i,j) == occurencies(c,j);

                    if(equal) break;
                }

                if(c == n_classes) // not found. create new class
                {
                    occurencies.row(c) = rhs.row(i);
                    n_classes++;
                }

                classes(i,0) = c + 1;
            }

            return classes;
        }

        UI()


    };

    struct Sort
    {
        NAME("Sort")

        template <typename U>
        inline auto fn(const U& rhs)
        {
            return arma::sort(rhs, *descending ? "descend" : "ascend", 0);
        }

        UI(layout->addWidget(new QLabel(QObject::tr("Descending"))); layout->addWidget(descending.widget()); SETTING(descending))

        tSetting<bool> descending = false;
    };

    struct Transpose
    {
        NAME("Transpose")
        FUNCTION(return rhs.t();)
        UI()
    };
}


#undef NAME
#undef FUNCTION
#undef UI
#undef SETTING
