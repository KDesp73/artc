#include "cli.h"
#include "files.h"
#include "io/cli.h"
#include "io/logging.h"
#include "version.h"

const char* template = ""
"window(800, 800)\n"
"palette(\"catppuccin\")\n"
"bg(palette.black)\n"
"fps(30)\n"
"\n"
"function setup()\n"
"end\n"
"\n"
"function update(dt)\n"
"end\n";


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
        "gif", "mp4", "png"
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
        cli_arg_new('h', "help", "Prints this message", no_argument),
        cli_arg_new('v', "version", "Prints the current version and exits", no_argument),
        cli_arg_new('x', "export", "Export the render in a media file", no_argument),
        cli_arg_new('F', "format", "Specify the format of the exported file", required_argument),
        cli_arg_new('o', "output", "Specify a different output path", required_argument),
        cli_arg_new('A', "ascii", "Render visuals in the terminal", no_argument),
        cli_arg_new('S', "no-sandbox", "Do not sandbox lua (Be careful)", no_argument),
        cli_arg_new('d', "duration", "Exit after the provided amount of seconds", required_argument),
        cli_arg_new('t', "template", "Generate a template lua file", no_argument),
        NULL
    );

    int opt;
    LOOP_ARGS(opt, args) {
        switch (opt) {
            case 'h':
                cli_help(args, "artc <FILE> [<OPTIONS>...]", "Written by KDesp73");
                // help();
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
                // TODO: validate input
                v->durations_s = (size_t) atoi(optarg);
                break;
            case 't':
                file_write("template.lua", template);
                INFO("Created 'template.lua'");
                exit(0);
            default:
                exit(1);
        }
    }
    cli_args_free(&args);
}

