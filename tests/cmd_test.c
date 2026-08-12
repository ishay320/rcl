#define CMD_IMPLEMENTATION
#include "cmd.h"

#include <assert.h>
#include <limits.h>

#include "ansi.h"
#include "test_suite.h"

void test_append(void) {
    cmd_builder cmd = {0};
    ASSERT_TRUE(cmd_append(&cmd, "echo"), "failed to append");
    ASSERT_TRUE(cmd_append(&cmd, "hello world"), "failed to append");
    cmd_builder_destroy(&cmd);
}

// test normal command
void test_spawn_real(void) {
    cmd_builder cmd = {0};
    ASSERT_TRUE(cmd_append(&cmd, "true"), "failed to append");
    cmd_proc proc = {0};
    ASSERT_TRUE(cmd_spawn(cmd, &proc), "spawn");
    cmd_destroy(&proc);
    cmd_builder_destroy(&cmd);
}

// test for running non existing command
void test_spawn_false(void) {
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
    ASSERT_FALSE(cmd_spawn(cmd, &proc), "spawn");
    cmd_builder_destroy(&cmd);
}

// test for destroy empty proc
void test_destroy_empty_proc(void) {
    cmd_proc proc = {0};
    cmd_destroy(&proc);
    ASSERT_TRUE(proc.p_stdout[0] == 0, "destroy closes pipes");
    ASSERT_TRUE(proc.p_stderr[0] == 0, "destroy closes pipes");
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

// test arguments passing
void test_arguments(void) {}

// test output:
// test status returned 0 / 1
// wait for end of proc
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

void test_wait_partial_long_commands(void) {
    cmd_builder cmd = {0};
    ASSERT_TRUE(cmd_append(&cmd, "sleep"), "failed to append");
    ASSERT_TRUE(cmd_append(&cmd, "0.5"), "failed to append");
    cmd_proc proc = {0};
    ASSERT_TRUE(cmd_spawn(cmd, &proc), "spawn");
    int status = 69;
    ASSERT_FALSE(cmd_wait(&proc, &status, 50), "exit code");
    ASSERT_TRUE(status == 69, "exit code should be unchanged");
    cmd_builder_destroy(&cmd);
}

// TODO: Implement this:
// test_return
// test_in_out

int main(void) {
    START_SUITE;
    RUN(test_append);
    RUN(test_spawn_real);
    RUN(test_spawn_false);
    RUN(test_spawn_empty);
    RUN(test_destroy_empty_proc);
    RUN(test_read_output);
    RUN(test_read_err);
    RUN(test_write_input);
    // RUN(test_arguments);
    RUN(test_wait_long_commands);
    RUN(test_wait_partial_long_commands);
    END_SUITE;
    return 0;
}