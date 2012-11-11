include(../../../common.pri)

TEMPLATE = lib
CONFIG += qt plugin
TARGET  = declarative_location$${BIN_SUFFIX}
QT += declarative


DEPENDPATH += .

# Where to search for the #include'd files
INCLUDEPATH += .

LIBS += -L$${DESTDIR} -lQtLocationSubset$${BIN_SUFFIX}

DESTDIR = $${QMLPLUGIN_DESTDIR}/QtMobilitySubset/location

# Force a clean to delete object_script files (if present)
QMAKE_CLEAN += object_script.*

# The header and source files of this project. Note that qmake generates dependency information
# and automatically adds necessary 'moc' files as it sees Q_OBJECT macros' in header files
HEADERS += qdeclarativeposition_p.h \
           qdeclarativepositionsource_p.h \
           qdeclarativecoordinate_p.h \
           qdeclarativegeoserviceprovider_p.h \
           qdeclarativegeoplace_p.h \
           qdeclarativegeoaddress_p.h \
           qdeclarativegeoboundingbox_p.h

SOURCES += qdeclarativeposition.cpp \
           location.cpp \
           qdeclarativepositionsource.cpp \
           qdeclarativecoordinate.cpp \
           qdeclarativegeoserviceprovider.cpp \
           qdeclarativegeoplace.cpp \
           qdeclarativegeoaddress.cpp \
           qdeclarativegeoboundingbox.cpp

# Ensure the DESTDIR folder exists.
create_destdir.commands = mkdir -p $${DESTDIR}
# Create the qmldir file.  We create it in DESTDIR because I can't figure out how to get QMAKE to put it there automatically.
# Debug build produces a file called "qmldir-d".  The debug file has to have a different name, otherwise it will overwrite
# the release file and we'll get errors that the debug plugin library can't be found.  If you need to use the
# debug file, rename it to "qmldir" manually.
qmldir.commands = echo plugin $${TARGET} > $${DESTDIR}/qmldir$${BIN_SUFFIX}
qmldir.depend = create_destdir
# Add the extra target.
QMAKE_EXTRA_TARGETS += qmldir create_destdir
# Make the qmldir target be built after the library is built.
POST_TARGETDEPS += qmldir
