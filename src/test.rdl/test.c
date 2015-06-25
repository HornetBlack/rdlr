
#include <unistd.h>

const char *EVENTS[] = {
    "init",
    NULL
};

/* Event init() */
void init(void)
{
    const char hello[] = "Hello from test.rdl\n";
    const size_t hello_size = sizeof(hello);
    write(1, hello, hello_size);
}
