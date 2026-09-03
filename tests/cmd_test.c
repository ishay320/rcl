#define CMD_IMPLEMENTATION
#include "cmd.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "test_suite.h"

TEST_CASE(test_append_builds_null_terminated_argv, {
    cmd_builder cmd = {0};
    ASSERT_TRUE(cmd_append(&cmd, "echo"), "failed to append");
    ASSERT_CMP(cmd.size, (size_t)2, "arg + NULL terminator");
    ASSERT_TRUE(cmd_append(&cmd, "hello"), "failed to append");
    ASSERT_CMP(cmd.size, (size_t)3, "terminator is reused, not duplicated");
    ASSERT_CMP(cmd.data[0], "echo");
    ASSERT_CMP(cmd.data[1], "hello");
    ASSERT_CMP(cmd.data[2], (char*)NULL, "argv stays NULL terminated");
    cmd_builder_destroy(&cmd);
})

TEST_CASE(test_spawn_rejects_empty_and_missing_command, {
    cmd_builder empty = {0};
    cmd_proc proc     = {0};
    ASSERT_FALSE(cmd_spawn(empty, &proc), "empty builder");
    cmd_builder_destroy(&empty);

    cmd_builder cmd = {0};
    ASSERT_TRUE(cmd_append(&cmd, "command-that-does-not-exist"), "failed to append");
    ASSERT_FALSE(cmd_spawn(cmd, &proc), "missing binary");
    ASSERT_CMP(errno, ENOENT);
    cmd_destroy(&proc);  // destroy after a failed spawn must be safe
    cmd_builder_destroy(&cmd);
})

// cmd_destroy on a zeroed proc used to close fds 0/0/0
TEST_CASE(test_destroy_of_empty_proc_is_a_noop, {
    cmd_proc proc = {0};
    cmd_destroy(&proc);
    ASSERT_CMP(proc.p_stdout[0], 0, "fd untouched");
    ASSERT_CMP(proc.p_stderr[0], 0, "fd untouched");
    ASSERT_CMP(proc.p_stdin[1], 0, "fd untouched");
})

TEST_CASE(test_echo_splits_appended_arguments, {
    cmd_builder cmd = {0};
    ASSERT_TRUE(cmd_append(&cmd, "echo"), "failed to append");
    ASSERT_TRUE(cmd_append(&cmd, "hello"), "failed to append");
    ASSERT_TRUE(cmd_append(&cmd, "world"), "failed to append");
    cmd_proc proc = {0};
    ASSERT_TRUE(cmd_spawn(cmd, &proc), "spawn");
    cmd_builder_destroy(&cmd);

    char buffer[128] = {0};
    ssize_t n        = cmd_read(proc, buffer, sizeof(buffer) - 1, 1000);
    ASSERT_CMP(n, (ssize_t)strlen("hello world\n"));
    ASSERT_CMP(buffer, "hello world\n", "each append is its own argv entry");

    cmd_destroy(&proc);
})

TEST_CASE(test_stdout_and_stderr_are_separate_streams, {
    cmd_builder cmd = {0};
    ASSERT_TRUE(cmd_append(&cmd, "sh"), "failed to append");
    ASSERT_TRUE(cmd_append(&cmd, "-c"), "failed to append");
    ASSERT_TRUE(cmd_append(&cmd, "echo out; echo err 1>&2"), "failed to append");
    cmd_proc proc = {0};
    ASSERT_TRUE(cmd_spawn(cmd, &proc), "spawn");
    cmd_builder_destroy(&cmd);

    char buffer[128] = {0};
    ASSERT_TRUE(cmd_read(proc, buffer, sizeof(buffer) - 1, 1000) > 0);
    ASSERT_CMP(buffer, "out\n");

    memset(buffer, 0, sizeof(buffer));
    ASSERT_TRUE(cmd_read_err(proc, buffer, sizeof(buffer) - 1, 1000) > 0);
    ASSERT_CMP(buffer, "err\n");

    cmd_destroy(&proc);
})

