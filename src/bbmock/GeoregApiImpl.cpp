/**
 * @copyright
 * Copyright Research In Motion Limited, 2011-2011
 * Research In Motion Limited. All rights reserved.
 */

#if !defined(BB_TEST_BUILD)

#include "GeoregApiImpl.hpp"

#include <QtDebug>

namespace bbmock
{

GeoregApiImpl::GeoregApiImpl()
{ 
}

GeoregApiImpl::~GeoregApiImpl()
{  
}

const char *GeoregApiImpl::geo_search_strerror(geo_search_error_t error)
{
    return ::geo_search_strerror( error );
}

geo_search_error_t GeoregApiImpl::geo_search_open(geo_search_handle_t *handle)
{
    geo_search_error_t err = ::geo_search_open( handle );
    if ( err != GEO_SEARCH_OK ) {
        // shouldn't fail under normal circumstances so add a debugging message.
        qWarning() << "geo_search_open( handle =" << (void*)*handle << "); err =" << err;
    }
    return err;
}

void GeoregApiImpl::geo_search_close(geo_search_handle_t *handle)
{
    return ::geo_search_close( handle );
}

void GeoregApiImpl::geo_search_free_reply(geo_search_reply_t *reply)
{
    return ::geo_search_free_reply( reply );
}

geo_search_error_t GeoregApiImpl::geo_search_geocode(geo_search_handle_t *handle, geo_search_reply_t *reply, const char *search_string)
{
    return ::geo_search_geocode( handle, reply, search_string );
}

geo_search_error_t GeoregApiImpl::geo_search_geocode_latlon(geo_search_handle_t *handle, geo_search_reply_t *reply, const char *search_string, double lat, double lon)
{
    return ::geo_search_geocode_latlon( handle, reply, search_string, lat, lon );
}

geo_search_error_t GeoregApiImpl::geo_search_reverse_geocode(geo_search_handle_t *handle, geo_search_reply_t *reply, double lat, double lon, geo_search_boundary_t boundary)
{
    return ::geo_search_reverse_geocode( handle, reply, lat, lon, boundary );
}

geo_search_error_t GeoregApiImpl::geo_search_reply_get_length(geo_search_reply_t *reply, int *length)
{
    return ::geo_search_reply_get_length( reply, length );
}

geo_search_error_t GeoregApiImpl::geo_search_reply_set_index(geo_search_reply_t *reply, int index)
{
    return ::geo_search_reply_set_index( reply, index );
}

geo_search_error_t GeoregApiImpl::geo_search_reply_get_name(geo_search_reply_t *reply, const char **name)
{
    return ::geo_search_reply_get_name( reply, name );
}

geo_search_error_t GeoregApiImpl::geo_search_reply_get_lat(geo_search_reply_t *reply, double *lat)
{
    return ::geo_search_reply_get_lat( reply, lat );
}

geo_search_error_t GeoregApiImpl::geo_search_reply_get_lon(geo_search_reply_t *reply, double *lon)
{
    return ::geo_search_reply_get_lon( reply, lon );
}

geo_search_error_t GeoregApiImpl::geo_search_reply_get_street(geo_search_reply_t *reply, const char **street)
{
    return ::geo_search_reply_get_street( reply, street );
}

geo_search_error_t GeoregApiImpl::geo_search_reply_get_city(geo_search_reply_t *reply, const char **city)
{
    return ::geo_search_reply_get_city( reply, city );
}

geo_search_error_t GeoregApiImpl::geo_search_reply_get_region(geo_search_reply_t *reply, const char **region)
{
    return ::geo_search_reply_get_region( reply, region );
}

geo_search_error_t GeoregApiImpl::geo_search_reply_get_county(geo_search_reply_t *reply, const char **county)
{
    return ::geo_search_reply_get_county( reply, county );
}

geo_search_error_t GeoregApiImpl::geo_search_reply_get_district(geo_search_reply_t *reply, const char **district)
{
    return ::geo_search_reply_get_district( reply, district );
}

geo_search_error_t GeoregApiImpl::geo_search_reply_get_country(geo_search_reply_t *reply, const char **country)
{
    return ::geo_search_reply_get_country( reply, country );
}

geo_search_error_t GeoregApiImpl::geo_search_reply_get_postal_code(geo_search_reply_t *reply, const char **postal)
{
    return ::geo_search_reply_get_postal_code( reply, postal );
}

geo_search_error_t GeoregApiImpl::geo_search_reply_get_iso_alpha3_country_code(geo_search_reply_t *reply, const char **iso3_country_code)
{
    return ::geo_search_reply_get_iso_alpha3_country_code( reply, iso3_country_code );
}

} // namespace bbmock

#endif // BB_TEST_BUILD
