#include "../include/model.h"
#include <stdlib.h>
#include <string.h>

/*
 * Implementation of the Language Model Interface
 */

/* 
 * Create a new model configuration with default values
 */
ModelConfig* eliza_model_config_create(void) {
    ModelConfig* config = (ModelConfig*)malloc(sizeof(ModelConfig));
    if (!config) return NULL;

    /* Initialize with safe defaults */
    config->model_name = NULL;
    config->api_key = NULL;
    config->api_endpoint = NULL;
    config->temperature = 0.7f;
    config->max_tokens = 1024;
    config->custom_config = NULL;

    return config;
}

/*
 * Free all resources associated with a model configuration
 */
void eliza_model_config_destroy(ModelConfig* config) {
    if (!config) return;

    free(config->model_name);
    free(config->api_key);
    free(config->api_endpoint);
    /* Note: custom_config cleanup is responsibility of the specific model implementation */
    free(config);
}

/*
 * Set configuration values based on key-value pairs
 * Returns 0 on success, -1 on failure
 */
int eliza_model_config_set(ModelConfig* config, const char* key, const void* value) {
    if (!config || !key || !value) return -1;

    if (strcmp(key, "model_name") == 0) {
        free(config->model_name);
        config->model_name = strdup((const char*)value);
    }
    else if (strcmp(key, "api_key") == 0) {
        free(config->api_key);
        config->api_key = strdup((const char*)value);
    }
    else if (strcmp(key, "api_endpoint") == 0) {
        free(config->api_endpoint);
        config->api_endpoint = strdup((const char*)value);
    }
    else if (strcmp(key, "temperature") == 0) {
        config->temperature = *(const float*)value;
    }
    else if (strcmp(key, "max_tokens") == 0) {
        config->max_tokens = *(const int*)value;
    }
    else {
        return -1; /* Unknown key */
    }

    return 0;
}

/*
 * Create a new model instance
 * The specific implementation is determined by the model_name in config
 */
Model* eliza_model_create(ModelConfig* config) {
    if (!config || !config->model_name) return NULL;

    Model* model = (Model*)malloc(sizeof(Model));
    if (!model) return NULL;

    /* Initialize with safe defaults */
    model->initialize = NULL;
    model->generate = NULL;
    model->cleanup = NULL;
    model->model_data = NULL;

    /* TODO: Based on model_name, set up the appropriate implementation */
    /* For now, we'll just return the empty structure */

    return model;
}

/*
 * Clean up and free all resources associated with a model
 */
void eliza_model_destroy(Model* model) {
    if (!model) return;

    if (model->cleanup) {
        model->cleanup();
    }

    free(model);
} 