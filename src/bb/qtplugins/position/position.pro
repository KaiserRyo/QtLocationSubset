include(../../../../common.pri)

TEMPLATE = lib
CONFIG += plugin
contains(DEFINES, BB_TEST_BUILD) {
    CONFIG += static
} else {
    CONFIG += shared
}

# Library name is bbposition (or bbposition-d for debug)
TARGET =    bbposition$${BIN_SUFFIX}
VERSION =   1.0.0
DESTDIR =   $${QTPLUGIN_DESTDIR}/position_subset

# QT = core gui network sql
QT = core

# Non-test build also depends on libbb (for PpsObject), libwmm (mag declination),
# libQtLocationSubset (position base classes)
!test {
    LIBS = -lbb -lwmm -lQtLocationSubset$${BIN_SUFFIX}
}

# For 'gcov' build target, depend on gcov library
gcov {
    LIBS += -lgcov
}

# Force a clean to delete object_script files (if present)
QMAKE_CLEAN += object_script.*

DEPENDPATH += .

INCLUDEPATH += \
               .


# even though these are not public headers use HEADERS instead of PRIVATE_HEADERS to 
# prevent unresolved symbol errors when the plugin is dynamically loaded
HEADERS += \
           GeoPositionInfoSourceBb.hpp \
           GeoSatelliteInfoSourceBb.hpp \
           LocationManagerUtil.hpp \
           GeoPositionInfoSourceFactoryBb.hpp \
           GeoPositionInfoSourceBbPrivate.hpp \
           GeoSatelliteInfoSourceBbPrivate.hpp \

SOURCES += \
           GeoPositionInfoSourceBb.cpp \
           GeoSatelliteInfoSourceBb.cpp \
           LocationManagerUtil.cpp \
           GeoPositionInfoSourceFactoryBb.cpp \

