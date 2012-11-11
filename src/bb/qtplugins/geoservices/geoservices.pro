include(../../../../common.pri)

TEMPLATE = lib
CONFIG += plugin
contains(DEFINES, BB_TEST_BUILD) {
    CONFIG += static
} else {
    CONFIG += shared
}

# Library name is bbgeosearch (or bbgeosearch-d for debug)
TARGET =    bbgeosearch$${BIN_SUFFIX}
VERSION =   1.0.0
DESTDIR =   $${QTPLUGIN_DESTDIR}/geoservices_subset

# QT = core gui network sql
QT = core

# libgeo_search (for geocoding/reverse geocoding), 
# libQtLocationSubset (geoservices base classes)
!test {
    LIBS = -lgeo_search -lQtLocationSubset$${BIN_SUFFIX}
}

# Force a clean to delete object_script files (if present)
QMAKE_CLEAN += object_script.*

DEPENDPATH += .

INCLUDEPATH += \
               .

# even though these are not public headers use HEADERS instead of PRIVATE_HEADERS to 
# prevent unresolved symbol errors when the plugin is dynamically loaded
HEADERS += \
           GeoSearchManagerEngineBb.hpp \
           GeoSearchReplyBb.hpp \
           GeoServiceProviderFactoryBb.hpp \
           ../../../../include/private/bbmock/GeoregApi.hpp \
           ../../../bbmock/GeoregApiImpl.hpp \
           

SOURCES += \
           GeoSearchManagerEngineBb.cpp \
           GeoSearchReplyBb.cpp \
           GeoServiceProviderFactoryBb.cpp \
           ../../../bbmock/GeoregApi.cpp \
           ../../../bbmock/GeoregApiImpl.cpp \