TEST_CASE(test_stdin_roundtrip_and_eof_after_close, {
    cmd_builder cmd = {0};
    ASSERT_TRUE(cmd_append(&cmd, "cat"), "failed to append");
    cmd_proc proc = {0};
    ASSERT_TRUE(cmd_spawn(cmd, &proc), "spawn");
    cmd_builder_destroy(&cmd);

    const char* msg = "hello from parent\n";
    ASSERT_CMP(cmd_write(proc, msg, strlen(msg)), (ssize_t)strlen(msg), "write to proc failed");

    char buffer[128] = {0};
    ssize_t n        = cmd_read(proc, buffer, sizeof(buffer) - 1, 1000);
    ASSERT_CMP(n, (ssize_t)strlen(msg), "short read");
    ASSERT_CMP(buffer, msg);

    close(proc.p_stdin[1]);  // EOF on stdin makes cat exit
    proc.p_stdin[1] = -1;
    ASSERT_CMP(cmd_read(proc, buffer, sizeof(buffer), 1000), (ssize_t)0, "EOF");

    int status = -1;
    // TODO: check if 0 is blocking or nonblocking
    ASSERT_TRUE(cmd_wait(&proc, &status, 0), "blocking wait");
    ASSERT_CMP(status, 0);
    cmd_destroy(&proc);
})

TEST_CASE(test_wait_reports_exit_code, {
    cmd_builder cmd = {0};
    ASSERT_TRUE(cmd_append(&cmd, "sh"), "failed to append");
    ASSERT_TRUE(cmd_append(&cmd, "-c"), "failed to append");
    ASSERT_TRUE(cmd_append(&cmd, "exit 42"), "failed to append");
    cmd_proc proc = {0};
    ASSERT_TRUE(cmd_spawn(cmd, &proc), "spawn");
    cmd_builder_destroy(&cmd);

    int status = -1;
    ASSERT_TRUE(cmd_wait(&proc, &status, 0), "blocking wait");
    ASSERT_CMP(status, 42);

    status = -1;
    ASSERT_TRUE(cmd_wait(&proc, &status, -1), "waiting again returns the stored status");
    ASSERT_CMP(status, 42);

    status = -1;
    ASSERT_TRUE(cmd_wait(&proc, &status, 50),
                "timed wait on a reaped proc returns the stored status");
    ASSERT_CMP(status, 42);
    cmd_destroy(&proc);
})

TEST_CASE(test_read_and_wait_time_out_on_idle_proc, {
    cmd_builder cmd = {0};
    ASSERT_TRUE(cmd_append(&cmd, "cat"), "failed to append");
    cmd_proc proc = {0};
    ASSERT_TRUE(cmd_spawn(cmd, &proc), "spawn");  // cat blocks on stdin forever
    cmd_builder_destroy(&cmd);

    char buffer[8];
    errno = 0;
    ASSERT_CMP(cmd_read(proc, buffer, sizeof(buffer), 50), (ssize_t)-1);
    ASSERT_CMP(errno, ETIMEDOUT);

    int status = -1;
    ASSERT_FALSE(cmd_wait(&proc, &status, -1), "non blocking wait on a running proc");
    errno = 0;
    ASSERT_FALSE(cmd_wait(&proc, &status, 50), "timed wait on a proc that never exits");
    ASSERT_CMP(errno, ETIMEDOUT);
    ASSERT_TRUE(cmd_is_alive(proc));

    cmd_destroy(&proc);
})

// regression: cmd_destroy used to SIGKILL without reaping, leaving a zombie
TEST_CASE(test_destroy_kills_running_proc_and_reaps_it, {
    cmd_builder cmd = {0};
    ASSERT_TRUE(cmd_append(&cmd, "cat"), "failed to append");
    cmd_proc proc = {0};
    ASSERT_TRUE(cmd_spawn(cmd, &proc), "spawn");
    cmd_builder_destroy(&cmd);

    pid_t pid = proc.pid;
    cmd_destroy(&proc);
    ASSERT_FALSE(cmd_is_alive(proc));

    int wstatus;
    errno = 0;
    ASSERT_CMP(waitpid(pid, &wstatus, WNOHANG), -1, "no zombie left behind");
    ASSERT_CMP(errno, ECHILD);

    int status = -1;
    ASSERT_TRUE(cmd_wait(&proc, &status, -1), "wait after destroy");
    ASSERT_CMP(status, 128 + SIGKILL);

    cmd_destroy(&proc);  // double destroy is a no-op
    ASSERT_CMP(proc.p_stdout[0], -1);
})

