#for now we do not actually have anything to build
#just ensure installation of public headers
TEMPLATE = subdirs

include(../../common.pri)
PUBLIC_HEADERS += ../../include/public/QtLocationSubset/qmobilitysubset.h \
                  ../../include/public/QtLocationSubset/qlatin1constant.h \
                  ../../include/public/QtLocationSubset/qmalgorithms.h
