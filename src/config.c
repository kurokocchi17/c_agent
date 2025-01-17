#include "../include/config.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * Implementation of the Configuration System
 */

/* Initial capacity for configuration objects */
#define INITIAL_CAPACITY 16

/*
 * Create a new configuration object
 */
ConfigObject* eliza_config_create(void) {
    ConfigObject* config = (ConfigObject*)malloc(sizeof(ConfigObject));
    if (!config) return NULL;

    /* Allocate initial arrays */
    config->keys = (char**)malloc(INITIAL_CAPACITY * sizeof(char*));
    config->values = (ConfigValue*)malloc(INITIAL_CAPACITY * sizeof(ConfigValue));
    
    if (!config->keys || !config->values) {
        free(config->keys);
        free(config->values);
        free(config);
        return NULL;
    }

    config->size = 0;
    config->capacity = INITIAL_CAPACITY;

    return config;
}

/*
 * Free a configuration value
 */
static void free_config_value(ConfigValue* value) {
    if (!value) return;

    switch (value->type) {
        case CONFIG_TYPE_STRING:
            free(value->value.string_val);
            break;
        case CONFIG_TYPE_OBJECT:
            eliza_config_destroy(value->value.object_val);
            break;
        default:
            break;
    }
}

/*
 * Destroy a configuration object
 */
void eliza_config_destroy(ConfigObject* config) {
    if (!config) return;

    /* Free all keys and values */
    for (size_t i = 0; i < config->size; i++) {
        free(config->keys[i]);
        free_config_value(&config->values[i]);
    }

    free(config->keys);
    free(config->values);
    free(config);
}

/*
 * Find a key in the configuration
 * Returns the index if found, -1 if not found
 */
static int find_key(ConfigObject* config, const char* key) {
    for (size_t i = 0; i < config->size; i++) {
        if (strcmp(config->keys[i], key) == 0) {
            return (int)i;
        }
    }
    return -1;
}

/*
 * Ensure capacity for new entries
 */
static int ensure_capacity(ConfigObject* config) {
    if (config->size >= config->capacity) {
        size_t new_capacity = config->capacity * 2;
        char** new_keys = (char**)realloc(config->keys, new_capacity * sizeof(char*));
        ConfigValue* new_values = (ConfigValue*)realloc(config->values, new_capacity * sizeof(ConfigValue));

        if (!new_keys || !new_values) {
            free(new_keys);
            free(new_values);
            return -1;
        }

        config->keys = new_keys;
        config->values = new_values;
        config->capacity = new_capacity;
    }
    return 0;
}

/*
 * Get a value from the configuration
 */
ConfigValue* eliza_config_get(ConfigObject* config, const char* key) {
    if (!config || !key) return NULL;

    int index = find_key(config, key);
    if (index < 0) return NULL;

    return &config->values[index];
}

/*
 * Set a string value
 */
int eliza_config_set_string(ConfigObject* config, const char* key, const char* value) {
    if (!config || !key || !value) return -1;

    int index = find_key(config, key);
    if (index < 0) {
        /* Add new key-value pair */
        if (ensure_capacity(config) < 0) return -1;
        
        index = config->size++;
        config->keys[index] = strdup(key);
    } else {
        /* Free existing value if it's a string */
        if (config->values[index].type == CONFIG_TYPE_STRING) {
            free(config->values[index].value.string_val);
        }
    }

    config->values[index].type = CONFIG_TYPE_STRING;
    config->values[index].value.string_val = strdup(value);

    return 0;
}

/*
 * Set an integer value
 */
int eliza_config_set_int(ConfigObject* config, const char* key, int value) {
    if (!config || !key) return -1;

    int index = find_key(config, key);
    if (index < 0) {
        if (ensure_capacity(config) < 0) return -1;
        
        index = config->size++;
        config->keys[index] = strdup(key);
    }

    config->values[index].type = CONFIG_TYPE_INT;
    config->values[index].value.int_val = value;

    return 0;
}

/*
 * Set a float value
 */
int eliza_config_set_float(ConfigObject* config, const char* key, float value) {
    if (!config || !key) return -1;

    int index = find_key(config, key);
    if (index < 0) {
        if (ensure_capacity(config) < 0) return -1;
        
        index = config->size++;
        config->keys[index] = strdup(key);
    }

    config->values[index].type = CONFIG_TYPE_FLOAT;
    config->values[index].value.float_val = value;

    return 0;
}

/*
 * Set a boolean value
 */
int eliza_config_set_bool(ConfigObject* config, const char* key, int value) {
    if (!config || !key) return -1;

    int index = find_key(config, key);
    if (index < 0) {
        if (ensure_capacity(config) < 0) return -1;
        
        index = config->size++;
        config->keys[index] = strdup(key);
    }

    config->values[index].type = CONFIG_TYPE_BOOL;
    config->values[index].value.bool_val = value;

    return 0;
}

