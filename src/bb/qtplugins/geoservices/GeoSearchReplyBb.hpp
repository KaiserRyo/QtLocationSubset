/**
 * @copyright
 * Copyright Research In Motion Limited, 2012-2012
 * Research In Motion Limited. All rights reserved.
 */

#ifndef BB_QTPLUGINS_GEOSERVICES_GEOSEARCHREPLYBB_H
#define BB_QTPLUGINS_GEOSERVICES_GEOSEARCHREPLYBB_H

#include "private/bbmock/GeoregApi.hpp"

#include <bb/PpsObject>

#include <QGeoSearchReply>
#include <QGeoCoordinate>
#include <QGeoAddress>
#include <QGeoBoundingArea>
#include <QGeoBoundingBox>
#include <QGeoBoundingCircle>
#include <QFutureWatcher>
#include <QFuture>

#include <QObject>
#include <QList>

namespace bb
{
namespace qtplugins
{
namespace geoservices
{

typedef struct GeoregReplyStruct
{
    QtMobilitySubset::QGeoSearchReply::Error error;
    QList<QtMobilitySubset::QGeoPlace> places;

} GeoregReply;


class GeoSearchReplyBb : public QtMobilitySubset::QGeoSearchReply
{
    Q_OBJECT

public:
    // create a search reply for a geocode request
    GeoSearchReplyBb( const QtMobilitySubset::QGeoAddress &address,
                     const QtMobilitySubset::QGeoBoundingArea * bounds,
                     QObject * parent = 0 );
    // create a search reply for a reverse geocode request
    GeoSearchReplyBb( const QtMobilitySubset::QGeoCoordinate &coordinate,
                     geo_search_boundary_t boundary,
                     const QtMobilitySubset::QGeoBoundingArea * bounds,
                     QObject * parent = 0 );

    virtual ~GeoSearchReplyBb();

    bool initialize( const QtMobilitySubset::QGeoBoundingArea *bounds );
    void boundPlaces( const QList<QtMobilitySubset::QGeoPlace> & unboundPlaces );
    void setBounds( const QtMobilitySubset::QGeoBoundingArea *bounds );
    void finishReply( QtMobilitySubset::QGeoSearchReply::Error error );

public Q_SLOTS:
    void receiveReply();


private:
    Q_DISABLE_COPY(GeoSearchReplyBb)

    QFuture<GeoregReply> _future;
    QFutureWatcher<GeoregReply> _futureWatcher;

    QtMobilitySubset::QGeoBoundingArea * _bounds;
    QtMobilitySubset::QGeoBoundingBox _boundingBox;
    QtMobilitySubset::QGeoBoundingCircle _boundingCircle;
};

} // namespace
} // namespace
} // namespace

#endif
