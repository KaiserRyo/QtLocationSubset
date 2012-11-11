/**
 * @copyright
 * Copyright Research In Motion Limited, 2012-2012
 * Research In Motion Limited. All rights reserved.
 */

#include "GeoServiceProviderFactoryBb.hpp"
#include "GeoSearchManagerEngineBb.hpp"

#include <QtPlugin>

namespace
{
const QString BbGeoServicesName = "BbGeoServices";
const int BbGeoServicesVersion = 1;
}

namespace bb
{
namespace qtplugins
{
namespace geoservices
{

QString GeoServiceProviderFactoryBb::providerName() const
{
    return BbGeoServicesName;
}

int GeoServiceProviderFactoryBb::providerVersion() const
{
    return BbGeoServicesVersion;
}

/*!
    Returns a new QGeoSearchManagerEngine instance, initialized with \a
    parameters, which implements as much of the places searching functionality
    as the service provider supports.

    If \a error is not 0 it should be set to QGeoServiceProvider::NoError on
    success or an appropriate QGeoServiceProvider::Error on failure.

    If \a errorString is not 0 it should be set to a string describing any
    error which occurred.

    The default implementation returns 0, which causes a
    QGeoServiceProvider::NotSupportedError in QGeoServiceProvider.
*/
QtMobilitySubset::QGeoSearchManagerEngine* GeoServiceProviderFactoryBb::createSearchManagerEngine(const QMap<QString, QVariant> &parameters,
        QtMobilitySubset::QGeoServiceProvider::Error *error,
        QString *errorString) const
{
    safeAssign( error, QtMobilitySubset::QGeoServiceProvider::NoError );
    safeAssign( errorString, QString() );

    return new GeoSearchManagerEngineBb( parameters );
}

}
}
}

Q_EXPORT_PLUGIN2(bbgeosearch, bb::qtplugins::geoservices::GeoServiceProviderFactoryBb)
