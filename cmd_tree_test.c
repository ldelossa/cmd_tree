#include "include/cmd_tree.h"

#include <stdio.h>
#include <string.h>

#define ROOT_NODE_EXEC_ENTERED 1001
#define ROOT_CMD_EXEC_ENTERED 1002
#define ROOT_CMD_2_NODE_EXEC_ENTERED 1003
#define ROOT_SUBCMD_NODE_EXEC_ENTERED 1004
#define ROOT_SUBCMD_2_NODE_EXEC_ENTERED 1005

#define RUN_TEST(test_func, test_name)                 \
    printf("\n=== Running Test: %s ===\n", test_name); \
    if ((exit_code = test_func()) != 1) {              \
        printf(test_name " Failed: %d", exit_code);    \
        return exit_code;                              \
    }

int root_node_exec(void *ctx, uint8_t argc, char **argv) {
    printf("unknown command \n");
    return ROOT_NODE_EXEC_ENTERED;
}

int test_sanity() {
    cmd_tree_node_t *cmd = 0;
    cmd_tree_node_t root = {.exec = root_node_exec};

    if (cmd_tree_search(&root, "no-op", &cmd) != 1) return -1;

    if (cmd->exec(0, 0, 0) != ROOT_NODE_EXEC_ENTERED) return -2;

    return 1;
}

int command_node_exec(void *ctx, uint8_t argc, char **argv) {
    printf("command entered \n");
    return ROOT_CMD_EXEC_ENTERED;
}

int test_single_command() {
    cmd_tree_node_t *cmd = 0;
    cmd_tree_node_t root = {.exec = root_node_exec};
    cmd_tree_node_t root_cmd = {.name = "command_1", .exec = command_node_exec};

    if (cmd_tree_node_add_child(&root, &root_cmd) != 1) return -1;

    if (cmd_tree_search(&root, "command_1", &cmd) != 1) return -2;

    if (cmd->exec(0, 0, 0) != ROOT_CMD_EXEC_ENTERED) return -3;

    return 1;
}

int command_2_node_exec(void *ctx, uint8_t argc, char **argv) {
    printf("command 2 entered \n");
    return ROOT_CMD_2_NODE_EXEC_ENTERED;
}

int test_two_commands() {
    cmd_tree_node_t *cmd = 0;
    cmd_tree_node_t root = {.exec = root_node_exec};
    cmd_tree_node_t root_cmd = {.name = "command_1", .exec = command_node_exec};
    cmd_tree_node_t root_cmd_2 = {.name = "command_2",
                                  .exec = command_2_node_exec};

    if (cmd_tree_node_add_child(&root, &root_cmd) != 1) return -1;

    if (cmd_tree_node_add_child(&root, &root_cmd_2) != 1) return -1;

    if (cmd_tree_search(&root, "command_1", &cmd) != 1) return -2;

    if (cmd->exec(0, 0, 0) != ROOT_CMD_EXEC_ENTERED) return -3;

    if (cmd_tree_search(&root, "command_2", &cmd) != 1) return -4;

    if (cmd->exec(0, 0, 0) != ROOT_CMD_2_NODE_EXEC_ENTERED) return -5;

    return 1;
}

int command_subcmd_exec(void *ctx, uint8_t argc, char **argv) {
    printf("command subcmd entered \n");

    // expect two extra arguments
    char *arg;

    if (strcmp(argv[0], "arg_1") != 0) return -1;
    printf("%s\n", argv[0]);

    if (strcmp(argv[1], "arg_2") != 0) return -2;
    printf("%s\n", argv[1]);

    if (argc != 2) {
        return -3;
    }

    return ROOT_SUBCMD_NODE_EXEC_ENTERED;
}

int test_subcmd_commands() {
    cmd_tree_node_t *cmd = 0;
    cmd_tree_node_t root = {.exec = root_node_exec};
    cmd_tree_node_t root_cmd = {.name = "command_1", .exec = command_node_exec};
    cmd_tree_node_t root_cmd_subcmd = {.name = "subcommand",
                                       .exec = command_subcmd_exec};
    cmd_tree_node_t root_cmd_2 = {.name = "command_2",
                                  .exec = command_2_node_exec};

    if (cmd_tree_node_add_child(&root, &root_cmd) != 1) return -1;
    if (cmd_tree_node_add_child(&root_cmd, &root_cmd_subcmd) != 1) return -1;

    if (cmd_tree_node_add_child(&root, &root_cmd_2) != 1) return -1;

    if (cmd_tree_search(&root, "command_1 subcommand arg_1 arg_2", &cmd) != 1)
        return -4;

    int result = cmd->exec(0, cmd->argc, cmd->argv);
    if (result != ROOT_SUBCMD_NODE_EXEC_ENTERED) return result;

    return 1;
}

int command_subcmd_1_exec(void *ctx, uint8_t argc, char **argv) {
    printf("command subcmd 1 entered \n");
    return ROOT_SUBCMD_NODE_EXEC_ENTERED;
}

int command_sub_subcmd_exec(void *ctx, uint8_t argc, char **argv) {
    printf("command sub subcmd entered \n");

    if (strcmp(argv[0], "arg_1") != 0) return -1;
    printf("%s\n", argv[0]);

    if (strcmp(argv[1], "arg_2") != 0) return -2;
    printf("%s\n", argv[1]);

    if (strcmp(argv[2], "arg_3") != 0) return -3;
    printf("%s\n", argv[2]);

    if (argc != 3) {
        return -4;
    }

    return ROOT_SUBCMD_2_NODE_EXEC_ENTERED;
}

int test_sub_subcmd_commands() {
    cmd_tree_node_t *cmd = 0;
    cmd_tree_node_t root = {.exec = root_node_exec};
    cmd_tree_node_t root_cmd = {.name = "command_1", .exec = command_node_exec};
    cmd_tree_node_t root_cmd_subcmd = {.name = "subcommand",
                                       .exec = command_subcmd_1_exec};
    cmd_tree_node_t root_cmd_sub_subcmd = {.name = "subcommand_2",
                                           .exec = command_sub_subcmd_exec};

    if (cmd_tree_node_add_child(&root, &root_cmd) != 1) return -1;
    if (cmd_tree_node_add_child(&root_cmd, &root_cmd_subcmd) != 1) return -1;

    if (cmd_tree_node_add_child(&root_cmd_subcmd, &root_cmd_sub_subcmd) != 1)
        return -1;

    if (cmd_tree_search(&root, "command_1 subcommand subcommand_2 arg_1 arg_2 arg_3", &cmd) != 1)
        return -4;

    int result = cmd->exec(0, cmd->argc, cmd->argv);
    if (result != ROOT_SUBCMD_2_NODE_EXEC_ENTERED) return result;

    return 1;
}

int main(int argc, char **argv) {
    int exit_code = 1;
    RUN_TEST(test_sanity, "Sanity Test");
    RUN_TEST(test_single_command, "Single Command Test");
    RUN_TEST(test_two_commands, "Two Command Test");
    RUN_TEST(test_subcmd_commands, "Root Subcommand Test");
    RUN_TEST(test_sub_subcmd_commands, "Root Sub Subcommand Test");
}