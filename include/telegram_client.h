#ifndef AI_DANCER_TELEGRAM_CLIENT_H
#define AI_DANCER_TELEGRAM_CLIENT_H

#include "ai_dancer.h"

/*
 * Telegram Client Interface
 * Implements the Client interface for Telegram Bot API
 */

/* 
 * Telegram-specific configuration
 */
typedef struct {
    char* token;            /* Telegram bot token */
    char* webhook_url;      /* Optional webhook URL for updates */
    int64_t* allowed_chats; /* List of allowed chat IDs */
    size_t num_chats;       /* Number of allowed chats */
    int update_timeout;     /* Long polling timeout in seconds */
    int last_update_id;     /* ID of last processed update */
} TelegramConfig;

/*
 * Telegram-specific client data
 */
typedef struct {
    TelegramConfig* config;
    void* curl;            /* CURL handle for API requests */
    int is_connected;      /* Connection status */
    char* base_url;        /* Base URL for API requests */
} TelegramClientData;

/*
 * Function Declarations
 */

/* Create a Telegram client */
Client* ai_dancer_telegram_client_create(const char* token);

/* Create Telegram configuration */
TelegramConfig* ai_dancer_telegram_config_create(void);

/* Destroy Telegram configuration */
void ai_dancer_telegram_config_destroy(TelegramConfig* config);

/* Add an allowed chat */
int ai_dancer_telegram_add_chat(TelegramConfig* config, int64_t chat_id);

/* Set webhook URL */
int ai_dancer_telegram_set_webhook(TelegramConfig* config, const char* url);

/* Initialize the Telegram client */
int ai_dancer_telegram_initialize(void* config);

/* Send a message to a Telegram chat */
int ai_dancer_telegram_send_message(const char* chat_id, const char* content);

/* Receive updates from Telegram */
int ai_dancer_telegram_receive_updates(Message* msg);

/* Clean up Telegram resources */
void ai_dancer_telegram_cleanup(void);

/* Helper functions */
int ai_dancer_telegram_send_photo(const char* chat_id, const char* photo_path, const char* caption);
int ai_dancer_telegram_send_document(const char* chat_id, const char* file_path, const char* caption);
int ai_dancer_telegram_send_location(const char* chat_id, float latitude, float longitude);
int ai_dancer_telegram_send_contact(const char* chat_id, const char* phone_number, const char* first_name);

#endif /* AI_DANCER_TELEGRAM_CLIENT_H */ 