#include "runtime.h"

volatile int artc_quit_requested;

void artc_request_quit(void)
{
    artc_quit_requested = 1;
}

int artc_quit_is_requested(void)
{
    return artc_quit_requested != 0;
}
