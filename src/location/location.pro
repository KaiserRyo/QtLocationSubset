include(../../common.pri)

TEMPLATE = lib
test {
    CONFIG += static
} else {
    CONFIG += shared
}

# Library name is QtLocationSubset (or QtLocationSubset-d for debug)
TARGET =    QtLocationSubset$${BIN_SUFFIX}
VERSION =   1.0.0

# QT = core gui network sql
QT = core

# For 'gcov' build target, depend on gcov library
gcov {
    LIBS += -lgcov
}

DEFINES += QT_BUILD_LOCATION_LIB QT_MAKEDLL

# Force a clean to delete object_script files (if present)
QMAKE_CLEAN += object_script.*

DEPENDPATH += .

INCLUDEPATH += \
                .

include(maps/maps.pri)

PUBLIC_HEADERS += \
                    ../../include/public/QtLocationSubset/qgeoaddress.h \
                    ../../include/public/QtLocationSubset/qgeoboundingarea.h \
                    ../../include/public/QtLocationSubset/qgeoboundingbox.h \
                    ../../include/public/QtLocationSubset/qgeoboundingcircle.h \
                    ../../include/public/QtLocationSubset/qgeocoordinate.h \
                    ../../include/public/QtLocationSubset/qgeoplace.h \
                    ../../include/public/QtLocationSubset/qgeopositioninfo.h \
                    ../../include/public/QtLocationSubset/qgeopositioninfosource.h \
                    ../../include/public/QtLocationSubset/qgeosatelliteinfo.h \
                    ../../include/public/QtLocationSubset/qgeosatelliteinfosource.h \
                    ../../include/public/QtLocationSubset/qnmeapositioninfosource.h \
                    ../../include/public/QtLocationSubset/qgeopositioninfosourcefactory.h

PRIVATE_HEADERS += \
                    qgeoaddress_p.h \
                    qgeoboundingbox_p.h \
                    qgeoboundingcircle_p.h \
                    qgeoplace_p.h \
                    qlocationutils_p.h \
                    qnmeapositioninfosource_p.h \
                    qgeocoordinate_p.h


HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS

SOURCES += \
            qgeoaddress.cpp \
            qgeoboundingarea.cpp \
            qgeoboundingbox.cpp \
            qgeoboundingcircle.cpp \
            qgeocoordinate.cpp \
            qgeoplace.cpp \
            qgeopositioninfo.cpp \
            qgeopositioninfosource.cpp \
            qgeosatelliteinfo.cpp \
            qgeosatelliteinfosource.cpp \
            qlocationutils.cpp \
            qnmeapositioninfosource.cpp \
            qgeopositioninfosourcefactory.cpp
