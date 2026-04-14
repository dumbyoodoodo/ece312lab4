#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lab4.h"

/* ----------------------------------------------------------------
 * ds.c  --  all data structures for the Tech Support Diagnosis Tool
 *
 * Implement every function marked TODO.  The only functions in this
 * entire lab permitted to use recursion are free_tree and count_nodes.
 * Everything else must be iterative.
 * ---------------------------------------------------------------- */


/* ====== Tree nodes ============================================== */

/* TODO 1 */
Node *create_question_node(const char *question) {
    Node *q = malloc(sizeof(Node));
    q->text = strdup(question);
    q->yes = NULL;
    q->no = NULL;
    q->isQuestion = 1;
    return q;
}

/* TODO 2 */
Node *create_solution_node(const char *solution) {
    Node *s = malloc(sizeof(Node));
    s->text = strdup(solution);
    s->yes = NULL;
    s->no = NULL;
    s->isQuestion = 0;
    return s;
}

/* TODO 3  (recursion allowed) */
void free_tree(Node *node) {
    if (node == NULL) return;
    free_tree(node->yes);
    free_tree(node->no);
    free(node->text);
    free(node);
}

/* TODO 4  (recursion allowed) */
int count_nodes(Node *root) {
    if(root == NULL)return 0;
    return 1 + count_nodes(root->yes) + count_nodes(root->no);
}


/* ====== FrameStack  (dynamic array, iterative traversal) ======== */

/* TODO 5 */
void fs_init(FrameStack *s) {
    s->size = 0;
    s->capacity = 4;
    s->frames = malloc(4 * sizeof(Frame));
}

/* TODO 6 */
void fs_push(FrameStack *s, Node *node, int answeredYes) {
    if (s->size >= s->capacity) {
        s->capacity *= 2;
        s->frames = realloc(s->frames, s->capacity * sizeof(Frame));
    }
    s->frames[s->size].node = node;
    s->frames[s->size].answeredYes = answeredYes;
    s->size++;
}

/* TODO 7 */
Frame fs_pop(FrameStack *s) {
    s->size--;
    return s->frames[s->size];
}

/* TODO 8 */
int fs_empty(FrameStack *s) {
    return s->size == 0;
}

/* TODO 9 */
void fs_free(FrameStack *s) {
    free(s->frames);
}


/* ====== EditStack  (dynamic array, undo/redo) =================== */

/* TODO 10 */
void es_init(EditStack *s) {
    s->size = 0;
    s->capacity = 4;
    s->edits = malloc(4 * sizeof(Edit));
}

/* TODO 11 */
void es_push(EditStack *s, Edit e) {
    if(s->size >= s->capacity){
        s->capacity *= 2;
        s->edits = realloc(s->edits, s->capacity * sizeof(Edit));
    }
    s->edits[s->size] = e;
    s->size++;
}

/* TODO 12 */
Edit es_pop(EditStack *s) {
    s->size--;
    return s->edits[s->size];
}

/* TODO 13 */
int es_empty(EditStack *s) {
    return s->size == 0;
}

/* TODO 14 */
void es_clear(EditStack *s) {
    s->size = 0;
}

/* provided -- do not modify */
void es_free(EditStack *s) {
    free(s->edits);
    s->edits    = NULL;
    s->size     = 0;
    s->capacity = 0;
}

void free_edit_stack(EditStack *s) { es_free(s); }


/* ====== Queue  (linked list, BFS) ============================== */

/* TODO 15 */
void q_init(Queue *q) {
    q->front = NULL;
    q->rear  = NULL;
    q->size  = 0;
}

/* TODO 16 */
void q_enqueue(Queue *q, Node *node, int id) {
    QueueNode* n = malloc(sizeof(QueueNode));
    n->treeNode = node;
    n->id = id;
    n->next = NULL;
    if(q->rear == NULL){
        q->front = n;
        q->rear = n;
    }
    else{
        q->rear->next = n;
        q->rear = n;
    }
    q->size++;
}

/* TODO 17 */
int q_dequeue(Queue *q, Node **node, int *id) {
    if(q->front == NULL) return 0;
    QueueNode* temp = q->front;
    *node = temp->treeNode;
    *id = temp->id;
    q->front = q->front->next;
    if(q->front == NULL) q->rear = NULL;
    free(temp);
    q->size--;
    return 1;
}

