#define CMD_IMPLEMENTATION
#include "cmd.h"

#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "ansi.h"
#include "test_suite.h"

void test_append(void) {
    cmd_builder cmd = {0};
    ASSERT_TRUE(cmd_append(&cmd, "echo"), "failed to append");
    ASSERT_TRUE(cmd_append(&cmd, "hello world"), "failed to append");
    cmd_builder_destroy(&cmd);
}

// test spawn real command
void test_spawn_real(void) {
    cmd_builder cmd = {0};
    ASSERT_TRUE(cmd_append(&cmd, "true"), "failed to append");
    cmd_proc proc = {0};
    ASSERT_TRUE(cmd_spawn(cmd, &proc), "spawn");
    cmd_destroy(&proc);
    cmd_builder_destroy(&cmd);
}

// test for running non existing command
void test_spawn_fail(void) {
    cmd_builder cmd = {0};
    ASSERT_TRUE(cmd_append(&cmd, "command that is not existing"), "failed to append");
    cmd_proc proc = {0};
    ASSERT_FALSE(cmd_spawn(cmd, &proc), "spawn");
    ASSERT_TRUE(errno == ENOENT, "errno should be ENOENT");
    cmd_destroy(&proc);
    cmd_builder_destroy(&cmd);
}

// test for running empty command
void test_spawn_empty(void) {
    cmd_builder cmd = {0};
    cmd_proc proc   = {0};
    ASSERT_FALSE(cmd_spawn(cmd, &proc), "spawn of empty cmd_builder");
    cmd_builder_destroy(&cmd);
}

// test for destroy empty proc
void test_destroy_empty_proc(void) {
    cmd_proc proc = {0};
    cmd_destroy(&proc);
    ASSERT_TRUE(proc.p_stdout[0] == 0, "destroy closes pipes");
    ASSERT_TRUE(proc.p_stderr[0] == 0, "destroy closes pipes");
    ASSERT_TRUE(proc.p_stdin[1] == 0, "destroy closes pipes");
}

// test read output stdout
void test_read_output(void) {
    cmd_builder cmd = {0};
    ASSERT_TRUE(cmd_append(&cmd, "echo"), "failed to append");
    ASSERT_TRUE(cmd_append(&cmd, "hello world"), "failed to append");
    cmd_proc proc = {0};
    ASSERT_TRUE(cmd_spawn(cmd, &proc), "spawn");

    char buffer[128] = {0};
    cmd_read(proc, buffer, sizeof(buffer) - 1, 1000);

    ASSERT_TRUE(strcmp(buffer, "hello world\n") == 0, "str matches");
    cmd_destroy(&proc);
    cmd_builder_destroy(&cmd);
}

// test read output stderr
void test_read_err(void) {
    cmd_builder cmd = {0};
    ASSERT_TRUE(cmd_append(&cmd, "sh"), "failed to append");
    ASSERT_TRUE(cmd_append(&cmd, "-c"), "failed to append");
    ASSERT_TRUE(cmd_append(&cmd, "echo hello world 1>&2"), "failed to append");
    cmd_proc proc = {0};
    ASSERT_TRUE(cmd_spawn(cmd, &proc), "spawn");

    char buffer[128] = {0};
    cmd_read_err(proc, buffer, sizeof(buffer) - 1, 1000);

    ASSERT_TRUE(strcmp(buffer, "hello world\n") == 0, "str matches");
    cmd_destroy(&proc);
    cmd_builder_destroy(&cmd);
}

// test write input stdin
void test_write_input(void) {
    cmd_builder cmd = {0};
    ASSERT_TRUE(cmd_append(&cmd, "cat"), "failed to append");
    cmd_proc proc = {0};
    ASSERT_TRUE(cmd_spawn(cmd, &proc), "spawn");

    const char* msg = "hello from parent\n";
    ssize_t n       = cmd_write(proc, (char*)msg, strlen(msg));
    ASSERT_TRUE(n == (ssize_t)strlen(msg), "write should write all bytes");

    char buffer[128] = {0};
    n                = cmd_read(proc, buffer, sizeof(buffer) - 1, 1000);
    ASSERT_TRUE(n == (ssize_t)strlen(msg), "read should read all bytes");
    buffer[n] = '\0';
    ASSERT_TRUE(strcmp(buffer, msg) == 0, "str matches");

    cmd_destroy(&proc);
    cmd_builder_destroy(&cmd);
}