// cmd_destroy used to skip reaping a proc that had already exited
TEST_CASE(test_destroy_reaps_exited_unwaited_proc, {
    cmd_builder cmd = {0};
    ASSERT_TRUE(cmd_append(&cmd, "sh"), "failed to append");
    ASSERT_TRUE(cmd_append(&cmd, "-c"), "failed to append");
    ASSERT_TRUE(cmd_append(&cmd, "exit 7"), "failed to append");
    cmd_proc proc = {0};
    ASSERT_TRUE(cmd_spawn(cmd, &proc), "spawn");
    cmd_builder_destroy(&cmd);

    char buffer[8];
    ASSERT_CMP(cmd_read(proc, buffer, sizeof(buffer), 0), (ssize_t)0, "EOF means it exited");

    pid_t pid = proc.pid;
    cmd_destroy(&proc);

    int wstatus;
    errno = 0;
    ASSERT_CMP(waitpid(pid, &wstatus, WNOHANG), -1, "no zombie left behind");
    ASSERT_CMP(errno, ECHILD);

    int status = -1;
    ASSERT_TRUE(cmd_wait(&proc, &status, -1), "destroy recorded the real exit code");
    ASSERT_CMP(status, 7);
})

TEST_CASE(test_signal_terminates_child, {
    cmd_builder cmd = {0};
    ASSERT_TRUE(cmd_append(&cmd, "cat"), "failed to append");
    cmd_proc proc = {0};
    ASSERT_TRUE(cmd_spawn(cmd, &proc), "spawn");
    cmd_builder_destroy(&cmd);

    ASSERT_CMP(cmd_signal(proc, SIGTERM), 0);
    int status = -1;
    ASSERT_TRUE(cmd_wait(&proc, &status, 0), "blocking wait");
    ASSERT_CMP(status, 128 + SIGTERM, "signal deaths are reported as 128+sig");
    cmd_destroy(&proc);
})

TEST_CASE(test_concurrent_procs_have_independent_io, {
    enum { COUNT = 10 };
    cmd_proc procs[COUNT];
    memset(procs, 0, sizeof(procs));
    for (int i = 0; i < COUNT; ++i) {
        cmd_builder cmd = {0};
        ASSERT_TRUE(cmd_append(&cmd, "cat"), "failed to append");
        ASSERT_TRUE(cmd_spawn(cmd, &procs[i]), "spawn");
        cmd_builder_destroy(&cmd);
    }
    for (int i = 0; i < COUNT; ++i) {
        char arg[8];
        int len = sprintf(arg, "%d", i);
        ASSERT_CMP(cmd_write(procs[i], arg, len), (ssize_t)len);
    }
    for (int i = 0; i < COUNT; ++i) {
        char buffer[32];
        ssize_t n = cmd_read(procs[i], buffer, sizeof(buffer) - 1, 1000);
        ASSERT_TRUE(n > 0);
        buffer[n] = '\0';
        char expected[8];
        sprintf(expected, "%d", i);
        ASSERT_CMP(buffer, expected, "each proc echoes only its own input");
        cmd_destroy(&procs[i]);
    }
})

int main(void) {
    START_SUITE;
    RUN(test_append_builds_null_terminated_argv);
    RUN(test_spawn_rejects_empty_and_missing_command);
    RUN(test_destroy_of_empty_proc_is_a_noop);
    RUN(test_echo_splits_appended_arguments);
    RUN(test_stdout_and_stderr_are_separate_streams);
    RUN(test_stdin_roundtrip_and_eof_after_close);
    RUN(test_wait_reports_exit_code);
    RUN(test_read_and_wait_time_out_on_idle_proc);
    RUN(test_destroy_kills_running_proc_and_reaps_it);
    RUN(test_destroy_reaps_exited_unwaited_proc);
    RUN(test_signal_terminates_child);
    RUN(test_concurrent_procs_have_independent_io);
    END_SUITE;
}
