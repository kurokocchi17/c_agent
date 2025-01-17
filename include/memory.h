#ifndef ELIZA_MEMORY_H
#define ELIZA_MEMORY_H

#include <time.h>

/*
 * Memory System Interface
 * Defines the structures and functions for managing agent memory
 */

/* 
 * Memory Entry structure
 * Represents a single memory/conversation entry
 */
typedef struct {
    char* content;           /* The content of the memory */
    time_t timestamp;        /* When the memory was created */
    float importance;        /* Importance score (0.0 to 1.0) */
    char* context;          /* Additional context or metadata */
    char* category;         /* Category or type of memory */
} MemoryEntry;

/*
 * Memory Store structure
 * Manages a collection of memories with search and retrieval capabilities
 */
typedef struct {
    MemoryEntry** entries;   /* Array of memory entries */
    size_t capacity;         /* Maximum number of entries */
    size_t size;            /* Current number of entries */
    
    /* Memory management functions */
    int (*add_memory)(struct MemoryStore* store, const char* content, 
                     float importance, const char* context, const char* category);
    MemoryEntry** (*search)(struct MemoryStore* store, const char* query, 
                          size_t max_results);
    int (*save_to_file)(struct MemoryStore* store, const char* filepath);
    int (*load_from_file)(struct MemoryStore* store, const char* filepath);
} MemoryStore;

/*
 * Function Declarations
 */

/* Create a new memory store with specified capacity */
MemoryStore* eliza_memory_create(size_t initial_capacity);

/* Destroy a memory store and free all associated resources */
void eliza_memory_destroy(MemoryStore* store);

/* Create a new memory entry */
MemoryEntry* eliza_memory_entry_create(const char* content, float importance,
                                     const char* context, const char* category);

/* Destroy a memory entry */
void eliza_memory_entry_destroy(MemoryEntry* entry);

/* Add a new memory to the store */
int eliza_memory_add(MemoryStore* store, const char* content,
                    float importance, const char* context, const char* category);

/* Search for memories based on a query */
MemoryEntry** eliza_memory_search(MemoryStore* store, const char* query,
                                size_t max_results);

/* Save memory store to a file */
int eliza_memory_save(MemoryStore* store, const char* filepath);

/* Load memory store from a file */
int eliza_memory_load(MemoryStore* store, const char* filepath);

#endif /* ELIZA_MEMORY_H */ 