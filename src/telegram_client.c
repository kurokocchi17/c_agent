#include "../include/telegram_client.h"
#include <curl/curl.h>
#include <json-c/json.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*
 * Implementation of the Telegram Client
 */

#define TELEGRAM_API_BASE "https://api.telegram.org/bot"
#define INITIAL_CHATS_CAPACITY 16
#define DEFAULT_UPDATE_TIMEOUT 30

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
 * Create Telegram configuration
 */
TelegramConfig* ai_dancer_telegram_config_create(void) {
    TelegramConfig* config = (TelegramConfig*)malloc(sizeof(TelegramConfig));
    if (!config) return NULL;

    config->token = NULL;
    config->webhook_url = NULL;
    config->allowed_chats = (int64_t*)malloc(INITIAL_CHATS_CAPACITY * sizeof(int64_t));
    config->num_chats = 0;
    config->update_timeout = DEFAULT_UPDATE_TIMEOUT;
    config->last_update_id = 0;

    if (!config->allowed_chats) {
        free(config);
        return NULL;
    }

    return config;
}

/*
 * Destroy Telegram configuration
 */
void ai_dancer_telegram_config_destroy(TelegramConfig* config) {
    if (!config) return;

    free(config->token);
    free(config->webhook_url);
    free(config->allowed_chats);
    free(config);
}

/*
 * Add an allowed chat
 */
int ai_dancer_telegram_add_chat(TelegramConfig* config, int64_t chat_id) {
    if (!config) return -1;

    /* Check if we need to resize the chats array */
    if (config->num_chats >= INITIAL_CHATS_CAPACITY) {
        size_t new_capacity = INITIAL_CHATS_CAPACITY * 2;
        int64_t* new_chats = (int64_t*)realloc(config->allowed_chats,
                                              new_capacity * sizeof(int64_t));
        if (!new_chats) return -1;
        config->allowed_chats = new_chats;
    }

    config->allowed_chats[config->num_chats++] = chat_id;
    return 0;
}

/*
 * Make a Telegram API request
 */
static char* make_telegram_request(TelegramClientData* data, const char* method,
                                 const char* params) {
    if (!data || !method) return NULL;

    CURL* curl = data->curl;
    if (!curl) return NULL;

    /* Construct URL */
    char url[1024];
    snprintf(url, sizeof(url), "%s%s/%s", TELEGRAM_API_BASE, data->config->token, method);

    /* Set up CURL */
    char* response = NULL;
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    if (params) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params);
    }

    /* Perform request */
    CURLcode res = curl_easy_perform(curl);

    /* Cleanup */
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        free(response);
        return NULL;
    }

    return response;
}

/*
 * Set webhook URL
 */
int ai_dancer_telegram_set_webhook(TelegramConfig* config, const char* url) {
    if (!config) return -1;

    char* old_url = config->webhook_url;
    config->webhook_url = url ? strdup(url) : NULL;
    free(old_url);

    return 0;
}

/*
 * Initialize the Telegram client
 */
int ai_dancer_telegram_initialize(void* config) {
    if (!config) return -1;

    TelegramConfig* telegram_config = (TelegramConfig*)config;
    if (!telegram_config->token) return -1;

    /* Create client data */
    TelegramClientData* data = (TelegramClientData*)malloc(sizeof(TelegramClientData));
    if (!data) return -1;

    /* Initialize CURL */
    data->curl = curl_easy_init();
    if (!data->curl) {
        free(data);
        return -1;
    }

    /* Set up base URL */
    data->config = telegram_config;
    data->is_connected = 1;

    /* Set webhook if provided */
    if (telegram_config->webhook_url) {
        struct json_object* webhook = json_object_new_object();
        json_object_object_add(webhook, "url", 
                             json_object_new_string(telegram_config->webhook_url));
        
        char* response = make_telegram_request(data, "setWebhook",
                                            json_object_to_json_string(webhook));
        json_object_put(webhook);
        
        if (!response) {
            curl_easy_cleanup(data->curl);
            free(data);
            return -1;
        }
        free(response);
    }

    return 0;
}

/*
 * Send a message to Telegram
 */
