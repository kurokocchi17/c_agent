#include "../include/discord_gateway.h"
#include <json-c/json.h>
#include <time.h>
#include <stdio.h>

/*
 * Implementation of Discord Gateway
 */

#define DISCORD_GATEWAY_VERSION 10
#define DISCORD_GATEWAY_URL "wss://gateway.discord.gg/?v=" #DISCORD_GATEWAY_VERSION "&encoding=json"

/* Forward declarations */
static void on_gateway_message(const WebSocketMessage* msg, void* user_data);
static void on_gateway_connect(void* user_data);
static void on_gateway_error(const char* error, void* user_data);
static void on_gateway_close(int code, const char* reason, void* user_data);

/*
 * Create a new Gateway connection
 */
DiscordGateway* eliza_discord_gateway_create(const char* token, void* user_data) {
    if (!token) return NULL;

    DiscordGateway* gateway = (DiscordGateway*)malloc(sizeof(DiscordGateway));
    if (!gateway) return NULL;

    /* Initialize gateway data */
    gateway->session_id = NULL;
    gateway->sequence = 0;
    gateway->heartbeat_interval = 0;
    gateway->last_heartbeat = 0;
    gateway->heartbeat_ack = 1;
    gateway->user_data = user_data;

    /* Set up WebSocket callbacks */
    WebSocketCallbacks callbacks = {
        .on_connect = on_gateway_connect,
        .on_message = on_gateway_message,
        .on_error = on_gateway_error,
        .on_close = on_gateway_close
    };

    /* Create WebSocket */
    gateway->ws = eliza_ws_create(DISCORD_GATEWAY_URL, &callbacks, gateway);
    if (!gateway->ws) {
        free(gateway);
        return NULL;
    }

    return gateway;
}

/*
 * Send identify payload
 */
static int send_identify(DiscordGateway* gateway, const char* token) {
    struct json_object* identify = json_object_new_object();
    struct json_object* data = json_object_new_object();
    struct json_object* properties = json_object_new_object();

    /* Set up properties */
    json_object_object_add(properties, "os", json_object_new_string("linux"));
    json_object_object_add(properties, "browser", json_object_new_string("eliza"));
    json_object_object_add(properties, "device", json_object_new_string("eliza"));

    /* Set up data */
    json_object_object_add(data, "token", json_object_new_string(token));
    json_object_object_add(data, "properties", properties);
    json_object_object_add(data, "intents", json_object_new_int(513)); /* Guilds + Guild Messages */

    /* Set up identify payload */
    json_object_object_add(identify, "op", json_object_new_int(DISCORD_OP_IDENTIFY));
    json_object_object_add(identify, "d", data);

    /* Send payload */
    const char* payload = json_object_to_json_string(identify);
    int result = eliza_ws_send_text(gateway->ws, payload);

    /* Cleanup */
    json_object_put(identify);
    return result;
}

/*
 * Send heartbeat payload
 */
static int send_heartbeat(DiscordGateway* gateway) {
    struct json_object* heartbeat = json_object_new_object();
    struct json_object* sequence = gateway->sequence > 0 ?
        json_object_new_int(gateway->sequence) : NULL;

    /* Set up heartbeat payload */
    json_object_object_add(heartbeat, "op", json_object_new_int(DISCORD_OP_HEARTBEAT));
    json_object_object_add(heartbeat, "d", sequence);

    /* Send payload */
    const char* payload = json_object_to_json_string(heartbeat);
    int result = eliza_ws_send_text(gateway->ws, payload);

    /* Update heartbeat state */
    gateway->last_heartbeat = time(NULL);
    gateway->heartbeat_ack = 0;

    /* Cleanup */
    json_object_put(heartbeat);
    return result;
}

/*
 * Handle Gateway messages
 */
