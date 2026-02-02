#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

static volatile sig_atomic_t got_signal = 0;
static void handle_sig(int sig) { (void)sig; got_signal = 1; }

static void usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s <seconds> [-- command [args...]]\n"
        "  Wait <seconds> (can be fractional) once, then run command if provided.\n"
        "Examples:\n"
        "  %s 5           # sleep 5s then exit\n"
        "  %s 2.5 -- ls -l\n",
        prog, prog, prog);
    exit(2);
}

int main(int argc, char **argv) {
    if (argc < 2) usage(argv[0]);

    char *endptr = NULL;
    double sec = strtod(argv[1], &endptr);
    if (endptr == argv[1] || sec < 0 || errno == ERANGE) {
        fprintf(stderr, "Invalid seconds value: %s\n", argv[1]);
        return 2;
    }

    /* find command start: if argv[2] == "--" then command is argv+3, else argv+2 */
    int cmd_index = 2;
    if (argc >= 3 && strcmp(argv[2], "--") == 0) cmd_index = 3;

    /* setup signal handlers to allow clean interrupt */
    signal(SIGINT, handle_sig);
    signal(SIGTERM, handle_sig);

    /* nanosleep with fractional seconds */
    struct timespec req, rem;
    req.tv_sec = (time_t)sec;
    req.tv_nsec = (long)((sec - (double)req.tv_sec) * 1e9);

    while (!got_signal) {
        if (nanosleep(&req, &rem) == 0) break;
        if (errno == EINTR) {
            if (got_signal) {
                fprintf(stderr, "Interrupted, exiting\n");
                return 130;
            }
            req = rem;
            continue;
        } else {
            perror("nanosleep");
            return 2;
        }
    }

    if (got_signal) return 130;

    /* If no command given, just exit successfully after sleep */
    if (cmd_index >= argc) return 0;

    /* spawn the command */
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 2;
    } else if (pid == 0) {
        /* child: execute command */
        execvp(argv[cmd_index], &argv[cmd_index]);
        /* execvp only returns on error */
        fprintf(stderr, "exec failed: %s: %s\n", argv[cmd_index], strerror(errno));
        _exit(127);
    } else {
        /* parent: wait for child and return its status */
        int wstatus = 0;
        if (waitpid(pid, &wstatus, 0) < 0) {
            perror("waitpid");
            return 2;
        }
        if (WIFEXITED(wstatus)) return WEXITSTATUS(wstatus);
        if (WIFSIGNALED(wstatus)) return 128 + WTERMSIG(wstatus);
        return 0;
    }
}
