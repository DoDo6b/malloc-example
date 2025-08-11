#include "logger.h"


char _logFile[NAME_MAX];


int _flog (const char* fileCalledFrom, unsigned int lineCalledFrom, const char* fname, const char* format, ...)
{
    FILE* stream = NULL;
    if (strcmp (fname, "stdout") == 0 || strcmp (fname, "") == 0)       stream = stdout;
    else if (strcmp (fname, "stderr") == 0)                             stream = stderr;
    else                                                                stream = fopen(fname, "a");

    if (!stream)
    {
        fprintf (
            stderr,
            "%s:%d: flog: " ESC_BOLD ESC_RED "perror:" ESC_DEFAULT " can't open stream (received NULL)" ESC_RESET "\n",
            fileCalledFrom,
            lineCalledFrom
        );
        return -1;
    }

    char*       buffer = (char*) calloc (BUFSIZ, sizeof(char));
    char* const buffer_start = buffer;

    if (!buffer)
    {
        fprintf (
            stderr,
            "%s:%d: flog: " ESC_BOLD ESC_RED "allocation error:" ESC_DEFAULT " can't allocate buffer" ESC_RESET "\n",
            fileCalledFrom,
            lineCalledFrom
        );
        fclose(stream);
        return -1;
    }

    va_list args;
    va_start (args, format);

    vsprintf (buffer, format, args);

    va_end (args);

    while (*buffer != '\0')
    {
        switch (*buffer)
        {
        case SPECIFICATOR:
            switch (*++buffer)
            {
            case SPEC_RST:
                fprintf(stream, SWITCH(ESC_RESET, ""));
                break;
            case SPEC_BLD:
                fprintf(stream, SWITCH(ESC_BOLD, HTML_BOLD));
                break;
            case SPEC_ITA:
                fprintf(stream, SWITCH(ESC_ITALIC, HTML_ITALIC));
                break;
            case SPEC_CLOSE:
                switch (*++buffer)
                {
                    case SPEC_BLD:
                        fprintf(stream, SWITCH(ESC_BOLD_CLOSE, HTML_BOLD_CLOSE));
                        break;
                    case SPEC_ITA:
                        fprintf(stream, SWITCH(ESC_ITALIC_CLOSE, HTML_ITALIC_CLOSE));
                        break;
                    default:
                        fprintf(stream, "$/%c", *buffer);
                }
                break;
            case SPEC_BLK:
                fprintf(stream, SWITCH(ESC_BLACK, HTML_BLACK));
                break;
            case SPEC_RED:
                fprintf(stream, SWITCH(ESC_RED, HTML_RED));
                break;
            case SPEC_GRN:
                fprintf(stream, SWITCH(ESC_GREEN, HTML_GREEN));
                break;
            case SPEC_YLW:
                fprintf(stream, SWITCH(ESC_YELLOW, HTML_YELLOW));
                break;
            case SPEC_BLU:
                fprintf(stream, SWITCH(ESC_BLUE, HTML_BLUE));
                break;
            case SPEC_MGN:
                fprintf(stream, SWITCH(ESC_MAGENTA, HTML_MAGENTA));
                break;
            case SPEC_CYN:
                fprintf(stream, SWITCH(ESC_CYAN, HTML_CYAN));
                break;
            case SPEC_WHT:
                fprintf(stream, SWITCH(ESC_WHITE, HTML_WHITE));
                break;
            case SPEC_DFT:
                fprintf(stream, SWITCH(ESC_DEFAULT, HTML_DEFAULT));
                break;

            default:
                fprintf(stream, "%c%c", SPECIFICATOR, *buffer);
                break;
            }
            break;
        case '\n':
            fprintf(stream, SWITCH("", "<br>"));
        default:
            fprintf(stream, "%c", *buffer);
        }
        buffer++;
    }

    free (buffer_start);
    if (stream != stdout && stream != stderr) fclose (stream);
    return (int)(buffer - buffer_start);
}