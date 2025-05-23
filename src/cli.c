#include "cli.h"
#include "help.h"
#include "io/cli.h"
#include "io/logging.h"
#include "version.h"


void CliValuesInit(CliValues* v)
{
    v->export = false;
    v->format = "mp4";
    v->output = NULL;
    v->ascii = false;
    v->sandbox = true;
    v->durations_s = 0;
}

static bool validate_format(char* fmt)
{
    char* allowed[] = {
        "gif", "mp4",
    };
    for(size_t i = 0; i < sizeof(allowed)/sizeof(allowed[0]); i++){
        if(!strcmp(fmt, allowed[i])) return true;
    }

    return false;
}

void CliParse(CliValues* v, int argc, char** argv)
{
    int maj, min, pat;
    version(&maj, &min, &pat);

    cli_args_t args = cli_args_make(
        cli_arg_new('h', "help", "", no_argument),
        cli_arg_new('v', "version", "", no_argument),
        cli_arg_new('x', "export", "", no_argument),
        cli_arg_new('F', "format", "", required_argument),
        cli_arg_new('o', "output", "", required_argument),
        cli_arg_new('A', "ascii", "", no_argument),
        cli_arg_new('S', "no-sandbox", "", no_argument),
        cli_arg_new('d', "duration", "", required_argument),
        NULL
    );

    int opt;
    LOOP_ARGS(opt, args) {
        switch (opt) {
            case 'h':
                help();
                exit(0);
            case 'v':
                printf("artc v%d.%d.%d\n", maj, min, pat);
                exit(0);
            case 'x':
                v->export = true;
                break;
            case 'F':
                v->format = optarg;
                if(!validate_format(v->format)) {
                    ERRO("Invalid format: %s", v->format);
                    exit(1);
                }

                break;
            case 'o':
                v->output = optarg;
                break;
            case 'A':
                v->ascii = true;
                break;
            case 'S':
                v->sandbox = false;
                break;
            case 'd':
                v->durations_s = (size_t) atoi(optarg);
                break;
            default:
                exit(1);
        }
    }
    cli_args_free(&args);
}

