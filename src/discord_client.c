#include "../include/discord_client.h"
#include "../include/discord_gateway.h"
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>

/*
 * Implementation of the Discord Client
 */

#define DISCORD_API_BASE "https://discord.com/api/v10"
#define INITIAL_CHANNELS_CAPACITY 16

/* Internal client data */
typedef struct {
    DiscordConfig* config;
    DiscordGateway* gateway;
    int is_connected;
    Message** message_queue;
    size_t queue_size;
    size_t queue_capacity;
} DiscordClientData;

/*
 * Create Discord configuration
 */
DiscordConfig* eliza_discord_config_create(void) {
    DiscordConfig* config = (DiscordConfig*)malloc(sizeof(DiscordConfig));
    if (!config) return NULL;

    config->token = NULL;
    config->prefix = strdup("!");  /* Default prefix */
    config->allowed_channels = (char**)malloc(INITIAL_CHANNELS_CAPACITY * sizeof(char*));
    config->num_channels = 0;

    if (!config->prefix || !config->allowed_channels) {
        free(config->token);
        free(config->prefix);
        free(config->allowed_channels);
        free(config);
        return NULL;
    }

    return config;
}

/*
 * Destroy Discord configuration
 */
void eliza_discord_config_destroy(DiscordConfig* config) {
    if (!config) return;

    free(config->token);
    free(config->prefix);
    
    for (size_t i = 0; i < config->num_channels; i++) {
        free(config->allowed_channels[i]);
    }
    free(config->allowed_channels);
    
    free(config);
}

/*
 * Add an allowed channel
 */
int eliza_discord_add_channel(DiscordConfig* config, const char* channel_id) {
    if (!config || !channel_id) return -1;

    /* Check if we need to resize the channels array */
    if (config->num_channels >= INITIAL_CHANNELS_CAPACITY) {
        size_t new_capacity = INITIAL_CHANNELS_CAPACITY * 2;
        char** new_channels = (char**)realloc(config->allowed_channels,
                                            new_capacity * sizeof(char*));
        if (!new_channels) return -1;
        config->allowed_channels = new_channels;
    }

    config->allowed_channels[config->num_channels] = strdup(channel_id);
    if (!config->allowed_channels[config->num_channels]) return -1;

    config->num_channels++;
    return 0;
}

/*
 * CURL write callback
 */
static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    char** response_ptr = (char**)userp;
    
    char* new_response = realloc(*response_ptr, realsize + 1);
    if (!new_response) return 0;

    *response_ptr = new_response;
    memcpy(*response_ptr, contents, realsize);
    (*response_ptr)[realsize] = 0;
    
    return realsize;
}

/*
 * Send a message to Discord
 */
int eliza_discord_send_message(const char* channel, const char* content) {
    DiscordClientData* data = NULL; /* TODO: Get client data */
    if (!data || !data->is_connected) return -1;

    CURL* curl = curl_easy_init();
    if (!curl) return -1;

    /* Construct the URL */
    char url[256];
    snprintf(url, sizeof(url), "%s/channels/%s/messages", DISCORD_API_BASE, channel);

    /* Create JSON payload */
    struct json_object* json = json_object_new_object();
    json_object_object_add(json, "content", json_object_new_string(content));
    const char* json_str = json_object_to_json_string(json);

    /* Set up headers */
    struct curl_slist* headers = NULL;
    char auth_header[256];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bot %s", data->config->token);
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "Content-Type: application/json");

    /* Set up CURL */
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    /* Perform request */
    CURLcode res = curl_easy_perform(curl);

    /* Cleanup */
    curl_slist_free_all(headers);
    json_object_put(json);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK) ? 0 : -1;
}

/*
 * Process message queue
 */
static int process_message_queue(DiscordClientData* data) {
    if (!data || data->queue_size == 0) return 0;

    /* Process one message from the queue */
    Message* msg = data->message_queue[0];
    
    /* Shift remaining messages */
    memmove(data->message_queue, data->message_queue + 1,
            (data->queue_size - 1) * sizeof(Message*));
    data->queue_size--;

    /* TODO: Process the message (pass to agent, etc.) */

    return 1;
}

/*
 * Add message to queue
 */
