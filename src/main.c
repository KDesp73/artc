#include <stdio.h>
#include "version.h"

int main(int argc, char** argv)
{
    int maj, min, pat;
    version(&maj, &min, &pat);
    printf("artc v%d.%d.%d\n", maj, min, pat);

    return 0;
}
