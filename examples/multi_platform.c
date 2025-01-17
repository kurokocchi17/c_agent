#include <ai_dancer.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

/*
 * Example demonstrating how to use multiple platform clients
 * This example shows:
 * 1. Setting up Discord and Telegram clients
 * 2. Message broadcasting across platforms
 * 3. Handling platform-specific features
 * 4. Graceful shutdown
 */

/* Global flag for controlling the main loop */
static volatile int running = 1;

/* Signal handler for graceful shutdown */
void handle_signal(int sig) {
    printf("\nReceived signal %d, shutting down...\n", sig);
    running = 0;
}

/* Example message handler */
void handle_message(const Message* msg, Client** clients, size_t num_clients) {
    printf("\nReceived message:\n");
    printf("Content: %s\n", msg->content);
    printf("From: %s\n", msg->sender_id);
    printf("To: %s\n", msg->receiver_id);

    /* Broadcast message to all platforms */
    for (size_t i = 0; i < num_clients; i++) {
        if (clients[i]->send_message("broadcast", msg->content) != 0) {
            fprintf(stderr, "Failed to broadcast to %s\n", clients[i]->platform_name);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <discord_token> <telegram_token>\n", argv[0]);
        return 1;
    }

    /* Set up signal handling */
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    /* Create an agent */
    Agent* agent = ai_dancer_create_agent(NULL);
    if (!agent) {
        fprintf(stderr, "Failed to create agent\n");
        return 1;
    }

    /* Create platform clients */
    Client* clients[2] = {NULL, NULL};
    size_t num_clients = 0;

    /* Create Discord client */
    clients[num_clients] = ai_dancer_create_client("Discord", argv[1]);
    if (clients[num_clients]) {
        if (clients[num_clients]->initialize(clients[num_clients]->platform_data) == 0) {
            printf("Discord client initialized\n");
            num_clients++;
        } else {
            fprintf(stderr, "Failed to initialize Discord client\n");
            ai_dancer_destroy_client(clients[num_clients]);
        }
    }

    /* Create Telegram client */
    clients[num_clients] = ai_dancer_create_client("Telegram", argv[2]);
    if (clients[num_clients]) {
        if (clients[num_clients]->initialize(clients[num_clients]->platform_data) == 0) {
            printf("Telegram client initialized\n");
            num_clients++;
        } else {
            fprintf(stderr, "Failed to initialize Telegram client\n");
            ai_dancer_destroy_client(clients[num_clients]);
        }
    }

    if (num_clients == 0) {
        fprintf(stderr, "No clients initialized\n");
        ai_dancer_destroy_agent(agent);
        return 1;
    }

    printf("Running with %zu clients. Press Ctrl+C to exit.\n", num_clients);

    /* Main event loop */
    while (running) {
        /* Process messages from all platforms */
        for (size_t i = 0; i < num_clients; i++) {
            Message msg = {0};
            if (clients[i]->receive_message(&msg) == 0) {
                /* Process the message with the agent */
                if (ai_dancer_process_message(agent, &msg) == 0) {
                    /* Broadcast to all platforms */
                    handle_message(&msg, clients, num_clients);
                }
            }
        }

        /* Small delay to prevent busy waiting */
        usleep(100000); /* 100ms */
    }

    printf("Shutting down...\n");

    /* Cleanup */
    for (size_t i = 0; i < num_clients; i++) {
        if (clients[i]) {
            clients[i]->cleanup();
            ai_dancer_destroy_client(clients[i]);
        }
    }

    ai_dancer_destroy_agent(agent);
    printf("Cleanup complete\n");

    return 0;
} 