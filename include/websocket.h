#ifndef ELIZA_WEBSOCKET_H
#define ELIZA_WEBSOCKET_H

#include <stddef.h>

/*
 * WebSocket Interface
 * Provides a generic WebSocket client implementation
 */

/* WebSocket connection states */
typedef enum {
    WS_STATE_DISCONNECTED,
    WS_STATE_CONNECTING,
    WS_STATE_CONNECTED,
    WS_STATE_CLOSING,
    WS_STATE_ERROR
} WebSocketState;

/* WebSocket message types */
typedef enum {
    WS_MESSAGE_TEXT,
    WS_MESSAGE_BINARY,
    WS_MESSAGE_PING,
    WS_MESSAGE_PONG,
    WS_MESSAGE_CLOSE
} WebSocketMessageType;

/* WebSocket message structure */
typedef struct {
    WebSocketMessageType type;
    char* data;
    size_t length;
} WebSocketMessage;

/* WebSocket event callbacks */
typedef struct {
    void (*on_connect)(void* user_data);
    void (*on_message)(const WebSocketMessage* msg, void* user_data);
    void (*on_error)(const char* error, void* user_data);
    void (*on_close)(int code, const char* reason, void* user_data);
} WebSocketCallbacks;

/* WebSocket client structure */
typedef struct WebSocket {
    char* url;
    WebSocketState state;
    WebSocketCallbacks callbacks;
    void* user_data;
    void* internal;  /* Internal implementation data */
} WebSocket;

/*
 * Function Declarations
 */

/* Create a new WebSocket client */
WebSocket* eliza_ws_create(const char* url, const WebSocketCallbacks* callbacks, void* user_data);

/* Connect to the WebSocket server */
int eliza_ws_connect(WebSocket* ws);

/* Send a message */
int eliza_ws_send(WebSocket* ws, const char* data, size_t length, WebSocketMessageType type);

/* Send a text message (convenience function) */
int eliza_ws_send_text(WebSocket* ws, const char* text);

/* Close the connection */
int eliza_ws_close(WebSocket* ws, int code, const char* reason);

/* Process WebSocket events (non-blocking) */
int eliza_ws_poll(WebSocket* ws);

/* Destroy the WebSocket client */
void eliza_ws_destroy(WebSocket* ws);

#endif /* ELIZA_WEBSOCKET_H */ 