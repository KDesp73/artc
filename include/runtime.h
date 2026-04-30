#ifndef ARTC_RUNTIME_H
#define ARTC_RUNTIME_H

/** Request graceful exit from the render loop (window closes / export stops). Safe from Lua or C. */
void artc_request_quit(void);

/** Non-zero after artc_request_quit() until the process exits. */
int artc_quit_is_requested(void);

#endif
