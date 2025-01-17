#ifndef ELIZA_CONFIG_H
#define ELIZA_CONFIG_H

#include <stddef.h>

/*
 * Configuration System
 * Handles parsing and managing configuration data
 */

/* 
 * Configuration value types
 */
typedef enum {
    CONFIG_TYPE_STRING,
    CONFIG_TYPE_INT,
    CONFIG_TYPE_FLOAT,
    CONFIG_TYPE_BOOL,
    CONFIG_TYPE_OBJECT
} ConfigValueType;

/*
 * Configuration value structure
 * Can hold different types of values
 */
typedef struct {
    ConfigValueType type;
    union {
        char* string_val;
        int int_val;
        float float_val;
        int bool_val;
        struct ConfigObject* object_val;
    } value;
} ConfigValue;

/*
 * Configuration object structure
 * Holds key-value pairs
 */
typedef struct ConfigObject {
    char** keys;
    ConfigValue* values;
    size_t size;
    size_t capacity;
} ConfigObject;

/*
 * Function Declarations
 */

/* Create a new configuration object */
ConfigObject* eliza_config_create(void);

/* Destroy a configuration object */
void eliza_config_destroy(ConfigObject* config);

/* Parse a configuration file */
ConfigObject* eliza_config_parse_file(const char* filepath);

/* Get a value from the configuration */
ConfigValue* eliza_config_get(ConfigObject* config, const char* key);

/* Set a value in the configuration */
int eliza_config_set_string(ConfigObject* config, const char* key, const char* value);
int eliza_config_set_int(ConfigObject* config, const char* key, int value);
int eliza_config_set_float(ConfigObject* config, const char* key, float value);
int eliza_config_set_bool(ConfigObject* config, const char* key, int value);
int eliza_config_set_object(ConfigObject* config, const char* key, ConfigObject* value);

/* Save configuration to file */
int eliza_config_save(ConfigObject* config, const char* filepath);

#endif /* ELIZA_CONFIG_H */ 