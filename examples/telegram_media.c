#include <ai_dancer.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

/*
 * Example demonstrating Telegram media handling capabilities
 * This example shows:
 * 1. Sending photos with captions
 * 2. Sending documents
 * 3. Sharing location data
 * 4. Sharing contact information
 */

/* Global flag for controlling the main loop */
static volatile int running = 1;

/* Signal handler for graceful shutdown */
void handle_signal(int sig) {
    printf("\nReceived signal %d, shutting down...\n", sig);
    running = 0;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <telegram_token> <chat_id>\n", argv[0]);
        return 1;
    }

    const char* token = argv[1];
    const char* chat_id = argv[2];

    /* Set up signal handling */
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    /* Create an agent */
    Agent* agent = ai_dancer_create_agent(NULL);
    if (!agent) {
        fprintf(stderr, "Failed to create agent\n");
        return 1;
    }

    /* Create Telegram client */
    Client* client = ai_dancer_create_client("Telegram", token);
    if (!client) {
        fprintf(stderr, "Failed to create Telegram client\n");
        ai_dancer_destroy_agent(agent);
        return 1;
    }

    /* Initialize client */
    if (client->initialize(client->platform_data) != 0) {
        fprintf(stderr, "Failed to initialize Telegram client\n");
        ai_dancer_destroy_client(client);
        ai_dancer_destroy_agent(agent);
        return 1;
    }

    printf("Telegram client initialized. Demonstrating media capabilities...\n");

    /* Send a photo */
    printf("\nSending photo...\n");
    if (ai_dancer_telegram_send_photo(chat_id, "examples/assets/demo.jpg",
                                    "Check out this cool photo!") == 0) {
        printf("Photo sent successfully\n");
    } else {
        fprintf(stderr, "Failed to send photo\n");
    }

    /* Send a document */
    printf("\nSending document...\n");
    if (ai_dancer_telegram_send_document(chat_id, "examples/assets/report.pdf",
                                       "Here's the report you requested") == 0) {
        printf("Document sent successfully\n");
    } else {
        fprintf(stderr, "Failed to send document\n");
    }

    /* Share location */
    printf("\nSharing location...\n");
    if (ai_dancer_telegram_send_location(chat_id, 37.7749, -122.4194) == 0) {
        printf("Location shared successfully\n");
    } else {
        fprintf(stderr, "Failed to share location\n");
    }

    /* Share contact */
    printf("\nSharing contact...\n");
    if (ai_dancer_telegram_send_contact(chat_id, "+1234567890", "John Doe") == 0) {
        printf("Contact shared successfully\n");
    } else {
        fprintf(stderr, "Failed to share contact\n");
    }

    /* Main event loop for receiving messages */
    printf("\nListening for messages. Press Ctrl+C to exit.\n");
    while (running) {
        Message msg = {0};
        if (client->receive_message(&msg) == 0) {
            /* Process the message with the agent */
            if (ai_dancer_process_message(agent, &msg) == 0) {
                printf("Received message: %s\n", msg.content);
            }
        }

        /* Small delay to prevent busy waiting */
        usleep(100000); /* 100ms */
    }

    printf("Shutting down...\n");

    /* Cleanup */
    client->cleanup();
    ai_dancer_destroy_client(client);
    ai_dancer_destroy_agent(agent);
    printf("Cleanup complete\n");

    return 0;
} 