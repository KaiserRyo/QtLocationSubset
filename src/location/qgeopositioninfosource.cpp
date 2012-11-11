/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include <qgeopositioninfosource.h>
#include "qgeopositioninfosourcefactory.h"

#include <QPluginLoader>
#include <QStringList>
#include <QSettings>
#include <QCryptographicHash>
#include "qmobilitypluginsearch.h"

#if defined(Q_OS_SYMBIAN)
#   include "qgeopositioninfosource_s60_p.h"
#elif defined(QT_SIMULATOR)
#   include "qgeopositioninfosource_simulator_p.h"
#elif defined(Q_OS_WINCE)
#   include "qgeopositioninfosource_wince_p.h"
#elif defined(Q_WS_MAEMO_6)
#   include "qgeopositioninfosource_maemo_p.h"
#elif defined(Q_WS_MAEMO_5)
#   include "qgeopositioninfosource_maemo5_p.h"
#endif

#if defined (Q_WS_MEEGO)
#include "qgeopositioninfosource_maemo_p.h"
#if defined (GEOCLUE_MASTER_AVAILABLE)
#include "qgeopositioninfosource_geocluemaster_p.h"
#endif
#endif

QTMS_BEGIN_NAMESPACE

/*!
    \class QGeoPositionInfoSource
    \brief The QGeoPositionInfoSource class is an abstract base class for the distribution of positional updates.

    \inmodule QtLocationSubset
    \since 1.0

    \ingroup location
        \headerfile qgeopositioninfosource.cpp <QtLocationSubset/QGeoPositionInfoSource>
    @xmlonly
    <apigrouping group="Location/Positioning and Geocoding"/>
    @endxmlonly

    The static function QGeoPositionInfoSource::createDefaultSource() creates a default
    position source on the BB10 platform.

    Users of a QGeoPositionInfoSource subclass can request the current position using
    requestUpdate(), or start and stop regular position updates using
    startUpdates() and stopUpdates(). When an update is available,
    positionUpdated() is emitted. The last known position can be accessed with
    lastKnownPosition().

    If regular position updates are required, setUpdateInterval() can be used
    to specify how often these updates should be emitted. If no interval is
    specified, updates are simply provided whenever they are available.
    For example:

    \code
        // Emit updates every 10 seconds if available
        QGeoPositionInfoSource *source = QGeoPositionInfoSource::createDefaultSource(0);
        if (source)
            source->setUpdateInterval(10000);
    \endcode

    To remove an update interval that was previously set, call
    setUpdateInterval() with a value of 0.

    Note that the position source may have a minimum value requirement for
    update intervals, as returned by minimumUpdateInterval().

    Users of a BB10 QGeoPositionInfoSource subclass (obtained from QGeoPositionInfoSource::createDefaultSource())
    can access the underlying backend (BB10 Location Manager) for additional functionality via the Qt property system.

    The following properties extend QGeoPositionInfoSource allowing use of additional features of the BB10 Location Manager.
    They correspond to the fields of the dat parameter of the location request. Set these properties prior to calling startUpdates()
    or requestUpdate().

        \property period
        \brief This property specifies the period of the location request, in seconds. A value of '0' indicates a one-time
        location request. Setting \a period is equivalent to setUpdateInterval(), apart from the units.
        \code
        source->setProperty( "period", 5.0 );
        \endcode

        \property accuracy
        \brief This property specifies the desired accuracy of the fix, in meters. A value of '0' disables accuracy criteria.
        \code
        source->setProperty( "accuracy", 2.0 );
        \endcode

        \property responseTime
        \brief This property specifies the desired response time of the fix, in seconds. A value of '0' disables response time criteria.
        \code
        source->setProperty( "responseTime", 10.0 );
        \endcode

        \property canRunInBackground
        \brief This property determines whether or not requests are allowed to run with the device in standby (i.e. screen off)
        \code
        source->setProperty( "canRunInBackground", true );
        \endcode

        \property provider
        \brief This property specifies the location provider you wish to use (hybrid, gnss, network). Setting \a provider is
        equivalent to setPreferredPositioningMethods().
        \code
        source->setProperty( "provider", "hybrid" );    // equivalent to QGeoPositionInfoSource::AllPositioningMethods
        source->setProperty( "provider", "gnss" );      // equivalent to QGeoPositionInfoSource::SatellitePositioningMethods
        source->setProperty( "provider", "network" );   // equivalent to QGeoPositionInfoSource::NonSatellitePositioningMethods
        \endcode

        \property fixType
        \brief This property specifies the fix type you wish to use (best, gps_ms_based, gps_ms_assisted, gps_autonomous, cellsite, wifi).
        Valid values depend on which provider is set.
        \code
        source->setProperty( "provider", "network" );
        source->setProperty( "fixType", "wifi" );
        \endcode

        \property appId
        \brief This property specifies a special application id.
        \code
        source->setProperty( "appId", "myId" );
        source->setProperty( "appPassword", "myPassword" );
        \endcode

        \property appPassword
        \brief This property specifies a special application password, goes with appId above.

        \property pdeUrl
        \brief This property specifies the PDE URL (i.e. tcp://user:pass@address.dom:1234).
        \code
        QUrl url("tcp://user:pass@address.dom:1234");
        source->setProperty( "pdeUrl", url );
        \endcode

        \property slpUrl
        \brief This property specifies the SLP URL (i.e. tcp://user:pass@address.dom:1234).
        QUrl url("tcp://user:pass@address.dom:1234");
        source->setProperty( "slpUrl", url );


    The following read-only properties are the fields of the BB10 Location Manager generic reply.

        \property replyDat
        \brief This property specifies the object containing the reply data (such as latitude, longitude, satellites, etc)
        exactly as output by the Location Manager. If the \a replyErr property is not empty then replyDat may be empty or
        stale. The \a replyDat property should be queried in the slot connected to the
        QGeoPositionInfoSource::positionUpdated() signal. Otherwise if it is queried at some other execution point then
        its contents are undefined. The \a replyDat property is represented by a QVariantMap that consists of a number
        of key/value pairs providing data relevant to the position update (see the Location Manager protocol). One of the
        QVariantMap values is an array of satellite data pertinent to the position update. This data is stored as a
        QVariantList of QVariantMap elements. Accessing positional data from within the slot can be done like this:
        \code
        double hdop = 20.0; // horizontal dilution of precision
        QVariant variant = source->property("replyDat");
        if ( variant.isValid() ) {
            // the replyDat property is a QVariantMap
            QVariantMap rawDat = variant.toMap();
            QVariant field = rawDat.value( "hdop" );
            if ( field.isValid() ) {
                hdop = field.toDouble();
            }
        }
        \endcode
        Accessing satellite data can be done like this:
        \code
        // This example gets the carrier-to-noise ratio for the first satellite in the list.
        double cno = 0.0; // satellite carrier-to-noise ratio
        QVariant variant = source->property("replyDat");
        if ( variant.isValid() ) {
            // the replyDat property is a QVariantMap
            QVariantMap rawDat = variant.toMap();
            QVariant field = rawDat.value( "satellites" );
            if ( field.isValid() ) {
                // the satellites data is stored in a QVariantList
                QVariantList satArray = field.toList();
                // each element in the list is a QVariantMap corresponding to info for a single satellite.
                // Here we just access the first satellite in the list
                QVariantMap satInfo = satArray.at(0).toMap();
                QVariant satField = satInfo.value( "cno" );
                cno = satField.toDouble();
            }
        }
        \endcode

        \property replyErr
        \brief If not empty this property indicates that an error has occurred, and identifies the error.
        The \a replyErr property should be queried in the slot connected to the QGeoPositionInfoSource::updateTimeout()
        signal, which is emitted when an error is detected. Otherwise if it is queried at some other
        execution point then its contents are undefined. Accessing error information from within the slot
        can be done like this:
        \code
        // check for error
        QString err = positionInfoSource->property("replyErr").toString();
        if ( !err.isEmpty() ) {
            err.append( ": " );
            // append the error description, if any
            err.append( positionInfoSource->property("replyErrStr").toString() );
            std::cout << "Location Manager Error: " << err.toLocal8Bit().constData() << std::endl;
        }
        \endcode

        \property replyErrStr
        \brief This property is not empty if and only if the \a replyErr property is not empty, it describes the error.
        The \a replyErrStr property should be queried in the slot connected to the QGeoPositionInfoSource::updateTimeout()
        signal, which is emitted when an error is detected. Otherwise if it is queried at some other
        execution point then its contents are undefined.

    When set, the following property causes a RESET request to be sent to all the location providers.

        \property reset
        \brief By setting this property a reset of all location providers is requested through the Location Manager.
        The value of reset specifies the type of reset to be performed. Valid reset types are "cold", "warm", "hot",
        and "factory". The reset is not actually carried out until position updates are restarted.
        \code
        positionInfoSource->setProperty("reset", "hot");
        positionInfoSource->requestUpdate();
        \endcode

*/

