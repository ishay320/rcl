#ifndef CMD_H
#define CMD_H

#define _GNU_SOURCE  // for pipe2()

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/*
The work order is like that:
1. #define CMD_IMPLEMENTATION
2. #include "cmd.h"
3. create cmd_builder
4. append commands
5. run command -> get cmd_proc
6. can destroy cmd / reuse
7. get in|out|error from proc
8. reap / kill / do whatever - `cmd_destroy` kill and reap and frees the pipes
*/

typedef struct {
    int p_stdin[2];
    int p_stdout[2];
    int p_stderr[2];
    pid_t pid;

    // status of the child process after it has exited - internal use only
    bool _reaped;
    int _exit_status;
} cmd_proc;

typedef struct {
    size_t size;
    size_t capacity;
    char** data;
} cmd_builder;

// FUNCTION NOTE: you can print errno on failure
bool cmd_spawn(const cmd_builder cmd, cmd_proc* out);

void cmd_destroy(cmd_proc* proc);

int cmd_signal(cmd_proc proc, uint8_t signal);

typedef enum {
    CMD_OK,
    CMD_TIMEOUT,
    CMD_ERROR,
} cmd_status;

cmd_status cmd__wait_on_fd(int fd, int timeout_ms);

ssize_t cmd__read_stream(int fd, char* buffer, size_t size, int timeout_ms);

/**
 * @brief Run a command and read its output or error stream with a timeout
 *
 * @param proc The structure containing the process information
 * @param buffer Buffer to read into
 * @param size Size of the buffer
 * @param timeout_ms Timeout in milliseconds. If 0 wait indefinitely.
 * @return
 *   >0  number of bytes read
 *    0  end of file (the stream has closed)
 *   -1  error occurred; check errno for the cause:
 *         ETIMEDOUT - the timeout elapsed with no data available
 *         (other)   - the underlying poll()/read() error
 */
ssize_t cmd_read(cmd_proc proc, char* buffer, size_t size, size_t timeout_ms);

/**
 * @brief Run a command and read its error stream with a timeout
 *
 * @param proc The structure containing the process information
 * @param buffer Buffer to read into
 * @param size Size of the buffer
 * @param timeout_ms Timeout in milliseconds. If 0 wait indefinitely.
 * @return
 *   >0  number of bytes read
 *    0  end of file (the stream has closed)
 *   -1  error occurred; check errno for the cause:
 *         ETIMEDOUT - the timeout elapsed with no data available
 *         (other)   - the underlying poll()/read() error
 */
ssize_t cmd_read_err(cmd_proc proc, char* buffer, size_t size, size_t timeout_ms);

ssize_t cmd_write(cmd_proc proc, const char* data, size_t size);

/**
 * @brief Check if the command is running
 *
 * @param proc The structure containing the process information
 * @return true if the command is running, false if it has finished or an error
 * occurred
 */
bool cmd_is_alive(const cmd_proc proc);

bool cmd_wait(cmd_proc* proc, int* status, ssize_t timeout_ms);

bool cmd_append(cmd_builder* cmd, char* arg);

#ifdef CMD_IMPLEMENTATION
#include "arrays/dynamic_array.h"

