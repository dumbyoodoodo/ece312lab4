#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "lab4.h"

extern Node *g_root;

/* ----------------------------------------------------------------
 * TODO 29  check_integrity
 *
 * Use BFS to verify:
 *   - Every question node has both yes and no children (non-NULL).
 *   - Every solution node has both children NULL.
 * Return 1 if valid, 0 if any violation is found.
 * ---------------------------------------------------------------- */
int check_integrity(void) {
    if(g_root == NULL) return 1;
    Queue q;
    q_init(&q);
    q_enqueue(&q, g_root, 0);
    Node *node;
    int id;
    while(!q_empty(&q)){
        q_dequeue(&q, &node, &id);
        if(node->isQuestion){
            if (node->yes == NULL || node->no == NULL) {
                q_free(&q);
                return 0;
            }
            q_enqueue(&q, node->yes, 0);
            q_enqueue(&q, node->no, 0);
        }
        else {
            if (node->yes != NULL || node->no != NULL) {
                q_free(&q);
                return 0;
            }
        }
    }
    q_free(&q);
    return 1;
}

/* ----------------------------------------------------------------
 * TODO 30  find_shortest_path
 *
 * Given the exact text of two solution leaves, display the
 * questions that distinguish them.  Use BFS with a parent-tracking
 * PathNode array to find both leaves, build ancestor arrays for
 * each, find the Lowest Common Ancestor (LCA), then print:
 *   - The shared path of questions both solutions pass through.
 *   - The divergence question (LCA) and which branch leads where.
 *
 * Display results with mvprintw.  Print an error if either
 * solution is not found.  Free all allocations before returning.
 * ---------------------------------------------------------------- */
void find_shortest_path(const char *sol1, const char *sol2) {
    clear();
    if (g_root == NULL) {
        mvprintw(10, 2, "Error: knowledge base is empty.");
        refresh();
        return;
    }
    int count = count_nodes(g_root);

    typedef struct {
        Node *node;
        int   parentId;
        int   branchYes;
    } PathNode;

    PathNode *pn = malloc(count * sizeof(PathNode));

    int front = 0, back = 0;
    pn[0].node = g_root; pn[0].parentId = -1; pn[0].branchYes = -1;
    back++;

    int id1 = -1, id2 = -1;

    while(front < back){
        int cur = front++;
        Node *n = pn[cur].node;

        if(!n->isQuestion){
            if(strcmp(n->text, sol1) == 0) id1 = cur;
            if(strcmp(n->text, sol2) == 0) id2 = cur;
            if(id1 != -1 && id2 != -1) break;
        }
        if(n->yes){
            pn[back].node = n->yes; pn[back].parentId = cur; pn[back].branchYes = 1;
            back++;
        }
        if(n->no){
            pn[back].node = n->no; pn[back].parentId = cur; pn[back].branchYes = 0;
            back++;
        }
    }

    if(id1 == -1 || id2 == -1){
        mvprintw(10, 2, "Error: one or both solutions not found.");
        refresh();
        free(pn);
        return;
    }
    
    int anc1[count], len1 = 0;
    int cur1 = id1;
    while(cur1 != -1){
        anc1[len1++] = cur1;
        cur1 = pn[cur1].parentId;
    }

    int anc2[count], len2 = 0;
    int cur2 = id2;
    while(cur2 != -1){
        anc2[len2++] = cur2;
        cur2 = pn[cur2].parentId;
    }

    int i1 = len1 - 1;
    int i2 = len2 - 1;
    while(i1 > 0 && i2 > 0 && anc1[i1-1] == anc2[i2-1]){
        i1--;
        i2--;
    }
    int lcaId = anc1[i1];
    int row = 2;
    mvprintw(row++, 2, "Shared path to divergence:");

    for(int i = len1 - 1; i >= i1; i--){
        mvprintw(row++, 4, "-> %s", pn[anc1[i]].node->text);
    }

    mvprintw(row++, 2, "Diverges at: %s", pn[lcaId].node->text);

    if(i1 > 0){
        mvprintw(row++, 4, "YES -> ... -> %s", pn[anc1[0]].node->text);
    }
    if(i2 > 0){
        mvprintw(row++, 4, "NO  -> ... -> %s", pn[anc2[0]].node->text);
    }

    refresh();
    getch();
    free(pn);
}