static void on_gateway_message(const WebSocketMessage* msg, void* user_data) {
    DiscordGateway* gateway = (DiscordGateway*)user_data;
    if (!gateway || !msg->data) return;

    /* Parse JSON */
    struct json_object* json = json_tokener_parse(msg->data);
    if (!json) return;

    /* Get opcode and data */
    struct json_object* op_obj, *data_obj, *seq_obj;
    json_object_object_get_ex(json, "op", &op_obj);
    json_object_object_get_ex(json, "d", &data_obj);
    json_object_object_get_ex(json, "s", &seq_obj);

    if (!op_obj) {
        json_object_put(json);
        return;
    }

    /* Update sequence if present */
    if (seq_obj) {
        gateway->sequence = json_object_get_int(seq_obj);
    }

    /* Handle different opcodes */
    int op = json_object_get_int(op_obj);
    switch (op) {
        case DISCORD_OP_HELLO:
            if (data_obj) {
                struct json_object* heartbeat_obj;
                json_object_object_get_ex(data_obj, "heartbeat_interval", &heartbeat_obj);
                if (heartbeat_obj) {
                    gateway->heartbeat_interval = json_object_get_int(heartbeat_obj);
                    send_heartbeat(gateway);
                }
            }
            break;

        case DISCORD_OP_HEARTBEAT_ACK:
            gateway->heartbeat_ack = 1;
            break;

        case DISCORD_OP_DISPATCH:
            /* Handle dispatch events */
            struct json_object* type_obj;
            json_object_object_get_ex(json, "t", &type_obj);
            if (type_obj) {
                const char* type = json_object_get_string(type_obj);
                if (strcmp(type, "READY") == 0) {
                    /* Store session ID */
                    if (data_obj) {
                        struct json_object* session_id_obj;
                        json_object_object_get_ex(data_obj, "session_id", &session_id_obj);
                        if (session_id_obj) {
                            free(gateway->session_id);
                            gateway->session_id = strdup(json_object_get_string(session_id_obj));
                        }
                    }
                }
                /* TODO: Handle other event types */
            }
            break;
    }

    json_object_put(json);
}

/*
 * Handle Gateway connection
 */
static void on_gateway_connect(void* user_data) {
    DiscordGateway* gateway = (DiscordGateway*)user_data;
    if (!gateway) return;

    /* Send identify payload */
    /* TODO: Get token from user_data */
    send_identify(gateway, "YOUR_TOKEN_HERE");
}

/*
 * Handle Gateway errors
 */
static void on_gateway_error(const char* error, void* user_data) {
    /* TODO: Implement error handling */
    fprintf(stderr, "Gateway error: %s\n", error);
}

/*
 * Handle Gateway close
 */
static void on_gateway_close(int code, const char* reason, void* user_data) {
    /* TODO: Implement reconnection logic */
    fprintf(stderr, "Gateway closed (%d): %s\n", code, reason);
}

/*
 * Connect to the Gateway
 */
int eliza_discord_gateway_connect(DiscordGateway* gateway) {
    if (!gateway || !gateway->ws) return -1;
    return eliza_ws_connect(gateway->ws);
}

/*
 * Process Gateway events
 */
int eliza_discord_gateway_poll(DiscordGateway* gateway) {
    if (!gateway || !gateway->ws) return -1;

    /* Check if we need to send a heartbeat */
    if (gateway->heartbeat_interval > 0) {
        time_t now = time(NULL);
        if (now - gateway->last_heartbeat >= gateway->heartbeat_interval / 1000) {
            if (!gateway->heartbeat_ack) {
                /* Connection probably died, reconnect */
                eliza_ws_close(gateway->ws, 1000, "Heartbeat timeout");
                return -1;
            }
            send_heartbeat(gateway);
        }
    }

    return eliza_ws_poll(gateway->ws);
}

/*
 * Send a Gateway payload
 */
int eliza_discord_gateway_send(DiscordGateway* gateway, DiscordGatewayOpcode op,
                             struct json_object* data) {
    if (!gateway || !gateway->ws) return -1;

    struct json_object* payload = json_object_new_object();
    json_object_object_add(payload, "op", json_object_new_int(op));
    json_object_object_add(payload, "d", data);

    const char* json_str = json_object_to_json_string(payload);
    int result = eliza_ws_send_text(gateway->ws, json_str);

    json_object_put(payload);
    return result;
}

/*
 * Close the Gateway connection
 */
void eliza_discord_gateway_close(DiscordGateway* gateway) {
    if (!gateway || !gateway->ws) return;
    eliza_ws_close(gateway->ws, 1000, "Normal closure");
}

/*
 * Destroy the Gateway connection
 */
void eliza_discord_gateway_destroy(DiscordGateway* gateway) {
    if (!gateway) return;

    if (gateway->ws) {
        eliza_ws_destroy(gateway->ws);
    }
    free(gateway->session_id);
    free(gateway);
} 