/**
 * @copyright
 * Copyright Research In Motion Limited, 2012-2012
 * Research In Motion Limited. All rights reserved.
 */

#ifndef BB_QTPLUGINS_POSITION_GEOPOSITIONINFOSOURCEBB_H
#define BB_QTPLUGINS_POSITION_GEOPOSITIONINFOSOURCEBB_H

#include "qmobilitysubset.h"
#include <QGeoPositionInfoSource>

#include <QString>
#include <QVariantMap>
#include <QScopedPointer>
#include <QUrl>

namespace bb
{
namespace qtplugins
{
namespace position
{
/**
    @class GeoPositionInfoSourceBb
    @brief The GeoPositionInfoSourceBb class is for the distribution of positional updates obtained from the
           underlying Qnx Location Manager.

    GeoPositionInfoSourceBb is a subclass of QGeoPositionInfoSource. The static function
    QGeoPositionInfoSource::createDefaultSource() creates a default
    position source that is appropriate for the platform, if one is available. On BB10 this is
    a GeoPositionInfoSourceBb instance.

    Users of a QGeoPositionInfoSource subclass can request the current position using
    requestUpdate(), or start and stop regular position updates using
    startUpdates() and stopUpdates(). When an update is available,
    positionUpdated() is emitted. The last known position can be accessed with
    lastKnownPosition().

    If regular position updates are required, setUpdateInterval() can be used
    to specify how often these updates should be emitted. If no interval is
    specified, updates are simply provided whenever they are available.
    For example:

    @code
        // Emit updates every 10 seconds if available
        QGeoPositionInfoSource *source = QGeoPositionInfoSource::createDefaultSource(0);
        if (source)
            source->setUpdateInterval(10000);
    @endcode

    To remove an update interval that was previously set, call
    setUpdateInterval() with a value of 0.

    Note that the position source may have a minimum value requirement for
    update intervals, as returned by minimumUpdateInterval().

*/

class GeoPositionInfoSourceBbPrivate;
class GeoPositionInfoSourceBb : public QtMobilitySubset::QGeoPositionInfoSource
{
    Q_OBJECT

    // these properties extend QGeoPositionInfoSource allowing use of additional features of the Qnx Location Manager
    // the following properties are the fields of the dat parameter of the location request
    Q_PROPERTY(double period READ period WRITE setPeriod FINAL)
    Q_PROPERTY(double accuracy READ accuracy WRITE setAccuracy FINAL)
    Q_PROPERTY(double responseTime READ responseTime WRITE setResponseTime FINAL)
    Q_PROPERTY(bool canRunInBackground READ canRunInBackground WRITE setCanRunInBackground FINAL)
    Q_PROPERTY(QString provider READ provider WRITE setProvider FINAL)
    Q_PROPERTY(QString fixType READ fixType WRITE setFixType FINAL)
    Q_PROPERTY(QString appId READ appId WRITE setAppId FINAL)
    Q_PROPERTY(QString appPassword READ appPassword WRITE setAppPassword FINAL)
    Q_PROPERTY(QUrl pdeUrl READ pdeUrl WRITE setPdeUrl FINAL)
    Q_PROPERTY(QUrl slpUrl READ slpUrl WRITE setSlpUrl FINAL)

    // the following read-only properties are the fields of the Location Manager generic reply
    Q_PROPERTY(QVariantMap replyDat READ replyDat FINAL)
    Q_PROPERTY(QString replyErr READ replyErr FINAL)
    Q_PROPERTY(QString replyErrStr READ replyErrStr FINAL)

    // when set, the following property causes a RESET request to be sent to all the location providers.
    Q_PROPERTY(QString reset READ resetType WRITE requestReset FINAL)

public:
    /**
        Creates a position source with the specified @a parent.
    */
    explicit GeoPositionInfoSourceBb(QObject *parent = 0);

    /**
        Destroys the position source.
    */
    virtual ~GeoPositionInfoSourceBb();

    /**
        If the update interval is not set (or is set to 0) the
        source will provide updates as often as necessary.

        If the update interval is set, the source will provide updates at an
        interval as close to the requested interval as possible. If the requested
        interval is less than the minimumUpdateInterval(),
        the minimum interval is used instead.

        Changes to the update interval will happen as soon as is practical, however the
        time the change takes may vary between implementations.  Whether or not the elapsed
        time from the previous interval is counted as part of the new interval is also
        implementation dependent.

        The default value for this property is 0.
    */
    void setUpdateInterval(int msec);

    /**
        Returns an update containing the last known position, or a null update
        if none is available.

        If @a fromSatellitePositioningMethodsOnly is true, this returns the last
        known position received from a satellite positioning method; if none
        is available, a null update is returned.
    */
    QtMobilitySubset::QGeoPositionInfo lastKnownPosition(bool fromSatellitePositioningMethodsOnly = false) const;

    /**
        Returns the positioning methods available to this source.

        @sa setPreferredPositioningMethods()
    */
    PositioningMethods supportedPositioningMethods() const;

    /**
        This is the minimum value accepted by setUpdateInterval() and
        requestUpdate().
    */
    int minimumUpdateInterval() const;

    /**
        @property GeoPositionInfoSourceBb::period
        @brief This property specifies the period of the location request, in seconds. A value of '0' indicates a one-time
        location request.
    */
    double period() const;
    void setPeriod( double period );

