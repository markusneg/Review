include( ../review.pri)

SOURCES += qcustomplot.cpp \
    tPlotView.cpp \
    tQCustomPlot.cpp \
    tView.cpp \
    tImageView.cpp

HEADERS  += qcustomplot.h \
    tPlotView.hpp \
    tQCustomPlot.hpp \
    tView.hpp \
    tImageView.hpp

LIBS += ../data/libdata.a
