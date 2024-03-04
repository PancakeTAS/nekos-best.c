# nekos-best.c
A simple, lightweight c wrapper for [nekos.best](https://nekos.best/) API compliant with C99.

Join the official Discord server [here](https://nekos.best/discord?ref=py).

## Requirements
- libcurl (tested with 8.6.0-3)
- cjson (tested with 1.7.17-1)

## Installation
Copy the single header file to source include directory and include the header. Specify `NEKOSBEST_IMPL` before including the header in the source file where you want to use the library.

```c
#define NEKOSBEST_IMPL
#include "nekosbest.h"
```

## Documentation
Doxygen documentation is available [here](https://pancake.gay/nekos-best.c).
(Jump to [nekosbest.h](https://pancake.gay/nekos-best.c/nekosbest_8h.html))

## Example
```c
#define NEKOSBEST_IMPL
#include <nekosbest.h>

#include <stdio.h>
#include <stdlib.h>

int main() {
    // manually create endpoint (use nekos_endpoints() to get all endpoints)
    nekos_endpoint endpoint;
    endpoint.name = "neko";
    endpoint.format = NEKOS_PNG;

    // fetch image
    nekos_result_list results;
    nekos_category(&results, &endpoint, 1);

    // download first image
    nekos_http_response http_response;
    nekos_download(&http_response, results.responses[0].url);

    // save image to file
    FILE *file = fopen("neko.png", "wb");
    fwrite(http_response.text, 1, http_response.len, file);
    fclose(file);

    // cleanup
    nekos_free_results(&results, endpoint.format);
    nekos_free_http_response(&http_response);
    return EXIT_SUCCESS;
}
```
This example fetches an image from the 'neko' endpoint saves it to a file. Please note that this example does not handle errors.

For more examples, see `tests/`.
