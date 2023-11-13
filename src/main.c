#include <stdio.h>

void input(char *buf) {
    gets(buf);
}

void other(void) {
    fprintf(stderr, "other\n");
}

int main(void) {
    char buf[8];

    input(buf);
    printf("&p\n", &other);

    return 0;
}

