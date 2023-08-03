# CMD_TREE

A ridiculously simple hierarchical command parser written in C.

`cmd_tree` is geared for CLI applications and CLI shells which have a need for
arbitrary depth commands and sub-commands.

`cmd_tree`, like the name suggests, creates a "first child-next sibling" tree
of `cmd_tree_node_t` structures.

Therefore the tree of commands in memory looks as follows:

```
                 1 (root)
                 /
                /
               /
              2---3---4
             /       /
            5---6   7
                   /
                  8---9
```

## Usage

`cmd_tree` allows you to define `cmd_tree_node_t` structures, each of which can
contain an `cmd_tree_node_exec` virtual function with the following signature.

```c
/**
 * Function pointer which implements a node's command.
 */
typedef int (*cmd_tree_node_exec)(void *ctx, uint8_t argc, char **argv);
```

The input to the `exec` functions are:

`ctx`: An application specific context. The user of `cmd_tree` is free to pass
in a pointer to any memory which the `exec` function can cast it to.

`argc`: The number of string arguments following the (sub)command that was
invoked

`argv`: A string pointer array of length=`argc` where arguments to the invoked
command can be read.

Every command is searched for by the `name` field in `cmd_tree_node_t`.

A set of functions are exposed in `cmd_tree.h` for working with the tree.

```c
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
```

Let's take a look at our simplest meaningful test as an example of building a
command tree.

```c
int test_root_node_exec(void *ctx, uint8_t argc, char **argv) {
    printf("unknown command %d \n", argc);
    return ROOT_NODE_EXECED;
}

int test_command_one_exec(void *ctx, uint8_t argc, char **argv) {
    printf("command 1 entered \n");

    if (strcmp(argv[0], "arg_1") != 0) return -1;
    printf("%s\n", argv[0]);

    return COMMAND_ONE_NODE_EXECED;
}

int test_single_command() {
    cmd_tree_node_t *cmd = 0;
    cmd_tree_node_t root = {.exec = test_root_node_exec};
    cmd_tree_node_t root_cmd = {.name = "command_1",
                                .exec = test_command_one_exec};

    if (cmd_tree_node_add_child(&root, &root_cmd) != 1) return -1;

    if (cmd_tree_search(&root, "command_1 arg_1", &cmd) != 1) return -2;

    if (cmd->exec(0, cmd->argc, cmd->argv) != COMMAND_ONE_NODE_EXECED)
        return -3;
    cmd_tree_node_free(cmd);

    return 1;
}
```

A few points become apparent with the example above.

A `cmd_tree` must have a single root which typically has no name and an `exec`
function which handles the case where no valid command was supplied.
This root node is called when no other valid command can be found in the `cmd_tree`.

A single command `root_cmd` which cooresponds to the string `command_1` is declared
and the `test_command_one_exec` function provides the `exec` function for the
command node.

`cmd_tree_node_add_child` is then used to add a child to the `root` node.

Next, the `cmd_tree_search` takes the root of the tree, a command string, and a
`cmd_tree_node_t *` and searches the tree for a command with the name `command_1`.

When `cmd_tree_search` completes the `cmd` pointer now points to the `root_cmd`
node and any arguments provided after `command_1` in the string is available in
`cmd->argv` along with the number of args in `cmd->argc`.

In our case one string, `arg_1`, exists after the command's name in the search
string, `command_1 arg_1`.
Because there are no sub-commands in our tree named `arg_1`, this string will be
assigned to `arg->v` as an argument to `command_1`'s `exec` function.

In constrast if `command_1` had a child `cmd_tree_node_t` who's name was `arg_1`
the `cmd_tree_search` function would return the command node for `arg_1`.

You can now call the command's `exec` function with the parsed arguments and the
argument count!

Finally, remember to call `cmd_tree_node_free` when the execution is finished.
This does **not** free the `root_cmd` since the calling code can define this
statically, however it does clean up some heap allocated strings used during
argument assignment to cmd->argc, cmd->argv. After free you should not call
`cmd->exec` until another `cmd_tree_search` is performed.

This is the general pattern of the library.
Check out `cmd_tree_test.c` for more complex samples.
