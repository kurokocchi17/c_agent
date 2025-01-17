#ifndef AI_DANCER_H
#define AI_DANCER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Core Types and Structures
 * These define the fundamental building blocks of the AI Dancer framework
 * Inspired by the classic ELIZA chatbot, but extended with modern capabilities
 */

/* 
 * Message structure to handle communication between agents and clients
 * Represents a single message in the conversation
 */
typedef struct {
    char* content;           /* The actual message content */
    char* sender_id;         /* Identifier of who sent the message */
    char* receiver_id;       /* Identifier of intended recipient */
    long timestamp;          /* Unix timestamp of when message was created */
} Message;

/* 
 * Agent structure representing an AI agent instance
 * Contains all necessary information and state for an agent
 */
typedef struct {
    char* id;               /* Unique identifier for the agent */
    char* name;             /* Display name of the agent */
    char* description;      /* Description of the agent's purpose/personality */
    void* memory;           /* Pointer to agent's memory system */
    void* model;            /* Pointer to the language model interface */
} Agent;

/* 
 * Client interface structure for different platforms
 * (Discord, Twitter, Telegram, etc.)
 */
typedef struct {
    char* platform_name;    /* Name of the platform (e.g., "Discord") */
    void* platform_data;    /* Platform-specific data */
    
    /* Function pointers for client operations */
    int (*initialize)(void* config);
    int (*send_message)(const char* channel, const char* content);
    int (*receive_message)(Message* msg);
    void (*cleanup)(void);
} Client;

/*
 * Core Function Declarations
 */

/* Agent Management Functions */
Agent* ai_dancer_create_agent(const char* config_path);
void ai_dancer_destroy_agent(Agent* agent);
int ai_dancer_process_message(Agent* agent, const Message* msg);

/* Message Management Functions */
Message* ai_dancer_create_message(const char* content, const char* sender, const char* receiver);
void ai_dancer_destroy_message(Message* msg);

/* Client Management Functions */
Client* ai_dancer_create_client(const char* platform, void* config);
void ai_dancer_destroy_client(Client* client);

/* Memory Management Functions */
int ai_dancer_save_memory(Agent* agent, const char* path);
int ai_dancer_load_memory(Agent* agent, const char* path);

#endif /* AI_DANCER_H */ 