/**
 * @copyright
 * Copyright Research In Motion Limited, 2012-2012
 * Research In Motion Limited. All rights reserved.
 */

#ifndef GEOPOSITIONINFOSOURCEFACTORYBB_H
#define GEOPOSITIONINFOSOURCEFACTORYBB_H

#include <bb/Global>

#include <QGeoPositionInfoSourceFactory>

#include <QObject>

namespace bb
{
namespace qtplugins
{
namespace position
{

class GeoPositionInfoSourceFactoryBb : public QObject, public QtMobilitySubset::QGeoPositionInfoSourceFactory
{
    Q_OBJECT
    Q_INTERFACES(QtMobilitySubset::QGeoPositionInfoSourceFactory)

public:
    virtual ~GeoPositionInfoSourceFactoryBb();

    virtual QString sourceName() const;
    virtual int sourceVersion() const;
    virtual int sourcePriority() const;

    virtual QtMobilitySubset::QGeoPositionInfoSource *positionInfoSource(QObject *parent);
    virtual QtMobilitySubset::QGeoSatelliteInfoSource *satelliteInfoSource(QObject *parent);
};

} // namespace
} // namespace
} // namespace

#endif // include guard
