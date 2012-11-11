PUBLIC_HEADERS += \
                    ../../include/public/QtLocationSubset/qgeosearchmanager.h \
                    ../../include/public/QtLocationSubset/qgeosearchmanagerengine.h \
                    ../../include/public/QtLocationSubset/qgeosearchreply.h \
                    ../../include/public/QtLocationSubset/qgeoserviceprovider.h \
                    ../../include/public/QtLocationSubset/qgeoserviceproviderfactory.h

PRIVATE_HEADERS += \
                    maps/qgeosearchmanager_p.h \
                    maps/qgeosearchmanagerengine_p.h \
                    maps/qgeosearchreply_p.h \
                    maps/qgeoserviceprovider_p.h

SOURCES += \
            maps/qgeosearchmanager.cpp \
            maps/qgeosearchmanagerengine.cpp \
            maps/qgeosearchreply.cpp \
            maps/qgeoserviceprovider.cpp \
            maps/qgeoserviceproviderfactory.cpp
