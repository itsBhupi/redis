#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

typedef struct ClassA ClassA;
extern ClassA* ClassA_new(size_t initial_capacity);
extern void ClassA_destroy(ClassA *obj);
extern int ClassA_appendData(ClassA *obj, const char *data, size_t len);
extern size_t ClassA_getSize(ClassA *obj);
extern const char* ClassA_getData(ClassA *obj);
extern int ClassA_verifyIntegrity(ClassA *obj);
extern void ClassA_clear(ClassA *obj);
extern uint32_t ClassA_calculateChecksum(ClassA *obj);
extern void ClassA_debug(ClassA *obj);

typedef struct ClassB {
    ClassA *storage;
    char *processing_buffer;
    size_t buffer_size;
    int processing_mode;
    int is_initialized;
    uint64_t operations_count;
} ClassB;
#define MODE_UPPERCASE 0
#define MODE_LOWERCASE 1
#define MODE_REVERSE   2
#define MODE_ROT13     3

ClassB* ClassB_new(size_t storage_capacity, int processing_mode) {
    ClassB *obj = malloc(sizeof(ClassB));
    if (!obj) {
        return NULL;
    }
    
    obj->storage = ClassA_new(storage_capacity);
    if (!obj->storage) {
        free(obj);
        return NULL;
    }
    
    obj->buffer_size = storage_capacity;
    obj->processing_buffer = malloc(obj->buffer_size);
    if (!obj->processing_buffer) {
        ClassA_destroy(obj->storage);
        free(obj);
        return NULL;
    }
    
    obj->processing_mode = processing_mode;
    obj->is_initialized = 1;
    obj->operations_count = 0;
    
    return obj;
}

void ClassB_destroy(ClassB *obj) {
    if (obj) {
        if (obj->storage) {
            ClassA_destroy(obj->storage);
        }
        if (obj->processing_buffer) {
            free(obj->processing_buffer);
        }
        obj->is_initialized = 0;
        free(obj);
    }
}

int ClassB_setProcessingMode(ClassB *obj, int mode) {
    if (!obj || !obj->is_initialized) {
        return 0;
    }
    
    if (mode < 0 || mode > 3) {
        return 0;
    }
    
    obj->processing_mode = mode;
    return 1;
}

char* ClassB_processText(ClassB *obj, const char *input, size_t len) {
    if (!obj || !obj->is_initialized || !input || len == 0) {
        return NULL;
    }
    
    if (len >= obj->buffer_size) {
        size_t new_size = len + 1;
        char *new_buffer = realloc(obj->processing_buffer, new_size);
        if (!new_buffer) {
            return NULL;
        }
        obj->processing_buffer = new_buffer;
        obj->buffer_size = new_size;
    }
    
    memcpy(obj->processing_buffer, input, len);
    obj->processing_buffer[len] = '\0';
    
    switch (obj->processing_mode) {
        case MODE_UPPERCASE:
            for (size_t i = 0; i < len; i++) {
                obj->processing_buffer[i] = toupper((unsigned char)obj->processing_buffer[i]);
            }
            break;
            
        case MODE_LOWERCASE:
            for (size_t i = 0; i < len; i++) {
                obj->processing_buffer[i] = tolower((unsigned char)obj->processing_buffer[i]);
            }
            break;
            
        case MODE_REVERSE:
            for (size_t i = 0; i < len / 2; i++) {
                char temp = obj->processing_buffer[i];
                obj->processing_buffer[i] = obj->processing_buffer[len - 1 - i];
                obj->processing_buffer[len - 1 - i] = temp;
            }
            break;
            
        case MODE_ROT13:
            for (size_t i = 0; i < len; i++) {
                char c = obj->processing_buffer[i];
                if (c >= 'a' && c <= 'z') {
                    obj->processing_buffer[i] = ((c - 'a' + 13) % 26) + 'a';
                } else if (c >= 'A' && c <= 'Z') {
                    obj->processing_buffer[i] = ((c - 'A' + 13) % 26) + 'A';
                }
            }
            break;
    }
    
    obj->operations_count++;
    return obj->processing_buffer;
}

