/**
 * @copyright
 * Copyright Research In Motion Limited, 2012-2012
 * Research In Motion Limited. All rights reserved.
 */

#ifndef BB_QTPLUGINS_POSITION_GEOPOSITIONINFOSOURCEBBPRIVATE_H
#define BB_QTPLUGINS_POSITION_GEOPOSITIONINFOSOURCEBBPRIVATE_H

#include "GeoPositionInfoSourceBb.hpp"
#include "qgeopositioninfo.h"

#include <QObject>
#include <QVariantMap>
#include <QTimer>
#include <QUrl>

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

class GeoPositionInfoSourceBbPrivate : public QObject
{
    Q_OBJECT
public:
    ~GeoPositionInfoSourceBbPrivate();

    void startUpdates();
    void stopUpdates();
    void requestUpdate(int msec);

    bool _startUpdatesInvoked;
    bool _requestUpdateInvoked;

private Q_SLOTS:
    void singleUpdateTimeout();
    void receivePeriodicPositionReply( );
    void receiveSinglePositionReply( );

private:
    Q_DECLARE_PUBLIC(GeoPositionInfoSourceBb)
    explicit GeoPositionInfoSourceBbPrivate(GeoPositionInfoSourceBb *parent);

    void periodicUpdatesTimeout();

    void emitPositionUpdated(const QtMobilitySubset::QGeoPositionInfo &update);
    bool requestPositionInfo( bool periodic );
    void cancelPositionInfo( bool periodic );
    void resetLocationProviders();
    QtMobilitySubset::QGeoPositionInfo lastKnownPosition(bool fromSatellitePositioningMethodsOnly) const;

    QVariantMap populateLocationRequest( bool periodic ) const;
    QVariantMap populateResetRequest() const;

    bool receivePositionReply( bb::PpsObject & ppsObject );

    bool _canEmitPeriodicUpdatesTimeout;

    GeoPositionInfoSourceBb * q_ptr;
    bb::PpsObject * _periodicUpdatePpsObject;
    bb::PpsObject * _singleUpdatePpsObject;
    QtMobilitySubset::QGeoPositionInfo _currentPosition;

    QTimer * _requestUpdateTimer;

    // these are Location Manager parameters that are represented by properties. These parameters represent
    // additional functionality beyond what is provided by the base class QGeoPositionInfoSource
    double _accuracy;
    double _responseTime;
    bool _canRunInBackground;
    QString _fixType;
    QString _appId;
    QString _appPassword;
    QUrl _pdeUrl;
    QUrl _slpUrl;
    QVariantMap _replyDat;
    QString _replyErr;
    QString _replyErrStr;
    QString _resetType;

};

} // namespace
} // namespace
} // namespace

#endif