/*!
    \enum QGeoPositionInfoSource::PositioningMethod
    Defines the types of positioning methods.

    \value SatellitePositioningMethods Satellite-based positioning methods such as GPS.
    \value NonSatellitePositioningMethods Other positioning methods.
    \value AllPositioningMethods A flag that matches all positioning methods.
*/

class QGeoPositionInfoSourcePrivate
{
public:
    int interval;
    QGeoPositionInfoSource::PositioningMethods methods;

    static QList<QGeoPositionInfoSourceFactory*> pluginsSorted();
    static QHash<QString, QGeoPositionInfoSourceFactory*> plugins(bool reload = false);
    static void loadDynamicPlugins(QHash<QString, QGeoPositionInfoSourceFactory*> &plugins);
    static void loadStaticPlugins(QHash<QString, QGeoPositionInfoSourceFactory*> &plugins);
};

QHash<QString, QGeoPositionInfoSourceFactory*> QGeoPositionInfoSourcePrivate::plugins(bool reload)
{
    static QHash<QString, QGeoPositionInfoSourceFactory*> plugins;
    static bool alreadyDiscovered = false;

    if (reload == true)
        alreadyDiscovered = false;

    if (!alreadyDiscovered) {
        loadStaticPlugins(plugins);
        loadDynamicPlugins(plugins);
        alreadyDiscovered = true;
    }
    return plugins;
}