bool cmd_spawn(const cmd_builder cmd, cmd_proc* out) {
    if (cmd.size == 0) return false;

    cmd_proc ret   = {0};
    ret.p_stdin[0] = ret.p_stdin[1] = -1;
    ret.p_stdout[0] = ret.p_stdout[1] = -1;
    ret.p_stderr[0] = ret.p_stderr[1] = -1;

    int exec_err[2] = {-1, -1};

    // TODO: understand the pipe2 thread closing, and choose if i need that
    int status = pipe2(ret.p_stdin, O_CLOEXEC);
    if (status == -1) {
        goto error;
    }
    status = pipe2(ret.p_stdout, O_CLOEXEC);
    if (status == -1) {
        goto error;
    }
    status = pipe2(ret.p_stderr, O_CLOEXEC);
    if (status == -1) {
        goto error;
    }
    status = pipe2(exec_err, O_CLOEXEC);
    if (status == -1) {
        goto error;
    }

    ret.pid = fork();
    switch (ret.pid) {
        case -1:
            goto error;
        case 0:                  // child
            close(exec_err[0]);  // child only writes to the notification pipe
            close(ret.p_stdout[0]);
            if (dup2(ret.p_stdout[1], STDOUT_FILENO) == -1) goto child_failure;
            close(ret.p_stdout[1]);
            close(ret.p_stdin[1]);
            if (dup2(ret.p_stdin[0], STDIN_FILENO) == -1) goto child_failure;
            close(ret.p_stdin[0]);
            close(ret.p_stderr[0]);
            if (dup2(ret.p_stderr[1], STDERR_FILENO) == -1) goto child_failure;
            close(ret.p_stderr[1]);

            execvp(cmd.data[0], cmd.data);

        child_failure: {
            int e = errno;
            (void)write(exec_err[1], &e, sizeof(e));
            _exit(127);
        }
    }
    // parent
    close(exec_err[1]);  // parent only reads
    close(ret.p_stdout[1]);
    close(ret.p_stderr[1]);
    close(ret.p_stdin[0]);

    // Blocks until the child either execs successfully or reports an
    // exec failure by writing its errno (n > 0)
    int child_errno = 0;
    ssize_t n       = read(exec_err[0], &child_errno, sizeof(child_errno));
    close(exec_err[0]);
    if (n > 0) {
        int status;
        waitpid(ret.pid, &status, 0);  // reap the child that failed to exec
        ret._reaped      = true;
        ret._exit_status = status;
        cmd_destroy(&ret);
        errno          = child_errno;  // caller can perror()
        ret.p_stdin[0] = -1;           // TODO: remove that
        *out           = ret;
        return false;
    }

    *out = ret;
    return true;

error: {
    int e = errno;
    close(ret.p_stdin[0]);
    ret.p_stdin[0] = -1;  // TODO: remove that
    close(ret.p_stdin[1]);
    close(ret.p_stdout[0]);
    close(ret.p_stdout[1]);
    close(ret.p_stderr[1]);
    close(exec_err[0]);
    close(exec_err[1]);
    errno = e;
    return false;
}
}

void cmd__proc_fd_close(cmd_proc* proc) {
    close(proc->p_stdout[0]);
    proc->p_stdout[0] = -1;
    close(proc->p_stderr[0]);
    proc->p_stderr[0] = -1;
    close(proc->p_stdin[1]);
    proc->p_stdin[1] = -1;
}

// kill if running, reap if exited, close fds
void cmd_destroy(cmd_proc* proc) {
    if (proc->_reaped || (proc->pid == 0)) {
        return;
    }

    if (cmd_is_alive(*proc)) {
        cmd_signal(*proc, SIGKILL);
        proc->_reaped      = true;
        proc->_exit_status = 128 + SIGKILL;
    }

    cmd__proc_fd_close(proc);
}

// TODO: add detach / use fd_close for that

int cmd_signal(cmd_proc proc, uint8_t signal) { return kill(proc.pid, signal); }

cmd_status cmd__wait_on_fd(int fd, int timeout_ms) {
    struct pollfd pfd = {.fd = fd, .events = POLLIN};
    int ret           = poll(&pfd, 1, timeout_ms);
    if (ret == 0) {
        return CMD_TIMEOUT;
    } else if (ret < 0) {
        return CMD_ERROR;
    }
    return CMD_OK;
}

ssize_t cmd__read_stream(int fd, char* buffer, size_t size, int timeout_ms) {
    if (timeout_ms > 0) {
        cmd_status status = cmd__wait_on_fd(fd, timeout_ms);
        switch (status) {
            case CMD_TIMEOUT:
                errno = ETIMEDOUT;
                return -1;
            case CMD_ERROR:
                return -1;
            case CMD_OK:
                break;
        }
    }
    return read(fd, buffer, size);
}

