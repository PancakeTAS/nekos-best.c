#include "nekosbest.h"

int main() {
    // get endpoints
    endpoint_list list;
    endpoints(&list);
    
    // print endpoints
    printf("Available endpoints:\n");
    for (size_t i = 0; i < list.len; i++) {
        printf(" %s", list.endpoints[i]->name);
    }
    printf("\n\n");

    // get 'neko' images
    response_list list2;
    category(&list2, list.endpoints[0], 4);

    // print results
    printf("Results:\n");
    for (size_t i = 0; i < list2.len; i++) {
        printf("  - URL: %s\n  Artist name: %s\n  Artist href: %s\n  Source url: %s\n", list2.responses[i]->url, list2.responses[i]->source.png->artist_name, list2.responses[i]->source.png->artist_href, list2.responses[i]->source.png->source_url);
    }
    printf("\n");

    // get 'kiss' gifs
    response_list list3;
    category(&list3, list.endpoints[34], 4);

    // print results
    printf("Results:\n");
    for (size_t i = 0; i < list3.len; i++) {
        printf("  - URL: %s\n  Anime name: %s\n", list3.responses[i]->url, list3.responses[i]->source.gif->anime_name);
    }
    printf("\n");

    // search for 'menhera' images
    response_list list4;
    search(&list4, "Senko", 2, GIF, list.endpoints[31]);

    // print results
    printf("Results:\n");
    for (size_t i = 0; i < list4.len; i++) {
        printf("  - URL: %s\n  Anime name: %s\n", list4.responses[i]->url, list4.responses[i]->source.gif->anime_name);
    }
    printf("\n");
}
