/**
 * @copyright
 * Copyright Research In Motion Limited, 2011-2011
 * Research In Motion Limited. All rights reserved.
 *
 */

#ifndef BBMOCK_GEOREGAPIIMPL_HPP
#define BBMOCK_GEOREGAPIIMPL_HPP

#include "private/bbmock/GeoregApi.hpp"

namespace bbmock {

/**
 * Real implementation of GeoregApi interface. All methods are mapped 1:1 to libgeoreg
 * functions.
 *
 * See libgeoreg docs for API details.
 */
class GeoregApiImpl : public GeoregApi
{
public:
    GeoregApiImpl();
    virtual ~GeoregApiImpl();

    virtual const char *geo_search_strerror(geo_search_error_t error);

    virtual geo_search_error_t geo_search_open(geo_search_handle_t *handle);
    virtual void geo_search_close(geo_search_handle_t *handle);

    virtual void geo_search_free_reply(geo_search_reply_t *reply);

    virtual geo_search_error_t geo_search_geocode(geo_search_handle_t *handle, geo_search_reply_t *reply, const char *search_string);
    virtual geo_search_error_t geo_search_geocode_latlon(geo_search_handle_t *handle, geo_search_reply_t *reply, const char *search_string, double lat, double lon);
    virtual geo_search_error_t geo_search_reverse_geocode(geo_search_handle_t *handle, geo_search_reply_t *reply, double lat, double lon, geo_search_boundary_t boundary);

    virtual geo_search_error_t geo_search_reply_get_length(geo_search_reply_t *reply, int *length);
    virtual geo_search_error_t geo_search_reply_set_index(geo_search_reply_t *reply, int index);

    virtual geo_search_error_t geo_search_reply_get_name(geo_search_reply_t *reply, const char **name);
    virtual geo_search_error_t geo_search_reply_get_lat(geo_search_reply_t *reply, double *lat);
    virtual geo_search_error_t geo_search_reply_get_lon(geo_search_reply_t *reply, double *lon);
    virtual geo_search_error_t geo_search_reply_get_street(geo_search_reply_t *reply, const char **street);
    virtual geo_search_error_t geo_search_reply_get_city(geo_search_reply_t *reply, const char **city);
    virtual geo_search_error_t geo_search_reply_get_region(geo_search_reply_t *reply, const char **region);
    virtual geo_search_error_t geo_search_reply_get_county(geo_search_reply_t *reply, const char **county);
    virtual geo_search_error_t geo_search_reply_get_district(geo_search_reply_t *reply, const char **district);
    virtual geo_search_error_t geo_search_reply_get_country(geo_search_reply_t *reply, const char **country);
    virtual geo_search_error_t geo_search_reply_get_postal_code(geo_search_reply_t *reply, const char **postal);
    virtual geo_search_error_t geo_search_reply_get_iso_alpha3_country_code(geo_search_reply_t *reply, const char **iso3_country_code);

};

} // namespace bbmock

#endif // BBMOCK_GEOREGAPIIMPL_HPP
