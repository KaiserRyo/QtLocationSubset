/**
 * @copyright
 * Copyright Research In Motion Limited, 2012-2012
 * Research In Motion Limited. All rights reserved.
 */

#include "LocationManagerUtil.hpp"

#include <bb/PpsObject>

#include <QtCore/QVariantMap>
#include <QtCore/QByteArray>
#include <QtDebug>

#include <errno.h>

namespace {

// Create a QVariantMap suitable for writing to a PpsObject specifying a cancel request to the Location Manager.
QVariantMap createCancelRequest()
{
    QVariantMap map;

    map.insert( "msg", "cancel" );
    map.insert( "id", bb::qtplugins::position::global::libQtLocationSubsetId );

    return map;
}

} // unnamed namespace

namespace bb
{
namespace qtplugins
{
namespace position
{

namespace global {

const QString libQtLocationSubsetId = "libQtLocationSubset";
const QString locationManagerPpsFile = "/pps/services/geolocation/control";
const int minUpdateInterval = 1000;
const QVariantMap cancelRequest = createCancelRequest();

} // namespace global


// send a generic server-mode request, wrapped in a @control map, to ppsObject
bool sendRequest( bb::PpsObject & ppsObject, const QVariantMap & request )
{
    if ( !ppsObject.isOpen() ) {
        if ( !ppsObject.open() ) {
            qWarning() << "LocationManagerUtil.cpp:sendRequest(): error opening pps object, errno =" << ppsObject.error() << "(" <<
                    strerror(ppsObject.error()) << "). Clients should verify that they have read_geolocation permission.";
            return false;
        }
    }

    // wrap the request in a @control map
    QVariantMap map;
    map.insert( "@control", request );


    // encode it
    bool ok;
    QByteArray encodedRequest = bb::PpsObject::encode( map, &ok );
    if ( !ok ) {
        qWarning() << "LocationManagerUtil.cpp:sendRequest(): error encoding position request";
        ppsObject.close();
        return false;
    }

    // write it
    bool success = ppsObject.write( encodedRequest );
    if ( !success ) {
        qWarning() << "LocationManagerUtil.cpp:sendRequest(): error" << ppsObject.error() << "writing position request";
        ppsObject.close();
        return false;
    }

    return true;
}

// receive a generic server-mode reply from ppsObject, removing the @control map container
bool receiveReply( QVariantMap * reply, bb::PpsObject & ppsObject )
{
    if ( !ppsObject.isOpen() ) {
        if ( !ppsObject.open() ) {
            qWarning() << "LocationManagerUtil.cpp:receiveReply(): error opening pps object";
            return false;
        }
    }

    // read the reply
    bool ok;
    QByteArray encodedReply = ppsObject.read( &ok );
    if ( !ok ) {
        qWarning() << "LocationManagerUtil.cpp:receiveReply(): error" << ppsObject.error() << "reading position reply";
        ppsObject.close();
        return false;
    }

    // decode the reply
    *reply = bb::PpsObject::decode( encodedReply, &ok );
    if ( !ok ) {
        qWarning() << "LocationManagerUtil.cpp:receiveReply(): error decoding position reply";
        ppsObject.close();
        return false;
    }

    // peel out the control map from the reply
    *reply = reply->value("@control").toMap();

    // check for an error in the reply
    if ( reply->contains("err") ) {
        qWarning() << "LocationManagerUtil.cpp:receiveReply():" <<
                reply->value("err").toString().toLocal8Bit().constData() << ":" <<
                reply->value("errstr").toString().toLocal8Bit().constData();
        return false;
    }

    return true;
}

} // namespace
} // namespace
} // namespace