// test status returned wait for end of proc
void test_wait_long_commands(void) {
    cmd_builder cmd = {0};
    ASSERT_TRUE(cmd_append(&cmd, "sleep"), "failed to append");
    ASSERT_TRUE(cmd_append(&cmd, "0.5"), "failed to append");
    cmd_proc proc = {0};
    ASSERT_TRUE(cmd_spawn(cmd, &proc), "spawn");
    // wait for it to finish
    int status;
    ASSERT_TRUE(cmd_wait(&proc, &status, 0), "exit code");
    ASSERT_TRUE(status == 0, "exit code should be 0");
    cmd_builder_destroy(&cmd);
}

// test wait is shorter then the command
void test_wait_partial_long_commands(void) {
    cmd_builder cmd = {0};
    ASSERT_TRUE(cmd_append(&cmd, "sleep"), "failed to append");
    ASSERT_TRUE(cmd_append(&cmd, "0.5"), "failed to append");
    cmd_proc proc = {0};
    ASSERT_TRUE(cmd_spawn(cmd, &proc), "spawn");
    int status = 69;
    ASSERT_TRUE(status == 69, "exit code should be unchanged");
    ASSERT_FALSE(cmd_wait(&proc, &status, 50), "exit code");
    cmd_builder_destroy(&cmd);
}

// test that the return value is correct
void test_return_status(void) {
    cmd_builder cmd_t = {0};
    ASSERT_TRUE(cmd_append(&cmd_t, "true"), "failed to append");
    cmd_proc proc_t = {0};
    ASSERT_TRUE(cmd_spawn(cmd_t, &proc_t), "spawn true");
    cmd_builder_destroy(&cmd_t);
    int status = -1;
    ASSERT_TRUE(cmd_wait(&proc_t, &status, 5), "exit code");
    ASSERT_TRUE(status == 0, "output status of true will be 0");
    cmd_destroy(&proc_t);

    cmd_builder cmd_f = {0};
    ASSERT_TRUE(cmd_append(&cmd_f, "false"), "failed to append");
    cmd_proc proc_f = {0};
    ASSERT_TRUE(cmd_spawn(cmd_f, &proc_f), "spawn true");
    cmd_builder_destroy(&cmd_f);
    status = -1;
    ASSERT_TRUE(cmd_wait(&proc_f, &status, 5), "exit code");
    ASSERT_TRUE(status == 1, "output status of true will be 0");
    cmd_destroy(&proc_f);
    cmd_destroy(&proc_f);
}

// test running couple of command together at the same time
void test_spawn_procs_together(void) {
    const int count = 10;
    cmd_proc procs[count];
    memset(procs, 0, count * sizeof(*procs));
    for (int i = 0; i < count; ++i) {
        cmd_builder cmd = {0};
        ASSERT_TRUE(cmd_append(&cmd, "sleep"), "failed to append");
        ASSERT_TRUE(cmd_append(&cmd, "0.2"), "failed to append");
        ASSERT_TRUE(cmd_spawn(cmd, &procs[i]), "spawn true");
        cmd_builder_destroy(&cmd);
    }
    for (int i = 0; i < count; ++i) {
        cmd_destroy(&procs[i]);
    }
}

// test the io of running together cmds
void test_spawn_procs_together_io(void) {
    const int count = 10;
    cmd_proc procs[count];
    memset(procs, 0, count * sizeof(*procs));
    for (int i = 0; i < count; ++i) {
        cmd_builder cmd = {0};
        ASSERT_TRUE(cmd_append(&cmd, "cat"), "failed to append");
        ASSERT_TRUE(cmd_spawn(cmd, &procs[i]), "spawn true");
        cmd_builder_destroy(&cmd);
    }
    for (int i = 0; i < count; ++i) {
        char arg[6];
        int len = sprintf(arg, "%d", i);
        cmd_write(procs[i], arg, len);
    }
    for (int i = 0; i < count; ++i) {
        char buffer[32];
        cmd_read(procs[i], buffer, 32, 20);
        char test_buffer[8];
        sprintf(test_buffer, "%d", i);
        ASSERT_TRUE(strcmp(buffer, test_buffer) == 0, "str io not match");
        cmd_destroy(&procs[i]);
    }
}

// TODO: Implement this:
// test_argument_spliting
// test_in_out_err_together

int main(void) {
    START_SUITE;
    RUN(test_append);
    RUN(test_spawn_real);
    RUN(test_spawn_fail);
    RUN(test_spawn_empty);
    RUN(test_destroy_empty_proc);
    RUN(test_read_output);
    RUN(test_read_err);
    RUN(test_write_input);
    RUN(test_wait_long_commands);
    RUN(test_wait_partial_long_commands);
    RUN(test_return_status);
    RUN(test_spawn_procs_together);
    RUN(test_spawn_procs_together_io);
    END_SUITE;
    return 0;
}