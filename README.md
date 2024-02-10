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
 * @param argc The length of argv.
 * @param argv An array of string pointers of size @argc
 * @param cmd_node A pointer to a cmd_tree_node_t pointer that will be set to
 * the found command node.
 *
 * @return 1 if the command node was found and -1 if an error occurred.
 */
int cmd_tree_search(cmd_tree_node_t *root, int argc, char *argv[],
                    cmd_tree_node_t **cmd_node);
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
    int argc = 2;
    char *argv[] = {"command_1", "arg_1"};
    cmd_tree_node_t *cmd = 0;
    cmd_tree_node_t root = {.exec = test_root_node_exec};
    cmd_tree_node_t root_cmd = {.name = "command_1",
                                .exec = test_command_one_exec};

    if (cmd_tree_node_add_child(&root, &root_cmd) != 1) return -1;

    if (cmd_tree_search(&root, argc, argv, &cmd) != 1) return -2;

    if (cmd->exec(0, cmd->argc, cmd->argv) != COMMAND_ONE_NODE_EXECED)
        return -3;

    return 1;
}
```

In the above example we first define callback "exec" functions for the root node
and the "command_1" node. 

We define our "argc" and "argv" variables which hold the number of arguments
and argument strings respectively. 

Next, we begin to define our command tree.
The tree always starts with a single root and children are added to it.
We add the "command_1" node to the tree under our root.

A search of the tree is then performed with the arguments "command_1 arg_1". 
The appropriate node to handle this command string is then returned in the
"cmd" variable.

When the appropriate node is returned its "argc" and "argv" variables will hold
the number of trailing arguments provided to the command. 
In our example this will be 1, the "arg_1" argument. 
The node's "exec" function can then be invoked with these variables. 
The node's "exec" function is responsible for interpreting any trailing arguments
along with returning an integer value who's meaning is specific to the application.

This is the general pattern of the library.
Check out `cmd_tree_test.c` for more complex examples.

## Memory Management

Memory is not allocated by any functions exposed in the header.

The application utilizing "cmd_tree" is expected to allocate the `argv` array
corresponding to the desired command and free it once the command tree is no
longer in use.

`cmd_tree_node_t` structures can be defined globally and linked into a tree 
during runtime. 
When `cmd_tree_search` is ran it will update only `argc` and `argv` values
of `cmd_tree_node_t`. 
The application should not depend on any `argc` and `argv` values of a node 
which was not returned from a recent call to `cmd_tree_search` as the values
maybe stale or used only during the search process.

## Compilation

The library is extremely lean and only depends on 
