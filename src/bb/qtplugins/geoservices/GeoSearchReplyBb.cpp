/**
 * @copyright
 * Copyright Research In Motion Limited, 2012-2012
 * Research In Motion Limited. All rights reserved.
 */

#include "GeoSearchReplyBb.hpp"

#include <QList>
#include <QtDebug>
#include <QtConcurrentRun>

using bb::qtplugins::geoservices::GeoregReply;

namespace
{

QMap<QtMobilitySubset::QGeoSearchReply::Error, QString> createGeoSearchReplyErrorStringMap()
{
    QMap<QtMobilitySubset::QGeoSearchReply::Error, QString> map;

    map.insert( QtMobilitySubset::QGeoSearchReply::NoError, QString("Geocoding service completed successfully.") );
    map.insert( QtMobilitySubset::QGeoSearchReply::EngineNotSetError, QString("The search manager that was used did not have a QGeoSearchManagerEngine instance associated with it.") );
    map.insert( QtMobilitySubset::QGeoSearchReply::CommunicationError, QString("An error occurred while communicating with the service provider.") );
    map.insert( QtMobilitySubset::QGeoSearchReply::ParseError, QString("The input to or response from the service provider was in an unrecognizable format.") );
    map.insert( QtMobilitySubset::QGeoSearchReply::UnsupportedOptionError, QString("The requested operation or one of the options for the operation are not supported by the service provider.") );
    map.insert( QtMobilitySubset::QGeoSearchReply::CombinationError, QString("An error occurred while results where being combined from multiple sources.") );
    map.insert( QtMobilitySubset::QGeoSearchReply::UnknownError, QString("An unknown error has occurred.") );

    return map;
}

// maps QtMobilitySubset::QGeoSearchReply::Error types (key) to their corresponding QString (value)
const QMap<QtMobilitySubset::QGeoSearchReply::Error, QString> geoSearchReplyErrorStringMap = createGeoSearchReplyErrorStringMap();

QMap<geo_search_error_t, QtMobilitySubset::QGeoSearchReply::Error> createGeoSearchReplyErrorMap()
{
    // ref: http://wikis.rim.net/display/lbsplaybooklib/Georeg+Interface
    QMap<geo_search_error_t, QtMobilitySubset::QGeoSearchReply::Error> map;

    map.insert(GEO_SEARCH_OK, QtMobilitySubset::QGeoSearchReply::NoError);
    map.insert(GEO_SEARCH_ERROR_INPUT, QtMobilitySubset::QGeoSearchReply::UnsupportedOptionError);
    map.insert(GEO_SEARCH_ERROR_SERVER_OPEN, QtMobilitySubset::QGeoSearchReply::CommunicationError);
    map.insert(GEO_SEARCH_ERROR_SERVER_INVALID_REQUEST, QtMobilitySubset::QGeoSearchReply::UnsupportedOptionError);
    map.insert(GEO_SEARCH_ERROR_SERVER_RESPONSE, QtMobilitySubset::QGeoSearchReply::ParseError);
    map.insert(GEO_SEARCH_ERROR_SERVER_EMPTY, QtMobilitySubset::QGeoSearchReply::UnknownError);
    map.insert(GEO_SEARCH_ERROR_REPLY, QtMobilitySubset::QGeoSearchReply::ParseError);

    return map;
}

// maps error codes from the georeg service (key) to the QtMobilitySubset::QGeoSearchReply::Error type (value).
const QMap<geo_search_error_t, QtMobilitySubset::QGeoSearchReply::Error> geoSearchReplyErrorMap = createGeoSearchReplyErrorMap();


// take the centre of the bounds as an indication of where to search near. Useful for geocoding.
QtMobilitySubset::QGeoCoordinate coordinateHint( const QtMobilitySubset::QGeoBoundingArea * bounds )
{
    QtMobilitySubset::QGeoCoordinate centre;

    // optional hint
    if ( bounds && bounds->isValid() ) {
        // use the bounds centre as a hint where to look
        if ( bounds->type() == QtMobilitySubset::QGeoBoundingArea::BoxType ) {
            centre = ( static_cast<const QtMobilitySubset::QGeoBoundingBox*>(bounds))->center();
        } else if ( bounds->type() == QtMobilitySubset::QGeoBoundingArea::CircleType ) {
            centre = ( static_cast<const QtMobilitySubset::QGeoBoundingCircle*>(bounds))->center();
        }
    }

    return centre;
}

// retrieves the list of places found by the georeg service and puts them in a QList<QtMobilitySubset::QGeoPlace>.
geo_search_error_t populatePlaces( bbmock::GeoregApi & georegApi, QList<QtMobilitySubset::QGeoPlace> * places, geo_search_reply_t reply )
{
    geo_search_error_t err = GEO_SEARCH_OK;
    int numPlaces;

    err = georegApi.geo_search_reply_get_length( &reply, &numPlaces );
    if ( err != GEO_SEARCH_OK ) {
        return err;
    }

    for ( int i = 0 ; i < numPlaces ; i++ )
    {
        const char * string;
        double number;

        // expect that the index can be set between 0 and numPlaces - 1. There's an error otherwise
        err = georegApi.geo_search_reply_set_index( &reply, i );
        if ( err != GEO_SEARCH_OK ) {
            return err;
        }

        QtMobilitySubset::QGeoCoordinate coordinate;
        err = georegApi.geo_search_reply_get_lat( &reply, &number );
        if ( err == GEO_SEARCH_OK ) {
            coordinate.setLatitude( number );
        }

        err = georegApi.geo_search_reply_get_lon( &reply, &number );
        if ( err == GEO_SEARCH_OK ) {
            coordinate.setLongitude( number );
        }

        QtMobilitySubset::QGeoAddress address;
        err = georegApi.geo_search_reply_get_name( &reply, &string );
        if ( err == GEO_SEARCH_OK ) {
            address.setText( QString::fromUtf8(string) );
        }

        err = georegApi.geo_search_reply_get_street( &reply, &string );
        if ( err == GEO_SEARCH_OK ) {
            address.setStreet( QString::fromUtf8(string) );
        }

        err = georegApi.geo_search_reply_get_district( &reply, &string );
        if ( err == GEO_SEARCH_OK ) {
            address.setDistrict( QString::fromUtf8(string) );
        }

        err = georegApi.geo_search_reply_get_city( &reply, &string );
        if ( err == GEO_SEARCH_OK ) {
            address.setCity( QString::fromUtf8(string) );
        }

        err = georegApi.geo_search_reply_get_county( &reply, &string );
        if ( err == GEO_SEARCH_OK ) {
            address.setCounty( QString::fromUtf8(string) );
        }

        err = georegApi.geo_search_reply_get_region( &reply, &string );
        if ( err == GEO_SEARCH_OK ) {
            address.setState( QString::fromUtf8(string) );
        }

        err = georegApi.geo_search_reply_get_country( &reply, &string );
        if ( err == GEO_SEARCH_OK ) {
            address.setCountry( QString::fromUtf8(string) );
        }

        err = georegApi.geo_search_reply_get_postal_code( &reply, &string );
        if ( err == GEO_SEARCH_OK ) {
            address.setPostcode( QString::fromUtf8(string) );
        }

        err = georegApi.geo_search_reply_get_iso_alpha3_country_code( &reply, &string );
        if ( err == GEO_SEARCH_OK ) {
            address.setCountryCode( QString::fromUtf8(string) );
        }

        QtMobilitySubset::QGeoPlace place;
        place.setCoordinate( coordinate );
        place.setAddress( address );

        places->append( place );
    }

    // success if execution makes it here
    return GEO_SEARCH_OK;
}

// This function is blocking and is meant to run in a separate thread using QFuture and QtConcurrent::run()
GeoregReply geocode( const QtMobilitySubset::QGeoAddress &address, const QtMobilitySubset::QGeoCoordinate & hintCoordinate )
{
    bbmock::GeoregApi & georegApi = bbmock::GeoregApi::getInstance();
    GeoregReply georegReply;

    geo_search_handle_t geoServiceHandle;
    geo_search_error_t err = georegApi.geo_search_open( &geoServiceHandle );
    if ( err == GEO_SEARCH_OK ) {
        // Since the georeg service is limited to free-form text string searches use the address.text() field as the input.
        // This is advantageous since if the QGeoAddress text field was not directly set by the caller a string is auto-generated
        // in the call to QGeoAddress::text(), which consists of the other QGeoAddress fields structured into a country
        // code-dependent address string.
        // remove extraneous whitespace from the address.text() string, particularly '\n' that are present when the text is
        // auto-generated from the other address fields.
        QByteArray simplifiedAddress = address.text().simplified().toUtf8();

        geo_search_reply_t reply;
        // this step is potentially blocking
        if ( hintCoordinate.isValid() ) {
            err = georegApi.geo_search_geocode_latlon( &geoServiceHandle, &reply, simplifiedAddress.constData(), hintCoordinate.latitude(), hintCoordinate.longitude() );
        } else {
            err = georegApi.geo_search_geocode( &geoServiceHandle, &reply, simplifiedAddress.constData() );
        }

        if ( err == GEO_SEARCH_OK ) {
            err = populatePlaces( georegApi, &georegReply.places, reply );
            georegApi.geo_search_free_reply( &reply );
        }
    }
    georegReply.error = geoSearchReplyErrorMap.value( err );

    georegApi.geo_search_close( &geoServiceHandle );

    return georegReply;
}

// This function is blocking and is meant to run in a separate thread using QFuture and QtConcurrent::run()
GeoregReply reverseGeocode( const QtMobilitySubset::QGeoCoordinate &coordinate, geo_search_boundary_t boundary )
{
    bbmock::GeoregApi & georegApi = bbmock::GeoregApi::getInstance();
    GeoregReply georegReply;

    geo_search_handle_t geoServiceHandle;
    geo_search_error_t err = georegApi.geo_search_open( &geoServiceHandle );
    if ( err == GEO_SEARCH_OK ) {
        geo_search_reply_t reply;

        // this line is potentially blocking
        err = georegApi.geo_search_reverse_geocode( &geoServiceHandle,
                                                    &reply,
                                                    coordinate.latitude(),
                                                    coordinate.longitude(),
                                                    boundary );
        if ( err == GEO_SEARCH_OK ) {
            err = populatePlaces( georegApi, &georegReply.places, reply );
            georegApi.geo_search_free_reply( &reply );
        }
    }
    georegReply.error = geoSearchReplyErrorMap.value( err );

    georegApi.geo_search_close( &geoServiceHandle );

    return georegReply;
}

}

