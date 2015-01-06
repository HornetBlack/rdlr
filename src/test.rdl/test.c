
#include <stdio.h>
#include <stdlib.h>

const char *EVENTS[] = {
    "init",
    NULL
};

/* Event init() */
void init(void)
{
    printf("test.rdl init event\n");
}
