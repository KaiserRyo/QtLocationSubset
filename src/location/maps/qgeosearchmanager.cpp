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

#include "qgeosearchmanager.h"
#include "qgeosearchmanager_p.h"
#include "qgeosearchmanagerengine.h"

#include "qgeoboundingbox.h"
#include "qgeoboundingcircle.h"

#include <QLocale>

QTMS_BEGIN_NAMESPACE

/*!
    \class QGeoSearchManager

    \brief The QGeoSearchManager class provides support for searching
    operations related to geographic information.


    \inmodule QtLocationSubset
    \since 1.1

    \ingroup maps-places
        \headerfile qgeosearchmanager.cpp <QtLocationSubset/QGeoSearchManager>
    @xmlonly
    <apigrouping group="Location/Positioning and Geocoding"/>
    @endxmlonly

    The geocode(), reverseGeocode() and search() functions return
    QGeoSearchReply objects, which manage these operations and report on the
    result of the operations and any errors which may have occurred.

    The geocode() and reverseGeocode() functions can be used to convert
    QGeoAddress instances to QGeoCoordinate instances and vice-versa.

    The search() function allows a user to perform a free text search
    for place information.  If the string provided can be interpreted as
    an address it can be geocoded to coordinate information.

    Note that the BB10 QGeoSearchManager supports geocode() and reverseGeocode()
    but not search().

    Instances of QGeoSearchManager can be accessed with
    QGeoServiceProvider::searchManager().

    A small example of the usage of QGeoSearchManager follows:

    \code
class MySearchHandler : public QObject
{
    Q_OBJECT
public:
    MySearchHandler(QGeoSearchManager *searchManager, QGeoAddress searchAddress)
    {
        QGeoSearchReply *reply = searchManager->geocode(searchAddress);

        if (reply->isFinished()) {
            if (reply->error() == QGeoSearchReply::NoError) {
                searchResults(reply);
            else
                searchError(reply, reply->error(), reply->errorString());
            return;
        }

        connect(searchManager,
                SIGNAL(finished(QGeoSearchReply*)),
                this,
                SLOT(searchResults(QGeoSearchReply*)));

        connect(searchManager,
                SIGNAL(error(QGeoSearchReply*,QGeoSearchReply::Error,QString)),
                this
                SLOT(searchError(QGeoSearchReply*,QGeoSearchReply::Error,QString)));
    }

private slots:
    void searchResults(QGeoSearchReply *reply)
    {
        // ... now we have to make use of the places ...
        reply->deleteLater();
    }

    void searchError(QGeoSearchReply *reply, QGeoSearchReply::Error error, const QString &errorString)
    {
        // ... inform the user that an error has occurred ...
        reply->deleteLater();
    }
};
    \endcode
*/

/*!
\enum QGeoSearchManager::SearchType

Describes the type of search that should be performed by search().

\value SearchNone
    Do not use the search string.
\value SearchGeocode
    Use the search string as a textual address in a geocoding operation.
\value SearchAll
    All available information sources should be used as part of the search.

Note that the BB10 QGeoSearchManager does not support search() and thus the
SearchType is not relevant.
*/

/*!
    Constructs a new manager with the specified \a parent and with the
    implementation provided by \a engine.

    This constructor is used interally by QGeoServiceProviderFactory. Regular
    users should acquire instances of QGeoSearchManager with
    QGeoServiceProvider::searchManager();
*/
QGeoSearchManager::QGeoSearchManager(QGeoSearchManagerEngine *engine, QObject *parent)
    : QObject(parent),
      d_ptr(new QGeoSearchManagerPrivate())
{
    d_ptr->engine = engine;
    if (d_ptr->engine) {
        d_ptr->engine->setParent(this);

        connect(d_ptr->engine,
                SIGNAL(finished(QGeoSearchReply*)),
                this,
                SIGNAL(finished(QGeoSearchReply*)));

        connect(d_ptr->engine,
                SIGNAL(error(QGeoSearchReply*, QGeoSearchReply::Error, QString)),
                this,
                SIGNAL(error(QGeoSearchReply*, QGeoSearchReply::Error, QString)));
    } else {
        qFatal("The search manager engine that was set for this search manager was NULL.");
    }
}

/*!
    Destroys this manager.
*/
QGeoSearchManager::~QGeoSearchManager()
{
    delete d_ptr;
}

/*!
    Returns the name of the engine which implements the behaviour of this
    search manager.

    The combination of managerName() and managerVersion() should be unique
    amongst the plugin implementations.
*/
QString QGeoSearchManager::managerName() const
{
//    if (!d_ptr->engine)
//        return QString();

    return d_ptr->engine->managerName();
}

/*!
    Returns the version of the engine which implements the behaviour of this
    search manager.

    The combination of managerName() and managerVersion() should be unique
    amongst the plugin implementations.
*/
int QGeoSearchManager::managerVersion() const
{
//    if (!d_ptr->engine)
//        return -1;

    return d_ptr->engine->managerVersion();
}