static bool pluginComparator(const QGeoPositionInfoSourceFactory *p1, const QGeoPositionInfoSourceFactory *p2)
{
    return (p1->sourcePriority() > p2->sourcePriority());
}

QList<QGeoPositionInfoSourceFactory*> QGeoPositionInfoSourcePrivate::pluginsSorted()
{
    QList<QGeoPositionInfoSourceFactory*> list = plugins().values();
    qStableSort(list.begin(), list.end(), pluginComparator);
    return list;
}

void QGeoPositionInfoSourcePrivate::loadDynamicPlugins(QHash<QString, QGeoPositionInfoSourceFactory *> &plugins)
{
    QStringList paths;
    paths << mobilityPlugins(QLatin1String("position_subset"));

    QPluginLoader qpl;
    QString blockName;

    QSettings settings(QSettings::SystemScope, QLatin1String("Nokia"), QLatin1String("QtLocationPosAndSat"));
    QVariant value = settings.value("position.plugin.operator.whitelist");
    if (value.isValid()) {
        QStringList parts = value.toString().split(",");
        if (parts.size() == 4) {
            QFile file(parts.at(1));
            file.open(QIODevice::ReadOnly);

            QCryptographicHash hash(QCryptographicHash::Sha1);
            while (!file.atEnd()) {
                QByteArray data = file.read(4096);
                hash.addData(data);
            }
            file.close();

            QByteArray hexHash = hash.result().toHex();

            bool loadIt = true;
            if (QString::number(file.size()) != parts.at(3)) {
                qCritical("Position info plugin: bad plugin size for %s",
                          qPrintable(parts.at(1)));
                qWarning("Will fall back to platform default");
                loadIt = false;
            }

            if (hexHash != parts.at(2).toLatin1()) {
                qCritical("Position info plugin: bad plugin hash for %s",
                          qPrintable(parts.at(1)));
                qWarning("Will fall back to platform default");
                loadIt = false;
            }

            if (loadIt) {
                qpl.setFileName(parts.at(1));
                QGeoPositionInfoSourceFactory *f =
                        qobject_cast<QGeoPositionInfoSourceFactory*>(qpl.instance());

                if (f) {
                    QString name = f->sourceName();
                    if (name == parts.at(0)) {
                        plugins.insert(name, f);
                    } else {
                        qCritical("Position info plugin: bad plugin name for %s",
                                  qPrintable(parts.at(1)));
                        qWarning("Will fall back to platform default");
                    }
                }
            }

            // still set blockName to ensure the plugin doesn't load
            blockName = parts.at(1);
        } else {
            qWarning("Position plugin whitelist: invalid format -- should be key,filename,hash,size");
        }
    }

    for (int i = 0; i < paths.count(); ++i) {
        if (paths.at(i) != blockName) {
            qpl.setFileName(paths.at(i));

            QGeoPositionInfoSourceFactory *f =
                    qobject_cast<QGeoPositionInfoSourceFactory*>(qpl.instance());
            if (f) {
                QString name = f->sourceName();

    #if !defined QT_NO_DEBUG
                const bool showDebug = qgetenv("QT_DEBUG_PLUGINS").toInt() > 0;
                if (showDebug)
                    qDebug("Dynamic: found a service provider plugin with name %s", qPrintable(name));
    #endif
                plugins.insertMulti(name, f);
            }
        }
    }
}

