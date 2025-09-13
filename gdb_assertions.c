#define _GNU_SOURCE

#include <err.h>
#include <linux/prctl.h>
#include <poll.h>
#include <fcntl.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/eventfd.h>
#include <stdlib.h>
#include <signal.h>

#include <stdio.h>

struct gdb_pipes {
	int gdb_pipefd_stdout[2];
	int gdb_pipefd_stdin[2];
	int signaling_fd;
};

static const char *GDB_STOPPED_PREFIX = "*stopped";
static const char *GDB_CONTINUE = "c\n";
static const char *GDB_INFO_LOCALS = "info locals\n";
static const char *GDB_INFO_LOCALS_OUT = "&\"info locals\\n\"";
static const char *GDB_CONSOLE_INTERPRETER = "new-ui console /dev/tty\n";
static const char *GDB_DONE_OUT = "^done";
static const char *GDB_DETACH = "detach\n";

int gdb_interactive(struct gdb_pipes pipes, int program_pid, int forked_pid) {
	if (close(pipes.gdb_pipefd_stdin[0]) == -1) { /* Close unused read end */
		perror("close pipes.gdb_pipefd_stdin[0]");
		return -1;
	}

	if (close(pipes.gdb_pipefd_stdout[1]) == -1) { /* Close unused write end */
		perror("close pipes.gdb_pipefd_stdin[0]");
		return -1;
	}


	/* Bufferize up the stdout/stdin pipe to the FILE */
	FILE *gdb_stdout_stream	= fdopen(pipes.gdb_pipefd_stdout[0], "r");
	FILE *gdb_stdin_stream	= fdopen(pipes.gdb_pipefd_stdin[1], "w");

	/* Disable buffering for the stream */
	setvbuf(gdb_stdin_stream, NULL, _IONBF, 0);


	char *lineptr = NULL;
	size_t line_cap = 0;
	ssize_t getline_readlen = 0;	

	int stops_ct = 0;

	int info_locals_printing = 0;
	int print_gdb = 0;

	const size_t STOPPED_PREF_LEN		= strlen(GDB_STOPPED_PREFIX);
	const size_t GDB_INFO_LOCALS_OUT_LEN	= strlen(GDB_INFO_LOCALS);
	const size_t GDB_DONE_OUT_LEN		= strlen(GDB_DONE_OUT);

	while ((getline_readlen = getline(&lineptr, &line_cap, gdb_stdout_stream)) != -1) {
		if (!strncmp(lineptr, GDB_STOPPED_PREFIX, STOPPED_PREF_LEN)) {
			if (stops_ct == 0) {
				const eventfd_t eventfd_one = 1;
				if (eventfd_write(pipes.signaling_fd, eventfd_one) < 0) {
					perror("eventfd signaling_fd");
					return -1;
				}
				close (pipes.signaling_fd);

				fprintf(gdb_stdin_stream, "%s", GDB_CONTINUE);
			} else if (stops_ct == 1) {
				fprintf(gdb_stdin_stream, "%s", GDB_INFO_LOCALS);
			}

			stops_ct++;
		}

		if (!strncmp(lineptr, GDB_DONE_OUT, GDB_DONE_OUT_LEN)) {
			if (info_locals_printing) {
				info_locals_printing = 0;

				printf(
					"Locals printed. What do you want to do next? \n"
					"[c]ontinue the program execution, [e]xit the program, "
					"continue with [g]db: "
				);
				fflush(stdout);
				char cmd = 0;
				scanf("%c", &cmd);

				switch (cmd) {
					case 'c':
						fprintf(gdb_stdin_stream, "%s", GDB_DETACH);
						break;
					case 'g':
						print_gdb = 1;
						break;
					case 'e':
					default:
						fprintf(gdb_stdin_stream, "%s", GDB_DETACH);
						kill(forked_pid, SIGHUP);
						kill(program_pid, SIGKILL);
						exit(EXIT_FAILURE);
						break;
				}

				if (print_gdb) {
					break;
				}

			} else {
				kill(forked_pid, SIGHUP);
			}
		}

		if (info_locals_printing) {
			lineptr[getline_readlen - 4] = '\n';
			lineptr[getline_readlen - 3] = '\0';
			printf("%s", lineptr + 2);
		}

		if (!strncmp(lineptr, GDB_INFO_LOCALS_OUT, GDB_INFO_LOCALS_OUT_LEN)) {
			info_locals_printing = 1;
		}
	}

	if (print_gdb) {
		printf("Redirecting tty to gdb in console variant...\n");
		fprintf(gdb_stdin_stream, "%s", GDB_CONSOLE_INTERPRETER);

		if (fclose(gdb_stdout_stream) == EOF) {
			perror("fclose gdb_stdout_stream");
			return -1;
		}
		return 0;
	}

	if (fclose(gdb_stdout_stream) == EOF) {
		perror("fclose gdb_stdout_stream");
		return -1;
	}

	if (fclose(gdb_stdin_stream) == EOF) {
		perror("fclose gdb_stdin_stream");
		return -1;
	}

	return 0;
}

#define GDB_COMMAND_LEN 128

