#include "common.h"
#include "config.h"

int config_init(const char *filename) {
    size_t keyr, valr;
    int i, ret = -1, line = 0;
    char *key = NULL, *value = NULL;
    size_t key_size = 0, value_size = 0;
    FILE *fp;

    if(filename == NULL)
        exit(1);

    fp = fopen(filename, "r");

    if(!fp)
        return ret;

    while(!feof(fp)) {
        ++line;
        keyr = getdelim(&key, &key_size, '=', fp);
        valr = getdelim(&value, &value_size, '\n', fp);
       
        if(keyr > 0 && valr > 0) {
            if(key[0] == '#' || feof(fp))
                goto inl_clean;

            key[--keyr] = 0;
            value[--valr] = 0;
            for(i=0; i<N_NAMES; i++)
                if(!strcmp(names[i], key)) {
                    config.val[i] = value;
                    value = NULL;
                    break;
                }

            if(i == N_NAMES) {
                fprintf(stderr, "[config] Error at line %d: invalid line\n", line);
                goto fail;
            }
        }

        /* Useless line, free memory */
inl_clean:
        if(key) { free(key); key = NULL; }
        if(value) { free(value); value = NULL; }
        key_size = value_size = 0;
    }

    config.config_file = filename;
    ret = 1;
fail:
    if(key) free(key);
    if(value) free(value); 
    if(fp) fclose(fp);
    return ret;
}

void config_reload(void) {
    int i;
    char *old[N_NAMES];

    if(config.config_file == NULL) {
        exit(1);
    }

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

void config_free(void) {
    int i;
    for(i=0;i<N_NAMES;i++)
        if(config.val[i]) {
            free(config.val[i]);
            config.val[i] = NULL;
        }
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

