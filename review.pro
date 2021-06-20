#-------------------------------------------------
#
# Project created by QtCreator 2015-04-10T14:48:35
#
#-------------------------------------------------

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = subdirs
SUBDIRS = main common util data view filter

OTHER_FILES += \
    etc/log.txt \
    etc/todo.txt \
    etc/bugs.txt \
    etc/scientific.txt \
    review.pri

main.depends = common data view filter
common.depends = util
data.depends = common util
view.depends = common util data
filter.depends = common util data

# build must be last:
#CONFIG += ordered
#SUBDIRS += build


