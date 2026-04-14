#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "lab4.h"

extern Node      *g_root;
extern EditStack  g_undo;
extern EditStack  g_redo;
extern Hash       g_index;

/* ----------------------------------------------------------------
 * TODO 31  run_diagnosis
 *
 * Walk the decision tree iteratively (no recursion) using a
 * FrameStack.  At each question node ask the user yes/no and push
 * the appropriate child.  At each solution leaf display the fix and
 * ask whether it solved the problem.
 *
 * If the fix did not help, enter the learning phase:
 *   - Ask the user what would actually fix the problem.
 *   - Ask for a yes/no question that distinguishes their problem
 *     from the solution just shown.
 *   - Ask which answer applies to their problem.
 *   - Create a new question node and a new solution node, wire them
 *     correctly, graft them into the tree, record an Edit for
 *     undo/redo, and index the new question with canonicalize/h_put.
 *
 * Edge case: if parent is NULL the root itself must be replaced.
 * ---------------------------------------------------------------- */
void run_diagnosis(void) {
    clear();
    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(0, 0, "%-80s", " Tech Support Diagnosis");
    attroff(COLOR_PAIR(5) | A_BOLD);

    mvprintw(2, 2, "I'll help diagnose your tech problem.");
    mvprintw(3, 2, "Answer each question with y or n.");
    mvprintw(4, 2, "Press any key to start...");
    refresh();
    getch();

    FrameStack stack;
    fs_init(&stack);

    /* TODO: implement */
    Node* parent = NULL;
    int parentAnswer = -1;
    //tracks previous node and yes or no to current node
    fs_push(&stack, g_root, -1);
    while(!fs_empty(&stack)){
        Frame f = fs_pop(&stack);
        Node* cur = f.node;
        clear();

        if(cur->isQuestion){
            mvprintw(2, 2, "%s (y/n): ", cur->text);
            refresh();
            int ch = getch();
            int answeredYes;
            if(ch == 'y' || ch == 'Y'){
                answeredYes = 1;
            } else {
                answeredYes = 0;
            }
            parent = cur;
            parentAnswer = answeredYes;
            if(answeredYes){
                fs_push(&stack, cur->yes, 1);
            } else {
                fs_push(&stack, cur->no, 0);
            }
        } 
        else {
            mvprintw(2, 2, "Suggested fix: %s", cur->text);
            mvprintw(4, 2, "Did this solve your problem? (y/n): ");
            refresh();
            int ch = getch();
            if(ch == 'y' || ch == 'Y'){
                mvprintw(6, 2, "Great! Glad I could help.");
                refresh();
                getch();
                break;
            } else {
                char newSolText[256];
                char newQText[256];

                char *tmp = get_input(6, 2, "What would fix this problem? ");
                strncpy(newSolText, tmp, 255);
                newSolText[255] = '\0';

                tmp = get_input(8, 2, "What yes/no question distinguishes your problem? ");
                strncpy(newQText, tmp, 255);
                newQText[255] = '\0';

                mvprintw(10, 2, "For your problem, is the answer (y/n)? ");
                refresh();
                int ans = getch();
                int userAnsweredYes;
                if(ans == 'y' || ans == 'Y'){
                    userAnsweredYes = 1;
                } else {
                    userAnsweredYes = 0;
                }

                //new nodes
                Node *newLeaf = create_solution_node(newSolText);
                Node *newQ = create_question_node(newQText);

                if(userAnsweredYes){
                    newQ->yes = newLeaf;
                    newQ->no  = cur;
                } else {
                    newQ->no  = newLeaf;
                    newQ->yes = cur;
                }

                
                if(parent == NULL){
                    g_root = newQ;
                } else if(parentAnswer == 1){
                    parent->yes = newQ;
                } else {
                    parent->no = newQ;
                }

                
                Edit e;
                e.type = EDIT_INSERT_SPLIT;
                e.parent = parent;
                if(parent == NULL){
                    e.wasYesChild = -1;
                } else {
                    e.wasYesChild = parentAnswer;
                }
                e.oldLeaf = cur;
                e.newQuestion = newQ;
                e.newLeaf = newLeaf;
                es_push(&g_undo, e);
                es_clear(&g_redo);

               
                char *canon = canonicalize(newSolText);
                h_put(&g_index, canon, count_nodes(g_root) - 1);
                free(canon);

                mvprintw(12, 2, "Thanks! I'll remember that. Press any key...");
                refresh();
                getch();
                break;
            }
        }
    }

    fs_free(&stack);
}

/* ----------------------------------------------------------------
 * TODO 32  undo_last_edit
 * Return 1 on success, 0 if the undo stack is empty.
 * ---------------------------------------------------------------- */
int undo_last_edit(void) {
    if(es_empty(&g_undo)) return 0;
    Edit e = es_pop(&g_undo);
    if(e.wasYesChild == -1){
        g_root = e.oldLeaf;
    } else if(e.wasYesChild == 1){
        e.parent->yes = e.oldLeaf;
    } else {
        e.parent->no = e.oldLeaf;
    }
    es_push(&g_redo, e);
    return 1;
}

/* ----------------------------------------------------------------
 * TODO 33  redo_last_edit
 * Return 1 on success, 0 if the redo stack is empty.
 * ---------------------------------------------------------------- */
int redo_last_edit(void) {
   if(es_empty(&g_redo)) return 0;
    Edit e = es_pop(&g_redo);
    if(e.wasYesChild == -1){
        g_root = e.newQuestion;
    } else if(e.wasYesChild == 1){
        e.parent->yes = e.newQuestion;
    } else {
        e.parent->no = e.newQuestion;
    }
    es_push(&g_undo, e);
    return 1;
}
