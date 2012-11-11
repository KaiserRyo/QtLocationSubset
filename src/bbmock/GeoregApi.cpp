/**
 * @copyright
 * Copyright Research In Motion Limited, 2011-2011
 * Research In Motion Limited. All rights reserved.
 */

#include <private/bbmock/GeoregApi.hpp>

#include "GeoregApiImpl.hpp"

#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtGlobal>

namespace bbmock
{

static GeoregApi *instance = NULL;
static QMutex instanceMutex;

GeoregApi::GeoregApi()
{ 
}

GeoregApi::~GeoregApi()
{  
}

GeoregApi& GeoregApi::getInstance()
{
    QMutexLocker lock(&instanceMutex);

    // lazy object creation of real implementation
    if (instance == NULL) {
#if defined(BB_TEST_BUILD)
        qFatal( "GeoregApi::getInstance: test mock not installed" );
#else
        instance = new GeoregApiImpl;
#endif // BB_TEST_BUILD
    }

    return *instance;
}

void GeoregApi::setInstance(GeoregApi& api)
{
    QMutexLocker lock(&instanceMutex);

    // abort if instance already exists
    if (instance != NULL) {
        qFatal( "GeoregApi::setInstance: instance already exists" );
    }

    // install custom instance
    instance = &api;
}

void GeoregApi::unsetInstance(GeoregApi& api)
{
    QMutexLocker lock(&instanceMutex);

    // uninstall custom instance if current
    if (instance == &api) {
        instance = NULL;
    }
}

} // namespace bbmock
