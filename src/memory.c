#include "../include/memory.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * Implementation of the Memory System
 */

/* 
 * Create a new memory entry
 */
MemoryEntry* eliza_memory_entry_create(const char* content, float importance,
                                     const char* context, const char* category) {
    if (!content) return NULL;

    MemoryEntry* entry = (MemoryEntry*)malloc(sizeof(MemoryEntry));
    if (!entry) return NULL;

    /* Initialize the entry */
    entry->content = strdup(content);
    entry->timestamp = time(NULL);
    entry->importance = importance;
    entry->context = context ? strdup(context) : NULL;
    entry->category = category ? strdup(category) : NULL;

    return entry;
}

/*
 * Destroy a memory entry and free its resources
 */
void eliza_memory_entry_destroy(MemoryEntry* entry) {
    if (!entry) return;

    free(entry->content);
    free(entry->context);
    free(entry->category);
    free(entry);
}

/*
 * Create a new memory store
 */
MemoryStore* eliza_memory_create(size_t initial_capacity) {
    MemoryStore* store = (MemoryStore*)malloc(sizeof(MemoryStore));
    if (!store) return NULL;

    /* Allocate the entries array */
    store->entries = (MemoryEntry**)malloc(initial_capacity * sizeof(MemoryEntry*));
    if (!store->entries) {
        free(store);
        return NULL;
    }

    /* Initialize the store */
    store->capacity = initial_capacity;
    store->size = 0;
    
    /* Set up function pointers */
    store->add_memory = eliza_memory_add;
    store->search = eliza_memory_search;
    store->save_to_file = eliza_memory_save;
    store->load_from_file = eliza_memory_load;

    return store;
}

/*
 * Destroy a memory store and all its entries
 */
void eliza_memory_destroy(MemoryStore* store) {
    if (!store) return;

    /* Free all entries */
    for (size_t i = 0; i < store->size; i++) {
        eliza_memory_entry_destroy(store->entries[i]);
    }

    free(store->entries);
    free(store);
}

/*
 * Add a new memory to the store
 * Returns 0 on success, -1 on failure
 */
int eliza_memory_add(MemoryStore* store, const char* content,
                    float importance, const char* context, const char* category) {
    if (!store || !content) return -1;

    /* Check if we need to resize */
    if (store->size >= store->capacity) {
        size_t new_capacity = store->capacity * 2;
        MemoryEntry** new_entries = (MemoryEntry**)realloc(store->entries,
                                   new_capacity * sizeof(MemoryEntry*));
        if (!new_entries) return -1;

        store->entries = new_entries;
        store->capacity = new_capacity;
    }

    /* Create and add the new entry */
    MemoryEntry* entry = eliza_memory_entry_create(content, importance, context, category);
    if (!entry) return -1;

    store->entries[store->size++] = entry;
    return 0;
}

/*
 * Simple string matching search
 * Returns an array of matching entries, terminated with a NULL pointer
 */
MemoryEntry** eliza_memory_search(MemoryStore* store, const char* query,
                                size_t max_results) {
    if (!store || !query) return NULL;

    /* Allocate result array (max_results + 1 for NULL terminator) */
    MemoryEntry** results = (MemoryEntry**)malloc((max_results + 1) * sizeof(MemoryEntry*));
    if (!results) return NULL;

    size_t found = 0;
    
    /* Simple string matching search */
    for (size_t i = 0; i < store->size && found < max_results; i++) {
        if (strstr(store->entries[i]->content, query) != NULL) {
            results[found++] = store->entries[i];
        }
    }

    results[found] = NULL; /* NULL terminate the array */
    return results;
}

/*
 * Save memory store to a file
 * Returns 0 on success, -1 on failure
 */
int eliza_memory_save(MemoryStore* store, const char* filepath) {
    if (!store || !filepath) return -1;

    FILE* file = fopen(filepath, "w");
    if (!file) return -1;

    /* Write header */
    fprintf(file, "ELIZA_MEMORY_STORE\n");
    fprintf(file, "SIZE:%zu\n", store->size);

    /* Write each entry */
    for (size_t i = 0; i < store->size; i++) {
        MemoryEntry* entry = store->entries[i];
        fprintf(file, "---ENTRY---\n");
        fprintf(file, "CONTENT:%s\n", entry->content);
        fprintf(file, "TIMESTAMP:%ld\n", (long)entry->timestamp);
        fprintf(file, "IMPORTANCE:%f\n", entry->importance);
        fprintf(file, "CONTEXT:%s\n", entry->context ? entry->context : "");
        fprintf(file, "CATEGORY:%s\n", entry->category ? entry->category : "");
    }

    fclose(file);
    return 0;
}

/*
 * Load memory store from a file
 * Returns 0 on success, -1 on failure
 */
int eliza_memory_load(MemoryStore* store, const char* filepath) {
    if (!store || !filepath) return -1;

    FILE* file = fopen(filepath, "r");
    if (!file) return -1;

    char buffer[1024];
    char header[64];

    /* Read and verify header */
    if (!fgets(buffer, sizeof(buffer), file) ||
        strcmp(buffer, "ELIZA_MEMORY_STORE\n") != 0) {
        fclose(file);
        return -1;
    }

    /* Clear existing entries */
    for (size_t i = 0; i < store->size; i++) {
        eliza_memory_entry_destroy(store->entries[i]);
    }
    store->size = 0;

    /* Read entries */
    while (fgets(buffer, sizeof(buffer), file)) {
        if (strcmp(buffer, "---ENTRY---\n") == 0) {
            char content[1024] = "";
            char context[1024] = "";
            char category[1024] = "";
            time_t timestamp = 0;
            float importance = 0.0f;

            /* Read entry fields */
            while (fgets(buffer, sizeof(buffer), file) &&
                   sscanf(buffer, "%[^:]:%s", header, buffer) == 2) {
                if (strcmp(header, "CONTENT") == 0) strcpy(content, buffer);
                else if (strcmp(header, "TIMESTAMP") == 0) timestamp = atol(buffer);
                else if (strcmp(header, "IMPORTANCE") == 0) importance = atof(buffer);
                else if (strcmp(header, "CONTEXT") == 0) strcpy(context, buffer);
                else if (strcmp(header, "CATEGORY") == 0) strcpy(category, buffer);
            }

            /* Add the entry */
            if (content[0] != '\0') {
                eliza_memory_add(store, content, importance,
                               context[0] ? context : NULL,
                               category[0] ? category : NULL);
            }
        }
    }

    fclose(file);
    return 0;
} 