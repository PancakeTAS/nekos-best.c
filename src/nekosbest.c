#include "nekosbest.h"

static char API_URL[26 + 8 + 9 + 1] = "https://nekos.best/api/v2/";

typedef struct HTTPResponse {
    char *text; // not null-terminated
    size_t len;
} http_response;

static size_t http_response_write(void *ptr, size_t size, size_t nmemb, http_response *http_response) {
    // resize response text
    size_t new_len = http_response->len + size * nmemb;
    http_response->text = realloc(http_response->text, new_len + 1);
    if (http_response->text == NULL) {
        fprintf(stderr, "Failed to reallocate memory for http response message\n");
        exit(EXIT_FAILURE);
    }

    // copy new data to response text
    memcpy(http_response->text + http_response->len, ptr, size * nmemb);
    http_response->len = new_len;
    return size * nmemb;
}

static void do_request(http_response *http_response, char* url) {
    // create http response object
    http_response->text = calloc(1, 1);
    http_response->len = 0;
    if (http_response->text == NULL) {
        fprintf(stderr, "Failed to allocate memory for http response message\n");
        exit(EXIT_FAILURE);
    }

    // initialize curl
    CURL *curl = curl_easy_init();
    if (curl == NULL) {
        fprintf(stderr, "Failed to initialize curl\n");
        exit(EXIT_FAILURE);
    }

    // configure curl request
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_CA_CACHE_TIMEOUT, 604800L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_response_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, http_response);

    // make request
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "Failed to perform curl request: %s\n", curl_easy_strerror(res));
        exit(EXIT_FAILURE);
    }

    // cleanup curl
    curl_easy_cleanup(curl);
}

void endpoints(endpoint_list* list) {
    // create endpoint url
    memcpy(API_URL + 26, "endpoints\0", 10);

    // make request
    http_response http_response;
    do_request(&http_response, API_URL);

    // parse response
    cJSON *json = cJSON_ParseWithLength(http_response.text, http_response.len);
    if (!json || !cJSON_IsObject(json)) {
        fprintf(stderr, "Failed to parse JSON\n");
        exit(EXIT_FAILURE);
    }

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
}

void category(response_list *list, api_endpoint *endpoint, int amount) {
    // check if amount is valid
    if (amount < 1 || amount > 9) { // TODO: fix range
        fprintf(stderr, "Invalid amount: %d\n", amount);
        exit(EXIT_FAILURE);
    }

    // create endpoint url
    size_t endpoint_len = strlen(endpoint->name);
    memcpy(API_URL + 26, endpoint->name, endpoint_len);
    memcpy(API_URL + 26 + endpoint_len, "?amount=", 8);
    API_URL[26 + endpoint_len + 8] = (amount + '0');
    API_URL[26 + endpoint_len + 8 + 1] = '\0';

    // make request
    http_response http_response;
    do_request(&http_response, API_URL);

    // parse response
    cJSON *json = cJSON_ParseWithLength(http_response.text, http_response.len);
    if (!json || !cJSON_IsObject(json)) {
        fprintf(stderr, "Failed to parse JSON\n");
        exit(EXIT_FAILURE);
    }

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

        const cJSON *url_obj = cJSON_GetObjectItemCaseSensitive(response_obj, "url");
        response->url = malloc(strlen(url_obj->valuestring) + 1);
        strcpy(response->url, url_obj->valuestring);

        if (endpoint->format == GIF) {
            response->source.gif = malloc(sizeof(gif_source));

            const cJSON *anime_name_obj = cJSON_GetObjectItemCaseSensitive(response_obj, "anime_name");
            response->source.gif->anime_name = malloc(strlen(anime_name_obj->valuestring) + 1);
            strcpy(response->source.gif->anime_name, anime_name_obj->valuestring);
        } else {
            response->source.png = malloc(sizeof(png_source));

            const cJSON *artist_name_obj = cJSON_GetObjectItemCaseSensitive(response_obj, "artist_name");
            response->source.png->artist_name = malloc(strlen(artist_name_obj->valuestring) + 1);
            strcpy(response->source.png->artist_name, artist_name_obj->valuestring);

            const cJSON *artist_href_obj = cJSON_GetObjectItemCaseSensitive(response_obj, "artist_href");
            response->source.png->artist_href = malloc(strlen(artist_href_obj->valuestring) + 1);
            strcpy(response->source.png->artist_href, artist_href_obj->valuestring);

            const cJSON *source_url_obj = cJSON_GetObjectItemCaseSensitive(response_obj, "source_url");
            response->source.png->source_url = malloc(strlen(source_url_obj->valuestring) + 1);
            strcpy(response->source.png->source_url, source_url_obj->valuestring);
        }
    }

    // cleanup
    cJSON_Delete(json);
    free(http_response.text);
}