/* TODO 18 */
int q_empty(Queue *q) {
    return q->size == 0;
    

}

/* TODO 19 */
void q_free(Queue *q) {
    while (q->front != NULL) {
        QueueNode* temp = q->front;
        q->front = temp->next;
        free(temp);
    }
}


/* ====== Hash table  (separate chaining) ======================== */

/* TODO 20
 * Convert a string to a canonical key:
 *   letters  -> lowercase
 *   spaces   -> underscore
 *   anything else -> drop
 * Caller owns the returned string and must free() it.
 */
char *canonicalize(const char *s) {
    if (s == NULL) return strdup("");
    int len = strlen(s);
    char *out = malloc(len+ 1);
    int j = 0;
    for (int i = 0; i < len; i++) {
        if (isalpha(s[i])) {
            out[j] = tolower(s[i]);
            j++;
        } else if (s[i] == ' ') {
            out[j] = '_';
            j++;
        }
    }
    out[j] = '\0';
    return out;
}

/* TODO 21  (djb2: hash = hash*33 + c, seed 5381) */
unsigned h_hash(const char *s) {
    unsigned hash = 5381;
    while(*s){
        hash = hash * 33 + *s;
        s++;
    }
    return hash;
}

/* TODO 22 */
void h_init(Hash *h, int nbuckets) {
    h->buckets = malloc(nbuckets * sizeof(Entry*));
    for(int i = 0; i < nbuckets; i++){
        h->buckets[i] = NULL;
    }
    h->nbuckets = nbuckets;
    h->size = 0;
}

/* TODO 23 */
int h_put(Hash *h, const char *key, int solutionId) {
    int index = h_hash(key) % h->nbuckets;
    Entry* e = h->buckets[index];
    while (e != NULL){
        if(strcmp(e->key, key) == 0){
            break;
        }
        e = e->next;
        
    }
    if(e == NULL){
            e = malloc(sizeof(Entry));
            e->key = strdup(key);
            e->vals.ids = malloc(4 * sizeof(int));
            e->vals.ids[0] = solutionId;
            e->vals.count = 1;
            e->vals.capacity = 4;
            e->next = h->buckets[index];
            h->buckets[index] = e;
            h->size++;
        }
        else {
            for (int i = 0; i < e->vals.count; i++) {
                if (e->vals.ids[i] == solutionId) return 1;
            }
            if (e->vals.count >= e->vals.capacity) {
                e->vals.capacity *= 2;
                e->vals.ids = realloc(e->vals.ids, e->vals.capacity * sizeof(int));
            }
            e->vals.ids[e->vals.count] = solutionId;
            e->vals.count++;
        }
        return 1;

}

/* TODO 24 */
int h_contains(const Hash *h, const char *key, int solutionId) {
    int index = h_hash(key) % h->nbuckets;
    Entry *e = h->buckets[index];
    while (e != NULL) {
        if (strcmp(e->key, key) == 0){
            for (int i = 0; i < e->vals.count; i++) {
                if (e->vals.ids[i] == solutionId) return 1;
            }
            return 0;
        }
        e = e->next;
    }
    return 0;
}

/* TODO 25 */
int *h_get_ids(const Hash *h, const char *key, int *outCount) {
    *outCount = 0;
    int index = h_hash(key) % h->nbuckets;
    Entry *e = h->buckets[index];
    while (e != NULL) {
        if (strcmp(e->key, key) == 0) {
            int *copy = malloc(e->vals.count * sizeof(int));
            memcpy(copy, e->vals.ids, e->vals.count * sizeof(int));
            *outCount = e->vals.count;
            return copy;
        }
        e = e->next;
    }
    return NULL;
}

/* TODO 26 */
void h_free(Hash *h) {
    for (int i = 0; i < h->nbuckets; i++) {
        Entry *e = h->buckets[i];
        while (e != NULL) {
            Entry *next = e->next;
            free(e->key);
            free(e->vals.ids);
            free(e);
            e = next;
        }
    }
    free(h->buckets);
}
