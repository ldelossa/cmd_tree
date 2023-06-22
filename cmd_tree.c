#include "include/cmd_tree.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int cmd_tree_node_add_sibling(cmd_tree_node_t *n, cmd_tree_node_t *sibling) {
    if (n == NULL || sibling == NULL) {
        return -1;
    }
    if (!n->sibling) {
        n->sibling = sibling;
        return 1;
    }
    n = n->sibling;
    for (;;) {
        if (n->sibling == NULL) break;
        n = n->sibling;
    }
    n->sibling = sibling;
    return 1;
}

int cmd_tree_node_add_child(cmd_tree_node_t *n, cmd_tree_node_t *child) {
    if (n == NULL || child == NULL) {
        return -1;
    }
    if (!n->child) {
        n->child = child;
        return 1;
    }
    return cmd_tree_node_add_sibling(n->child, child);
}

int cmd_tree_search_assign_args(cmd_tree_node_t *root, char *token,
                                char **cmd) {
    // use a scratch buffer for holding malloc'd string buffers,
    // once number of arguments is known, malloc the correct size
    // buffer onto root->args and copy the malloc'd strings to it.
    char *buffer[256] = {0};
    char *next_token = token;
    uint8_t argc = 0;
    uint8_t i = 0;

    do {
        buffer[i] = malloc(strlen(token) + 1);
        if (strcpy(buffer[i], next_token) == NULL) {
            goto error;
        }
        argc++;
        i++;
    } while ((next_token = strsep(cmd, " ")));

    root->argv = malloc(sizeof(char *) * argc);
    if (root->argv == NULL) {
        goto error;
    }
    memcpy(root->argv, buffer, sizeof(char *) * argc);
    root->argc = argc;
    return 1;

error:
    for (i = 0; i < argc; i++) {
        free(buffer[i]);
    }
    return -1;
}

cmd_tree_node_t *cmd_tree_search_recur(cmd_tree_node_t *root, char *token,
                                       char **cmd) {
    // search children for token
    cmd_tree_node_t *child = root->child;

    while (child && token) {
        if (strcmp((char *)child->name, token) == 0) {
            return cmd_tree_search_recur(child, strsep(cmd, " "), cmd);
        }
        child = child->sibling;
    }

    // no childen with the token name were found for this node.
    // if the token is not nil, treat it, and another further, as arguments
    // to this command, assigning them to root->argv.
    if (token)
        if (cmd_tree_search_assign_args(root, token, cmd) != 1) return NULL;
    return root;
}

int cmd_tree_search(cmd_tree_node_t *root, char *cmd,
                    cmd_tree_node_t **cmd_node) {
    if (!root || !cmd) {
        return -1;
    }

    char *dup = strdup(cmd);
    if (dup == NULL) {
        return -1;
    }

    char *token = strsep(&dup, " ");
    if (!token) {
        return -1;
    }

    *cmd_node = cmd_tree_search_recur(root, token, &dup);

    return 1;
}

void cmd_tree_node_free(cmd_tree_node_t *n) {
    if (n->argc > 0) {
        for (uint8_t i = 0; i < n->argc; i++) {
            free(n->argv[i]);
        }
    }
    free(n->argv);
    n->argv = NULL;
}
