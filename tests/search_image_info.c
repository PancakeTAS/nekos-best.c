#define NEKOSBEST_IMPL
#include <nekosbest.h>
#include "tests_common.h"

int main() {
    fprintf(stderr, WHITE BOLD "Searching for images with the query 'Mauve'... ");

    // search image
    nekos_result_list results;
    nekos_status status = nekos_search(&results, "Mauve", 5, NEKOS_PNG, NULL);
    if (status != NEKOS_OK) {
        fprintf(stderr, RED "failed!" BOLD " Error code: %d\n", status);
        return EXIT_FAILURE;
    }
    fprintf(stderr, GREEN "success.\n\n");

    // print results
    fprintf(stderr, WHITE BOLD "-> \\\n");
    for (size_t i = 0; i < results.len; i++) {
        fprintf(stderr, PINK BOLD "  %s " WHITE BOLD "(" PINK BOLD "%s" WHITE BOLD ") made by " CYAN BOLD "%s" WHITE BOLD " (" PINK BOLD "%s" WHITE BOLD ")\n", results.responses[i]->url, results.responses[i]->source.png->source_url, results.responses[i]->source.png->artist_name, results.responses[i]->source.png->artist_href);
    }

    return EXIT_SUCCESS;
}
