QT += core gui printsupport

win32-g++: DESTDIR = $$OUT_PWD

#Includes common configuration for all subdirectory .pro files.
win32: INCLUDEPATH += C:\lib\armadillo\include C:\lib\boost C:\lib

WARNINGS += -Wall

TEMPLATE = lib
CONFIG += staticlib c++14

QMAKE_CXXFLAGS += -std=c++1y

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3


# The following keeps the generated files at least somewhat separate
# from the source files.
#UI_DIR = uics
#MOC_DIR = mocs
#OBJECTS_DIR = objs
