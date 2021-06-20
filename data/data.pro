include( ../review.pri)

SOURCES += tData.cpp \
    tData.inl \
    tDataReference.inl \
    tMatrixData.cpp \
    tMatrixData.inl \
    tDiscreteSelection.cpp \
    tDiscreteSelection.inl \
    tMatrixDataSelection.cpp \
    tMatrixDataSelection.inl \
    tDataSelectionPolicy.inl

HEADERS  += tData.hpp \
    tDataReference.hpp \
    tMatrixData.hpp \
    tDiscreteSelection.hpp \
    tMatrixDataSelection.hpp \
    tDataSelectionPolicy.hpp

unix: INCLUDEPATH += ../../boost