void search(response_list *list, char* query, int amount, response_format type, api_endpoint *category) {
    // check if amount is valid
    if (amount < 1 || amount > 9) {
        fprintf(stderr, "Invalid amount: %d\n", amount);
        exit(EXIT_FAILURE);
    }

    // create endpoint url
    char* url;
    if (category)
        url = malloc(26 + 5 + 1 + 6 + strlen(query) + 1 + 6 + 1 + 8 + 1 + 9 + strlen(category->name) + 1);
    else
        url = malloc(26 + 5 + 1 + 6 + strlen(query) + 1 + 6 + 1 + 8 + 1);
    
    memcpy(url, API_URL, 26);
    memcpy(url + 26, "search?query=", 13);
    memcpy(url + 26 + 13, query, strlen(query));
    memcpy(url + 26 + 13 + strlen(query), "&type=", 6);
    url[26 + 13 + strlen(query) + 6] = (type + '1');
    memcpy(url + 26 + 13 + strlen(query) + 6 + 1, "&amount=", 8);
    url[26 + 13 + strlen(query) + 6 + 1 + 8] = (amount + '0');

    if (category) {
        memcpy(url + 26 + 13 + strlen(query) + 6 + 1 + 8, "&category=", 10);
        memcpy(url + 26 + 13 + strlen(query) + 6 + 1 + 8 + 10, category->name, strlen(category->name));
        url[26 + 13 + strlen(query) + 6 + 1 + 8 + 10 + strlen(category->name)] = '\0';
    } else {
        url[26 + 13 + strlen(query) + 6 + 1 + 8 + 1] = '\0';
    }

    // make request
    http_response http_response;
    do_request(&http_response, url);

    // parse response
    cJSON *json = cJSON_ParseWithLength(http_response.text, http_response.len);
    if (!json || !cJSON_IsObject(json)) {
        fprintf(stderr, "Failed to parse JSON\n");
        exit(EXIT_FAILURE);
    }

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

        const cJSON *url_obj = cJSON_GetObjectItemCaseSensitive(response_obj, "url");
        response->url = malloc(strlen(url_obj->valuestring) + 1);
        strcpy(response->url, url_obj->valuestring);

        if (type == GIF) {
            response->source.gif = malloc(sizeof(gif_source));

            const cJSON *anime_name_obj = cJSON_GetObjectItemCaseSensitive(response_obj, "anime_name");
            response->source.gif->anime_name = malloc(strlen(anime_name_obj->valuestring) + 1);
            strcpy(response->source.gif->anime_name, anime_name_obj->valuestring);
        } else {
            response->source.png = malloc(sizeof(png_source));

            const cJSON *artist_name_obj = cJSON_GetObjectItemCaseSensitive(response_obj, "artist_name");
            response->source.png->artist_name = malloc(strlen(artist_name_obj->valuestring) + 1);
            strcpy(response->source.png->artist_name, artist_name_obj->valuestring);

            const cJSON *artist_href_obj = cJSON_GetObjectItemCaseSensitive(response_obj, "artist_href");
            response->source.png->artist_href = malloc(strlen(artist_href_obj->valuestring) + 1);
            strcpy(response->source.png->artist_href, artist_href_obj->valuestring);

            const cJSON *source_url_obj = cJSON_GetObjectItemCaseSensitive(response_obj, "source_url");
            response->source.png->source_url = malloc(strlen(source_url_obj->valuestring) + 1);
            strcpy(response->source.png->source_url, source_url_obj->valuestring);
        }
    }

    // cleanup
    cJSON_Delete(json);
    free(http_response.text);
    free(url);
}