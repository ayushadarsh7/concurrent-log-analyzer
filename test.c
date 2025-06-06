/* test_strcasestr.c */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>

int main(void) {
    const char *hay = "Hello World";
    const char *needle = "world";
    if (strcasestr(hay, needle) != NULL) {
        printf("strcasestr exists and works\n");
        return 0;
    }
    return 1;
}
