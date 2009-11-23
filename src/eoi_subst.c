#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <libkeys.h>

char *
eoi_subst_keys(const char* text, keys_t* keys, const char* context)
{
    char *bp;
    size_t size;
    FILE *stream;

    stream = open_memstream (&bp, &size);

    while(*text)
    {
        if(*text == '@')
        {
            text+=1;
            char* end = text;
            while(*end!='@')
            {
                if(!*end)
                {
                    fputs(text, stream);
                    fprintf(stderr, "substitute keys: malformed input\n");
                    goto done;
                }
                ++end;
            }
            char* macro = strndup(text, end-text);
            Eina_List* reverse = keys_reverse_lookup(keys, context, macro);
            char* keysym = NULL;
            if(reverse)
            {
                keysym = eina_list_nth(reverse, 0);
            }
            if(keysym)
                fputs(keys_get_key_name(keysym), stream);
            else
                fputs(macro, stream);
            text = end;
            free(macro);
        }
        else
            fputc(*text, stream);
        ++text;
    }

done:
    fclose(stream);
//    printf("subst: %s\n", bp);
    return bp;
}
