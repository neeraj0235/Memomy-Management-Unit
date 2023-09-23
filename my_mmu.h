#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>

#define PAGESIZE 1024

// Include your Headers below

// You are not allowed to use the function malloc and calloc directly .
typedef struct Node {
    size_t size;
    struct Node* next;
    struct Node* prev;
    bool is_free;
} Node;


Node* head = NULL;
Node* tail = NULL;
Node* last_allocated_block = NULL;

// void add_to_freelist(Node* node) {
//     node->prev = NULL;
//     node->next = NULL;

//     if(head == NULL || head > node) {
//         if(head != NULL) {
//             head->prev = node;
//         }
//         node->next = head;
//         head = node;
//     } else {
//         Node* current = head;
//         while(current->next != NULL && current->next < b) {
//             current = current->next;
//         }
//         node->next = current->next;
//         current->next = node;

//         node->prev = current;
//         if(current->next != NULL) 
//             current->next->prev = node;
//     }
// }

// void remove_from_freelist(Node* node) {
//     if(node->prev == NULL) {
//         head = (node->next != NULL ? node->next : NULL);
//     } else {
//         node->prev->next = node->next;
//     }
//     if(node->next != NULL) {
//         node->next->prev = node->prev;
//     }
// }


// |---------------------node---------------------|
// |------node-------|----------new_block---------|  
// Shrinks the block pointed by 'node' to 'size' and add a new_block in the remaining space           
void split(Node* node, size_t size) {
    Node* new_block;

    new_block->size = node->size - sizeof(Node) - size;
    new_block->prev = node;
    new_block->next = node->next;
    new_block->is_free = true;

    node->next = new_block;
    node->size = size;
    node->is_free = false;
}

void coalesce(Node* node) {
    if(node->next != NULL && node->next->is_free) {
        node->size += sizeof(Node) + node->next->size;
        if(node->next->next != NULL) 
            node->next->next->prev = node;
        node->next = node->next->next;
    }
    if(node->prev != NULL && node->prev->is_free) {
        node->size += sizeof(Node) + node->prev->size;
        if(node->prev->prev != NULL) 
            node->prev->prev->next = node;
        node->prev = node->prev->prev;
    }
}

// Function to allocate memory using mmap
void* my_malloc(size_t size) {
    // Your implementation of my_malloc goes here
    if(head == NULL) {
        head = (Node*)sbrk(PAGESIZE);

        head->size = size;
        head->is_free = true;

        split(head, size);
        tail = head->next;

        last_allocated_block = head;
        void* allocated_ptr = (char*)head + sizeof(Node);

        return allocated_ptr;
    }

    Node* current = last_allocated_block;

    do {
        if(current->is_free && current->size >= size + sizeof(Node)) {
            split(current, size);

            last_allocated_block = current;
            void* allocated_ptr = (char*)current + sizeof(Node);

            return allocated_ptr;
        }
        current = (current->next == NULL ? head : current->next);
    } while(current != last_allocated_block);

    Node* new_block = (Node*)sbrk(PAGESIZE);

    tail->next = new_block;
    new_block->prev = tail;
    tail = new_block;

    coalesce(tail);
    split(tail, size);

    last_allocated_block = tail;
    void* allocated_ptr = (char*)tail + sizeof(Node);

    tail = tail->next;

    return allocated_ptr;
}



// Function to allocate and initialize memory to zero using mmap
void* my_calloc(size_t nelem, size_t size) {
    // Your implementation of my_calloc goes here

    void* allocated_ptr = my_malloc(nelem * size);

    if(allocated_ptr != NULL)
        memset(allocated_ptr, 0, nelem * size);

    return allocated_ptr;
    
}
// Function to release memory allocated using my_malloc and my_calloc
void my_free(void* ptr) {
    // Your implementation of my_free goes here
    Node* current = head;
    while(current != NULL) {
        if((char*)current + sizeof(Node) == ptr) {
            current->is_free = true;
            break;
        }
        current = current->next;
    }
    coalesce(current);
}

void debug() {
    Node* current = head;

    while(current != NULL) {
        if(current->is_free) 
            printf("Free block of size %d available at address %d", current->size, (char*)current + sizeof(Node));

        else
            printf("Occupied block of size %d at address %d", current->size, (char*)current + sizeof(Node));
    }
}



