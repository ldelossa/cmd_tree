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

/**
 * Adds a child node to a given node in the cmd_tree.
 *
 * @param n The node to add a child to.
 * @param child The child node to add.
 *
 * @return 1 on success, -1 on failure.
 */
int cmd_tree_node_add_child(cmd_tree_node_t *n, cmd_tree_node_t *child);

/**
 * Searches for a command node in the cmd_tree starting from the given root
 * node.
 *
 * @param root The root node of the cmd_tree to search from.
 * @param cmd The name of the command to search for.
 * @param cmd_node A pointer to a cmd_tree_node_t pointer that will be set to
 * the found command node.
 *
 * @return 1 if the command node was found and -1 if an error occurred.
 */
int cmd_tree_search(cmd_tree_node_t *root, char *cmd,
                    cmd_tree_node_t **cmd_node);

/**
 * Frees any memory allocated for the node returned from "cmd_tree_search."
 *
 * The argument n itself is not freed as the caller is responsible for creating
 * and freeing all of the command tree's nodes.
 *
 * Rather, this function frees any memory allocated during the last search and
 * parsing of the string command.
 *
 * It should be used once the caller has called the node's exec function pointer
 * and will no longer utilize the node.
 *
 * @param n The node to free.
 */
void cmd_tree_node_free(cmd_tree_node_t *n);

#endif  // CMD_TREE_H