#ifndef ELIZA_DISCORD_GATEWAY_H
#define ELIZA_DISCORD_GATEWAY_H

#include "discord_client.h"
#include "websocket.h"

/*
 * Discord Gateway Interface
 * Handles real-time communication with Discord
 */

/* Gateway opcodes */
typedef enum {
    DISCORD_OP_DISPATCH = 0,
    DISCORD_OP_HEARTBEAT = 1,
    DISCORD_OP_IDENTIFY = 2,
    DISCORD_OP_PRESENCE_UPDATE = 3,
    DISCORD_OP_VOICE_STATE = 4,
    DISCORD_OP_RESUME = 6,
    DISCORD_OP_RECONNECT = 7,
    DISCORD_OP_REQUEST_GUILD_MEMBERS = 8,
    DISCORD_OP_INVALID_SESSION = 9,
    DISCORD_OP_HELLO = 10,
    DISCORD_OP_HEARTBEAT_ACK = 11
} DiscordGatewayOpcode;

/* Gateway event types */
typedef enum {
    DISCORD_EVENT_READY,
    DISCORD_EVENT_MESSAGE_CREATE,
    DISCORD_EVENT_MESSAGE_UPDATE,
    DISCORD_EVENT_MESSAGE_DELETE,
    DISCORD_EVENT_GUILD_CREATE,
    DISCORD_EVENT_UNKNOWN
} DiscordEventType;

/* Gateway event data */
typedef struct {
    DiscordEventType type;
    char* raw_data;
    struct json_object* json;
} DiscordEvent;

/* Gateway connection data */
typedef struct {
    WebSocket* ws;
    char* session_id;
    int sequence;
    int heartbeat_interval;
    int last_heartbeat;
    int heartbeat_ack;
    void* user_data;
} DiscordGateway;

/*
 * Function Declarations
 */

/* Create a new Gateway connection */
DiscordGateway* eliza_discord_gateway_create(const char* token, void* user_data);

/* Connect to the Gateway */
int eliza_discord_gateway_connect(DiscordGateway* gateway);

/* Process Gateway events */
int eliza_discord_gateway_poll(DiscordGateway* gateway);

/* Send a Gateway payload */
int eliza_discord_gateway_send(DiscordGateway* gateway, DiscordGatewayOpcode op,
                             struct json_object* data);

/* Close the Gateway connection */
void eliza_discord_gateway_close(DiscordGateway* gateway);

/* Destroy the Gateway connection */
void eliza_discord_gateway_destroy(DiscordGateway* gateway);

#endif /* ELIZA_DISCORD_GATEWAY_H */ 