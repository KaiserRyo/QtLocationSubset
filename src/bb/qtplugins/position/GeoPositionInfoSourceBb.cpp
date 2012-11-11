/**
 * @copyright
 * Copyright Research In Motion Limited, 2012-2012
 * Research In Motion Limited. All rights reserved.
 */

#include "GeoPositionInfoSourceBbPrivate.hpp"
#include "LocationManagerUtil.hpp"

#include <bb/PpsObject>

extern "C" {
#include <wmm/wmm.h>
}

#include <QMap>
#include <QVariantMap>
#include <QByteArray>
#include <QtDebug>
#include <QStringList>


///////////////////////////
//
// local variables/functions
//
///////////////////////////

namespace global
{

// Currently the default behaviour for Location Manager is simply to set a constant default interval.
// 5 sec has been chosen as a compromise between timely updates and conserving power.
static const int defaultPositionUpdatePeriod = 5;

} // namespace global

namespace
{

// map the Location Manager provider names to the QGeoPositionInfoSource positioning methods
QMap<QtMobilitySubset::QGeoPositionInfoSource::PositioningMethods, QString> createPositioningMethodsToProviderMap()
{
    QMap<QtMobilitySubset::QGeoPositionInfoSource::PositioningMethods, QString> map;

    map.insert( QtMobilitySubset::QGeoPositionInfoSource::SatellitePositioningMethods, QString( "gnss" ) );
    map.insert( QtMobilitySubset::QGeoPositionInfoSource::NonSatellitePositioningMethods, QString( "network" ) );
    map.insert( QtMobilitySubset::QGeoPositionInfoSource::AllPositioningMethods, QString( "hybrid" ) );

    return map;
}

const QMap<QtMobilitySubset::QGeoPositionInfoSource::PositioningMethods, QString> positioningMethodsToProviderMap =
        createPositioningMethodsToProviderMap();

// list of valid strings for the Location Manager reset types
QStringList createValidResetTypesList()
{
    QStringList list;
    list.append("cold");
    list.append("warm");
    list.append("hot");
    list.append("factory");

    return list;
}

const QStringList validResetTypes = createValidResetTypesList();

void printGetGeomagneticFieldInputs( const wmm_location_t & location, const struct tm & date )
{
    qWarning() << "location = (" << location.latitude_deg << "," << location.longitude_deg << "," << location.altitude_meters << ")";
    qWarning() << "date = (" <<        date.tm_sec <<
                                "," << date.tm_min <<
                                "," << date.tm_hour <<
                                "," << date.tm_mday <<
                                "," << date.tm_mon <<
                                "," << date.tm_year <<
                                "," << date.tm_wday <<
                                "," << date.tm_yday <<
                                "," << date.tm_isdst <<
#ifndef BB_TEST_BUILD
// the following fields are not present on host (at least Win32)
                                "," << date.tm_gmtoff <<
                                "," << date.tm_zone <<
#endif
                                ")";
}

bool magneticDeclination( double * declination, const QtMobilitySubset::QGeoPositionInfo & position )
{
    if ( !declination ) {
        return false;
    }

    wmm_location_t location;
    struct tm date;
    wmm_geomagnetic_field_t field;

    location.latitude_deg = position.coordinate().latitude();
    location.longitude_deg = position.coordinate().longitude();
    if ( position.coordinate().type() == QtMobilitySubset::QGeoCoordinate::Coordinate3D ) {
        location.altitude_meters = position.coordinate().altitude();
    } else {
        location.altitude_meters = 0.0;
    }

    time_t time = (time_t)position.timestamp().toTime_t();
#ifdef BB_TEST_BUILD
    // since gmtime_r() is not defined on host (at least Win32) risk reentrant side effects on the returned data.
    struct tm * pDate = gmtime( &time );
    if ( pDate == NULL ) {
        qWarning() << "GeoPositionInfoSourceBbPrivate.cpp:magneticDeclination(): gmtime() returned NULL";
        return false;
    }
    date = *pDate;
#else
    if ( gmtime_r( &time, &date ) == NULL ) {
        qWarning() << "GeoPositionInfoSourceBbPrivate.cpp:magneticDeclination(): gmtime_r() returned NULL";
        return false;
    }
#endif

    switch ( wmm_get_geomagnetic_field( &location, &date, &field ) ) {

    case 0:
        break;

    case 1:
        qWarning() << "GeoPositionInfoSourceBbPrivate.cpp:magneticDeclination(): wmm_get_geomagnetic_field() returned: inputs limited to model range";
        printGetGeomagneticFieldInputs( location, date );
        break;

    case -1:
    default:
        qWarning() << "GeoPositionInfoSourceBbPrivate.cpp:magneticDeclination(): wmm_get_geomagnetic_field() returned: error";
        printGetGeomagneticFieldInputs( location, date );
        return false;
    }

    *declination = field.declination_deg;
    return true;
}

QVariantMap populateLastKnownPositionRequest(bool fromSatellitePositioningMethodsOnly)
{
    QVariantMap map;
    QVariantMap datMap;

    if ( fromSatellitePositioningMethodsOnly ) {
        datMap.insert( "provider", "gnss" );
    } else {
        datMap.insert( "provider", "hybrid" );
    }
    datMap.insert( "last_known", true );
    datMap.insert( "period", 0 );

    map.insert( "msg", "location" );
    map.insert( "id", bb::qtplugins::position::global::libQtLocationSubsetId );
    map.insert( "dat", datMap );

    return map;
}

// From a QvariantMap representing a location response from the Location Manager fill a QGeoPositionInfo instance
// intended to be emitted via positionUpdated() signal. Returns true if the position info was successfully populated.
bool populatePositionInfo( QtMobilitySubset::QGeoPositionInfo * position, const QVariantMap & map )
{
    // populate position

    // set the reply dat property, which can be accessed by the user in the slot connected to the positionUpdated() signal
    QVariantMap replyDat = map.value("dat").toMap();

    // check for required fields
    if ( !replyDat.contains("latitude") || !replyDat.contains("longitude") || !replyDat.contains("accuracy") ) {
        return false;
    }

    // set the lat/long/alt coordinate
    QtMobilitySubset::QGeoCoordinate coord;
    coord.setLatitude( replyDat.value("latitude").toDouble());
    coord.setLongitude( replyDat.value("longitude").toDouble());
    if ( replyDat.contains("altitude") ) {
        coord.setAltitude( replyDat.value("altitude").toDouble() );
    }

    if ( !coord.isValid() ) {
        return false;
    }

    position->setCoordinate( coord );

    // set the time stamp
    QDateTime dateTime;
    dateTime.setTimeSpec(Qt::UTC);
    if ( replyDat.contains("utc") && static_cast<int>(replyDat.value("utc").toDouble()) != 0 ) {
        // utc is msec since epoch (1970-01-01T00:00:00)
        dateTime.setTime_t( static_cast<int>(( replyDat.value("utc").toDouble() / 1000.0 + 0.5 )) );
    } else {
        // this relies on the device's clock being accurate
        dateTime = QDateTime::currentDateTimeUtc();
    }
    position->setTimestamp( dateTime );

    // attributes
    if ( replyDat.contains("heading") ) {
        position->setAttribute( QtMobilitySubset::QGeoPositionInfo::Direction,
                                     static_cast<qreal>(replyDat.value("heading").toDouble()) );
    } else {
        position->removeAttribute( QtMobilitySubset::QGeoPositionInfo::Direction );
    }

    if ( replyDat.contains("speed") ) {
        position->setAttribute( QtMobilitySubset::QGeoPositionInfo::GroundSpeed,
                                     static_cast<qreal>(replyDat.value("speed").toDouble()) );
    } else {
        position->removeAttribute( QtMobilitySubset::QGeoPositionInfo::GroundSpeed );
    }

    if ( replyDat.contains("verticalSpeed") ) {
        position->setAttribute( QtMobilitySubset::QGeoPositionInfo::VerticalSpeed,
                                     static_cast<qreal>(replyDat.value("verticalSpeed").toDouble()) );
    } else {
        position->removeAttribute( QtMobilitySubset::QGeoPositionInfo::VerticalSpeed );
    }

    if ( replyDat.contains("declination") ) {
        position->setAttribute( QtMobilitySubset::QGeoPositionInfo::MagneticVariation,
                                     static_cast<qreal>(replyDat.value("declination").toDouble()) );
    } else {
        double declination;

        if ( magneticDeclination( &declination, *position ) == true ) {
            position->setAttribute( QtMobilitySubset::QGeoPositionInfo::MagneticVariation,
                                         static_cast<qreal>(declination) );
        } else {
            position->removeAttribute( QtMobilitySubset::QGeoPositionInfo::MagneticVariation );
        }
    }

    // replyDat.contains("accuracy") was confirmed above
    position->setAttribute( QtMobilitySubset::QGeoPositionInfo::HorizontalAccuracy,
                                 static_cast<qreal>(replyDat.value("accuracy").toDouble()) );

    if ( replyDat.contains("altitudeAccuracy") ) {
        position->setAttribute( QtMobilitySubset::QGeoPositionInfo::VerticalAccuracy,
                                     static_cast<qreal>(replyDat.value("altitudeAccuracy").toDouble()) );
    } else {
        position->removeAttribute( QtMobilitySubset::QGeoPositionInfo::VerticalAccuracy );
    }

    return true;
}

} // unnamed namespace