/*
 * Set an object value
 */
int eliza_config_set_object(ConfigObject* config, const char* key, ConfigObject* value) {
    if (!config || !key || !value) return -1;

    int index = find_key(config, key);
    if (index < 0) {
        if (ensure_capacity(config) < 0) return -1;
        
        index = config->size++;
        config->keys[index] = strdup(key);
    } else if (config->values[index].type == CONFIG_TYPE_OBJECT) {
        eliza_config_destroy(config->values[index].value.object_val);
    }

    config->values[index].type = CONFIG_TYPE_OBJECT;
    config->values[index].value.object_val = value;

    return 0;
}

/*
 * Parse a configuration file
 * Supports a simple key-value format:
 * key = value
 * section {
 *     key = value
 * }
 */
ConfigObject* eliza_config_parse_file(const char* filepath) {
    if (!filepath) return NULL;

    FILE* file = fopen(filepath, "r");
    if (!file) return NULL;

    ConfigObject* config = eliza_config_create();
    if (!config) {
        fclose(file);
        return NULL;
    }

    char line[1024];
    char key[256];
    char value[768];
    ConfigObject* current_section = config;

    while (fgets(line, sizeof(line), file)) {
        /* Skip comments and empty lines */
        if (line[0] == '#' || line[0] == '\n') continue;

        /* Handle section */
        if (strchr(line, '{')) {
            if (sscanf(line, "%s {", key) == 1) {
                ConfigObject* section = eliza_config_create();
                if (section) {
                    eliza_config_set_object(current_section, key, section);
                    current_section = section;
                }
            }
            continue;
        }

        /* Handle section end */
        if (strchr(line, '}')) {
            if (current_section != config) {
                current_section = config;
            }
            continue;
        }

        /* Parse key-value pair */
        if (sscanf(line, "%[^=] = %[^\n]", key, value) == 2) {
            /* Trim whitespace */
            char* k = key;
            char* v = value;
            while (*k == ' ') k++;
            while (*v == ' ') v++;
            char* end = k + strlen(k) - 1;
            while (end > k && *end == ' ') *end-- = '\0';
            end = v + strlen(v) - 1;
            while (end > v && *end == ' ') *end-- = '\0';

            /* Try to parse as different types */
            char* endptr;
            long int_val = strtol(v, &endptr, 10);
            if (*endptr == '\0') {
                eliza_config_set_int(current_section, k, (int)int_val);
                continue;
            }

            float float_val = strtof(v, &endptr);
            if (*endptr == '\0') {
                eliza_config_set_float(current_section, k, float_val);
                continue;
            }

            if (strcmp(v, "true") == 0 || strcmp(v, "false") == 0) {
                eliza_config_set_bool(current_section, k, strcmp(v, "true") == 0);
                continue;
            }

            eliza_config_set_string(current_section, k, v);
        }
    }

    fclose(file);
    return config;
}

/*
 * Save configuration to file
 */
static void write_config_value(FILE* file, ConfigValue* value, int indent) {
    if (!file || !value) return;

    for (int i = 0; i < indent; i++) fprintf(file, "    ");

    switch (value->type) {
        case CONFIG_TYPE_STRING:
            fprintf(file, "%s", value->value.string_val);
            break;
        case CONFIG_TYPE_INT:
            fprintf(file, "%d", value->value.int_val);
            break;
        case CONFIG_TYPE_FLOAT:
            fprintf(file, "%f", value->value.float_val);
            break;
        case CONFIG_TYPE_BOOL:
            fprintf(file, "%s", value->value.bool_val ? "true" : "false");
            break;
        case CONFIG_TYPE_OBJECT:
            fprintf(file, "{\n");
            ConfigObject* obj = value->value.object_val;
            for (size_t i = 0; i < obj->size; i++) {
                for (int j = 0; j < indent + 1; j++) fprintf(file, "    ");
                fprintf(file, "%s = ", obj->keys[i]);
                write_config_value(file, &obj->values[i], indent + 1);
                fprintf(file, "\n");
            }
            for (int i = 0; i < indent; i++) fprintf(file, "    ");
            fprintf(file, "}");
            break;
    }
}

/*
 * Save configuration to file
 */
int eliza_config_save(ConfigObject* config, const char* filepath) {
    if (!config || !filepath) return -1;

    FILE* file = fopen(filepath, "w");
    if (!file) return -1;

    for (size_t i = 0; i < config->size; i++) {
        fprintf(file, "%s = ", config->keys[i]);
        write_config_value(file, &config->values[i], 0);
        fprintf(file, "\n");
    }

    fclose(file);
    return 0;
} 