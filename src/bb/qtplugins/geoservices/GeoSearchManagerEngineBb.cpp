/**
 * @copyright
 * Copyright Research In Motion Limited, 2012-2012
 * Research In Motion Limited. All rights reserved.
 */


#include "GeoSearchManagerEngineBb.hpp"
#include "GeoSearchReplyBb.hpp"

#include <QObject>
#include <QMap>
#include <QVariant>
#include <QtDebug>

#include <QGeoAddress>
#include <QGeoCoordinate>

namespace
{

    QMap<QString, geo_search_boundary_t> createStringToBoundaryMap()
    {
        // ref: http://wikis.rim.net/display/lbsplaybooklib/Georeg+Interface
        QMap<QString, geo_search_boundary_t> map;

        // These are the boundaries from the georeg interface that correspond to fields in QGeoAddress (in QGeoPlace)
        map.insert("address", GEO_SEARCH_BOUNDARY_ADDRESS);
        map.insert("postal", GEO_SEARCH_BOUNDARY_POSTAL);
        map.insert("city", GEO_SEARCH_BOUNDARY_CITY);
        map.insert("province", GEO_SEARCH_BOUNDARY_PROVINCE);
        map.insert("country", GEO_SEARCH_BOUNDARY_COUNTRY);

        return map;
    }

    // maps strings that specify a boundary in the georeg interface to the corresponding geo_search boundary enum
    const QMap<QString, geo_search_boundary_t> stringToBoundaryMap = createStringToBoundaryMap();

} // namespace