/*!
    Begins the geocoding of \a address. Geocoding is the process of finding a
    coordinate that corresponds to a given address.

    A QGeoSearchReply object will be returned, which can be used to manage the
    geocoding operation and to return the results of the operation.

    This manager and the returned QGeoSearchReply object will emit signals
    indicating if the operation completes or if errors occur.

    If supportsGeocoding() returns false an
    QGeoSearchReply::UnsupportedOptionError will occur.

    Once the operation has completed, QGeoSearchReply::places() can be used to
    retrieve the results, which will consist of a list of QGeoPlace objects.
    These objects represent a combination of coordinate and address data.

    The address data returned in the results may be different from \a address.
    This will usually occur if the geocoding service backend uses a different
    canonical form of addresses or if \a address was only partially filled out.

    If \a bounds is non-null and a valid QGeoBoundingArea it will be used to
    limit the results to those that are contained within \a bounds. This is
    particularly useful if \a address is only partially filled out, as the
    service will attempt to geocode all matches for the specified data.

    The user is responsible for deleting the returned reply object, although
    this can be done in the slot connected to QGeoSearchManager::finished(),
    QGeoSearchManager::error(), QGeoSearchReply::finished() or
    QGeoSearchReply::error() with deleteLater().

    Note that the BB10 QGeoSearchManager may give very different results depending if bounds is
    specified or not. Searching for an obscure address without providing a latitude/longitude hint
    can result in no matches found. The bounds center is used as a latitude/longitude hint of where
    to search for the address. To perform an unbounded search with a latitude/longitude hint ensure
    bounds is empty (i.e. bounds->isEmpty() is true). This is done by setting the radius to 0 (for
    QGeoBoundingCircle) or by ensuring the height and width are both 0 (for QGeoBoundingBox).

    Also note for the BB10 QGeoSearchManager that if address.isTextGenerated() is false then a free
    text search is performed by geocode(), using address.text().
*/
QGeoSearchReply* QGeoSearchManager::geocode(const QGeoAddress &address, QGeoBoundingArea *bounds)
{
//    if (!d_ptr->engine)
//        return new QGeoSearchReply(QGeoSearchReply::EngineNotSetError, "The search manager was not created with a valid engine.", this);

    return d_ptr->engine->geocode(address, bounds);
}


/*!
    Begins the reverse geocoding of \a coordinate. Reverse geocoding is the
    process of finding an address that corresponds to a given coordinate.

    A QGeoSearchReply object will be returned, which can be used to manage the
    reverse geocoding operation and to return the results of the operation.

    This manager and the returned QGeoSearchReply object will emit signals
    indicating if the operation completes or if errors occur.

    If supportsReverseGeocoding() returns false an
    QGeoSearchReply::UnsupportedOptionError will occur.

    At that point QGeoSearchReply::places() can be used to retrieve the
    results, which will consist of a list of QGeoPlace objects. These object
    represent a combination of coordinate and address data.

    The coordinate data returned in the results may be different from \a
    coordinate. This will usually occur if the reverse geocoding service
    backend shifts the coordinates to be closer to the matching addresses, or
    if the backend returns results at multiple levels of detail.

    If multiple results are returned by the reverse geocoding service backend
    they will be provided in order of specificity. This normally occurs if the
    backend is configured to reverse geocode across multiple levels of detail.
    As an example, some services will return address and coordinate pairs for
    the street address, the city, the state and the country.

    If \a bounds is non-null and a valid QGeoBoundingBox it will be used to
    limit the results to those that are contained within \a bounds.

    The user is responsible for deleting the returned reply object, although
    this can be done in the slot connected to QGeoSearchManager::finished(),
    QGeoSearchManager::error(), QGeoSearchReply::finished() or
    QGeoSearchReply::error() with deleteLater().

    Note that the BB10 QGeoSearchManager by default returns a single result
    at the maximum level of detail (i.e. address), if a match is found. The
    underlying BB10 geocoding service can provide more coarse levels of detail
    (i.e. city, country, etc.) and this functionality can be requested via
    QGeoSearchManager::setProperty() and the Qt property system. These levels
    of detail correspond to the
    boundaries that can be specified through the geo_search C API. The
    property that can be set is "boundary" and the valid values are "address",
    "postal", "city", "province", and "country". If any other values are
    specified then the BB10 QGeoSearchManager defaults to "address".
    "postal" corresponds to the QGeoAddress field \a postcode, and "province"
    corresponds to the
    QGeoAddress field \a state. The remaining boundaries "mcc" and "timezone"
    do not have corresponding fields in QGeoAddress and are not supported as
    levels of detail through QGeoSearchManager. They can be accessed using the
    geo_search C API instead. For example, the following initiates a reverse
    geocode operation for the city within which the specified point lies:
    \code
    searchManager->setProperty( "boundary", "city" );
    searchManager->reverseGeocode( myCoordinate );
    \endcode

    Note that the boundary specified with QGeoSearchManager::setProperty()
    persists and subsequent calls to reverseGeocode() will continue to request
    this boundary, until the boundary is changed again using
    QGeoSearchManager::setProperty().
*/
QGeoSearchReply* QGeoSearchManager::reverseGeocode(const QGeoCoordinate &coordinate, QGeoBoundingArea *bounds)
{
//    if (!d_ptr->engine)
//        return new QGeoSearchReply(QGeoSearchReply::EngineNotSetError, "The search manager was not created with a valid engine.", this);

    return d_ptr->engine->reverseGeocode(coordinate, bounds);
}