void QGeoPositionInfoSourcePrivate::loadStaticPlugins(QHash<QString, QGeoPositionInfoSourceFactory *> &plugins)
{
#if !defined QT_NO_DEBUG
    const bool showDebug = qgetenv("QT_DEBUG_PLUGINS").toInt() > 0;
#endif

    QObjectList staticPlugins = QPluginLoader::staticInstances();
    for (int i = 0; i < staticPlugins.count(); ++i) {
        QGeoPositionInfoSourceFactory *f =
                qobject_cast<QGeoPositionInfoSourceFactory*>(staticPlugins.at(i));

        if (f) {
            QString name = f->sourceName();

#if !defined QT_NO_DEBUG
            if (showDebug)
                qDebug("Static: found a service provider plugin with name %s", qPrintable(name));
#endif
            if (!name.isEmpty()) {
                plugins.insertMulti(name, f);
            }
        }

    }
}

/*!
    Creates a position source with the specified \a parent.
*/

QGeoPositionInfoSource::QGeoPositionInfoSource(QObject *parent)
        : QObject(parent),
        d(new QGeoPositionInfoSourcePrivate)
{
    d->interval = 0;
    d->methods = 0;
}

/*!
    Destroys the position source.
*/
QGeoPositionInfoSource::~QGeoPositionInfoSource()
{
    delete d;
}

/*!
    \property QGeoPositionInfoSource::updateInterval
    \brief This property holds the requested interval in milliseconds between each update.

    If the update interval is not set (or is set to 0) the
    source will provide updates as often as necessary.

    If the update interval is set, the source will provide updates at an
    interval as close to the requested interval as possible. If the requested
    interval is less than the minimumUpdateInterval(),
    the minimum interval is used instead.

    Changes to the update interval will happen as soon as is practical, however the
    time the change takes may vary between implementations.  Whether or not the elapsed
    time from the previous interval is counted as part of the new interval is also
    implementation dependent.

    The default value for this property is 0.

    Note: Subclass implementations must call the base implementation of
    setUpdateInterval() so that updateInterval() returns the correct value.
*/
void QGeoPositionInfoSource::setUpdateInterval(int msec)
{
    d->interval = msec;
}

