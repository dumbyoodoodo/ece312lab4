#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "lab4.h"

extern Node *g_root;

#define MAGIC   0x54454348u   /* "TECH" */
#define VERSION 1u

typedef struct { Node *node; int id; } NodeMapping;

/* ----------------------------------------------------------------
 * TODO 27  save_tree
 *
 * Serialize the entire tree to a binary file using BFS order.
 *
 * File format:
 *   Header:  uint32 magic | uint32 version | uint32 nodeCount
 *   Per node (BFS order):
 *     uint8  isQuestion
 *     uint32 textLen          (bytes, no null terminator in file)
 *     char[] text             (exactly textLen bytes)
 *     int32  yesId            (-1 if NULL)
 *     int32  noId             (-1 if NULL)
 *
 * Return 1 on success, 0 on failure.
 * ---------------------------------------------------------------- */
int save_tree(const char *filename) {
    if(g_root == NULL) return 0;
    int count = count_nodes(g_root);
    Node** b = malloc(count * sizeof(Node*));
    int front = 0;
    int back = 0;
    b[0] = g_root;
    back++;
    //bfs to put nodes in order
    while(front < back){
        Node* cur = b[front];
        front++;
        if(cur->yes){
            b[back] = cur->yes;
            back++;
        }
        if(cur->no){
            b[back] = cur->no;
            back++;
        }
    }

    int* yes = malloc(count * sizeof(int));
    int* no = malloc(count *sizeof(int));
    //record indices for children node
    int next = 1;
    for(int i = 0; i < count; i++){
        if(b[i]->yes){
            yes[i] = next;
            next++;
        }
        else{
            yes[i] = -1;
        }
        if(b[i]->no){
            no[i] = next;
            next++;
        }
        else{
            no[i] = -1;
        }
    }

    FILE *f = fopen(filename, "wb");

    if(!f){
        free(b);
        free(yes);
        free(no);
        return 0;
    }

    uint32_t m = MAGIC;
    uint32_t v = VERSION;
    uint32_t c = (uint32_t)count;
    //write header
    fwrite(&m, sizeof(uint32_t),1,f);
    fwrite(&v,sizeof(uint32_t),1,f);
    fwrite(&c, sizeof(uint32_t),1,f);
    //write nodes
    for(int i = 0; i < count; i++){
        uint8_t isQ = (uint8_t)b[i]->isQuestion;
        uint32_t textLen = (uint32_t)strlen(b[i]->text);
        int32_t yId = (int32_t)yes[i];
        int32_t nId = (int32_t)no[i];
        fwrite(&isQ, sizeof(uint8_t), 1, f);
        fwrite(&textLen, sizeof(uint32_t), 1, f);
        fwrite(b[i]->text, 1, textLen, f);
        fwrite(&yId, sizeof(int32_t), 1, f);
        fwrite(&nId, sizeof(int32_t), 1, f);
    }

    fclose(f);
    free(b);
    free(yes);
    free(no);
    return 1;
}

/* ----------------------------------------------------------------
 * TODO 28  load_tree
 *
 * Read a file written by save_tree and reconstruct the tree.
 * Validate the magic number.  Read all nodes into a flat array
 * first, then link children in a second pass.
 * Free any existing g_root before installing the new one.
 * Return 1 on success, 0 on any error (free partial allocations).
 * ---------------------------------------------------------------- */
int load_tree(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if(!f) return 0;

    uint32_t m, v, count;
    fread(&m, sizeof(uint32_t), 1, f);
    //not valid
    if(m != MAGIC){
        fclose(f);
        return 0;

    }
    fread(&v, sizeof(uint32_t), 1, f);
    //number of nodes
    fread(&count, sizeof(uint32_t), 1, f);
    //node pointers
    Node **nodes = malloc(count * sizeof(Node *));
    //children indices
    int *yes = malloc(count * sizeof(int));
    int *no  = malloc(count * sizeof(int));
    //create nodes
    for(uint32_t i = 0; i < count; i++){
        uint8_t q;
        uint32_t len;
        int32_t y, n;
        fread(&q,sizeof(uint8_t), 1, f);
        fread(&len,sizeof(uint32_t), 1, f);
        char *text = malloc(len+1);
        fread(text, 1, len, f);
        text[len] = '\0';

        fread(&y, sizeof(int32_t), 1, f);
        fread(&n, sizeof(int32_t), 1, f);

        if (q) {
            nodes[i] = create_question_node(text);
        } 
        else {
            nodes[i] = create_solution_node(text);
        }
        free(text);
        yes[i] = y;
        no[i] = n;
    }
    fclose(f);
    //link children
    for(uint32_t i = 0; i < count; i++){
        if (yes[i] != -1) {
            nodes[i]->yes = nodes[yes[i]];
        } else {
            nodes[i]->yes = NULL;
        }
        if (no[i] != -1) {
            nodes[i]->no = nodes[no[i]];
        } else {
            nodes[i]->no = NULL;
        }
    }
    free_tree(g_root);
    g_root = nodes[0];

    free(nodes);
    free(yes);
    free(no);
    return 1;
}
