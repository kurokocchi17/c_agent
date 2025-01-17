#ifndef ELIZA_DISCORD_CLIENT_H
#define ELIZA_DISCORD_CLIENT_H

#include "eliza.h"

/*
 * Discord Client Interface
 * Implements the Client interface for Discord
 */

/* 
 * Discord-specific configuration
 */
typedef struct {
    char* token;            /* Discord bot token */
    char* prefix;           /* Command prefix */
    char** allowed_channels; /* List of allowed channel IDs */
    size_t num_channels;    /* Number of allowed channels */
} DiscordConfig;

/*
 * Discord-specific client data
 */
typedef struct {
    DiscordConfig* config;
    void* discord;         /* Internal discord connection handle */
    int is_connected;      /* Connection status */
} DiscordClientData;

/*
 * Function Declarations
 */

/* Create a Discord client */
Client* eliza_discord_client_create(const char* token);

/* Create Discord configuration */
DiscordConfig* eliza_discord_config_create(void);

/* Destroy Discord configuration */
void eliza_discord_config_destroy(DiscordConfig* config);

/* Add an allowed channel */
int eliza_discord_add_channel(DiscordConfig* config, const char* channel_id);

/* Initialize the Discord client */
int eliza_discord_initialize(void* config);

/* Send a message to a Discord channel */
int eliza_discord_send_message(const char* channel, const char* content);

/* Receive a message from Discord */
int eliza_discord_receive_message(Message* msg);

/* Clean up Discord resources */
void eliza_discord_cleanup(void);

#endif /* ELIZA_DISCORD_CLIENT_H */ 