namespace bb
{
namespace qtplugins
{
namespace position
{

///////////////////////////
//
// GeoPositionInfoSourceBbPrivate
//
///////////////////////////


// Create a QVariantMap suitable for writing to a PpsObject specifying a location request to the Location Manager.
// If the request is periodic then the update interval is used. Otherwise 0 indicates to the Location Manager that
// it is a request for a single, immediate location response.
QVariantMap GeoPositionInfoSourceBbPrivate::populateLocationRequest( bool periodic ) const
{
    Q_Q(const GeoPositionInfoSourceBb);

    QVariantMap map;
    QVariantMap datMap;

    int period;
    if ( periodic ) {
        // rounding is performed here because the Location Manager truncates to nearest integer
        period = ( q->updateInterval() + 500 ) / 1000;
        // The Qt MObility API treats a period of 0 as indicating default behaviour
        if ( period == 0 ) {
            // specify global::defaultPositionUpdatePeriod as the default behaviour for Location Manager
            period = ::global::defaultPositionUpdatePeriod;
        }
    } else {
        period = 0;
    }

    // period is the only mandatory field
    datMap.insert( "period", period );

    if ( _accuracy > 0.0 ) {
        datMap.insert( "accuracy", _accuracy );
    }
    if ( _responseTime > 0.0 ) {
        datMap.insert( "response_time", _responseTime );
    }

    // since there is no uninitialized state for bool always specify the background mode
    datMap.insert( "background", _canRunInBackground );

    QString provider = positioningMethodsToProviderMap.value( q->preferredPositioningMethods() );
    if ( !provider.isEmpty() ) {
        datMap.insert( "provider", provider );
    }

    if ( !_fixType.isEmpty() ) {
        datMap.insert( "fix_type", _fixType );
    }

    if ( !_appId.isEmpty() ) {
        datMap.insert( "app_id", _appId);
    }

    if ( !_appPassword.isEmpty() ) {
        datMap.insert( "app_password", _appPassword );
    }

    if ( !_pdeUrl.isEmpty() ) {
        datMap.insert( "pde_url", _pdeUrl.toEncoded().constData() );
    }

    if ( !_slpUrl.isEmpty() ) {
        datMap.insert( "slp_url", _slpUrl.toEncoded().constData());
    }

    map.insert( "msg", "location" );
    map.insert( "id", global::libQtLocationSubsetId );
    map.insert( "dat", datMap );

    return map;
}

QVariantMap GeoPositionInfoSourceBbPrivate::populateResetRequest() const
{
    QVariantMap map;
    QVariantMap datMap;

    datMap.insert( "reset_type", _resetType );

    map.insert( "msg", "reset" );
    map.insert( "id", bb::qtplugins::position::global::libQtLocationSubsetId );
    map.insert( "dat", datMap );

    return map;
}

bool GeoPositionInfoSourceBbPrivate::requestPositionInfo( bool periodic )
{
    // build up the request
    QVariantMap request = populateLocationRequest( periodic );

    bb::PpsObject *ppsObject;
    if ( periodic ) {
        ppsObject = _periodicUpdatePpsObject;
    } else {
        ppsObject = _singleUpdatePpsObject;
    }

    return sendRequest( *ppsObject, request );
}

void GeoPositionInfoSourceBbPrivate::cancelPositionInfo( bool periodic )
{
    bb::PpsObject *ppsObject;
    if ( periodic ) {
        ppsObject = _periodicUpdatePpsObject;
    } else {
        ppsObject = _singleUpdatePpsObject;
    }
    (void)sendRequest( *ppsObject, global::cancelRequest );
}

void GeoPositionInfoSourceBbPrivate::resetLocationProviders()
{
    QVariantMap map = populateResetRequest();
    (void)sendRequest( *_periodicUpdatePpsObject, map );
}

// Get the last known position from the Location Manager. Any error results in the return of an invalid position.
QtMobilitySubset::QGeoPositionInfo GeoPositionInfoSourceBbPrivate::lastKnownPosition(bool fromSatellitePositioningMethodsOnly) const
{
    QtMobilitySubset::QGeoPositionInfo position = QtMobilitySubset::QGeoPositionInfo();
    bb::PpsObject ppsObject( global::locationManagerPpsFile );
    QVariantMap lastKnown = populateLastKnownPositionRequest( fromSatellitePositioningMethodsOnly );

    if ( !ppsObject.open() ) {
        return position;
    }
    // Location Manager promises to reply immediately with the last known position or an error.
    ppsObject.setBlocking( true );

    if ( !sendRequest( ppsObject, lastKnown ) ) {
        return position;
    }

    if ( !receiveReply( &lastKnown, ppsObject ) ) {
        return position;
    }

    if ( !lastKnown.contains("res") || lastKnown.value("res").toString() != "location" ) {
        return position;
    }

    // the return value of populatePositionInfo() is ignored since either way position is returned by lastKnownPosition()
    (void)populatePositionInfo( &position, lastKnown );

    return position;
}

// Constructor. Note there are two PpsObjects for handling the two different types of requests that can be
// simultaneously made and which must be handled independently (apart from both being emitted through the same
// signal when done-part of Qt Mobility spec.
GeoPositionInfoSourceBbPrivate::GeoPositionInfoSourceBbPrivate(GeoPositionInfoSourceBb *parent)
        :   QObject(parent),
            _startUpdatesInvoked(false),
            _requestUpdateInvoked(false),
            _canEmitPeriodicUpdatesTimeout(true),
            q_ptr(parent),
            _periodicUpdatePpsObject(new bb::PpsObject( global::locationManagerPpsFile, this )),
            _singleUpdatePpsObject(new bb::PpsObject( global::locationManagerPpsFile, this )),
            _requestUpdateTimer(new QTimer(this)),
            _accuracy(0.0),
            _responseTime(0.0),
            _canRunInBackground(false),
            _fixType(QString()),
            _appId(QString()),
            _appPassword(QString()),
            _pdeUrl(QUrl()),
            _slpUrl(QUrl()),
            _replyDat(QVariantMap()),
            _replyErr(QString()),
            _replyErrStr(QString()),
            _resetType(QString())
{
    // connect to periodic update PpsObject::readyRead()
    bool connected = connect( _periodicUpdatePpsObject, SIGNAL(readyRead()), SLOT(receivePeriodicPositionReply()) );
    if ( !connected ) {
        qWarning() << "GeoPositionInfoSourceBbPrivate::GeoPositionInfoSourceBbPrivate(): error connecting readyRead()";
    }

    // connect to single update PpsObject::readyRead()
    connected = connect( _singleUpdatePpsObject, SIGNAL(readyRead()), SLOT(receiveSinglePositionReply()) );
    if ( !connected ) {
        qWarning() << "GeoPositionInfoSourceBbPrivate::GeoPositionInfoSourceBbPrivate(): error connecting readyRead()";
    }

    // connect to the requestUpdate timer timeout()
    _requestUpdateTimer->setSingleShot( true );
    connected = connect( _requestUpdateTimer, SIGNAL(timeout()), SLOT(singleUpdateTimeout()) );
    if ( !connected ) {
        qWarning() << "GeoPositionInfoSourceBbPrivate::GeoPositionInfoSourceBbPrivate(): error connecting timeout()";
    }
}

GeoPositionInfoSourceBbPrivate::~GeoPositionInfoSourceBbPrivate()
{
    stopUpdates();
}

// request periodic updates
void GeoPositionInfoSourceBbPrivate::startUpdates()
{
    // do nothing if periodic updates have already been started
    if (_startUpdatesInvoked) {
        return;
    }

    // This flag is used to limit emitting the timeout signal to once per each interruption in the periodic
    // updates. Since updates are being started here ensure the flag is set to true.
    _canEmitPeriodicUpdatesTimeout = true;

    // build a request and initiate it
    if ( requestPositionInfo( true ) ) {
        _startUpdatesInvoked = true;
        _currentPosition = QtMobilitySubset::QGeoPositionInfo();
    } else {
        // user is expecting a signal to be emitted, cannot emit positionUpdated() because of error so emit timeout signal
        Q_Q(GeoPositionInfoSourceBb);
        Q_EMIT q->updateTimeout();
    }
}

// stop periodic updates
void GeoPositionInfoSourceBbPrivate::stopUpdates()
{
    // do nothing if periodic updates have not been started
    if (!_startUpdatesInvoked) {
        return;
    }

    cancelPositionInfo( true );
    _startUpdatesInvoked = false;
    _currentPosition = QtMobilitySubset::QGeoPositionInfo();
}

// periodic updates have timed out
void GeoPositionInfoSourceBbPrivate::periodicUpdatesTimeout()
{
    // do nothing if periodic updates have not been started
    if (!_startUpdatesInvoked) {
        return;
    }

    // timeout has occurred, but periodic updates are still active. Ensure the timeout signal is emitted only once
    // per interruption of updates. _canEmitPeriodicUpdatesTimeout is set back to true when the next successful periodic
    // update occurs (see emitPositionUpdated()). This behaviour is per the Qt Mobility Location API documentation.
    if ( _canEmitPeriodicUpdatesTimeout ) {
        _canEmitPeriodicUpdatesTimeout = false;
        Q_Q(GeoPositionInfoSourceBb);
        Q_EMIT q->updateTimeout();
    }
}

// request single update
void GeoPositionInfoSourceBbPrivate::requestUpdate(int msec)
{
    // do nothing if an immediate update has already been requested
    if ( _requestUpdateInvoked ) {
        return;
    }

    // If it is not possible to update in msec timeout right away.
    Q_Q(GeoPositionInfoSourceBb);
    if (msec < 0 || msec < q->minimumUpdateInterval()) {
        Q_EMIT q->updateTimeout();
        return;
    }
    _requestUpdateInvoked = true;

    requestPositionInfo( false );

    // if msec == 0 rely on the device to timeout as usual
    if ( msec > 0 )  {
        // start the timeout timer
        _requestUpdateTimer->start( msec );
    }
}

// single update has timed out. This is a slot for the requestUpdate timer
void GeoPositionInfoSourceBbPrivate::singleUpdateTimeout()
{
    // don't emit a timeout signal if a update is not currently requested (avoid sending more than one timeout signal)
    if ( !_requestUpdateInvoked ) {
        return;
    }

    cancelPositionInfo( false );
    _requestUpdateInvoked = false;

    Q_Q(GeoPositionInfoSourceBb);
    Q_EMIT q->updateTimeout();
}

void GeoPositionInfoSourceBbPrivate::emitPositionUpdated(const QtMobilitySubset::QGeoPositionInfo &update)
{
    // having successfully received a position update, set _canEmitPeriodicUpdatesTimeout to true, which (re)enables a
    // timeout to be emitted upon any subsequent error in periodic updating.
    _canEmitPeriodicUpdatesTimeout = true;

    Q_Q(GeoPositionInfoSourceBb);
    Q_EMIT q->positionUpdated(update);
}

bool GeoPositionInfoSourceBbPrivate::receivePositionReply(bb::PpsObject & ppsObject )
{
    QVariantMap reply;
    // receiveReply() tests for errors associated with the request being replied to
    if ( !receiveReply( &reply, ppsObject ) ) {
        // if there is an error from Location Manager report it so user can access it through the properties when responding to the
        // updateTimeout() signal.
        if ( reply.contains("err") ) {
            _replyErr = reply.value("err").toString();
            if ( reply.contains("errstr") ) {
                _replyErrStr = reply.value("errstr").toString();
            }
        }
        return false;
    }

    // clear any errors
    _replyErr = QString();
    _replyErrStr = QString();

    // check that this is a location reply (could be a reply to another request type, eg. cancel, which is ignored here)
    if ( reply.contains("res") && reply.value("res").toString() == "location" ) {
        // keep the raw LM reply for access via Qt properties.
        _replyDat = reply.value("dat").toMap();

        // extract the geo position info from the reply into _currentPosition
        if ( populatePositionInfo( &_currentPosition, reply ) ) {
            emitPositionUpdated(_currentPosition);
        }
    }

    return true;
}

void GeoPositionInfoSourceBbPrivate::receivePeriodicPositionReply()
{
    // don't try to receive a reply if periodic updates have not been started. This is
    // necessary because this slot is connected to PpsObject::readyRead() and could be
    // invoked any time the pps file is updated by the server. Under error conditions
    // this would otherwise lead to a circular calling sequence: receive, timeout due to
    // error, cancel, receive...
    if (!_startUpdatesInvoked) {
        return;
    }

    if ( !receivePositionReply( *_periodicUpdatePpsObject ) ) {
        periodicUpdatesTimeout();
    }
}

void GeoPositionInfoSourceBbPrivate::receiveSinglePositionReply()
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

