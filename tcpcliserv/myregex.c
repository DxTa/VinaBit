#include <stdlib.h>
#include "stdbool.h"
#include "string.h"
#include "regex.h"

#define MAX_ERROR_MSG 0x1000

int compile_regex (regex_t * r, char * regex_text)
{
    int status = regcomp (r, regex_text, REG_EXTENDED|REG_NEWLINE);
    if (status != 0) {
	char error_message[MAX_ERROR_MSG];
	regerror (status, r, error_message, MAX_ERROR_MSG);
        printf ("Regex error compiling '%s': %s\n",
                 regex_text, error_message);
        return 1;
    }
    return 0;
}

/*
  Match the string in "to_match" against the compiled regular
  expression in "r".
 */

char* match_regex (regex_t * r, char * to_match)
{
    /* "P" is a pointer into the string which points to the end of the
       previous match. */
    const char * p = to_match;
    /* "N_matches" is the maximum number of matches allowed. */
    const int n_matches = 10;
    /* "M" contains the matches found. */
    regmatch_t m[n_matches];

    while (1) {
        int i = 0;
        int nomatch = regexec (r, p, n_matches, m, 0);
        if (nomatch) {
            printf ("No more matches.\n");
            return NULL;
        }
        for (i = 0; i < n_matches; i++) {
            int start;
            int finish;
            if (m[i].rm_so == -1) {
                break;
            }
            start = m[i].rm_so + (p - to_match);
            finish = m[i].rm_eo + (p - to_match);
            if (i == 0) {
				/* printf ("$& is "); */
            }
            else {
				/* printf ("$%d is ", i); */
				/* printf ("---%.*s--", finish - start); */
				char *temp;
				temp = (char*)malloc(strlen(to_match));
				sprintf(temp,"%.*s",(finish - start), to_match + start);
				return temp;
            }
			/* printf ("'%.*s' (bytes %d:%d)\n", (finish - start), to_match + start, start, finish); */
        }
        p += m[0].rm_eo;
    }
    return NULL;
}
