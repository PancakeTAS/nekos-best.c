#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

typedef enum ReponseFormat {
    PNG,
    GIF
} response_format;

typedef struct ApiEndpoint {
    char *name;
    response_format format;
} api_endpoint;

typedef struct EndpointList {
    api_endpoint **endpoints;
    size_t len;
} endpoint_list;

void endpoints(endpoint_list *list);
