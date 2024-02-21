#define NEKOSBEST_IMPL
#include <nekosbest.h>
#include "tests_common.h"

int main() {
    fprintf(stderr, WHITE BOLD "Searching for gifs with the query 'Citrus' in the category 'kiss'... ");

    // create endpoint
    nekos_endpoint endpoint;
    endpoint.name = "kiss";
    endpoint.format = NEKOS_GIF;

    // search gif
    nekos_result_list results;
    nekos_status status = nekos_search(&results, "Citrus", 1, NEKOS_GIF, &endpoint);
    if (status != NEKOS_OK) {
        fprintf(stderr, RED "failed!" BOLD " Error code: %d\n", status);
        return EXIT_FAILURE;
    }
    fprintf(stderr, GREEN "success.\n");

    // print results
    fprintf(stderr, WHITE BOLD "-> \\\n");
    for (size_t i = 0; i < results.len; i++) {
        fprintf(stderr, PINK BOLD "  %s" WHITE BOLD " from " CYAN BOLD "%s\n", results.responses[i]->url, results.responses[i]->source.gif->anime_name);
    }

    return EXIT_SUCCESS;
}
