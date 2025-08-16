#include "logger.h"


char LogFile[NAME_MAX] = "";
FILE* LogStream = NULL;
static char Buffer[BUFSIZ] = "";


FILE* log_start (const char* fname)
{
    FILE* buffer = NULL;
    if      (!strncmp(fname, "stdout", NAME_MAX))   LogStream = stdout;
    else if (!strncmp(fname, "stderr", NAME_MAX))   LogStream = stderr;
    else
    {
        buffer = fopen (fname, "a");
        if (!buffer)
        {
            fprintf
            (
                stderr,
                "%s:%d: %s:" ESC_BOLD ESC_RED "fopen error:" ESC_DEFAULT " fopen returned a NULL" ESC_BOLD_CLOSE "\n",
                __FILE__,
                __LINE__,
                __func__
            );
            return NULL;
        }
        LogStream = buffer;

        atexit (log_close);
    }

    strncpy (LogFile, fname, NAME_MAX);

    fprintf (LogStream, "<pre>\n");
    return LogStream;
}

#define PRINT_SPEC_(spec) fprintf(LogStream, ( (LogStream == stdout || LogStream == stderr) ? ESC_ ## spec : HTML_ ## spec) )

int log_string (const char* format, ...)
{
    if (format[0] == '\0') return 0;
    if (!LogStream)
    {
        fprintf
        (
            stderr,
            "%s:%d: %s:" ESC_BOLD ESC_RED "error:" ESC_DEFAULT " LogStream wasn't open, call log_start()" ESC_BOLD_CLOSE "\n",
            __FILE__,
            __LINE__,
            __func__
        );
        return -1;
    }

    va_list args;
    va_start (args, format);

    vsprintf (Buffer, format, args);

    va_end (args);

    size_t printed = 0;
    for (printed = 0; Buffer[printed] != '\0' && printed < sizeof(Buffer); printed++)
    {
        if (Buffer[printed] == SPECIFICATOR)
        {
            switch (Buffer[++printed])
            {
                case SPEC_RST:  PRINT_SPEC_(RESET);     break;
                case SPEC_BLD:  PRINT_SPEC_(BOLD);      break;
                case SPEC_ITA:  PRINT_SPEC_(ITALIC);    break;
                case SPEC_BLK:  PRINT_SPEC_(BLACK);     break;
                case SPEC_RED:  PRINT_SPEC_(RED);       break;
                case SPEC_GRN:  PRINT_SPEC_(GREEN);     break;
                case SPEC_YLW:  PRINT_SPEC_(YELLOW);    break;
                case SPEC_BLU:  PRINT_SPEC_(BLUE);      break;
                case SPEC_CYN:  PRINT_SPEC_(CYAN);      break;
                case SPEC_MGN:  PRINT_SPEC_(MAGENTA);   break;
                case SPEC_WHT:  PRINT_SPEC_(WHITE);     break;
                case SPEC_DFT:  PRINT_SPEC_(DEFAULT);   break;

                case SPEC_CLOSE:
                    switch (Buffer[++printed])
                    {
                        case SPEC_BLD:  PRINT_SPEC_(BOLD_CLOSE);      break;
                        case SPEC_ITA:  PRINT_SPEC_(ITALIC_CLOSE);    break;

                        default:        fprintf(LogStream, "%c%c%c", SPECIFICATOR, SPEC_CLOSE, Buffer[printed]);
                    }
                    break;

                default:    fprintf(LogStream, "%c%c", SPECIFICATOR, Buffer[printed]);
            }
        }
        else fprintf(LogStream, "%c", Buffer[printed]);
    }

    if (LogStream != stderr || LogStream != stdout) fflush (LogStream);
    return (int)printed;
}

#undef PRINT_SPEC_

void log_close()
{
    fprintf (LogStream, "\n</pre>\n");
    if (LogStream && (LogStream != stdout || LogStream != stderr) ) fclose (LogStream);
}

const char* get_log()
{
    return LogFile;
}