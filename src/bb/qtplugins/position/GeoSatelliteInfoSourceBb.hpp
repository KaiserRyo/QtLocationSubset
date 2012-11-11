/**
 * @copyright
 * Copyright Research In Motion Limited, 2012-2012
 * Research In Motion Limited. All rights reserved.
 */

#ifndef BB_QTPLUGINS_POSITION_GEOSATELLITEINFOSOURCEBB_H
#define BB_QTPLUGINS_POSITION_GEOSATELLITEINFOSOURCEBB_H

#include "qmobilitysubset.h"
#include <QGeoSatelliteInfoSource>

#include <QtCore/QString>
#include <QtCore/QScopedPointer>

namespace bb
{
namespace qtplugins
{
namespace position
{
/**
    @class GeoSatelliteInfoSourceBb
    @brief The GeoSatelliteInfoSourceBb class is for the distribution of positional updates obtained from the
           underlying Qnx Location Manager.

    GeoSatelliteInfoSourceBb is a subclass of QGeoSatelliteInfoSource.
    The static function QGeoSatelliteInfoSource::createDefaultSource() creates a default
    satellite data source that is appropriate for the platform, if one is
    available. On BB10 this is a GeoSatelliteInfoSourceBb instance. Otherwise, available
    QGeoPositionInfoSourceFactory plugins will
    be checked for one that has a satellite data source available.

    Call startUpdates() and stopUpdates() to start and stop regular updates,
    or requestUpdate() to request a single update.
    When an update is available, satellitesInViewUpdated() and/or
    satellitesInUseUpdated() will be emitted.


*/

class GeoSatelliteInfoSourceBbPrivate;
class GeoSatelliteInfoSourceBb : public QtMobilitySubset::QGeoSatelliteInfoSource
{
    Q_OBJECT

    // these properties extend QGeoSatelliteInfoSource allowing use of additional features of the Qnx Location Manager
    Q_PROPERTY(double period READ period WRITE setPeriod FINAL)
    Q_PROPERTY(bool backgroundMode READ backgroundMode WRITE setBackgroundMode FINAL)
    Q_PROPERTY(int responseTime READ responseTime WRITE setResponseTime FINAL)

public:
    /**
        Creates a satellite source with the specified @a parent.
    */
    explicit GeoSatelliteInfoSourceBb(QObject *parent = 0);

    /**
        Destroys the satellite source.
    */
    virtual ~GeoSatelliteInfoSourceBb();

    /**
        @property GeoSatelliteInfoSourceBb::period
        @brief The period of the location request, in seconds. A value of '0' indicates that this would be a one-time location request.
    */
    double period() const;
    void setPeriod( double period );

    /**
        @property GeoSatelliteInfoSourceBb::backgroundMode
        @brief This property determines whether or not requests are allowed to run with the device in standby (i.e. screen off)
    */
    bool backgroundMode() const;
    void setBackgroundMode( bool mode );

    /**
        @property GeoSatelliteInfoSourceBb::responseTime
        @brief This property specifies the desired response time of the fix, in seconds. A value of '0' disables response time criteria.
    */
    int responseTime() const;
    void setResponseTime( int responseTime );

public Q_SLOTS:
    /**
        Starts emitting updates at regular intervals. The updates will be
        provided whenever new satellite information becomes available.
    */
    void startUpdates();

    /**
        Stops emitting updates at regular intervals.
    */
    void stopUpdates();

    /*!
        Attempts to get the current satellite information and emit
        satellitesInViewUpdated() and satellitesInUseUpdated() with this
        information. If the current position cannot be found
        within the given \a timeout (in milliseconds), requestTimeout() is
        emitted.

        If the timeout is zero, the timeout defaults to a reasonable timeout
        period as appropriate for the source.

        This does nothing if another update request is in progress. However
        it can be called even if startUpdates() has already been called and
        regular updates are in progress.
    */
    void requestUpdate(int timeout = 0);

private:
    Q_DECLARE_PRIVATE(GeoSatelliteInfoSourceBb)
    Q_DISABLE_COPY(GeoSatelliteInfoSourceBb)
    QScopedPointer<GeoSatelliteInfoSourceBbPrivate> d_ptr;
};

} // namespace
} // namespace
} // namespace

#endif
