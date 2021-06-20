include( ../review.pri )

TEMPLATE = app
TARGET = ../Review

SOURCES += main.cpp \
    Review.cpp \
    tSupervisor.cpp

HEADERS  += Review.hpp \
    tSupervisor.hpp

FORMS += Review.ui

LIBS += ../filter/libfilter.a
LIBS += ../view/libview.a
LIBS += ../data/libdata.a
LIBS += ../common/libcommon.a
LIBS += ../util/libutil.a

PRE_TARGETDEPS += ../filter/libfilter.a
PRE_TARGETDEPS += ../view/libview.a
PRE_TARGETDEPS += ../data/libdata.a
PRE_TARGETDEPS += ../common/libcommon.a
PRE_TARGETDEPS += ../util/libutil.a

win32 {
    LIBS += C:\msys64\home\Markus\dev\armadillo-7.800.2\libarmadillo.a
    LIBS += C:\msys64\home\Markus\dev\armadillo-7.800.2\examples\lib_win64\blas_win64_MT.lib
    LIBS += C:\msys64\home\Markus\dev\armadillo-7.800.2\examples\lib_win64\lapack_win64_MT.lib
    LIBS += -LC:\msys64\mingw64\bin -LC:\msys64\mingw64\lib
    QMAKE_LFLAGS_RELEASE += -s
}

unix:LIBS += -larmadillo -llapack -lblas -licui18n -licuuc -licudata

DEPENDPATH += ../filter

RESOURCES += \
    main.qrc

