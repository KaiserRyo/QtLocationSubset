/**
 * @copyright
 * Copyright Research In Motion Limited, 2012-2012
 * Research In Motion Limited. All rights reserved.
 */


#ifndef BB_QTPLUGINS_GEOSERVICES_GEOSEARCHMANAGERENGINEBB_HPP
#define BB_QTPLUGINS_GEOSERVICES_GEOSEARCHMANAGERENGINEBB_HPP

#include <QGeoSearchManagerEngine>

#include <QObject>
#include <QScopedPointer>
#include <QList>

// The following using statement is necessary so the SIGNAL()/SLOT() macros can have matching signatures.
// This avoids a namespace mismatch that throws off connect().
using ::QtMobilitySubset::QGeoSearchReply;

namespace bb
{
namespace qtplugins
{
namespace geoservices
{

class GeoSearchManagerEngineBb : public QtMobilitySubset::QGeoSearchManagerEngine
{
    Q_OBJECT
public:
    GeoSearchManagerEngineBb(const QMap<QString, QVariant> &parameters, QObject *parent = 0);
    virtual ~GeoSearchManagerEngineBb();

    virtual QtMobilitySubset::QGeoSearchReply* geocode(const QtMobilitySubset::QGeoAddress &address,
            QtMobilitySubset::QGeoBoundingArea *bounds);
    virtual QtMobilitySubset::QGeoSearchReply* reverseGeocode(const QtMobilitySubset::QGeoCoordinate &coordinate,
            QtMobilitySubset::QGeoBoundingArea *bounds);

    void    connectReplySignals( const QtMobilitySubset::QGeoSearchReply & reply );

public Q_SLOTS:
    void replyFinishedSignalEmitted();
    void replyErrorSignalEmitted( QGeoSearchReply::Error error, const QString & errorString );

private:
    Q_DISABLE_COPY(GeoSearchManagerEngineBb)
};

}
}
}

#endif
