#define NEKOSBEST_IMPL
#include <nekosbest.h>
#include "tests_common.h"

int main() {
    fprintf(stderr, WHITE BOLD "Finding a specific endpoint... ");

    // get endpoints
    nekos_endpoint_list endpoints;
    nekos_status status = nekos_endpoints(&endpoints);
    if (status != NEKOS_OK) {
        fprintf(stderr, RED "failed!" BOLD " Error code: %d\n", status);
        return EXIT_FAILURE;
    }

    // find endpoint
    nekos_endpoint* endpoint = nekos_find_endpoint(&endpoints, "kiss");
    if (endpoint == NULL) {
        fprintf(stderr, RED "failed!" BOLD " Endpoint not found.\n");
        return EXIT_FAILURE;
    }
    fprintf(stderr, GREEN "success.\n");

    // print endpoint information
    fprintf(stderr, WHITE BOLD "->" PINK " %s: %s\n" WHITE, endpoint->name, endpoint->format == NEKOS_PNG ? "PNG" : "GIF");

    // free endpoints
    nekos_free_endpoints(&endpoints);

    return EXIT_SUCCESS;
}
