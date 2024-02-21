#define NEKOSBEST_IMPL
#include <nekosbest.h>
#include "tests_common.h"

int main() {
    fprintf(stderr, WHITE BOLD "Fetching endpoints... ");

    // get endpoints
    nekos_endpoint_list endpoints;
    nekos_status status = nekos_endpoints(&endpoints);
    if (status != NEKOS_OK) {
        fprintf(stderr, RED "failed!" BOLD " Error code: %d\n", status);
        return EXIT_FAILURE;
    }
    fprintf(stderr, GREEN "success.\n\n");

    // print endpoints
    fprintf(stderr, WHITE BOLD "->" PINK);
    for (size_t i = 0; i < endpoints.len; i++)
        fprintf(stderr, " %s", endpoints.endpoints[i]->name);
    fprintf(stderr, "\n" WHITE);
    
    return EXIT_SUCCESS;
}