int execv_gdb(struct gdb_pipes pipes, pid_t program_pid) {
	/*
	 * Redirects stdout to pipe.
	 */
	/* Close unused read end */
	if (close(pipes.gdb_pipefd_stdout[0]) == -1) {
		perror("close pipes.gdb_pipefd_stdout[0]");
		return -1;
	}
	/* Redirect stdout to pipe */
	if (dup2(pipes.gdb_pipefd_stdout[1], STDOUT_FILENO) == -1) {
		perror("dup2 pipes.gdb_pipefd_stdout[1] STDOUT");
		return -1;
	}
	/* Close pipe write end since stdout is used now */
	if (close(pipes.gdb_pipefd_stdout[1]) == -1) {
		perror("close pipes.gdb_pipefd_stdout[1]");
		return -1;
	}

	// Uncomment if you want to redirect stderr to stdout => to the pipefd
	// if (dup2(STDOUT_FILENO, STDERR_FILENO) == -1) {
	// 	perror("redirect stderr to stdout");
	// 	return -1;
	// }

	/**
	 * Redirects pipe to stdin
	 */
	/* Close unused write end */
	if (close(pipes.gdb_pipefd_stdin[1]) == -1) {
		perror("close pipes.gdb_pipefd_stdin[1]");
		return -1;
	}
	/* Redirect pipe write end to stdin */
	if (dup2(pipes.gdb_pipefd_stdin[0], STDIN_FILENO) == -1) {
		perror("dup2 pipes.gdb_pipefd_stdin[0] STDIN");
		return -1;
	}
	/* Close pipe read end since stdin is used now */
	if (close(pipes.gdb_pipefd_stdin[0]) == -1) {
		perror("close pipes.gdb_pipefd_stdin[0]");
		return -1;
	}

	int ret = 0;
	char cmd[GDB_COMMAND_LEN] = {0};

	ret = snprintf(cmd, GDB_COMMAND_LEN, "gdb -p %d -i=mi", program_pid);
	if (ret >= GDB_COMMAND_LEN) {
		fprintf(stderr, "The gdb command is larger than buf\n");
		return -1;
	}

	ret = execl("/bin/sh", "sh", "-c", cmd, (char *) NULL);
	return -1;
}

int gdb_running(pid_t program_pid, int signaling_fd) {
	pid_t forked_pid = 0;

	struct gdb_pipes pipes = {0};
	pipes.signaling_fd = signaling_fd;
	
	if (pipe2(pipes.gdb_pipefd_stdout, O_CLOEXEC)) {
		perror("pipe2 pipes.gdb_pipefd_stdout");
		goto err_gdb_pipes_stdout;
	}

	if (pipe2(pipes.gdb_pipefd_stdin, O_CLOEXEC)) {
		perror("pipe2 pipes.gdb_pipefd_stdin");
		goto err_gdb_pipes_stdin;
	}

	forked_pid = fork();
	if (forked_pid == -1) {
		perror("fork_gdb_execv/interactive");
		goto err;
	}

	if (forked_pid == 0) {
		if (execv_gdb(pipes, program_pid)) {
			_exit(EXIT_FAILURE);
		}
	} else {
		if (gdb_interactive(pipes, program_pid, forked_pid)) {
			return -1;
		}

		if (wait(NULL) == -1) /* Wait for child */
			err(EXIT_FAILURE, "wait");
	}

	return 0;

err:
	close (pipes.gdb_pipefd_stdin[0]);
	close (pipes.gdb_pipefd_stdin[1]);
err_gdb_pipes_stdin:
	close (pipes.gdb_pipefd_stdout[0]);
	close (pipes.gdb_pipefd_stdout[1]);
err_gdb_pipes_stdout:
	return -1;
}

int fork_for_gdb(void) {
	int first_eventfd = eventfd(0, 0);
	if (first_eventfd < 0) {
		perror("first_eventfd");
		return -1;
	}

	int second_eventfd = eventfd(0, 0);
	if (second_eventfd < 0) {
		perror("second_eventfd");
		return -1;
	}

	pid_t gdb_pid = fork();
	if (gdb_pid == -1) {
		perror("fork_for_gdb fork");
		return -1;
	}

	if (gdb_pid == 0) {
		prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY);

		const eventfd_t eventfd_one = 1;
		if (eventfd_write(first_eventfd, eventfd_one) < 0) {
			perror("eventfd_write");
			return -1;
		}
		close(first_eventfd);

		eventfd_t eventfd_ct = 0;
		if (eventfd_read(second_eventfd, &eventfd_ct) < 0) {
			printf("eventfd_read");
			return -1;
		}
		close(second_eventfd);

		return 0;
	} else {
		eventfd_t eventfd_ct = 0;

		if (eventfd_read(first_eventfd, &eventfd_ct) < 0) {
			printf("eventfd_read");
			return -1;
		}
		close(first_eventfd);

		gdb_running(gdb_pid, second_eventfd);

		if (wait(NULL) == -1)	/* Wait for child */
			err(EXIT_FAILURE, "wait");

		exit(EXIT_SUCCESS);
	}

	return 0;
}

void cassert_gdb_fork(void) {
	if (fork_for_gdb()) {
		fprintf(stderr, "Error while calling gdb!\n");
		exit(EXIT_FAILURE);
	}
}

#define cassert(condition)							\
	if (!(condition)) {							\
		fprintf(stderr, "\nAn assertion %s failed!\n\n", #condition);	\
		cassert_gdb_fork();						\
										\
		asm ("int $3");							\
	}									\
	(void)0									\
	

int main() {
	int ret = 0;
	printf("The program is running\n");	

	cassert(1 == 0);

	for (;;) {
		printf("Uwu\n");
		sleep(1);
	}
	
	return 0;
}