namespace bb
{
namespace qtplugins
{
namespace geoservices
{

/*!
    Constructs a new engine with the specified \a parent, using \a parameters
    to pass any implementation specific data to the engine.
*/
GeoSearchManagerEngineBb::GeoSearchManagerEngineBb(const QMap<QString, QVariant> &parameters, QObject *parent)
    : QGeoSearchManagerEngine(parameters,parent)
{
    setSupportedSearchTypes(QtMobilitySubset::QGeoSearchManager::SearchNone);
    setSupportsGeocoding( true );
    setSupportsReverseGeocoding( true );
}

/*!
    Destroys this engine.
*/
GeoSearchManagerEngineBb::~GeoSearchManagerEngineBb()
{
}

/*!
    Begins the geocoding of \a address. Geocoding is the process of finding a
    coordinate that corresponds to a given address.

    A QGeoSearchReply object will be returned, which can be used to manage the
    geocoding operation and to return the results of the operation.

    This engine and the returned QGeoSearchReply object will emit signals
    indicating if the operation completes or if errors occur.

    If supportsGeocoding() returns false an
    QGeoSearchReply::UnsupportedOptionError will occur.

    Once the operation has completed, QGeoSearchReply::places() can be used to
    retrieve the results, which will consist of a list of QGeoPlace objects.
    These objects represent a combination of coordinate and address data.

    The address data returned in the results may be different from \a address.
    This will usually occur if the geocoding service backend uses a different
    canonical form of addresses or if \a address was only partially filled out.

    If \a bounds is non-null and a valid QGeoBoundingArea it will be used to
    limit the results to those that are contained by \a bounds. This is
    particularly useful if \a address is only partially filled out, as the
    service will attempt to geocode all matches for the specified data.

    The user is responsible for deleting the returned reply object, although
    this can be done in the slot connected to GeoSearchManagerEngineBb::finished(),
    GeoSearchManagerEngineBb::error(), QGeoSearchReply::finished() or
    QGeoSearchReply::error() with deleteLater().
*/
QtMobilitySubset::QGeoSearchReply* GeoSearchManagerEngineBb::geocode(const QtMobilitySubset::QGeoAddress &address,
        QtMobilitySubset::QGeoBoundingArea *bounds)
{
    QtMobilitySubset::QGeoSearchReply * reply = new GeoSearchReplyBb( address, bounds, this);
    connectReplySignals( *reply );
    return reply;
}

/*!
    Begins the reverse geocoding of \a coordinate. Reverse geocoding is the
    process of finding an address that corresponds to a given coordinate.

    A QGeoSearchReply object will be returned, which can be used to manage the
    reverse geocoding operation and to return the results of the operation.

    This engine and the returned QGeoSearchReply object will emit signals
    indicating if the operation completes or if errors occur.

    If supportsReverseGeocoding() returns false an
    QGeoSearchReply::UnsupportedOptionError will occur.

    Once the operation has completed QGeoSearchReply::places() can be used to retrieve the
    results, which will consist of a list of QGeoPlace objects. These objects
    represent a combination of coordinate and address data.

    The coordinate data returned in the results may be different from \a
    coordinate. This will usually occur if the reverse geocoding service
    backend shifts the coordinates to be closer to the matching addresses, or
    if the backend returns results at multiple levels of detail.

    If multiple results are returned by the reverse geocoding service backend
    they will be provided in order of specificity. This normally occurs if the
    backend is configured to reverse geocode across multiple levels of detail.
    As an example, some services will return address and coordinate pairs for
    the street address, the city, the state and the country.

    If \a bounds is non-null and a valid QGeoBoundingArea it will be used to
    limit the results to those that are contained by \a bounds.

    The user is responsible for deleting the returned reply object, although
    this can be done in the slot connected to GeoSearchManagerEngineBb::finished(),
    GeoSearchManagerEngineBb::error(), QGeoSearchReply::finished() or
    QGeoSearchReply::error() with deleteLater().
*/
QtMobilitySubset::QGeoSearchReply* GeoSearchManagerEngineBb::reverseGeocode(const QtMobilitySubset::QGeoCoordinate &coordinate,
        QtMobilitySubset::QGeoBoundingArea *bounds)
{
    // To support the boundary specification that can be handled by the BB10 backend server, check the parent QGeoSearchManager
    // to see if a (dynamic) "boundary" property has been set. If so, and it is valid, use it to perform the reverse geocoding.
    // Otherwise the default is GEO_SEARCH_BOUNDARY_ADDRESS
    geo_search_boundary_t boundaryType = GEO_SEARCH_BOUNDARY_ADDRESS;
    QtMobilitySubset::QGeoSearchManager * searchManager = qobject_cast<QtMobilitySubset::QGeoSearchManager *>(parent());
    if ( searchManager ) {
        QVariant variant = searchManager->property( "boundary" );
        if ( variant.isValid() && variant.type() == QVariant::String ) {
            boundaryType = stringToBoundaryMap.value( variant.toString(), boundaryType );
        }
    }

    QtMobilitySubset::QGeoSearchReply * reply = new GeoSearchReplyBb( coordinate, boundaryType, bounds, this);
    connectReplySignals( *reply );
    return reply;
}

// A QGeoSearchReply instance has emitted its finished() signal, emit this GeoSearchManagerEngineBb instance's
// finished(const QGeoSearchReply &) signal.
void GeoSearchManagerEngineBb::replyFinishedSignalEmitted()
{
    QtMobilitySubset::QGeoSearchReply * reply = qobject_cast<QtMobilitySubset::QGeoSearchReply*>(sender());
    if ( !reply ) {
        return;
    }

    Q_EMIT finished( reply );
}

// A QGeoSearchReply instance has emitted its error( QGeoSearchReply::Error, QString ) signal, emit
// this GeoSearchManagerEngineBb instance's finished(const QGeoSearchReply &, QGeoSearchReply::Error, QString ) signal.
void GeoSearchManagerEngineBb::replyErrorSignalEmitted( QGeoSearchReply::Error err, const QString & errorString )
{
    QtMobilitySubset::QGeoSearchReply * reply = qobject_cast<QtMobilitySubset::QGeoSearchReply*>(sender());
    if ( !reply ) {
        return;
    }

    Q_EMIT error( reply, err, errorString );
}

// The Qt Mobility Location API docs indicate that the QGeoSearchReply signal finished() and the QGeoSearchManager signal
// finished(const QGeoSearchReply &) are both emitted when the search has completed. Same applies to the error() signals.
// However emitting finished(const QGeoSearchReply &) appears to be left to the implementer of the QGeoSearchManagerEngine
// subclass. This method connects a QGeoSearchReply instance's signals to the above slots which in turn emit the
// QGeoSearchManagerEngine subclass' signals.
void GeoSearchManagerEngineBb::connectReplySignals( const QtMobilitySubset::QGeoSearchReply & reply )
{
    bool connected = connect(&reply, SIGNAL(finished()), this, SLOT(replyFinishedSignalEmitted()));
    if ( !connected ) {
        qWarning() << "GeoSearchManagerEngineBb::connectReplySignals(): error connecting";
    }
    connected = connect(&reply, SIGNAL(error(QGeoSearchReply::Error, const QString &)), this, SLOT(replyErrorSignalEmitted(QGeoSearchReply::Error, const QString &)));
    if ( !connected ) {
        qWarning() << "GeoSearchManagerEngineBb::connectReplySignals(): error connecting";
    }
}

} // namespace
} // namespace
} // namespace
