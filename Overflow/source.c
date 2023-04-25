#include <stdio.h>
#include <stdlib.h>

void target() {
    printf("You find a secret room!\n");
    exit(0);
}

void getbuf() {
    char buf[16];
    gets(buf);
    return;
}

int main() {
    printf("Enter the password to the secret root:\n");
    getbuf();
    printf("Failed to find the secret room!\n");

    return 0;
}