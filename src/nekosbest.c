#include "nekosbest.h"

#define API_URL "https://nekos.best/api/v2/"

typedef struct HTTPResponse {
    char *text; // not null-terminated
    size_t len;
} http_response;

static size_t http_response_write(void *ptr, size_t count, size_t nmemb, http_response *http_response) {
    size_t size = count * nmemb;
    size_t new_len = http_response->len + size;

    // resize response text
    char* new_text = realloc(http_response->text, new_len + 1);
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

static error_message do_request(http_response *http_response, char* url) {
    // initialize http response object
    http_response->len = 0;
    http_response->text = malloc(1);
    if (!http_response->text)
        return NEKOSBEST_MEM_ERR;

    // initialize curl
    CURL *curl = curl_easy_init();
    if (!curl)
        return NEKOSBEST_LIBCURL_ERR;

    // configure curl request
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_CA_CACHE_TIMEOUT, 604800L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_response_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, http_response);

    // make request
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
        return NEKOSBEST_LIBCURL_ERR;

    // cleanup curl
    curl_easy_cleanup(curl);
    return NEKOSBEST_OK;
}

static char* jsondup(const cJSON *json, char* key) {
    const cJSON *element = cJSON_GetObjectItemCaseSensitive(json, key);
    char* str = malloc(strlen(element->valuestring) + 1);
    strcpy(str, element->valuestring);
    return str;
}

error_message endpoints(endpoint_list* list) {
    // make request
    http_response http_response;
    error_message err = do_request(&http_response, API_URL "endpoints");
    if (err != NEKOSBEST_OK)
        return err;

    // parse response
    cJSON *json = cJSON_ParseWithLength(http_response.text, http_response.len);
    if (!json || !cJSON_IsObject(json))
        return NEKOSBEST_CJSON_ERR;

    // parse json
    list->len = cJSON_GetArraySize(json);
    list->endpoints = malloc(list->len * sizeof(api_endpoint*));

    // iterate through endpoints
    const cJSON *endpoint_obj;
    size_t i = 0;
    cJSON_ArrayForEach(endpoint_obj, json) {
        api_endpoint* endpoint = malloc(sizeof(api_endpoint));
        list->endpoints[i++] = endpoint;

        char* name = endpoint_obj->string;
        endpoint->name = malloc(strlen(name) + 1);
        strcpy(endpoint->name, name);

        const cJSON *format_obj = cJSON_GetObjectItemCaseSensitive(endpoint_obj, "format");
        endpoint->format = strcmp(format_obj->valuestring, "png") == 0 ? PNG : GIF;
    }
    
    // cleanup
    cJSON_Delete(json);
    free(http_response.text);
    return NEKOSBEST_OK;
}

error_message category(response_list *list, api_endpoint *endpoint, int amount) {
    // check if amount is valid
    if (amount < 1 || amount > 20)
        return NEKOSBEST_INVALID_PARAM_ERR;

    // create endpoint url
    char url[48];
    sprintf(url, "%s%s?amount=%d", API_URL, endpoint->name, amount);

    // make request
    http_response http_response;
    error_message err = do_request(&http_response, url);
    if (err != NEKOSBEST_OK)
        return err;

    // parse response
    cJSON *json = cJSON_ParseWithLength(http_response.text, http_response.len);
    if (!json || !cJSON_IsObject(json))
        return NEKOSBEST_CJSON_ERR;

    // parse json
    cJSON *results = cJSON_GetObjectItemCaseSensitive(json, "results");
    list->len = cJSON_GetArraySize(results);
    list->responses = malloc(list->len * sizeof(api_response*));

    // iterate through responses
    const cJSON *response_obj;
    size_t i = 0;
    cJSON_ArrayForEach(response_obj, results) {
        api_response* response = malloc(sizeof(api_response));
        list->responses[i++] = response;

        response->url = jsondup(response_obj, "url");
        if (endpoint->format == GIF) {
            response->source.gif = malloc(sizeof(gif_source));
            response->source.gif->anime_name = jsondup(response_obj, "anime_name");
        } else {
            response->source.png = malloc(sizeof(png_source));
            response->source.png->artist_name = jsondup(response_obj, "artist_name");
            response->source.png->artist_href = jsondup(response_obj, "artist_href");
            response->source.png->source_url = jsondup(response_obj, "source_url");
        }
    }

    // cleanup
    cJSON_Delete(json);
    free(http_response.text);
    return NEKOSBEST_OK;
}

error_message search(response_list *list, char* query, int amount, response_format type, api_endpoint *category) {
    // check if amount is valid
    if (amount < 1 || amount > 20)
        return NEKOSBEST_INVALID_PARAM_ERR;

    // check if query is valid
    size_t query_len = strlen(query);
    if (query_len < 3 || query_len > 150)
        return NEKOSBEST_INVALID_PARAM_ERR;

    // create endpoint url
    char url[256];
    if (category)
        sprintf(url, "%ssearch?query=%s&type=%d&amount=%d&category=%s", API_URL, query, type + 1, amount, category->name);
    else
        sprintf(url, "%ssearch?query=%s&type=%d&amount=%d", API_URL, query, type + 1, amount);

    // make request
    http_response http_response;
    error_message err = do_request(&http_response, url);
    if (err != NEKOSBEST_OK)
        return err;

    // parse response
    cJSON *json = cJSON_ParseWithLength(http_response.text, http_response.len);
    if (!json || !cJSON_IsObject(json))
        return NEKOSBEST_CJSON_ERR;

    // parse json
    cJSON *results = cJSON_GetObjectItemCaseSensitive(json, "results");
    list->len = cJSON_GetArraySize(results);
    list->responses = malloc(list->len * sizeof(api_response*));

    // iterate through responses
    const cJSON *response_obj;
    size_t i = 0;
    cJSON_ArrayForEach(response_obj, results) {
        api_response* response = malloc(sizeof(api_response));
        list->responses[i++] = response;

        response->url = jsondup(response_obj, "url");
        if (type == GIF) {
            response->source.gif = malloc(sizeof(gif_source));
            response->source.gif->anime_name = jsondup(response_obj, "anime_name");
        } else {
            response->source.png = malloc(sizeof(png_source));
            response->source.png->artist_name = jsondup(response_obj, "artist_name");
            response->source.png->artist_href = jsondup(response_obj, "artist_href");
            response->source.png->source_url = jsondup(response_obj, "source_url");
        }
    }

    // cleanup
    cJSON_Delete(json);
    free(http_response.text);
    return NEKOSBEST_OK;
}