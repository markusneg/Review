include( ../review.pri)

SOURCES += tBaselineFilter.cpp \
    tDerivationFilter.cpp \
    tFilter.cpp \
    tObserver.cpp \
    tPCAFilter.cpp \
    tRotationFilter.cpp \
    tVarianceFilter.cpp \
    tArithmeticFilter.cpp \
    tArithmeticFilter.inl \
    tUnaryFilter.cpp \
    tUnaryFilter.inl \
    tBinaryFilter.inl \
    tBinaryFilter.cpp \
    tFourierFilter.cpp \
    tBandObserver.cpp \
    tFilter.inl \
    tBandCombination.cpp \
    tLeastSquaresFit.cpp

HEADERS  += tBaselineFilter.hpp \
    tDerivationFilter.hpp \
    tFilter.hpp \
    tObserver.hpp \
    tPCAFilter.hpp \
    tRotationFilter.hpp \
    tVarianceFilter.hpp \
    tArithmeticFilter.hpp \
    tUnaryFilter.hpp \
    tBinaryFilter.hpp \
    tFourierFilter.hpp \
    helpers.hpp \
    tBandObserver.hpp \
    tBandCombination.hpp \
    tLeastSquaresFit.hpp

win32 {
    LIBS += C:\msys64\home\Markus\dev\armadillo-7.800.2\libarmadillo.a
    LIBS += C:\msys64\home\Markus\dev\armadillo-7.800.2\examples\lib_win64\blas_win64_MT.lib
    LIBS += C:\msys64\home\Markus\dev\armadillo-7.800.2\examples\lib_win64\lapack_win64_MT.lib
}

unix: LIBS += -larmadillo -llapack -lblas

