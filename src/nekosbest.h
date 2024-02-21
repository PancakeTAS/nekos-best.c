/**
 * \file
 * This file contains structs and functions for interacting with the nekos.best API.
 */

#ifndef NEKOSBEST_H
#define NEKOSBEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

/**
 * Base URL for nekos.best API.
 */
#define NEKOS_BASE_URL "https://nekos.best/api/v2/"

/**
 * Maximum amount of results that can be requested.
 */
#define NEKOS_MAX_AMOUNT 20

/**
 * Minimum length of query string.
 */
#define NEKOS_MIN_QUERY_LEN 3

/**
 * Maximum length of query string.
 */
#define NEKOS_MAX_QUERY_LEN 150

/**
 * Status codes for the nekos.best c wrapper. 
 */
typedef enum {

    /**
     * Indicates that the operation was successful.
     */
    NEKOS_OK,

    /**
     * Indicates that there was a memory allocation error.
     */
    NEKOS_MEM_ERR,

    /**
     * Indicates that there was an error with libcurl. 
     */
    NEKOS_LIBCURL_ERR,

    /**
     * Indicates that there was an error with cJSON. 
     */
    NEKOS_CJSON_ERR,

    /**
     * Indicates that an invalid parameter was passed to a function. 
     */
    NEKOS_INVALID_PARAM_ERR

} nekos_status;

/**
 * Enum for the format of the image. 
 */
typedef enum {

    /**
     * Indicates that the response image is a png and \link nekos_source_png nekos_source_png \endlink should be used.
     */
    NEKOS_PNG,

    /**
     * Indicates that the response image is a gif and \link nekos_source_gif nekos_source_gif \endlink should be used.
     */
    NEKOS_GIF

} nekos_format;

/**
 * Struct for an endpoint/category.
 */
typedef struct {

    /**
     * Name of the endpoint/category.
     */
    char *name;

    /**
     * Format of the endpoint/category.
     */
    nekos_format format;

} nekos_endpoint;

/**
 * Struct for a list of endpoints/categories.
 */
typedef struct {

    /**
     * [out] Array of endpoints/categories. 
     */
    nekos_endpoint **endpoints;

    /**
     * [out] Amount of endpoints/categories.
     */
    size_t len;

} nekos_endpoint_list;

/**
 * Struct for a gif source.
 */
typedef struct {

    /**
     * [out] Name of the anime the gif is from.
     */
    char *anime_name;

} nekos_source_gif;

/**
 * Struct for a png source.
 */
typedef struct {

    /**
     * [out] Name of the artist of the png.
     */
    char *artist_name;

    /**
     * [out] URL to the artist's page.
     */
    char *artist_href;

    /**
     * [out] URL to the source of the png.
     */
    char *source_url;

} nekos_source_png;

/**
 * Struct for a result image.
 */
typedef struct {

    /**
     * [out] Source information of the image.
     */
    union {

        /**
         * [out] Source information for a gif. Only use if the format is \link NEKOS_GIF nekos_format::NEKOS_GIF \endlink.
         */
        nekos_source_gif *gif;

        /**
         * [out] Source information for a png. Only use if the format is \link NEKOS_PNG nekos_format::NEKOS_PNG \endlink. 
         */
        nekos_source_png *png;

    } source;

    /**
     * [out] URL to the image.
     */
    char* url;

} nekos_result;

/**
 * Struct for a list of result images. 
 */
typedef struct {

    /**
     * [out] Array of result images.
     */
    nekos_result **responses;

    /**
     * [out] Amount of result images.
     */
    size_t len;

} nekos_result_list;

/**
 * Struct for an http response.
 */
typedef struct {

    /**
     * [out] Text of the response. Not null-terminated. 
     */
    char *text;

    /**
     * [out] Length of the response text. 
     */
    size_t len;

} nekos_http_response;

#ifndef NEKOSBEST_IMPL

