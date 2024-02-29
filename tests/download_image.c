#define NEKOSBEST_IMPL
#include <nekosbest.h>
#include "tests_common.h"

#define URL "https://nekos.best/api/v2/neko/4c8285d0-60a9-4ccf-ac61-3bf744fafa03.png"
#define SIZE 1138412

int main() {
    fprintf(stderr, WHITE BOLD "Downloading image... ");

    // download image
    nekos_http_response http_response;
    nekos_status status = nekos_download(&http_response, URL);
    if (status != NEKOS_OK) {
        fprintf(stderr, RED "failed!" BOLD " Error code: %d\n", status);
        return EXIT_FAILURE;
    }

    // check size
    if (http_response.len != SIZE) {
        fprintf(stderr, RED "failed!" BOLD " Size mismatch: %ld != %d\n", http_response.len, SIZE);
        return EXIT_FAILURE;
    }
    fprintf(stderr, GREEN "success.\n");
    fprintf(stderr, WHITE BOLD "-> filesize matches\n");

    // free response
    nekos_free_http_response(&http_response);
    
    return EXIT_SUCCESS;
}
