#include "../include/websocket.h"
#include <libwebsockets.h>
#include <string.h>
#include <stdlib.h>

/*
 * Implementation of WebSocket client using libwebsockets
 */

/* Internal WebSocket data */
typedef struct {
    struct lws_context* context;
    struct lws* connection;
    char* pending_tx;
    size_t pending_tx_len;
    int force_exit;
} WebSocketInternal;

/* Protocol name for WebSocket connection */
#define PROTOCOL_NAME "eliza-protocol"

/*
 * Callback for libwebsockets events
 */
static int callback_eliza(struct lws* wsi, enum lws_callback_reasons reason,
                         void* user, void* in, size_t len) {
    WebSocket* ws = (WebSocket*)user;
    if (!ws) return 0;

    WebSocketInternal* internal = (WebSocketInternal*)ws->internal;
    if (!internal) return 0;

    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            ws->state = WS_STATE_CONNECTED;
            if (ws->callbacks.on_connect) {
                ws->callbacks.on_connect(ws->user_data);
            }
            break;

        case LWS_CALLBACK_CLIENT_RECEIVE:
            if (ws->callbacks.on_message) {
                WebSocketMessage msg = {
                    .type = WS_MESSAGE_TEXT,
                    .data = (char*)in,
                    .length = len
                };
                ws->callbacks.on_message(&msg, ws->user_data);
            }
            break;

        case LWS_CALLBACK_CLIENT_WRITEABLE:
            if (internal->pending_tx) {
                int n = lws_write(wsi, (unsigned char*)internal->pending_tx,
                                internal->pending_tx_len, LWS_WRITE_TEXT);
                free(internal->pending_tx);
                internal->pending_tx = NULL;
                internal->pending_tx_len = 0;
                
                if (n < 0) {
                    if (ws->callbacks.on_error) {
                        ws->callbacks.on_error("Write failed", ws->user_data);
                    }
                    return -1;
                }
            }
            break;

        case LWS_CALLBACK_CLIENT_CLOSED:
            ws->state = WS_STATE_DISCONNECTED;
            if (ws->callbacks.on_close) {
                ws->callbacks.on_close(0, "Connection closed", ws->user_data);
            }
            break;

        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            ws->state = WS_STATE_ERROR;
            if (ws->callbacks.on_error) {
                ws->callbacks.on_error(in ? (char*)in : "Connection error", ws->user_data);
            }
            break;

        default:
            break;
    }

    return 0;
}

/* Protocol definition for libwebsockets */
static struct lws_protocols protocols[] = {
    {
        PROTOCOL_NAME,
        callback_eliza,
        0,  /* per_session_data_size */
        0,  /* rx_buffer_size */
        0,  /* id */
        NULL,/* user */
        0   /* tx_packet_size */
    },
    { NULL, NULL, 0, 0, 0, NULL, 0 } /* terminator */
};

/*
 * Create a new WebSocket client
 */
WebSocket* eliza_ws_create(const char* url, const WebSocketCallbacks* callbacks, void* user_data) {
    if (!url || !callbacks) return NULL;

    WebSocket* ws = (WebSocket*)malloc(sizeof(WebSocket));
    if (!ws) return NULL;

    ws->url = strdup(url);
    ws->state = WS_STATE_DISCONNECTED;
    ws->callbacks = *callbacks;
    ws->user_data = user_data;

    /* Create internal data */
    WebSocketInternal* internal = (WebSocketInternal*)malloc(sizeof(WebSocketInternal));
    if (!internal) {
        free(ws->url);
        free(ws);
        return NULL;
    }

    /* Initialize internal data */
    internal->pending_tx = NULL;
    internal->pending_tx_len = 0;
    internal->force_exit = 0;

    /* Create libwebsockets context */
    struct lws_context_creation_info info = {
        .port = CONTEXT_PORT_NO_LISTEN,
        .protocols = protocols,
        .gid = -1,
        .uid = -1,
    };

    internal->context = lws_create_context(&info);
    if (!internal->context) {
        free(internal);
        free(ws->url);
        free(ws);
        return NULL;
    }

    ws->internal = internal;
    return ws;
}

/*
 * Connect to the WebSocket server
 */
int eliza_ws_connect(WebSocket* ws) {
    if (!ws || !ws->internal) return -1;

    WebSocketInternal* internal = (WebSocketInternal*)ws->internal;
    
    /* Parse URL */
    char protocol[256], address[256], path[256];
    int port;
    
    if (lws_parse_uri(ws->url, protocol, address, &port, path)) {
        return -1;
    }

    /* Connection info */
    struct lws_client_connect_info conn_info = {
        .context = internal->context,
        .address = address,
        .port = port,
        .path = path,
        .host = address,
        .origin = address,
        .protocol = PROTOCOL_NAME,
        .ssl_connection = !strcmp(protocol, "wss") ? LCCSCF_USE_SSL : 0,
        .userdata = ws,
    };

    /* Connect */
    internal->connection = lws_client_connect_via_info(&conn_info);
    if (!internal->connection) {
        return -1;
    }

    ws->state = WS_STATE_CONNECTING;
    return 0;
}

/*
 * Send a message
 */
int eliza_ws_send(WebSocket* ws, const char* data, size_t length, WebSocketMessageType type) {
    if (!ws || !ws->internal || !data || ws->state != WS_STATE_CONNECTED) return -1;

    WebSocketInternal* internal = (WebSocketInternal*)ws->internal;

    /* Only support text messages for now */
    if (type != WS_MESSAGE_TEXT) return -1;

    /* Allocate buffer with LWS pre and post padding */
    size_t total_len = LWS_PRE + length;
    internal->pending_tx = (char*)malloc(total_len);
    if (!internal->pending_tx) return -1;

    /* Copy data after pre-padding */
    memcpy(internal->pending_tx + LWS_PRE, data, length);
    internal->pending_tx_len = length;

    /* Request callback for writable */
    lws_callback_on_writable(internal->connection);
    return 0;
}

/*
 * Send a text message
 */
int eliza_ws_send_text(WebSocket* ws, const char* text) {
    return eliza_ws_send(ws, text, strlen(text), WS_MESSAGE_TEXT);
}

/*
 * Close the connection
 */
int eliza_ws_close(WebSocket* ws, int code, const char* reason) {
    if (!ws || !ws->internal) return -1;

    WebSocketInternal* internal = (WebSocketInternal*)ws->internal;
    internal->force_exit = 1;

    if (internal->connection) {
        lws_close_reason(internal->connection, code, (unsigned char*)reason, strlen(reason));
    }

    return 0;
}

/*
 * Process WebSocket events
 */
int eliza_ws_poll(WebSocket* ws) {
    if (!ws || !ws->internal) return -1;

    WebSocketInternal* internal = (WebSocketInternal*)ws->internal;
    return lws_service(internal->context, 0);
}

/*
 * Destroy the WebSocket client
 */
void eliza_ws_destroy(WebSocket* ws) {
    if (!ws) return;

    WebSocketInternal* internal = (WebSocketInternal*)ws->internal;
    if (internal) {
        if (internal->pending_tx) {
            free(internal->pending_tx);
        }
        if (internal->context) {
            lws_context_destroy(internal->context);
        }
        free(internal);
    }

    free(ws->url);
    free(ws);
} 