/**
 * Get a list of endpoints/categories.
 * 
 * This function fetches the `endpoints` endpoint of the api
 * and parses the response into a list of endpoints.
 * 
 * It will allocate memory for the list of endpoints and the endpoints themselves.
 * 
 * \param [out] endpoints
 *   Pointer to a \link nekos_endpoint_list nekos_endpoint_list \endlink to store the endpoints in.
 * 
 * \return
 *   ::NEKOS_OK \n
 *   ::NEKOS_MEM_ERR \n
 *   ::NEKOS_LIBCURL_ERR \n
 *   ::NEKOS_CJSON_ERR
 */
nekos_status nekos_endpoints(nekos_endpoint_list* endpoints);

/**
 * Get a list of images from a category.
 * 
 * This function fetches the specified category endpoint of the api
 * and parses the response into a list of images.
 * 
 * It will allocate memory for the list of results, the results themselves, and the source information.
 * 
 * \param [out] results
 *   Pointer to a \link nekos_result_list nekos_result_list \endlink to store the results in.
 * \param [in] endpoint
 *   Pointer to a \link nekos_endpoint nekos_endpoint \endlink to specify the category.
 * \param [in] amount
 *   Amount of images to fetch. Must be between 1 and \link NEKOS_MAX_AMOUNT \endlink.
 * 
 * \return
 *   ::NEKOS_OK \n
 *   ::NEKOS_MEM_ERR \n
 *   ::NEKOS_LIBCURL_ERR \n
 *   ::NEKOS_CJSON_ERR \n
 *   ::NEKOS_INVALID_PARAM_ERR
 */
nekos_status nekos_category(nekos_result_list *results, nekos_endpoint *endpoint, int amount);

/**
 * Search for images.
 * 
 * This function fetches the `search` endpoint of the api
 * and parses the response into a list of images.
 * 
 * The search can optionally specify a endpoint/category.
 * 
 * It will allocate memory for the list of results, the results themselves, and the source information.
 * 
 * \param [out] results
 *   Pointer to a \link nekos_result_list nekos_result_list \endlink to store the results in.
 * \param [in] query
 *   Query to search for. Must be between \link NEKOS_MIN_QUERY_LEN \endlink and \link NEKOS_MAX_QUERY_LEN \endlink.
 * \param [in] amount
 *   Amount of images to fetch. Must be between 1 and \link NEKOS_MAX_AMOUNT \endlink.
 * \param [in] format
 *   Format of the images to search for.
 * \param [in] endpoint
 *   Pointer to a \link nekos_endpoint nekos_endpoint \endlink to specify a category. Can be NULL.
 * 
 * \return
 *   ::NEKOS_OK \n
 *   ::NEKOS_MEM_ERR \n
 *   ::NEKOS_LIBCURL_ERR \n
 *   ::NEKOS_CJSON_ERR \n
 *   ::NEKOS_INVALID_PARAM_ERR
 */
nekos_status nekos_search(nekos_result_list *results, const char* query, int amount, nekos_format format, nekos_endpoint *endpoint);

/**
 * Download an image.
 * 
 * This function fetches the specified image url
 * and stores the response in a \link nekos_http_response nekos_http_response \endlink.
 * 
 * It will allocate memory for the response text.
 * 
 * \param [out] http_response
 *   Pointer to a \link nekos_http_response nekos_http_response \endlink to store the response in.
 * \param [in] url
 *   URL of the image to download.
 * 
 * \return
 *   ::NEKOS_OK \n
 *   ::NEKOS_MEM_ERR \n
 *   ::NEKOS_LIBCURL_ERR
 */
nekos_status nekos_download(nekos_http_response *http_response, const char* url);

#else // NEKOSBEST_IMPL

