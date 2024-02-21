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

#define NEKOS_BASE_URL "https://nekos.best/api/v2/"
#define NEKOS_MAX_AMOUNT 20
#define NEKOS_MIN_QUERY_LEN 3
#define NEKOS_MAX_QUERY_LEN 150

typedef enum {
    NEKOS_OK,
    NEKOS_MEM_ERR,
    NEKOS_LIBCURL_ERR,
    NEKOS_CJSON_ERR,
    NEKOS_INVALID_PARAM_ERR
} nekos_status;

typedef enum {
    NEKOS_PNG,
    NEKOS_GIF
} nekos_format;

typedef struct {
    char *name;
    nekos_format format;
} nekos_endpoint;

typedef struct {
    nekos_endpoint **endpoints;
    size_t len;
} nekos_endpoint_list;

typedef struct {
    char *anime_name;
} nekos_source_gif;

typedef struct {
    char *artist_name;
    char *artist_href;
    char *source_url;
} nekos_source_png;

typedef struct {
    union {
        nekos_source_gif *gif;
        nekos_source_png *png;
    } source;
    char* url;
} nekos_result;

typedef struct {
    nekos_result **responses;
    size_t len;
} nekos_result_list;

typedef struct {
    char *text; // not null-terminated
    size_t len;
} nekos_http_response;

#ifndef NEKOSBEST_IMPL
nekos_status nekos_endpoints(nekos_endpoint_list* endpoints);
nekos_status nekos_category(nekos_result_list *results, nekos_endpoint *endpoint, int amount);
nekos_status nekos_search(nekos_result_list *results, const char* query, int amount, nekos_format format, nekos_endpoint *endpoint);
nekos_status nekos_download(nekos_http_response *http_response, const char* url);
#else
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
#endif

#ifdef __cplusplus
}
#endif

#endif