int QGeoPositionInfoSource::updateInterval() const
{
    return d->interval;
}

/*!
    Sets the preferred positioning methods for this source to \a methods.

    If \a methods includes a method that is not supported by the source, the
    unsupported method will be ignored.

    If \a methods does not include any methods supported by the source, the
    preferred methods will be set to the set of methods which the source supports.

    \bold {Note:} When reimplementing this method, subclasses must call the
    base method implementation to ensure preferredPositioningMethods() returns the correct value.

    \sa supportedPositioningMethods()
*/
void QGeoPositionInfoSource::setPreferredPositioningMethods(PositioningMethods methods)
{
    d->methods = methods & supportedPositioningMethods();
    if (d->methods == 0) {
        d->methods = supportedPositioningMethods();
    }
}

/*!
    Returns the positioning methods set by setPreferredPositioningMethods().
*/
QGeoPositionInfoSource::PositioningMethods QGeoPositionInfoSource::preferredPositioningMethods() const
{
    return d->methods;
}

/*!
    Creates and returns a position source with the given \a parent that
    reads from the system's default sources of location data, or the plugin
    with the highest available priority.

    Returns 0 if the system has no default position source and no valid plugins
    could be found.

    Note: BB10 applications require the read_geolocation permission 
    otherwise createDefaultSource() returns 0.
*/

QGeoPositionInfoSource *QGeoPositionInfoSource::createDefaultSource(QObject *parent)
{
    QSettings pluginSettings(QSettings::SystemScope, QLatin1String("Nokia"), QLatin1String("QtLocationPosAndSat"));
    QVariant value = pluginSettings.value("position.plugin.operator.whitelist");
    if (value.isValid()) {
        QStringList parts = value.toString().split(",");
        if (parts.size() == 4) {
            QGeoPositionInfoSource *source = createSource(parts.at(0), parent);
            if (source)
                return source;
        }
    }

#if defined(Q_OS_SYMBIAN)
    QGeoPositionInfoSource *ret = NULL;
    TRAPD(error, QT_TRYCATCH_LEAVING(ret = CQGeoPositionInfoSourceS60::NewL(parent)));
    if (error == KErrNone)
        return ret;
#elif defined(QT_SIMULATOR)
    return new QGeoPositionInfoSourceSimulator(parent);
#elif defined(Q_OS_WINCE)
    return new QGeoPositionInfoSourceWinCE(parent);
#elif (defined(Q_WS_MAEMO_6)) || (defined(Q_WS_MAEMO_5))
    QGeoPositionInfoSourceMaemo *source = new QGeoPositionInfoSourceMaemo(parent);
    int status = source->init();
    if (status != -1)
        return source;
    else
        delete source;
#elif defined(Q_WS_MEEGO)
    // Use Maemo6 backend if its available, otherwise use Geoclue backend
    QSettings maemo6Settings(QSettings::UserScope, QLatin1String("Nokia"), QLatin1String("QtLocationPosAndSatMaemo6"));
    if (!maemo6Settings.value("maemo6positioningavailable").isValid()) {
        QGeoPositionInfoSourceMaemo* maemo6Source = new QGeoPositionInfoSourceMaemo(parent);
        int status = maemo6Source->init();
        if (status == -1) {
            delete maemo6Source;
            maemo6Source = 0;
            maemo6Settings.setValue("maemo6positioningavailable", false);
        } else {
            return maemo6Source;
        }
    }
#ifdef GEOCLUE_MASTER_AVAILABLE
    QGeoPositionInfoSourceGeoclueMaster *geoclueSource = new QGeoPositionInfoSourceGeoclueMaster(parent);
    int status = geoclueSource->init();
    if (status >= 0)
       return geoclueSource;
    delete geoclueSource;
#endif // GEOCLUE_MASTER_AVAILABLE
#endif
    // no good platform source, try plugins
    Q_FOREACH (QGeoPositionInfoSourceFactory *f, QGeoPositionInfoSourcePrivate::pluginsSorted()) {
        QGeoPositionInfoSource *src = f->positionInfoSource(parent);
        if (src)
            return src;
    }

    return 0;
}