static int queue_message(DiscordClientData* data, Message* msg) {
    if (!data || !msg) return -1;

    /* Check if we need to resize the queue */
    if (data->queue_size >= data->queue_capacity) {
        size_t new_capacity = data->queue_capacity * 2;
        Message** new_queue = (Message**)realloc(data->message_queue,
                                               new_capacity * sizeof(Message*));
        if (!new_queue) return -1;

        data->message_queue = new_queue;
        data->queue_capacity = new_capacity;
    }

    /* Add message to queue */
    data->message_queue[data->queue_size++] = msg;
    return 0;
}

/*
 * Initialize the Discord client
 */
int eliza_discord_initialize(void* config) {
    if (!config) return -1;

    DiscordConfig* discord_config = (DiscordConfig*)config;
    if (!discord_config->token) return -1;

    /* Initialize CURL */
    curl_global_init(CURL_GLOBAL_ALL);

    /* Create Gateway connection */
    DiscordClientData* data = (DiscordClientData*)malloc(sizeof(DiscordClientData));
    if (!data) return -1;

    data->config = discord_config;
    data->is_connected = 0;
    data->queue_size = 0;
    data->queue_capacity = INITIAL_CHANNELS_CAPACITY;
    data->message_queue = (Message**)malloc(data->queue_capacity * sizeof(Message*));

    if (!data->message_queue) {
        free(data);
        return -1;
    }

    /* Create and connect Gateway */
    data->gateway = eliza_discord_gateway_create(discord_config->token, data);
    if (!data->gateway) {
        free(data->message_queue);
        free(data);
        return -1;
    }

    if (eliza_discord_gateway_connect(data->gateway) != 0) {
        eliza_discord_gateway_destroy(data->gateway);
        free(data->message_queue);
        free(data);
        return -1;
    }

    data->is_connected = 1;
    return 0;
}

/*
 * Create a Discord client
 */
Client* eliza_discord_client_create(const char* token) {
    if (!token) return NULL;

    Client* client = (Client*)malloc(sizeof(Client));
    if (!client) return NULL;

    /* Create client data */
    DiscordClientData* data = (DiscordClientData*)malloc(sizeof(DiscordClientData));
    if (!data) {
        free(client);
        return NULL;
    }

    /* Create configuration */
    data->config = eliza_discord_config_create();
    if (!data->config) {
        free(data);
        free(client);
        return NULL;
    }

    /* Set token */
    data->config->token = strdup(token);
    if (!data->config->token) {
        eliza_discord_config_destroy(data->config);
        free(data);
        free(client);
        return NULL;
    }

    /* Initialize data */
    data->gateway = NULL;
    data->is_connected = 0;
    data->queue_size = 0;
    data->queue_capacity = INITIAL_CHANNELS_CAPACITY;
    data->message_queue = (Message**)malloc(data->queue_capacity * sizeof(Message*));

    if (!data->message_queue) {
        eliza_discord_config_destroy(data->config);
        free(data);
        free(client);
        return NULL;
    }

    /* Set up client */
    client->platform_name = strdup("Discord");
    client->platform_data = data;
    client->initialize = eliza_discord_initialize;
    client->send_message = eliza_discord_send_message;
    client->receive_message = eliza_discord_receive_message;
    client->cleanup = eliza_discord_cleanup;

    return client;
}

/*
 * Receive a message from Discord
 */
int eliza_discord_receive_message(Message* msg) {
    DiscordClientData* data = NULL; /* TODO: Get client data */
    if (!data || !data->is_connected) return -1;

    /* Poll Gateway for events */
    if (eliza_discord_gateway_poll(data->gateway) < 0) {
        return -1;
    }

    /* Process message queue */
    return process_message_queue(data);
}

/*
 * Clean up Discord resources
 */
void eliza_discord_cleanup(void) {
    DiscordClientData* data = NULL; /* TODO: Get client data */
    if (!data) return;

    if (data->gateway) {
        eliza_discord_gateway_close(data->gateway);
        eliza_discord_gateway_destroy(data->gateway);
    }

    if (data->message_queue) {
        /* Free any remaining messages in the queue */
        for (size_t i = 0; i < data->queue_size; i++) {
            eliza_destroy_message(data->message_queue[i]);
        }
        free(data->message_queue);
    }

    eliza_discord_config_destroy(data->config);
    free(data);

    curl_global_cleanup();
} 