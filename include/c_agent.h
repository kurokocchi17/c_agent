#ifndef C_AGENT_H
#define C_AGENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Core Types and Structures
 * These define the fundamental building blocks of the C Agent framework
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
 * Contains all the necessary components for agent operation
 */
typedef struct {
    char* id;               /* Unique identifier for the agent */
    char* name;             /* Display name of the agent */
    char* description;      /* Description of the agent's purpose/personality */
    void* memory;           /* Pointer to agent's memory system */
    void* model;            /* Pointer to the language model interface */
} Agent;

/*
 * Client interface structure for platform-specific implementations
 * Provides a common interface for different messaging platforms
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

/* Core API Functions */
Agent* c_agent_create_agent(const char* config_path);
void c_agent_destroy_agent(Agent* agent);
int c_agent_process_message(Agent* agent, const Message* msg);
Message* c_agent_create_message(const char* content, const char* sender, const char* receiver);
void c_agent_destroy_message(Message* msg);
Client* c_agent_create_client(const char* platform, void* config);
void c_agent_destroy_client(Client* client);
int c_agent_save_memory(Agent* agent, const char* path);
int c_agent_load_memory(Agent* agent, const char* path);

#endif /* C_AGENT_H */ 