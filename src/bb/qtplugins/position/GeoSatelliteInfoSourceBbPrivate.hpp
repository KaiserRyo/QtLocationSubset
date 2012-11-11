/**
 * @copyright
 * Copyright Research In Motion Limited, 2012-2012
 * Research In Motion Limited. All rights reserved.
 */

#ifndef BB_QTPLUGINS_POSITION_GEOSATELLITEINFOSOURCEBBPRIVATE_H
#define BB_QTPLUGINS_POSITION_GEOSATELLITEINFOSOURCEBBPRIVATE_H

#include "GeoSatelliteInfoSourceBb.hpp"
#include "qgeosatelliteinfo.h"

#include <QObject>
#include <QtCore/QVariantMap>
#include <QtCore/QTimer>

namespace bb
{
class PpsObject;
}

namespace bb
{
namespace qtplugins
{
namespace position
{

class GeoSatelliteInfoSourceBbPrivate : public QObject
{
    Q_OBJECT
public:
    ~GeoSatelliteInfoSourceBbPrivate();

    void startUpdates();
    void stopUpdates();
    void requestUpdate(int msec);

    bool _startUpdatesInvoked;
    bool _requestUpdateInvoked;

private Q_SLOTS:
    void singleUpdateTimeout();
    void receivePeriodicSatelliteReply( );
    void receiveSingleSatelliteReply( );

private:
    Q_DECLARE_PUBLIC(GeoSatelliteInfoSourceBb)
    explicit GeoSatelliteInfoSourceBbPrivate(GeoSatelliteInfoSourceBb *parent);

    void emitSatelliteUpdated(const QtMobilitySubset::QGeoSatelliteInfo &update);
    bool requestSatelliteInfo( bool periodic );
    void cancelSatelliteInfo( bool periodic );

    QVariantMap populateLocationRequest( bool periodic );
    void populateSatelliteLists( const QVariantMap & reply );

    bool receiveSatelliteReply( bb::PpsObject & ppsObject );

    GeoSatelliteInfoSourceBb * q_ptr;
    bb::PpsObject * _periodicUpdatePpsObject;
    bb::PpsObject * _singleUpdatePpsObject;
    QList<QtMobilitySubset::QGeoSatelliteInfo> _satellitesInUse;
    QList<QtMobilitySubset::QGeoSatelliteInfo> _satellitesInView;

    QTimer * _requestUpdateTimer;

    // properties (extension of QGeoSatelliteInfoSource for additional Location Manager features)
    double _period;
    bool _backgroundMode;
    int _responseTime;

};

} // namespace
} // namespace
} // namespace

#endif
