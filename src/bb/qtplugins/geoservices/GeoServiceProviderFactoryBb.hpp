/**
 * @copyright
 * Copyright Research In Motion Limited, 2012-2012
 * Research In Motion Limited. All rights reserved.
 */

#ifndef BB_QTPLUGINS_GEOSERVICES_GEOSERVICEPROVIDERFACTORYBB_HPP
#define BB_QTPLUGINS_GEOSERVICES_GEOSERVICEPROVIDERFACTORYBB_HPP

#include <QGeoServiceProviderFactory>

#include <QMap>
#include <QString>

namespace bb
{
namespace qtplugins
{
namespace geoservices
{

/**
 * Assigning a value to a pointee, but only if the pointer is non-null.
 *
 * @param pointer Pointer to the value to be set.
 * @param value Value to assign to the pointee.
 */
template <typename T>
void safeAssign( T * pointer, const T & value )
{
    if ( pointer ) {
        *pointer = value;
    }
}



class GeoServiceProviderFactoryBb : public QObject, public QtMobilitySubset::QGeoServiceProviderFactory
{
    Q_OBJECT
    Q_INTERFACES(QtMobilitySubset::QGeoServiceProviderFactory)

public:
    virtual ~GeoServiceProviderFactoryBb() {}

    virtual QString providerName() const;
    virtual int providerVersion() const;

    virtual QtMobilitySubset::QGeoSearchManagerEngine* createSearchManagerEngine(const QMap<QString, QVariant> &parameters,
            QtMobilitySubset::QGeoServiceProvider::Error *error,
            QString *errorString) const;
};

}
}
}

#endif
