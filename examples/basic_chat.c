#include <c_agent.h>
#include <stdio.h>
#include <string.h>

/*
 * Basic example demonstrating how to use the C Agent framework
 * This example shows:
 * 1. Creating and configuring an agent
 * 2. Basic message handling
 * 3. Using the Discord client
 */

/* Example message handler */
void handle_message(const Message* msg) {
    printf("Received message: %s\n", msg->content);
    printf("From: %s\n", msg->sender_id);
    printf("To: %s\n", msg->receiver_id);
}

int main(int argc, char* argv[]) {
    /* Create a new agent */
    Agent* agent = ai_dancer_create_agent(NULL);
    if (!agent) {
        fprintf(stderr, "Failed to create agent\n");
        return 1;
    }

    /* Create a Discord client (if token provided) */
    Client* client = NULL;
    if (argc > 1) {
        client = ai_dancer_create_client("Discord", argv[1]);
        if (!client) {
            fprintf(stderr, "Failed to create Discord client\n");
            ai_dancer_destroy_agent(agent);
            return 1;
        }

        /* Initialize the client */
        if (client->initialize(client->platform_data) != 0) {
            fprintf(stderr, "Failed to initialize Discord client\n");
            ai_dancer_destroy_client(client);
            ai_dancer_destroy_agent(agent);
            return 1;
        }
    }

    /* Create some test messages */
    const char* test_messages[] = {
        "Hello, how are you?",
        "What's the weather like?",
        "Tell me a joke!",
        "Goodbye!"
    };

    /* Process test messages */
    for (size_t i = 0; i < sizeof(test_messages) / sizeof(test_messages[0]); i++) {
        /* Create message */
        Message* msg = ai_dancer_create_message(
            test_messages[i],
            "user123",
            agent->id
        );

        if (!msg) {
            fprintf(stderr, "Failed to create message\n");
            continue;
        }

        /* Process the message */
        printf("\nSending message: %s\n", msg->content);
        int result = ai_dancer_process_message(agent, msg);
        
        if (result != 0) {
            fprintf(stderr, "Failed to process message\n");
        }

        /* If we have a client, try to send the message */
        if (client) {
            result = client->send_message("test-channel", msg->content);
            if (result != 0) {
                fprintf(stderr, "Failed to send message to Discord\n");
            }
        }

        /* Handle the message */
        handle_message(msg);

        /* Cleanup message */
        ai_dancer_destroy_message(msg);
    }

    /* Cleanup */
    if (client) {
        client->cleanup();
        ai_dancer_destroy_client(client);
    }
    ai_dancer_destroy_agent(agent);

    return 0;
} 