static size_t nekos_write_callback(void *ptr, size_t count, size_t nmemb, nekos_http_response *http_response) {
    size_t size = count * nmemb;
    size_t new_len = http_response->len + size;

    // resize response text
    char* new_text = (char*) realloc(http_response->text, new_len + 1);
    if (!new_text) {
        free(http_response->text);
        return CURLE_ABORTED_BY_CALLBACK;
    }

    // copy new data to response text
    memcpy(new_text + http_response->len, ptr, size);
    http_response->text = new_text;
    http_response->len = new_len;
    return size;
}

static nekos_status nekos_do_request(nekos_http_response *http_response, const char* url) {
    // initialize http response object
    http_response->len = 0;
    http_response->text = (char*) malloc(1);
    if (!http_response->text)
        return NEKOS_MEM_ERR;

    // initialize curl
    CURL *curl = curl_easy_init();
    if (!curl)
        return NEKOS_LIBCURL_ERR;

    // configure curl request
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_CA_CACHE_TIMEOUT, 604800L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, nekos_write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, http_response);

    // make request
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
        return NEKOS_LIBCURL_ERR;

    // cleanup curl
    curl_easy_cleanup(curl);
    return NEKOS_OK;
}

static char* nekos_jsondup(const cJSON *json, const char* key) {
    const cJSON *element = cJSON_GetObjectItemCaseSensitive(json, key);
    char* str = (char*) malloc(strlen(element->valuestring) + 1);
    strcpy(str, element->valuestring);
    return str;
}

nekos_status nekos_endpoints(nekos_endpoint_list* endpoints) {
    // make request
    nekos_http_response http_response;
    nekos_status http_status = nekos_do_request(&http_response, NEKOS_BASE_URL "endpoints");
    if (http_status != NEKOS_OK)
        return http_status;

    // parse response
    cJSON *json = cJSON_ParseWithLength(http_response.text, http_response.len);
    if (!json || !cJSON_IsObject(json))
        return NEKOS_CJSON_ERR;

    // parse json
    endpoints->len = cJSON_GetArraySize(json);
    endpoints->endpoints = (nekos_endpoint**) malloc(endpoints->len * sizeof(nekos_endpoint*));

    // iterate through endpoints
    const cJSON *endpoint_obj;
    size_t i = 0;
    cJSON_ArrayForEach(endpoint_obj, json) {
        nekos_endpoint* endpoint = (nekos_endpoint*) malloc(sizeof(nekos_endpoint));
        endpoints->endpoints[i++] = endpoint;

        char* name = endpoint_obj->string;
        endpoint->name = (char*) malloc(strlen(name) + 1);
        strcpy(endpoint->name, name);

        const cJSON *format_obj = cJSON_GetObjectItemCaseSensitive(endpoint_obj, "format");
        endpoint->format = strcmp(format_obj->valuestring, "png") == 0 ? NEKOS_PNG : NEKOS_GIF;
    }
    
    // cleanup
    cJSON_Delete(json);
    free(http_response.text);
    return NEKOS_OK;
}

