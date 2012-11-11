/**
 * @copyright
 * Copyright Research In Motion Limited, 2012-2012
 * Research In Motion Limited. All rights reserved.
 */

#include "GeoPositionInfoSourceFactoryBb.hpp"
#include "GeoPositionInfoSourceBb.hpp"
#include "GeoSatelliteInfoSourceBb.hpp"

#include <QGeoPositionInfoSource>
#include <QGeoSatelliteInfoSource>

#include <QtPlugin>

#include <iostream>


namespace bb
{
namespace qtplugins
{
namespace position
{

///////////////////////////
//
// GeoPositionInfoSourceFactoryBb
//
///////////////////////////

GeoPositionInfoSourceFactoryBb::~GeoPositionInfoSourceFactoryBb()
{
}

QtMobilitySubset::QGeoPositionInfoSource *GeoPositionInfoSourceFactoryBb::positionInfoSource(QObject *parent)
{
    return new GeoPositionInfoSourceBb(parent);
}

QtMobilitySubset::QGeoSatelliteInfoSource *GeoPositionInfoSourceFactoryBb::satelliteInfoSource(QObject *parent)
{
    return new GeoSatelliteInfoSourceBb(parent);
}

QString GeoPositionInfoSourceFactoryBb::sourceName() const
{
    return QString("BbPositionAndSatellite");
}

int GeoPositionInfoSourceFactoryBb::sourceVersion() const
{
    return 1;
}

int GeoPositionInfoSourceFactoryBb::sourcePriority() const
{
    return INT_MAX;
}

} // namespace
} // namespace
} // namespace

Q_EXPORT_PLUGIN2(bbposition, bb::qtplugins::position::GeoPositionInfoSourceFactoryBb)