int ai_dancer_telegram_send_message(const char* chat_id, const char* content) {
    TelegramClientData* data = NULL; /* TODO: Get client data */
    if (!data || !data->is_connected) return -1;

    /* Create JSON payload */
    struct json_object* json = json_object_new_object();
    json_object_object_add(json, "chat_id", json_object_new_string(chat_id));
    json_object_object_add(json, "text", json_object_new_string(content));
    json_object_object_add(json, "parse_mode", json_object_new_string("HTML"));

    /* Send request */
    char* response = make_telegram_request(data, "sendMessage",
                                         json_object_to_json_string(json));
    json_object_put(json);

    if (!response) return -1;
    free(response);
    return 0;
}

/*
 * Process updates from Telegram
 */
static int process_update(TelegramClientData* data, struct json_object* update) {
    if (!data || !update) return -1;

    /* Get update ID */
    struct json_object* update_id_obj;
    json_object_object_get_ex(update, "update_id", &update_id_obj);
    if (update_id_obj) {
        data->config->last_update_id = json_object_get_int(update_id_obj);
    }

    /* Get message */
    struct json_object* message_obj;
    json_object_object_get_ex(update, "message", &message_obj);
    if (!message_obj) return 0;

    /* Get message text */
    struct json_object* text_obj;
    json_object_object_get_ex(message_obj, "text", &text_obj);
    if (!text_obj) return 0;

    /* Get chat ID */
    struct json_object* chat_obj, *chat_id_obj;
    json_object_object_get_ex(message_obj, "chat", &chat_obj);
    if (!chat_obj) return 0;

    json_object_object_get_ex(chat_obj, "id", &chat_id_obj);
    if (!chat_id_obj) return 0;

    /* TODO: Create and queue message */
    return 0;
}

/*
 * Receive updates from Telegram
 */
int ai_dancer_telegram_receive_updates(Message* msg) {
    TelegramClientData* data = NULL; /* TODO: Get client data */
    if (!data || !data->is_connected) return -1;

    /* Create JSON payload */
    struct json_object* json = json_object_new_object();
    json_object_object_add(json, "offset", 
                          json_object_new_int(data->config->last_update_id + 1));
    json_object_object_add(json, "timeout", 
                          json_object_new_int(data->config->update_timeout));

    /* Get updates */
    char* response = make_telegram_request(data, "getUpdates",
                                         json_object_to_json_string(json));
    json_object_put(json);

    if (!response) return -1;

    /* Parse response */
    struct json_object* resp_json = json_tokener_parse(response);
    free(response);

    if (!resp_json) return -1;

    /* Process updates */
    struct json_object* result_obj;
    json_object_object_get_ex(resp_json, "result", &result_obj);
    if (result_obj && json_object_get_type(result_obj) == json_type_array) {
        size_t num_updates = json_object_array_length(result_obj);
        for (size_t i = 0; i < num_updates; i++) {
            struct json_object* update = json_object_array_get_idx(result_obj, i);
            process_update(data, update);
        }
    }

    json_object_put(resp_json);
    return 0;
}

/*
 * Create a Telegram client
 */
Client* ai_dancer_telegram_client_create(const char* token) {
    if (!token) return NULL;

    Client* client = (Client*)malloc(sizeof(Client));
    if (!client) return NULL;

    /* Create client data */
    TelegramClientData* data = (TelegramClientData*)malloc(sizeof(TelegramClientData));
    if (!data) {
        free(client);
        return NULL;
    }

    /* Create configuration */
    data->config = ai_dancer_telegram_config_create();
    if (!data->config) {
        free(data);
        free(client);
        return NULL;
    }

    /* Set token */
    data->config->token = strdup(token);
    if (!data->config->token) {
        ai_dancer_telegram_config_destroy(data->config);
        free(data);
        free(client);
        return NULL;
    }

    /* Initialize data */
    data->curl = NULL;
    data->is_connected = 0;

    /* Set up client */
    client->platform_name = strdup("Telegram");
    client->platform_data = data;
    client->initialize = ai_dancer_telegram_initialize;
    client->send_message = ai_dancer_telegram_send_message;
    client->receive_message = ai_dancer_telegram_receive_updates;
    client->cleanup = ai_dancer_telegram_cleanup;

    return client;
}

/*
 * Clean up Telegram resources
 */
void ai_dancer_telegram_cleanup(void) {
    TelegramClientData* data = NULL; /* TODO: Get client data */
    if (!data) return;

    if (data->curl) {
        curl_easy_cleanup(data->curl);
    }

    ai_dancer_telegram_config_destroy(data->config);
    free(data);
}

/*
 * Send a photo to Telegram
 */