int ClassB_storeData(ClassB *obj, const char *data, size_t len) {
    if (!obj || !obj->is_initialized || !data || len == 0) {
        return 0;
    }
    
    if (!ClassA_appendData(obj->storage, data, len)) {
        return 0;
    }
    
    if (!ClassA_verifyIntegrity(obj->storage, len)) {
        printf("Warning: Data integrity check failed\n");
        return 0;
    }
    
    return 1;
}

int ClassB_processAndStore(ClassB *obj, const char *input, size_t len) {
    if (!obj || !obj->is_initialized) {
        return 0;
    }
    
    char *processed = ClassB_processText(obj, input, len);
    if (!processed) {
        return 0;
    }
    
    return ClassB_storeData(obj, processed, strlen(processed));
}

const char* ClassB_getStoredData(ClassB *obj) {
    if (!obj || !obj->is_initialized) {
        return NULL;
    }
    
    return ClassA_getData(obj->storage);
}

size_t ClassB_getStoredSize(ClassB *obj) {
    if (!obj || !obj->is_initialized) {
        return 0;
    }
    
    return ClassA_getSize(obj->storage);
}

void ClassB_clear(ClassB *obj) {
    if (obj && obj->is_initialized && obj->storage) {
        ClassA_clear(obj->storage);
        obj->operations_count = 0;
    }
}

uint64_t ClassB_getOperationsCount(ClassB *obj) {
    if (!obj || !obj->is_initialized) {
        return 0;
    }
    
    return obj->operations_count;
}

const char* ClassB_getModeName(ClassB *obj) {
    if (!obj || !obj->is_initialized) {
        return "INVALID";
    }
    
    switch (obj->processing_mode) {
        case MODE_UPPERCASE: return "UPPERCASE";
        case MODE_LOWERCASE: return "lowercase";
        case MODE_REVERSE:   return "REVERSE";
        case MODE_ROT13:     return "ROT13";
        default:             return "UNKNOWN";
    }
}

void ClassB_debug(ClassB *obj) {
    if (!obj) {
        printf("ClassB: NULL object\n");
        return;
    }
    
    printf("ClassB Debug Info:\n");
    printf("  Initialized: %s\n", obj->is_initialized ? "Yes" : "No");
    printf("  Processing mode: %s (%d)\n", ClassB_getModeName(obj), obj->processing_mode);
    printf("  Operations count: %llu\n", (unsigned long long)obj->operations_count);
    printf("  Buffer size: %zu\n", obj->buffer_size);
    printf("  Storage size: %zu\n", ClassB_getStoredSize(obj));
    
    if (obj->storage) {
        printf("  Internal ClassA storage:\n");
        ClassA_debug(obj->storage);
    }
}

#ifdef DEMO_CLASS_B
int main() {
    printf("=== Class B Demo ===\n");
    
    ClassB *processor = ClassB_new(256, MODE_UPPERCASE);
    if (!processor) {
        printf("Failed to create ClassB object\n");
        return 1;
    }
    
    const char *test_text = "Hello, World! This is a test.";
    
    printf("Original text: %s\n", test_text);
    
    ClassB_processAndStore(processor, test_text, strlen(test_text));
    printf("Uppercase result: %s\n", ClassB_getStoredData(processor));
    
    ClassB_clear(processor);
    
    ClassB_setProcessingMode(processor, MODE_LOWERCASE);
    ClassB_processAndStore(processor, test_text, strlen(test_text));
    printf("Lowercase result: %s\n", ClassB_getStoredData(processor));
    
    ClassB_clear(processor);
    
    ClassB_setProcessingMode(processor, MODE_REVERSE);
    ClassB_processAndStore(processor, test_text, strlen(test_text));
    printf("Reverse result: %s\n", ClassB_getStoredData(processor));
    
    ClassB_clear(processor);
    
    ClassB_setProcessingMode(processor, MODE_ROT13);
    ClassB_processAndStore(processor, test_text, strlen(test_text));
    printf("ROT13 result: %s\n", ClassB_getStoredData(processor));
    
    printf("\nFinal debug information:\n");
    ClassB_debug(processor);
    
    ClassB_destroy(processor);
    printf("\nProcessor destroyed successfully\n");
    
    return 0;
}
#endif 
