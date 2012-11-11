/**
 * @copyright
 * Copyright Research In Motion Limited, 2012-2012
 * Research In Motion Limited. All rights reserved.
 */

#ifndef PRIVATE_BBMOCK_GEOREG_HPP
#define PRIVATE_BBMOCK_GEOREG_HPP

#include <geo_search.h>

namespace bbmock {

/**
 * Wrapper for georeg API. All code in libQtLocationSubset should use this wrapper instead of talking
 * directly georeg. Using an interface to access libgeoreg allows this dependency to be 
 * mocked during testing.
 *
 * See georeg docs for API details.
 */
class GeoregApi
{
public:
    GeoregApi();
    virtual ~GeoregApi();

    /**
     * Returns an object which implements the GeoregApi interface. If no such object 
     * currently exists, an instance of GeoregApiImpl (real implementation) is created.
     */
    static GeoregApi& getInstance();


    virtual const char *geo_search_strerror(geo_search_error_t error) = 0;

    virtual geo_search_error_t geo_search_open(geo_search_handle_t *handle) = 0;
    virtual void geo_search_close(geo_search_handle_t *handle) = 0;

    virtual void geo_search_free_reply(geo_search_reply_t *reply) = 0;

    virtual geo_search_error_t geo_search_geocode(geo_search_handle_t *handle, geo_search_reply_t *reply, const char *search_string) = 0;
    virtual geo_search_error_t geo_search_geocode_latlon(geo_search_handle_t *handle, geo_search_reply_t *reply, const char *search_string, double lat, double lon) = 0;
    virtual geo_search_error_t geo_search_reverse_geocode(geo_search_handle_t *handle, geo_search_reply_t *reply, double lat, double lon, geo_search_boundary_t boundary) = 0;

    virtual geo_search_error_t geo_search_reply_get_length(geo_search_reply_t *reply, int *length) = 0;
    virtual geo_search_error_t geo_search_reply_set_index(geo_search_reply_t *reply, int index) = 0;

    virtual geo_search_error_t geo_search_reply_get_name(geo_search_reply_t *reply, const char **name) = 0;
    virtual geo_search_error_t geo_search_reply_get_lat(geo_search_reply_t *reply, double *lat) = 0;
    virtual geo_search_error_t geo_search_reply_get_lon(geo_search_reply_t *reply, double *lon) = 0;
    virtual geo_search_error_t geo_search_reply_get_street(geo_search_reply_t *reply, const char **street) = 0;
    virtual geo_search_error_t geo_search_reply_get_city(geo_search_reply_t *reply, const char **city) = 0;
    virtual geo_search_error_t geo_search_reply_get_region(geo_search_reply_t *reply, const char **region) = 0;
    virtual geo_search_error_t geo_search_reply_get_county(geo_search_reply_t *reply, const char **county) = 0;
    virtual geo_search_error_t geo_search_reply_get_district(geo_search_reply_t *reply, const char **district) = 0;
    virtual geo_search_error_t geo_search_reply_get_country(geo_search_reply_t *reply, const char **country) = 0;
    virtual geo_search_error_t geo_search_reply_get_postal_code(geo_search_reply_t *reply, const char **postal) = 0;
    virtual geo_search_error_t geo_search_reply_get_iso_alpha3_country_code(geo_search_reply_t *reply, const char **iso3_country_code) = 0;

protected:
    /**
     * Installs a custom object which implements the ScreenApi interface. This method is
     * intended to be used by a mock object during testing.
     */
    static void setInstance(GeoregApi& api);

    /**
     * Uninstalls the object which implements the ScreenApi interface (if the specified 
     * object is currently installed; otherwise do nothing). This method is intended to 
     * be used by a mock object during testing.
     */
    static void unsetInstance(GeoregApi& api);
};

} // namespace bbmock


#endif // PRIVATE_BBMOCK_GEOREG_HPP
