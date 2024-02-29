#define NEKOSBEST_IMPL
#include <nekosbest.h>
#include "tests_common.h"

int main() {
    fprintf(stderr, WHITE BOLD "Fetching image info from neko category... ");

    // create endpoint
    nekos_endpoint endpoint;
    endpoint.name = "neko";
    endpoint.format = NEKOS_PNG;

    // get image
    nekos_result_list results;
    nekos_status status = nekos_category(&results, &endpoint, 4);
    if (status != NEKOS_OK) {
        fprintf(stderr, RED "failed!" BOLD " Error code: %d\n", status);
        return EXIT_FAILURE;
    }
    fprintf(stderr, GREEN "success.\n");

    // print results
    fprintf(stderr, WHITE BOLD "-> \\\n");
    for (size_t i = 0; i < results.len; i++) {
        fprintf(stderr, PINK BOLD "  %s " WHITE BOLD "(" PINK BOLD "%s" WHITE BOLD ") made by " CYAN BOLD "%s" WHITE BOLD " (" PINK BOLD "%s" WHITE BOLD ")\n", results.responses[i]->url, results.responses[i]->source.png->source_url, results.responses[i]->source.png->artist_name, results.responses[i]->source.png->artist_href);
    }

    // free results
    nekos_free_results(&results, NEKOS_PNG);

    return EXIT_SUCCESS;
}