namespace bb
{
namespace qtplugins
{
namespace geoservices
{

// create a search reply for a geocode request
GeoSearchReplyBb::GeoSearchReplyBb(const QtMobilitySubset::QGeoAddress &address, const QtMobilitySubset::QGeoBoundingArea * bounds, QObject * parent )
    : QGeoSearchReply(parent),
      _bounds(NULL)
{
    if ( !initialize( bounds ) ) {
        return;
    }

    QtMobilitySubset::QGeoCoordinate hintCoordinate = coordinateHint( bounds );

    _future = QtConcurrent::run( geocode, address, hintCoordinate );
    _futureWatcher.setFuture( _future );
}

// create a search reply for a reverse geocode request
GeoSearchReplyBb::GeoSearchReplyBb(const QtMobilitySubset::QGeoCoordinate &coordinate,
                                   geo_search_boundary_t boundary,
                                   const QtMobilitySubset::QGeoBoundingArea * bounds,
                                   QObject * parent )
    : QGeoSearchReply(parent),
      _bounds(NULL)
{
    if ( !initialize( bounds ) ) {
        return;
    }

    _future = QtConcurrent::run( reverseGeocode, coordinate, boundary );
    _futureWatcher.setFuture( _future );
}

/*!
    Destroys this search reply object.
*/
GeoSearchReplyBb::~GeoSearchReplyBb()
{
}

// store the bounds and connect the future watcher to receiveReply()
bool GeoSearchReplyBb::initialize( const QtMobilitySubset::QGeoBoundingArea *bounds )
{
    // the BB Geocode Service does not provide bounding of the request so the bounds are
    // saved here for filtering of the BB Geocode Service reply manually.
    setBounds( bounds );

    // connect the future watcher to receiveReply()
    bool connected = connect( &_futureWatcher, SIGNAL(finished()), SLOT(receiveReply()) );
    if ( !connected ) {
        finishReply( QtMobilitySubset::QGeoSearchReply::CommunicationError );
        return false;
    }

    return true;
}


// SLOT
void GeoSearchReplyBb::receiveReply()
{
    // Get the (unbounded) list of places from the future (this is exciting!)
    GeoregReply georegReply = _future.result();

    if ( georegReply.error != QtMobilitySubset::QGeoSearchReply::NoError ) {
        finishReply( georegReply.error );
        return;
    }

    // apply the bounds and set this GeoSearchReplyBb's places list to the bound subset.
    boundPlaces( georegReply.places );

    // signal that the list of places is ready
    finishReply( QtMobilitySubset::QGeoSearchReply::NoError );
}

// Reduce the list of places to those that are contained within the bounds
void GeoSearchReplyBb::boundPlaces( const QList<QtMobilitySubset::QGeoPlace> & unboundPlaces )
{
    // Bound only if the bounds are valid and not empty. If the bounds are empty the bounds area is zero, so assume the bounds
    // are only useful for the centre location, as a hint where to search in the case of geocoding.
    if ( _bounds && _bounds->isValid() && !_bounds->isEmpty() ) {
        for ( int i = 0 ; i < unboundPlaces.size() ; i++ ) {
            if ( _bounds->contains( unboundPlaces.at(i).coordinate() ) ) {
                addPlace( unboundPlaces.at(i) );
            }
        }
    } else {
        // there are no bounds, add all places.
        setPlaces( unboundPlaces );
    }

}

// Make a copy of the bounds. The QGeoSearchReply design presumes that the bounds are used as part of the request to the underlying
// service. The BB Geocode Service does not provide bounding of the request so the bounds must be copied and held in this instance
// for use after the BB Geocode Service reply is received.
void GeoSearchReplyBb::setBounds( const QtMobilitySubset::QGeoBoundingArea *bounds )
{
    if ( bounds )
    {
        switch ( bounds->type() )
        {
        case QtMobilitySubset::QGeoBoundingArea::BoxType:
            _boundingBox = *( static_cast<const QtMobilitySubset::QGeoBoundingBox*>(bounds) );
            _bounds = &_boundingBox;
            break;

        case QtMobilitySubset::QGeoBoundingArea::CircleType:
            _boundingCircle = *( static_cast<const QtMobilitySubset::QGeoBoundingCircle*>(bounds) );
            _bounds = &_boundingCircle;
            break;

        default:
            _bounds = NULL;
            break;
        }
    }
}

void GeoSearchReplyBb::finishReply( QtMobilitySubset::QGeoSearchReply::Error error )
{
    // Since QGeoSearchReply and its descendents are left to the user to destroy, release unnecessary resources now.
    GeoregReply georegReply = _future.result();
    georegReply.places.clear();

    if ( error == QtMobilitySubset::QGeoSearchReply::NoError ) {
        // this causes finished() to be emitted
        setFinished( true );
    } else {
        // this causes error() and finished() to be emitted
        setError( error, geoSearchReplyErrorStringMap.value( error ) );
    }
}

} // namespace
} // namespace
} // namespace