nekos_status nekos_category(nekos_result_list *results, nekos_endpoint *endpoint, int amount) {
    // check if amount is valid
    if (amount < 1 || amount > NEKOS_MAX_AMOUNT)
        return NEKOS_INVALID_PARAM_ERR;

    // create endpoint url
    char url[96];
    snprintf(url, 96, "%s%s?amount=%d", NEKOS_BASE_URL, endpoint->name, amount);

    // make request
    nekos_http_response http_response;
    nekos_status http_status = nekos_do_request(&http_response, url);
    if (http_status != NEKOS_OK)
        return http_status;

    // parse response
    cJSON *json = cJSON_ParseWithLength(http_response.text, http_response.len);
    if (!json || !cJSON_IsObject(json))
        return NEKOS_CJSON_ERR;

    // parse json
    cJSON *results_obj = cJSON_GetObjectItemCaseSensitive(json, "results");
    results->len = cJSON_GetArraySize(results_obj);
    results->responses = (nekos_result**) malloc(results->len * sizeof(nekos_result*));

    // iterate through responses
    const cJSON *response_obj;
    size_t i = 0;
    cJSON_ArrayForEach(response_obj, results_obj) {
        nekos_result* response = (nekos_result*) malloc(sizeof(nekos_result));
        results->responses[i++] = response;

        response->url = nekos_jsondup(response_obj, "url");
        if (endpoint->format == NEKOS_GIF) {
            response->source.gif = (nekos_source_gif*) malloc(sizeof(nekos_source_gif));
            response->source.gif->anime_name = nekos_jsondup(response_obj, "anime_name");
        } else {
            response->source.png = (nekos_source_png*) malloc(sizeof(nekos_source_png));
            response->source.png->artist_name = nekos_jsondup(response_obj, "artist_name");
            response->source.png->artist_href = nekos_jsondup(response_obj, "artist_href");
            response->source.png->source_url = nekos_jsondup(response_obj, "source_url");
        }
    }

    // cleanup
    cJSON_Delete(json);
    free(http_response.text);
    return NEKOS_OK;
}

nekos_status nekos_search(nekos_result_list *results, const char* query, int amount, nekos_format format, nekos_endpoint *endpoint) {
    // check if amount is valid
    if (amount < 1 || amount > NEKOS_MAX_AMOUNT)
        return NEKOS_INVALID_PARAM_ERR;

    // check if query is valid
    size_t query_len = strlen(query);
    if (query_len < NEKOS_MIN_QUERY_LEN || query_len > NEKOS_MAX_QUERY_LEN)
        return NEKOS_INVALID_PARAM_ERR;

    // create endpoint url
    char url[256];
    if (endpoint)
        snprintf(url, 256, "%ssearch?query=%s&type=%d&amount=%d&category=%s", NEKOS_BASE_URL, query, format + 1, amount, endpoint->name);
    else
        snprintf(url, 256, "%ssearch?query=%s&type=%d&amount=%d", NEKOS_BASE_URL, query, format + 1, amount);

    // make request
    nekos_http_response http_response;
    nekos_status http_status = nekos_do_request(&http_response, url);
    if (http_status != NEKOS_OK)
        return http_status;

    // parse response
    cJSON *json = cJSON_ParseWithLength(http_response.text, http_response.len);
    if (!json || !cJSON_IsObject(json))
        return NEKOS_CJSON_ERR;

    // parse json
    cJSON *results_obj = cJSON_GetObjectItemCaseSensitive(json, "results");
    results->len = cJSON_GetArraySize(results_obj);
    results->responses = (nekos_result**) malloc(results->len * sizeof(nekos_result*));

    // iterate through responses
    const cJSON *response_obj;
    size_t i = 0;
    cJSON_ArrayForEach(response_obj, results_obj) {
        nekos_result* response = (nekos_result*) malloc(sizeof(nekos_result));
        results->responses[i++] = response;

        response->url = nekos_jsondup(response_obj, "url");
        if (format == NEKOS_GIF) {
            response->source.gif = (nekos_source_gif*) malloc(sizeof(nekos_source_gif));
            response->source.gif->anime_name = nekos_jsondup(response_obj, "anime_name");
        } else {
            response->source.png = (nekos_source_png*) malloc(sizeof(nekos_source_png));
            response->source.png->artist_name = nekos_jsondup(response_obj, "artist_name");
            response->source.png->artist_href = nekos_jsondup(response_obj, "artist_href");
            response->source.png->source_url = nekos_jsondup(response_obj, "source_url");
        }
    }

    // cleanup
    cJSON_Delete(json);
    free(http_response.text);
    return NEKOS_OK;
}

nekos_status nekos_download(nekos_http_response *http_response, const char* url) {
    return nekos_do_request(http_response, url);
}
#endif // NEKOSBEST_IMPL

#ifdef __cplusplus
}
#endif

#endif // NEKOSBEST_H