ssize_t cmd_read(cmd_proc proc, char* buffer, size_t size, size_t timeout_ms) { return cmd__read_stream(proc.p_stdout[0], buffer, size, timeout_ms); }

ssize_t cmd_read_err(cmd_proc proc, char* buffer, size_t size, size_t timeout_ms) {
    return cmd__read_stream(proc.p_stderr[0], buffer, size, timeout_ms);
}

ssize_t cmd_write(cmd_proc proc, const char* data, size_t size) { return write(proc.p_stdin[1], data, size); }

bool cmd_is_alive(const cmd_proc proc) {
    siginfo_t si = {0};
    if (waitid(P_PID, proc.pid, &si, WEXITED | WNOHANG | WNOWAIT) == -1) {
        /* child is not running or an error occurred */
        return false;
    }
    if (si.si_pid == 0) {
        return true;
    }
    return false;
}

bool cmd_reap(cmd_proc* proc, int* status) {
    if (proc->_reaped) {
        *status = proc->_exit_status;
        return true;
    }
    if (cmd_is_alive(*proc)) {
        return false;
    }
    int wstatus = 0;
    if (waitpid(proc->pid, &wstatus, 0) == -1) {
        perror("waitpid failed");
        return false;
    }
    if (WIFEXITED(wstatus)) {
        *status            = WEXITSTATUS(wstatus);
        proc->_exit_status = *status;
        proc->_reaped      = true;
        cmd__proc_fd_close(proc);
    } else if (WIFSIGNALED(wstatus)) {
        *status            = 128 + WTERMSIG(wstatus);
        proc->_exit_status = *status;
        proc->_reaped      = true;
        cmd__proc_fd_close(proc);
    } else {
        return false;
    }
    return true;
}

bool cmd_wait(cmd_proc* proc, int* status, ssize_t timeout_ms) {
    if (timeout_ms < 0) {
        return cmd_reap(proc, status);
    }

    if (proc->_reaped) {
        *status = proc->_exit_status;
        return true;
    }

    if (timeout_ms == 0) {
        siginfo_t si = {0};
        if (waitid(P_PID, proc->pid, &si, WEXITED | WNOWAIT) == -1) {
            perror("waitid failed");
            return false;
        }
        proc->_reaped      = true;
        proc->_exit_status = si.si_status;
        *status            = proc->_exit_status;
        cmd__proc_fd_close(proc);
        return true;
    }

    int fd = syscall(SYS_pidfd_open, proc->pid, 0);
    if (fd == -1) {
        perror("pidfd_open failed");
        return false;
    }
    bool ret = true;

    cmd_status c_status = cmd__wait_on_fd(fd, timeout_ms);
    if (c_status == CMD_TIMEOUT) {
        errno = ETIMEDOUT;
        ret   = false;
    } else if (c_status == CMD_ERROR) {
        fprintf(stderr, "%s: poll failed on pidfd %d: %s\n", __FUNCTION__, fd, strerror(errno));
        ret = false;
    }
    close(fd);
    return ret;
}

bool cmd_append(cmd_builder* cmd, char* arg) {
    if (!(cmd->size == 0) && cmd->data[cmd->size - 1] == NULL) {
        cmd->data[cmd->size - 1] = arg;
        return da_append(*cmd, NULL);
    }
    bool ok = da_append(*cmd, arg);
    ok &= da_append(*cmd, NULL);
    return ok;
}

void cmd_builder_destroy(cmd_builder* cmd) { da_destroy(cmd); }

#endif  // CMD_IMPLEMENTATION
#endif  // CMD_H

typedef struct {
    // list / append them already
    // list of input and output for connection of the parts
} cmd_pipeline;