    if ( !receivePositionReply( *_singleUpdatePpsObject ) ) {
        singleUpdateTimeout();
    }

    _requestUpdateInvoked = false;
}


///////////////////////////
//
// GeoPositionInfoSourceBb
//
///////////////////////////


/*!
    Constructs a GeoPositionInfoSourceBb instance with the given \a parent
    and \a updateMode.
*/
GeoPositionInfoSourceBb::GeoPositionInfoSourceBb(QObject *parent)
        : QtMobilitySubset::QGeoPositionInfoSource(parent),
        d_ptr(new GeoPositionInfoSourceBbPrivate(this))
{
}

/*!
    Destroys the position source.
*/
GeoPositionInfoSourceBb::~GeoPositionInfoSourceBb()
{
}

/*!
    \reimp
*/
void GeoPositionInfoSourceBb::setUpdateInterval(int msec)
{
    int interval = msec;
    if (interval != 0)
        interval = qMax(msec, minimumUpdateInterval());
    QtMobilitySubset::QGeoPositionInfoSource::setUpdateInterval(interval);

    Q_D(GeoPositionInfoSourceBb);
    if (d->_startUpdatesInvoked) {
        d->stopUpdates();
        d->startUpdates();
    }
}

/*!
    \reimp
*/
void GeoPositionInfoSourceBb::startUpdates()
{
    Q_D(GeoPositionInfoSourceBb);
    d->startUpdates();
}

/*!
    \reimp
*/
void GeoPositionInfoSourceBb::stopUpdates()
{
    Q_D(GeoPositionInfoSourceBb);
    d->stopUpdates();
}

/*!
    \reimp
*/
void GeoPositionInfoSourceBb::requestUpdate(int msec)
{
    Q_D(GeoPositionInfoSourceBb);
    d->requestUpdate(msec);
}

/*!
    \reimp
*/
QtMobilitySubset::QGeoPositionInfo GeoPositionInfoSourceBb::lastKnownPosition(bool fromSatellitePositioningMethodsOnly) const
{
    Q_D(const GeoPositionInfoSourceBb);
    return d->lastKnownPosition(fromSatellitePositioningMethodsOnly);
}

/*!
    \reimp
*/
QtMobilitySubset::QGeoPositionInfoSource::PositioningMethods GeoPositionInfoSourceBb::supportedPositioningMethods() const
{
    return AllPositioningMethods;
}

/*!
    \reimp
*/
int GeoPositionInfoSourceBb::minimumUpdateInterval() const
{
    return global::minUpdateInterval;
}

// property managers. These properties extend QGeoPositionInfoSource by allowing additional control provided by the
// Location Manager

double GeoPositionInfoSourceBb::period() const
{
    // convert from msec to sec
    return static_cast<double>(( updateInterval() / 1000.0 ));
}

void GeoPositionInfoSourceBb::setPeriod( double period )
{
    // convert from sec to msec, round to nearest msec
    setUpdateInterval( static_cast<int>(( period * 1000 + 0.5 )) );
}

double GeoPositionInfoSourceBb::accuracy() const
{
    Q_D(const GeoPositionInfoSourceBb);
    return d->_accuracy;
}

void GeoPositionInfoSourceBb::setAccuracy( double accuracy )
{
    Q_D(GeoPositionInfoSourceBb);
    d->_accuracy = accuracy;
}

double GeoPositionInfoSourceBb::responseTime() const
{
    Q_D(const GeoPositionInfoSourceBb);
    return d->_responseTime;
}

void GeoPositionInfoSourceBb::setResponseTime( double responseTime )
{
    Q_D(GeoPositionInfoSourceBb);
    d->_responseTime = responseTime;
}

bool GeoPositionInfoSourceBb::canRunInBackground() const
{
    Q_D(const GeoPositionInfoSourceBb);
    return d->_canRunInBackground;
}

void GeoPositionInfoSourceBb::setCanRunInBackground( bool canRunInBackground )
{
    Q_D(GeoPositionInfoSourceBb);
    d->_canRunInBackground = canRunInBackground;
}

QString GeoPositionInfoSourceBb::provider() const
{
    return positioningMethodsToProviderMap.value( preferredPositioningMethods() );
}

void GeoPositionInfoSourceBb::setProvider( const QString & provider )
{
    setPreferredPositioningMethods( positioningMethodsToProviderMap.key( provider ) );
}

QString GeoPositionInfoSourceBb::fixType() const
{
    Q_D(const GeoPositionInfoSourceBb);
    return d->_fixType;
}

void GeoPositionInfoSourceBb::setFixType( const QString & fixType )
{
    Q_D(GeoPositionInfoSourceBb);
    d->_fixType = fixType;
}

QString GeoPositionInfoSourceBb::appId() const
{
    Q_D(const GeoPositionInfoSourceBb);
    return d->_appId;
}

void GeoPositionInfoSourceBb::setAppId( const QString & appId )
{
    Q_D(GeoPositionInfoSourceBb);
    d->_appId = appId;
}

QString GeoPositionInfoSourceBb::appPassword() const
{
    Q_D(const GeoPositionInfoSourceBb);
    return d->_appPassword;
}

void GeoPositionInfoSourceBb::setAppPassword( const QString & appPassword )
{
    Q_D(GeoPositionInfoSourceBb);
    d->_appPassword = appPassword;
}

QUrl GeoPositionInfoSourceBb::pdeUrl() const
{
    Q_D(const GeoPositionInfoSourceBb);
    return d->_pdeUrl;
}

void GeoPositionInfoSourceBb::setPdeUrl( const QUrl & pdeUrl )
{
    Q_D(GeoPositionInfoSourceBb);
    d->_pdeUrl = pdeUrl;
}

QUrl GeoPositionInfoSourceBb::slpUrl() const
{
    Q_D(const GeoPositionInfoSourceBb);
    return d->_slpUrl;
}

void GeoPositionInfoSourceBb::setSlpUrl( const QUrl & slpUrl )
{
    Q_D(GeoPositionInfoSourceBb);
    d->_slpUrl = slpUrl;
}

QVariantMap GeoPositionInfoSourceBb::replyDat() const
{
    Q_D(const GeoPositionInfoSourceBb);
    return d->_replyDat;
}

QString GeoPositionInfoSourceBb::replyErr() const
{
    Q_D(const GeoPositionInfoSourceBb);
    return d->_replyErr;
}

QString GeoPositionInfoSourceBb::replyErrStr() const
{
    Q_D(const GeoPositionInfoSourceBb);
    return d->_replyErrStr;
}

QString GeoPositionInfoSourceBb::resetType() const
{
    Q_D(const GeoPositionInfoSourceBb);
    return d->_resetType;
}

void GeoPositionInfoSourceBb::requestReset( const QString & resetType )
{
    if ( validResetTypes.contains( resetType ) ) {
        Q_D(GeoPositionInfoSourceBb);
        d->_resetType = resetType;
        d->resetLocationProviders();
    }
}


} // namespace
} // namespace
} // namespace
