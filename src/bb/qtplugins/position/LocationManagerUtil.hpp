/**
 * @copyright
 * Copyright Research In Motion Limited, 2012-2012
 * Research In Motion Limited. All rights reserved.
 */

#ifndef BB_QTPLUGINS_POSITION_QGEOLOCATIONMANAGERUTIL_H
#define BB_QTPLUGINS_POSITION_QGEOLOCATIONMANAGERUTIL_H

#include <QtCore/QVariantMap>

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

namespace global {

/* the libQtLocationSubset id for the server-mode accessed pps file id field */
extern const QString libQtLocationSubsetId;

/* the path to the location manager pps file that is the gateway for positioning requests/replies */
extern const QString locationManagerPpsFile;

/* the minimum interval (in msec) that positional and satellite updates can be provided for */
extern const int minUpdateInterval;

// a QVariantMap suitable for writing to a PpsObject specifying a cancel request to the Location Manager.
// This cancels the current request
extern const QVariantMap cancelRequest;

} // namespace global

// send a generic server-mode request, wrapped in a @control map, to ppsObject
bool sendRequest( bb::PpsObject & ppsObject, const QVariantMap & request );

// receive a generic server-mode reply from ppsObject, removing the @control map container
bool receiveReply( QVariantMap * reply, bb::PpsObject & ppsObject );

} // namespace
} // namespace
} // namespace

#endif
