#pragma once
#ifndef CMD_TREE_H
#define CMD_TREE_H
#include <stdint.h>

/**
 * Function pointer which implements a node's command.
 */
typedef int (*cmd_tree_node_exec)(void *ctx, uint8_t argc, char **argv);

/**
 * A node in the cmd_tree.
 */
typedef struct cmd_tree_node {
    // Pointer to the next sibling node
    struct cmd_tree_node *sibling;
    // Pointer to the first child node
    struct cmd_tree_node *child;
    // Function pointer to the node's command
    cmd_tree_node_exec exec;
    // Array of arguments for the command
    char **argv;
    // Number of arguments for the command
    uint8_t argc;
    // Name of the command
    char name[256];
} cmd_tree_node_t;

int cmd_tree_node_add_child(cmd_tree_node_t *n, cmd_tree_node_t *child);

int cmd_tree_search(cmd_tree_node_t *root, char *cmd,
                    cmd_tree_node_t **cmd_node);

#endif  // CMD_TREE_H