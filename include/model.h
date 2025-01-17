#ifndef ELIZA_MODEL_H
#define ELIZA_MODEL_H

#include <stddef.h>

/*
 * Language Model Interface
 * Defines the structure and functions for interacting with AI language models
 */

/* 
 * Model configuration structure
 * Contains settings for model initialization and operation
 */
typedef struct {
    char* model_name;        /* Name/identifier of the model to use */
    char* api_key;          /* API key for cloud-based models */
    char* api_endpoint;     /* API endpoint for cloud-based models */
    float temperature;      /* Sampling temperature for generation */
    int max_tokens;         /* Maximum tokens in response */
    void* custom_config;    /* Additional model-specific configuration */
} ModelConfig;

/* 
 * Model interface structure
 * Contains function pointers for model operations
 */
typedef struct {
    /* Initialize the model with given configuration */
    int (*initialize)(ModelConfig* config);
    
    /* Generate a response given a prompt */
    char* (*generate)(const char* prompt);
    
    /* Cleanup and free model resources */
    void (*cleanup)(void);
    
    /* Model-specific data */
    void* model_data;
} Model;

/*
 * Function Declarations
 */

/* Create a new model instance with the given configuration */
Model* eliza_model_create(ModelConfig* config);

/* Destroy a model instance and free all associated resources */
void eliza_model_destroy(Model* model);

/* Create a new model configuration */
ModelConfig* eliza_model_config_create(void);

/* Destroy a model configuration and free all associated resources */
void eliza_model_config_destroy(ModelConfig* config);

/* Helper function to set model configuration values */
int eliza_model_config_set(ModelConfig* config, const char* key, const void* value);

#endif /* ELIZA_MODEL_H */ 