int ai_dancer_telegram_send_photo(const char* chat_id, const char* photo_path, const char* caption) {
    TelegramClientData* data = NULL; /* TODO: Get client data */
    if (!data || !data->is_connected || !photo_path) return -1;

    /* Set up form data for multipart/form-data request */
    struct curl_httppost* form = NULL;
    struct curl_httppost* last = NULL;

    /* Add chat_id field */
    curl_formadd(&form, &last,
                CURLFORM_COPYNAME, "chat_id",
                CURLFORM_COPYCONTENTS, chat_id,
                CURLFORM_END);

    /* Add photo file */
    curl_formadd(&form, &last,
                CURLFORM_COPYNAME, "photo",
                CURLFORM_FILE, photo_path,
                CURLFORM_END);

    /* Add caption if provided */
    if (caption) {
        curl_formadd(&form, &last,
                    CURLFORM_COPYNAME, "caption",
                    CURLFORM_COPYCONTENTS, caption,
                    CURLFORM_END);
    }

    /* Set up CURL for multipart request */
    CURL* curl = data->curl;
    char url[1024];
    snprintf(url, sizeof(url), "%s%s/sendPhoto", TELEGRAM_API_BASE, data->config->token);
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, form);

    /* Perform request */
    char* response = NULL;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);

    /* Cleanup */
    curl_formfree(form);
    if (response) free(response);

    return (res == CURLE_OK) ? 0 : -1;
}

/*
 * Send a document to Telegram
 */
int ai_dancer_telegram_send_document(const char* chat_id, const char* file_path, const char* caption) {
    TelegramClientData* data = NULL; /* TODO: Get client data */
    if (!data || !data->is_connected || !file_path) return -1;

    /* Set up form data */
    struct curl_httppost* form = NULL;
    struct curl_httppost* last = NULL;

    /* Add chat_id field */
    curl_formadd(&form, &last,
                CURLFORM_COPYNAME, "chat_id",
                CURLFORM_COPYCONTENTS, chat_id,
                CURLFORM_END);

    /* Add document file */
    curl_formadd(&form, &last,
                CURLFORM_COPYNAME, "document",
                CURLFORM_FILE, file_path,
                CURLFORM_END);

    /* Add caption if provided */
    if (caption) {
        curl_formadd(&form, &last,
                    CURLFORM_COPYNAME, "caption",
                    CURLFORM_COPYCONTENTS, caption,
                    CURLFORM_END);
    }

    /* Set up CURL for multipart request */
    CURL* curl = data->curl;
    char url[1024];
    snprintf(url, sizeof(url), "%s%s/sendDocument", TELEGRAM_API_BASE, data->config->token);
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, form);

    /* Perform request */
    char* response = NULL;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);

    /* Cleanup */
    curl_formfree(form);
    if (response) free(response);

    return (res == CURLE_OK) ? 0 : -1;
}

/*
 * Send a location to Telegram
 */
int ai_dancer_telegram_send_location(const char* chat_id, float latitude, float longitude) {
    TelegramClientData* data = NULL; /* TODO: Get client data */
    if (!data || !data->is_connected) return -1;

    /* Create JSON payload */
    struct json_object* json = json_object_new_object();
    json_object_object_add(json, "chat_id", json_object_new_string(chat_id));
    json_object_object_add(json, "latitude", json_object_new_double(latitude));
    json_object_object_add(json, "longitude", json_object_new_double(longitude));

    /* Send request */
    char* response = make_telegram_request(data, "sendLocation",
                                         json_object_to_json_string(json));
    json_object_put(json);

    if (!response) return -1;
    free(response);
    return 0;
}

/*
 * Send a contact to Telegram
 */
int ai_dancer_telegram_send_contact(const char* chat_id, const char* phone_number,
                                  const char* first_name) {
    TelegramClientData* data = NULL; /* TODO: Get client data */
    if (!data || !data->is_connected) return -1;

    /* Create JSON payload */
    struct json_object* json = json_object_new_object();
    json_object_object_add(json, "chat_id", json_object_new_string(chat_id));
    json_object_object_add(json, "phone_number", json_object_new_string(phone_number));
    json_object_object_add(json, "first_name", json_object_new_string(first_name));

    /* Send request */
    char* response = make_telegram_request(data, "sendContact",
                                         json_object_to_json_string(json));
    json_object_put(json);

    if (!response) return -1;
    free(response);
    return 0;
} 