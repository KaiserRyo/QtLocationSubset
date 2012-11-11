/**
 * @copyright
 * Copyright Research In Motion Limited, 2012-2012
 * Research In Motion Limited. All rights reserved.
 */

#include <bb/PpsObject>

#include "GeoSatelliteInfoSourceBbPrivate.hpp"
#include "LocationManagerUtil.hpp"

extern "C" {
#include <wmm/wmm.h>
}

#include <QtCore/QVariantMap>
#include <QtCore/QByteArray>

#include <iostream>


///////////////////////////
//
// local variables/functions
//
///////////////////////////

namespace global
{

// While waiting for a position fix a satellite update period of 1 sec is considered reasonable.
static const double defaultSatelliteUpdatePeriod = 1.0;

} // namespace global

namespace bb
{
namespace qtplugins
{
namespace position
{

///////////////////////////
//
// GeoSatelliteInfoSourceBbPrivate
//
///////////////////////////


// Create a QVariantMap suitable for writing to a PpsObject specifying a location request to the Location Manager.
// If the request is periodic then the update interval is used. Otherwise 0 indicates to the Location Manager that
// it is a request for a single, immediate location response
QVariantMap GeoSatelliteInfoSourceBbPrivate::populateLocationRequest( bool periodic )
{
    QVariantMap map;
    QVariantMap datMap;

    int period;
    if ( periodic ) {
        period = _period;
    } else {
        period = 0;
    }

    datMap.insert( "period", period );
    datMap.insert( "accuracy", 0 );
    datMap.insert( "response_time", _responseTime );
    datMap.insert( "background", _backgroundMode );

    datMap.insert( "provider", "gnss" );
    datMap.insert( "fix_type", "gps_autonomous" );

    // have the Location Manager return the satellite information even if it does not have a position fix.
    datMap.insert( "report_sat", true );

    map.insert( "msg", "location" );
    map.insert( "id", global::libQtLocationSubsetId );
    map.insert( "dat", datMap );

    return map;
}

// From a QvariantMap representing a location response from the Location Manager fill the lists of QGeoSatelliteInfo instances
// intended to be emitted via satellitesInUseUpdated() and satellitesInViewUpdated() signals.
void GeoSatelliteInfoSourceBbPrivate::populateSatelliteLists( const QVariantMap & map )
{
    // populate _currentSatelliteInfo
    QVariantMap datMap = map.value("dat").toMap();
    QVariantList satelliteList = datMap.value("satellites").toList();

    _satellitesInView.clear();
    _satellitesInUse.clear();

    QVariant satelliteData;
    Q_FOREACH( satelliteData, satelliteList ) {
        datMap = satelliteData.toMap();
        QtMobilitySubset::QGeoSatelliteInfo satelliteInfo = QtMobilitySubset::QGeoSatelliteInfo();

        if ( datMap.contains( "id" ) ) {
            satelliteInfo.setPrnNumber( (int)datMap.value( "id" ).toDouble() );
        }

        if ( datMap.contains( "cno" ) ) {
            // assuming cno equals signal strength
            satelliteInfo.setSignalStrength( (int)datMap.value( "cno" ).toDouble() );
        }

        // attributes
        if ( datMap.contains( "elevation" ) ) {
            satelliteInfo.setAttribute( QtMobilitySubset::QGeoSatelliteInfo::Elevation, (qreal)datMap.value( "elevation" ).toDouble() );
        } else {
            satelliteInfo.removeAttribute( QtMobilitySubset::QGeoSatelliteInfo::Elevation );
        }

        if ( datMap.contains( "azimuth" ) ) {
            satelliteInfo.setAttribute( QtMobilitySubset::QGeoSatelliteInfo::Azimuth, (qreal)datMap.value( "azimuth" ).toDouble() );
        } else {
            satelliteInfo.removeAttribute( QtMobilitySubset::QGeoSatelliteInfo::Azimuth );
        }

        // each satellite in this list is considered "in view"
        _satellitesInView.append( satelliteInfo );

        if ( datMap.value( "used" ).toBool() == true ) {
            _satellitesInUse.append( satelliteInfo );
        }
    }
}

// The satellite data is retrieved from a location request
bool GeoSatelliteInfoSourceBbPrivate::requestSatelliteInfo( bool periodic )
{
    // build up the request
    QVariantMap request = populateLocationRequest( periodic );

    bb::PpsObject *ppsObject;
    if ( periodic ) {
        ppsObject = _periodicUpdatePpsObject;
    } else {
        ppsObject = _singleUpdatePpsObject;
    }
    if ( sendRequest( *ppsObject, request ) == false ) {
        stopUpdates();
        return false;
    }

    return true;
}

void GeoSatelliteInfoSourceBbPrivate::cancelSatelliteInfo( bool periodic )
{
    bb::PpsObject *ppsObject;
    if ( periodic ) {
        ppsObject = _periodicUpdatePpsObject;
    } else {
        ppsObject = _singleUpdatePpsObject;
    }

    (void)sendRequest( *ppsObject, global::cancelRequest );
}

// Constructor. Note there are two PpsObjects for handling the two different types of requests that can be
// simultaneously made and which must be handled independently (apart from both being emitted through the same
// signal when done-part of Qt Mobility spec.
GeoSatelliteInfoSourceBbPrivate::GeoSatelliteInfoSourceBbPrivate(GeoSatelliteInfoSourceBb *parent)
        :   QObject(parent),
            _startUpdatesInvoked(false),
            _requestUpdateInvoked(false),
            q_ptr(parent),
            _periodicUpdatePpsObject(new bb::PpsObject( global::locationManagerPpsFile, this )),
            _singleUpdatePpsObject(new bb::PpsObject( global::locationManagerPpsFile, this )),
            _requestUpdateTimer(new QTimer(this)),
            _period(::global::defaultSatelliteUpdatePeriod),
            _backgroundMode(false),
            _responseTime(0)
{
    // connect to periodic update PpsObject::readyRead()
    bool connected = connect( _periodicUpdatePpsObject, SIGNAL(readyRead()), SLOT(receivePeriodicSatelliteReply()) );
    if ( !connected ) {
        std::cout << "GeoSatelliteInfoSourceBbPrivate::GeoSatelliteInfoSourceBbPrivate(): error connecting readyRead()" << std::endl;
    }

    // connect to single update PpsObject::readyRead()
    connected = connect( _singleUpdatePpsObject, SIGNAL(readyRead()), SLOT(receiveSingleSatelliteReply()) );
    if ( !connected ) {
        std::cout << "GeoSatelliteInfoSourceBbPrivate::GeoSatelliteInfoSourceBbPrivate(): error connecting readyRead()" << std::endl;
    }

    // connect to the requestUpdate timer timeout()
    _requestUpdateTimer->setSingleShot( true );
    connected = connect( _requestUpdateTimer, SIGNAL(timeout()), SLOT(singleUpdateTimeout()) );
    if ( !connected ) {
        std::cout << "GeoSatelliteInfoSourceBbPrivate::GeoSatelliteInfoSourceBbPrivate(): error connecting timeout()" << std::endl;
    }
}

GeoSatelliteInfoSourceBbPrivate::~GeoSatelliteInfoSourceBbPrivate()
{
    stopUpdates();
}

// request periodic updates
void GeoSatelliteInfoSourceBbPrivate::startUpdates()
{
    // do nothing if periodic updates have already been started
    if (_startUpdatesInvoked) {
        return;
    }

    // build a request and initiate it
    if ( requestSatelliteInfo( true ) ) {
        _startUpdatesInvoked = true;
    }
}

// stop periodic updates
void GeoSatelliteInfoSourceBbPrivate::stopUpdates()
{
    // do nothing if periodic updates have not been started
    if (!_startUpdatesInvoked) {
        return;
    }

    cancelSatelliteInfo( true );
    _startUpdatesInvoked = false;
}

// request single update
void GeoSatelliteInfoSourceBbPrivate::requestUpdate(int msec)
{
    // do nothing if an immediate update has already been requested
    if ( _requestUpdateInvoked ) {
        return;
    }

    // If it is not possible to update in msec timeout right away.
    if (msec < 0 || msec < global::minUpdateInterval) {
        Q_Q(GeoSatelliteInfoSourceBb);
        Q_EMIT q->requestTimeout();
        return;
    }
    _requestUpdateInvoked = true;

    requestSatelliteInfo( false );

    // if msec == 0 rely on the device to timeout as usual
    if ( msec > 0 )  {
        // start the timeout timer
        _requestUpdateTimer->start( msec );
    }
}

// single update has timed out. This is a slot for the requestUpdate timer
void GeoSatelliteInfoSourceBbPrivate::singleUpdateTimeout()
{
    // don't emit a timeout signal if a update is not currently requested (avoid sending more than one timeout signal)
    if ( !_requestUpdateInvoked ) {
        return;
    }

    cancelSatelliteInfo( false );
    _requestUpdateInvoked = false;

    Q_Q(GeoSatelliteInfoSourceBb);
    Q_EMIT q->requestTimeout();
}

bool GeoSatelliteInfoSourceBbPrivate::receiveSatelliteReply(bb::PpsObject & ppsObject )
{
    QVariantMap reply;
    // receiveReply() tests for errors associated with the request being replied to
    if ( !receiveReply( &reply, ppsObject ) ) {
        return false;
    }

    // check that this is a location reply (could be a reply to another request type, eg. cancel, which is ignored here)
    if ( reply.contains("res") && reply.value("res").toString() == "location" ) {
        // extract the satellite info from the reply into _satellitesInView and _satellitesInUse
        populateSatelliteLists( reply );

        Q_Q(GeoSatelliteInfoSourceBb);
        Q_EMIT q->satellitesInUseUpdated( _satellitesInUse );
        Q_EMIT q->satellitesInViewUpdated( _satellitesInView );
    }

    return true;
}

void GeoSatelliteInfoSourceBbPrivate::receivePeriodicSatelliteReply()
{
    // don't try to receive a reply if periodic updates have not been started. This is
    // necessary because this slot is connected to PpsObject::readyRead() and could be
    // invoked any time the pps file is updated by the server. Under error conditions
    // this would otherwise lead to a circular calling sequence: receive, timeout due to
    // error, cancel, receive...
    if (!_startUpdatesInvoked) {
        return;
    }

    // there is no recourse if the periodic satellite reply indicates failure.
    (void)receiveSatelliteReply( *_periodicUpdatePpsObject );
}

void GeoSatelliteInfoSourceBbPrivate::receiveSingleSatelliteReply()
{
    // don't try to receive a reply if a single update has not been requested. This is
    // necessary because this slot is connected to PpsObject::readyRead() and could be
    // invoked any time the pps file is updated by the server. Under error conditions
    // this would otherwise lead to a circular calling sequence: receive, timeout due to
    // error, cancel, receive...
    if ( !_requestUpdateInvoked ) {
        return;
    }

    _requestUpdateTimer->stop();

    if ( !receiveSatelliteReply( *_singleUpdatePpsObject ) ) {
        singleUpdateTimeout();
    }

    _requestUpdateInvoked = false;
}


///////////////////////////
//
// GeoSatelliteInfoSourceBb
//
///////////////////////////


/*!
    Constructs a GeoSatelliteInfoSourceBb instance with the given \a parent
    and \a updateMode.
*/
GeoSatelliteInfoSourceBb::GeoSatelliteInfoSourceBb(QObject *parent)
        : QtMobilitySubset::QGeoSatelliteInfoSource(parent),
        d_ptr(new GeoSatelliteInfoSourceBbPrivate(this))
{
}

/*!
    Destroys the satellite source.
*/
GeoSatelliteInfoSourceBb::~GeoSatelliteInfoSourceBb()
{
}

/*!
    \reimp
*/
void GeoSatelliteInfoSourceBb::startUpdates()
{
    Q_D(GeoSatelliteInfoSourceBb);
    d->startUpdates();
}

/*!
    \reimp
*/
void GeoSatelliteInfoSourceBb::stopUpdates()
{
    Q_D(GeoSatelliteInfoSourceBb);
    d->stopUpdates();
}

/*!
    \reimp
*/
void GeoSatelliteInfoSourceBb::requestUpdate(int msec)
{
    Q_D(GeoSatelliteInfoSourceBb);
    d->requestUpdate(msec);
}

// property managers. These properties extend QGeoSatelliteInfoSource by allowing additional control provided by the
// Location Manager

double GeoSatelliteInfoSourceBb::period() const
{
    Q_D(const GeoSatelliteInfoSourceBb);
    return d->_period;
}

void GeoSatelliteInfoSourceBb::setPeriod( double period )
{
    Q_D(GeoSatelliteInfoSourceBb);
    d->_period = period;
}

bool GeoSatelliteInfoSourceBb::backgroundMode() const
{
    Q_D(const GeoSatelliteInfoSourceBb);
    return d->_backgroundMode;
}

void GeoSatelliteInfoSourceBb::setBackgroundMode( bool mode )
{
    Q_D(GeoSatelliteInfoSourceBb);
    d->_backgroundMode = mode;
}

int GeoSatelliteInfoSourceBb::responseTime() const
{
    Q_D(const GeoSatelliteInfoSourceBb);
    return d->_responseTime;
}

void GeoSatelliteInfoSourceBb::setResponseTime( int responseTime )
{
    Q_D(GeoSatelliteInfoSourceBb);
    d->_responseTime = responseTime;
}

} // namespace
} // namespace
} // namespace
