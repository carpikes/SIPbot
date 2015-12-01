#include "common.h"
#include "config.h"

/** Check if this character is not a useful character (space, newline, equal */
int is_delimiter(char c) {
    return isspace(c) || c == '=' || c == '\r';
}

/** Trim the word given in input */
char *getword(char *str, int len) {
    char *ret = NULL;
    int s=0,e=0;

    /* Left trim */
    while(s<len && is_delimiter(str[s]))
        ++s;

    /* Right trim */
    e = s;
    while(e<len && !is_delimiter(str[e]))
        ++e;

    /* Duplicate and return */
    ret = (char *) calloc(e-s+1,1);
    memcpy(ret, str+s, e-s);
    printf("WORD: '%s'\n", ret);
    return ret;
}

/** Read config file */
int config_init(const char *filename) {
    size_t i, bufr;
    int eq = 0;
    int ret = -1, line = 0;
    char *lbuf = NULL, *key = NULL, *value = NULL;
    size_t buf_size = 0;
    FILE *fp;

    if(filename == NULL)
        exit(1);

    fp = fopen(filename, "r");

    if(!fp)
        return ret;

    while(!feof(fp)) {
        ++line;
        bufr = getline(&lbuf, &buf_size, fp); 
        if(bufr > 1 && !feof(fp)) {
            /* remove newline */
            --bufr; 

            /* comment? mark that as end of line */
            for(i=0;i<bufr;i++) 
                if(lbuf[i] == '#')
                    bufr = i;

            /* check if this line is skippable without warnings */
            for(i=0;i<bufr;i++)
                if(lbuf[i] != ' ' && lbuf[i] != '\t')
                    break;

            if(i == bufr) { 
                goto clean_line;
            }

            /* find the equal sign */
            eq = -1;
            for(i=0;i<bufr;i++)
                if(lbuf[i] == '=') {
                    eq = i;
                    break;
                }

            if(eq < 1)
                goto err;

            key = getword(lbuf, eq);
            value = getword(lbuf+eq+1, bufr-eq-1);

            if(key == NULL || value == NULL)
                goto err;

            if(strlen(key) == 0 || strlen(value) == 0)
                goto err;

            for(i=0; i<N_NAMES; i++)
                if(!strcmp(names[i], key)) {
                    config.val[i] = value;
                    value = NULL;
                    break;
                }

            if(i == N_NAMES)
                goto err;
clean_line:
            if(lbuf) free(lbuf);
            buf_size = 0;
        }
    }

    config.config_file = filename;
    ret = 1;

end:
    if(ret != 1) config_free();
    if(key) free(key);
    if(value) free(value); 
    if(lbuf) free(lbuf);
    if(fp) fclose(fp);
    return ret;

err:
    fprintf(stderr, "[config] Error at line %d: invalid line\n", line);
    goto end;
}

/** Reload config file */
void config_reload(void) {
    int i;
    char *old[N_NAMES];

    if(config.config_file == NULL)
        exit(1);

    fprintf(stderr, "[config] Reloading configuration file\n");

    /* Save old config */
    for(i=0;i<N_NAMES;i++) {
        old[i] = config.val[i];
        config.val[i] = NULL;
    }

    /* Reload config */
    if(config_init(config.config_file) == -1) {
        fprintf(stderr, "[config] Error while loading configuration file\n");
        for(i=0;i<N_NAMES;i++) {
            if(config.val[i])
                free(config.val[i]);
            config.val[i] = old[i];
        }
    } else {
        fprintf(stderr, "[config] Reload ok\n");
        for(i=0;i<N_NAMES;i++) 
            if(old[i])
                free(old[i]);
    }
}

/** Free configuration */
void config_free(void) {
    int i;
    for(i=0;i<N_NAMES;i++)
        if(config.val[i]) {
            free(config.val[i]);
            config.val[i] = NULL;
        }
}

/** Get config line as integer */
int config_readint(uint8_t pos) {
    if(pos >= N_NAMES)
        return 0;

    return atoi(config.val[pos]);
}

/** Get config line as string */
const char *config_readstring(uint8_t pos) {
    if(pos >= N_NAMES)
        return 0;

    return config.val[pos];
}