    /**
        @property GeoPositionInfoSourceBb::accuracy
        @brief This property specifies the desired accuracy of the fix, in meters. A value of '0' disables accuracy criteria.
    */
    double accuracy() const;
    void setAccuracy( double accuracy );

    /**
        @property GeoPositionInfoSourceBb::responseTime
        @brief This property specifies the desired response time of the fix, in seconds. A value of '0' disables response time criteria.
    */
    double responseTime() const;
    void setResponseTime( double responseTime );

    /**
        @property GeoPositionInfoSourceBb::canRunInBackground
        @brief This property determines whether or not requests are allowed to run with the device in standby (i.e. screen off)
    */
    bool canRunInBackground() const;
    void setCanRunInBackground( bool canRunInBackground );

    /**
        @property GeoPositionInfoSourceBb::provider
        @brief This property specifies the location provider you wish to use (hybrid, gnss, network).
    */
    QString provider() const;
    void setProvider( const QString & provider );

    /**
        @property GeoPositionInfoSourceBb::fixType
        @brief This property specifies the fix type you wish to use (best, gps_ms_based, gps_ms_assisted, gps_autonomous, cellsite, wifi).
    */
    QString fixType() const;
    void setFixType( const QString & fixType );

    /**
        @property GeoPositionInfoSourceBb::appId
        @brief This property specifies a special application id – if you don't know what this is, don't use it.
    */
    QString appId() const;
    void setAppId( const QString & appId );

    /**
        @property GeoPositionInfoSourceBb::appPassword
        @brief This property specifies a special application password, goes with appId above.
    */
    QString appPassword() const;
    void setAppPassword( const QString & appPassword );

    /**
        @property GeoPositionInfoSourceBb::pdeUrl
        @brief This property specifies the PDE URL (i.e. tcp://user:pass@address.dom:1234).
    */
    QUrl pdeUrl() const;
    void setPdeUrl( const QUrl & pdeUrl );

    /**
        @property GeoPositionInfoSourceBb::slpUrl
        @brief This property specifies the SLP URL (i.e. tcp://user:pass@address.dom:1234).
    */
    QUrl slpUrl() const;
    void setSlpUrl( const QUrl & slpUrl );

    /**
        @property GeoPositionInfoSourceBb::replyDat
        @brief This property specifies the object containing the reply data (such as latitude, longitude, satellites, etc).
        If the replyErr is not empty then replyDat may be empty or stale. replyDat is expected to be consumed in the slot
        connected to the positionUpdated() signal, otherwise its contents are undefined.
    */
    QVariantMap replyDat() const;

    /**
        @property GeoPositionInfoSourceBb::replyErr
        @brief If not empty this property indicates that an error has occurred, and identifies the error. replyErr is
        expected to be consumed in the slot connected to the updateTimeout() signal, which is emitted when an error
        is detected. Otherwise its contents are undefined.
    */
    QString replyErr() const;

    /**
        @property GeoPositionInfoSourceBb::replyErrStr
        @brief This property is not empty if and only if the replyErr parameter is present, it describes the error.
        replyErrStr is expected to be consumed in the slot connected to the updateTimeout() signal, which is emitted
        when an error is detected. Otherwise its contents are undefined.
    */
    QString replyErrStr() const;

    /**
        @property GeoPositionInfoSourceBb::reset
        @brief By setting this property a reset of all location providers is requested through the Location Manager.
        The value of reset specifies the type of reset to be performed. Valid reset types are "cold", "warm", "hot",
        and "factory". The reset is not actually carried out until position updates are restarted. The current value
        of this property, i.e. property("reset"), is not particularly useful, it is simply the reset type
        corresponding to the last time setProperty("reset", resetType) was called. A Qt property must have a READ
        method, hence the reason for defining resetType().
    */
    QString resetType( ) const;
    void requestReset( const QString & resetType );


public Q_SLOTS:
    /**
        Starts emitting updates at regular intervals as specified by setUpdateInterval().

        If setUpdateInterval() has not been called, the source will emit updates
        as soon as they become available.

        An updateTimout() signal will be emitted if this QGeoPositionInfoSource subclass determines
        that it will not be able to provide regular updates.  This could happen if a satellite fix is
        lost or if a hardware error is detected.  Position updates will recommence if the data becomes
        available later on.  The updateTimout() signal will not be emitted again until after the
        periodic updates resume.
    */
    void startUpdates();

    /**
        Stops emitting updates at regular intervals.
    */
    void stopUpdates();

    /**
        Attempts to get the current position and emit positionUpdated() with
        this information. If the current position cannot be found within the given @a timeout
        (in milliseconds) or if @a timeout is less than the value returned by
        minimumUpdateInterval(), updateTimeout() is emitted.

        If the timeout is zero, the timeout defaults to a reasonable timeout
        period as appropriate for the source.

        This does nothing if another update request is in progress. However
        it can be called even if startUpdates() has already been called and
        regular updates are in progress.

        If the source uses multiple positioning methods, it tries to gets the
        current position from the most accurate positioning method within the
        given timeout.
    */
    void requestUpdate(int timeout = 0);

private:
    Q_DECLARE_PRIVATE(GeoPositionInfoSourceBb)
    Q_DISABLE_COPY(GeoPositionInfoSourceBb)
    QScopedPointer<GeoPositionInfoSourceBbPrivate> d_ptr;
};

} // namespace
} // namespace
} // namespace

#endif