/*!
    Begins searching for a place matching \a searchString.  The value of
    \a searchTypes will determine whether the search is for addresses only,
    for landmarks only or for both. Note that searching for landmarks is not
    currently supported in this QtLocation module subset.

    A QGeoSearchReply object will be returned, which can be used to manage the
    geocoding operation and to return the results of the operation.

    This manager and the returned QGeoSearchReply object will emit signals
    indicating if the operation completes or if errors occur.

    If supportsGeocoding() returns false and \a searchTypes is
    QGeoSearchManager::SearchGeocode an
    QGeoSearchReply::UnsupportedOptionError will occur.

    Once the operation has completed, QGeoSearchReply::places() can be used to
    retrieve the results, which will consist of a list of QGeoPlace objects.
    These objects represent a combination of coordinate and address data.

    If \a limit is -1 the entire result set will be returned, otherwise at most
    \a limit results will be returned.

    The \a offset parameter is used to ask the search service to not return the
    first \a offset results.

    The \a limit and \a offset results are used together to implement paging.

    If \a bounds is non-null and a valid QGeoBoundingArea it will be used to
    limit the results to thos that are contained within \a bounds.

    The user is responsible for deleting the returned reply object, although
    this can be done in the slot connected to QGeoSearchManager::finished(),
    QGeoSearchManager::error(), QGeoSearchReply::finished() or
    QGeoSearchReply::error() with deleteLater().

    Note that the BB10 QGeoSearchManager does not support search(). A free text search
    can be performed using geocode(), with the text being set through QGeoAddress::setText().
*/
QGeoSearchReply* QGeoSearchManager::search(const QString &searchString,
        QGeoSearchManager::SearchTypes searchTypes,
        int limit,
        int offset,
        QGeoBoundingArea *bounds)
{
//    if (!d_ptr->engine)
//        return new QGeoSearchReply(QGeoSearchReply::EngineNotSetError, "The search manager was not created with a valid engine.", this);

    QGeoSearchReply *reply = d_ptr->engine->search(searchString,
                             searchTypes,
                             limit,
                             offset,
                             bounds);

    return reply;
}

/*!
    Returns whether this manager supports geocoding.
*/
bool QGeoSearchManager::supportsGeocoding() const
{
//    if (!d_ptr->engine)
//        return false;

    return d_ptr->engine->supportsGeocoding();
}

/*!
    Returns whether this manager supports reverse geocoding.
*/
bool QGeoSearchManager::supportsReverseGeocoding() const
{
    return d_ptr->engine->supportsReverseGeocoding();
}

/*!
    Returns the search types supported by the search() function with this manager.
*/
QGeoSearchManager::SearchTypes QGeoSearchManager::supportedSearchTypes() const
{
//    if (!d_ptr->engine)
//        return QGeoSearchManager::SearchTypes();

    return d_ptr->engine->supportedSearchTypes();
}

/*!
    Sets the locale to be used by this manager to \a locale.

    If this search manager supports returning the results
    in different languages, they will be returned in the language of \a locale.

    The locale used defaults to the system locale if this is not set.
*/
void QGeoSearchManager::setLocale(const QLocale &locale)
{
    d_ptr->engine->setLocale(locale);
}

/*!
    Returns the locale used to hint to this search manager about what
    language to use for the results.
*/
QLocale QGeoSearchManager::locale() const
{
    return d_ptr->engine->locale();
}

/*!
\fn void QGeoSearchManager::finished(QGeoSearchReply* reply)

    This signal is emitted when \a reply has finished processing.

    If reply::error() equals QGeoSearchReply::NoError then the processing
    finished successfully.

    This signal and QGeoSearchReply::finished() will be emitted at the same
    time.

    \note Do not delete the \a reply object in the slot connected to this
    signal. Use deleteLater() instead.
*/

/*!
\fn void QGeoSearchManager::error(QGeoSearchReply* reply, QGeoSearchReply::Error error, QString errorString)

    This signal is emitted when an error has been detected in the processing of
    \a reply. The QGeoSearchManager::finished() signal will probably follow.

    The error will be described by the error code \a error. If \a errorString is
    not empty it will contain a textual description of the error.

    This signal and QGeoSearchReply::error() will be emitted at the same time.

    \note Do not delete the \a reply object in the slot connected to this
    signal. Use deleteLater() instead.
*/

/*******************************************************************************
*******************************************************************************/

QGeoSearchManagerPrivate::QGeoSearchManagerPrivate()
    : engine(0) {}

QGeoSearchManagerPrivate::~QGeoSearchManagerPrivate()
{
    if (engine)
        delete engine;
}

/*******************************************************************************
*******************************************************************************/

#include "moc_qgeosearchmanager.cpp"

QTMS_END_NAMESPACE
