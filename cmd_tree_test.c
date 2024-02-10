#include "include/cmd_tree.h"

#include <stdio.h>
#include <string.h>

enum TEST_CMD_ENTER {
    ROOT_NODE_EXECED,
    COMMAND_ONE_NODE_EXECED,
    COMMAND_TWO_NODE_EXECED,
    SUBCOMMAND_NODE_EXECED,
    SUB_SUBCOMMAND_NODE_EXECED
};

#define RUN_TEST(test_func, test_name)                 \
    printf("\n=== Running Test: %s ===\n", test_name); \
    if ((exit_code = test_func()) != 1) {              \
        printf(test_name " Failed: %d", exit_code);    \
        return exit_code;                              \
    }

int test_root_node_exec(void *ctx, uint8_t argc, char **argv) {
    printf("unknown command %d \n", argc);
    return ROOT_NODE_EXECED;
}

int test_sanity() {
    int argc = 0;
    char *argv[] = {};
    cmd_tree_node_t *cmd = 0;
    cmd_tree_node_t root = {.exec = test_root_node_exec};

    if (cmd_tree_search(&root, argc, argv, &cmd) != 1) return -1;

    if (cmd->exec(0, cmd->argc, cmd->argv) != ROOT_NODE_EXECED) return -2;

    return 1;
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

int test_command_two_exec(void *ctx, uint8_t argc, char **argv) {
    printf("command 2 entered \n");
    return COMMAND_TWO_NODE_EXECED;
}

int test_two_commands() {
    int argc = 2;
    char *argv[] = {"command_1", "arg_1"};
    int argc2 = 1;
    char *argv2[] = {"command_2"};
    cmd_tree_node_t *cmd = 0;
    cmd_tree_node_t root = {.exec = test_root_node_exec};
    cmd_tree_node_t root_cmd = {.name = "command_1",
                                .exec = test_command_one_exec};
    cmd_tree_node_t root_cmd_2 = {.name = "command_2",
                                  .exec = test_command_two_exec};

    if (cmd_tree_node_add_child(&root, &root_cmd) != 1) return -1;

    if (cmd_tree_node_add_child(&root, &root_cmd_2) != 1) return -1;

    if (cmd_tree_search(&root, argc, argv, &cmd) != 1) return -2;

    if (cmd->exec(0, cmd->argc, cmd->argv) != COMMAND_ONE_NODE_EXECED)
        return -3;

    if (cmd_tree_search(&root, argc2, argv2, &cmd) != 1) return -4;

    if (cmd->exec(0, 0, 0) != COMMAND_TWO_NODE_EXECED) return -5;

    return 1;
}

int test_subcommand_exec(void *ctx, uint8_t argc, char **argv) {
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

    return SUBCOMMAND_NODE_EXECED;
}

int test_subcmd_commands() {
    int argc = 4;
    char *argv[] = {"command_1", "subcommand", "arg_1", "arg_2"};
    cmd_tree_node_t *cmd = 0;
    cmd_tree_node_t root = {.exec = test_root_node_exec};
    cmd_tree_node_t root_cmd = {.name = "command_1",
                                .exec = test_command_one_exec};
    cmd_tree_node_t root_cmd_subcmd = {.name = "subcommand",
                                       .exec = test_subcommand_exec};
    // here to just ensure adding another node doesn't break anything.
    int argc2 = 1;
    char *argv2[] = {"command_2"};
    cmd_tree_node_t root_cmd_2 = {.name = "command_2",
                                  .exec = test_command_two_exec};

    if (cmd_tree_node_add_child(&root, &root_cmd) != 1) return -1;
    if (cmd_tree_node_add_child(&root_cmd, &root_cmd_subcmd) != 1) return -1;

    if (cmd_tree_node_add_child(&root, &root_cmd_2) != 1) return -1;

    if (cmd_tree_search(&root, argc, argv, &cmd) != 1) return -4;

    int result = cmd->exec(0, cmd->argc, cmd->argv);
    if (result != SUBCOMMAND_NODE_EXECED) return result;

    return 1;
}

int test_sub_subcommand_exec(void *ctx, uint8_t argc, char **argv) {
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

    return SUB_SUBCOMMAND_NODE_EXECED;
}

int test_sub_subcmd_commands() {
    int argc = 6;
    char *argv[] = {"command_1", "subcommand", "subcommand_2",
                    "arg_1",     "arg_2",      "arg_3"};
    cmd_tree_node_t *cmd = 0;
    cmd_tree_node_t root = {.exec = test_root_node_exec};
    cmd_tree_node_t root_cmd = {.name = "command_1",
                                .exec = test_command_one_exec};
    cmd_tree_node_t root_cmd_subcmd = {.name = "subcommand",
                                       .exec = test_subcommand_exec};
    int argc2 = 1;
    char *argv2[] = {"command_2"};
    cmd_tree_node_t root_cmd_sub_subcmd = {.name = "subcommand_2",
                                           .exec = test_sub_subcommand_exec};

    if (cmd_tree_node_add_child(&root, &root_cmd) != 1) return -1;
    if (cmd_tree_node_add_child(&root_cmd, &root_cmd_subcmd) != 1) return -1;

    if (cmd_tree_node_add_child(&root_cmd_subcmd, &root_cmd_sub_subcmd) != 1)
        return -1;

    if (cmd_tree_search(&root, argc, argv, &cmd) != 1) return -4;

    int result = cmd->exec(0, cmd->argc, cmd->argv);
    if (result != SUB_SUBCOMMAND_NODE_EXECED) return result;

    return 1;
}

int main(int argc, char **argv) {
    int exit_code = 1;
    RUN_TEST(test_sanity, "Sanity Test");
    RUN_TEST(test_single_command, "Single Command Test");
    RUN_TEST(test_two_commands, "Two Command Test");
    RUN_TEST(test_subcmd_commands, "Subcommand Test");
    RUN_TEST(test_sub_subcmd_commands, "Sub Subcommand Test");
}
