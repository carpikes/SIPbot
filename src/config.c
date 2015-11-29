#include "common.h"
#include "config.h"

int config_init(const char *filename) {
    size_t keyr, valr;
    int i, ret = -1, line = 1;
    char *key = NULL, *value = NULL;
    size_t key_size = 0, value_size = 0;
    FILE *fp = fopen(filename, "r");

    if(!fp)
        return ret;

    while(!feof(fp)) {
        keyr = getdelim(&key, &key_size, '=', fp);
        valr = getdelim(&value, &value_size, '\n', fp);
       
        if(!feof(fp) && keyr > 0 && valr > 0) {
            if(key[0] == '#')
                goto inl_clean;

            key[--keyr] = 0;
            for(i=0; i<N_NAMES; i++)
                if(!strcmp(names[i], key)) {
                    config.val[i] = value;
                    value = NULL;
                    break;
                }

            if(i == N_NAMES) {
                fprintf(stderr, "Setting at line %d is not used.\n", line);
                goto fail;
            }
        }

        /* Useless line, free memory */
inl_clean:
        if(key) { free(key); key = NULL; }
        if(value) { free(value); value = NULL; }
        key_size = value_size = 0;

        ++line;
    }

    ret = 1;
fail:
    if(key) free(key);
    if(value) free(value); 
    if(fp) fclose(fp);
    return ret;
}

void config_free() {
    int i;
    for(i=0;i<N_NAMES;i++)
        free(config.val[i]);
}

int config_readint(uint8_t pos) {
    if(pos >= N_NAMES)
        return 0;

    return atoi(config.val[pos]);
}

const char *config_readstring(uint8_t pos) {
    if(pos >= N_NAMES)
        return 0;

    return config.val[pos];
}