/*!
    Creates and returns a position source with the given \a parent,
    by loading the plugin named \a sourceName.

    Returns 0 if the plugin cannot be found.
*/
QGeoPositionInfoSource *QGeoPositionInfoSource::createSource(const QString &sourceName, QObject *parent)
{
    QGeoPositionInfoSourceFactory *f = QGeoPositionInfoSourcePrivate::plugins().value(sourceName);
    if (f) {
        QGeoPositionInfoSource *src = f->positionInfoSource(parent);
        if (src)
            return src;
    }
    return 0;
}


/*!
    Returns a list of available source plugins. Note that this list does not
    include the default platform backend, if one is available.
*/
QStringList QGeoPositionInfoSource::availableSources()
{
    return QGeoPositionInfoSourcePrivate::plugins().keys();
}

/*!
    \fn QGeoPositionInfo QGeoPositionInfoSource::lastKnownPosition(bool fromSatellitePositioningMethodsOnly = false) const = 0;

    Returns an update containing the last known position, or a null update
    if none is available.

    If \a fromSatellitePositioningMethodsOnly is true, this returns the last
    known position received from a satellite positioning method; if none
    is available, a null update is returned.
*/

/*!
    \fn virtual PositioningMethods QGeoPositionInfoSource::supportedPositioningMethods() const = 0;

    Returns the positioning methods available to this source.

    \sa setPreferredPositioningMethods()
*/


/*!
    \property QGeoPositionInfoSource::minimumUpdateInterval
    \brief This property holds the minimum time (in milliseconds) required to retrieve a position update.

    This is the minimum value accepted by setUpdateInterval() and
    requestUpdate().
*/


/*!
    \fn virtual void QGeoPositionInfoSource::startUpdates() = 0;

    Starts emitting updates at regular intervals as specified by setUpdateInterval().

    If setUpdateInterval() has not been called, the source will emit updates
    as soon as they become available.

    An updateTimout() signal will be emitted if this QGeoPositionInfoSource subclass determines
    that it will not be able to provide regular updates.  This could happen if a satellite fix is
    lost or if a hardware error is detected.  Position updates will recommence if the data becomes
    available later on.  The updateTimout() signal will not be emitted again until after the
    periodic updates resume.
*/

/*!
    \fn virtual void QGeoPositionInfoSource::stopUpdates() = 0;

    Stops emitting updates at regular intervals.
*/

/*!
    \fn virtual void QGeoPositionInfoSource::requestUpdate(int timeout = 0);

    Attempts to get the current position and emit positionUpdated() with
    this information. If the current position cannot be found within the given \a timeout
    (in milliseconds) or if \a timeout is less than the value returned by
    minimumUpdateInterval(), updateTimeout() is emitted.

    If the timeout is zero, the timeout defaults to a reasonable timeout
    period as appropriate for the source.

    This does nothing if another update request is in progress. However
    it can be called even if startUpdates() has already been called and
    regular updates are in progress.

    If the source uses multiple positioning methods, it tries to get the
    current position from the most accurate positioning method within the
    given timeout.
*/

/*!
    \fn void QGeoPositionInfoSource::positionUpdated(const QGeoPositionInfo &update);

    If startUpdates() or requestUpdate() is called, this signal is emitted
    when an update becomes available.

    The \a update value holds the value of the new update.
*/

/*!
    \fn void QGeoPositionInfoSource::updateTimeout();

    If requestUpdate() was called, this signal will be emitted if the current position could not
    be retrieved within the specified timeout.

    If startUpdates() has been called, this signal will be emitted if this QGeoPositionInfoSource
    subclass determines that it will not be able to provide further regular updates.  This signal
    will not be emitted again until after the regular updates resume.
*/

#include "moc_qgeopositioninfosource.cpp"

QTMS_END_NAMESPACE
