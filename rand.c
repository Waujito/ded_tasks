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
#include <stdlib.h>
#include <signal.h>

#include <stdio.h>

#define STRING_LEN 128

int gdb_running(pid_t program_pid, int signaling_fd) {
	int ret = 0;
	
	int pipefd[2] = {0};

	if (pipe2(pipefd, O_CLOEXEC)) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	int pipefd_in[2] = {0};
	if (pipe2(pipefd_in, O_CLOEXEC)) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	pid_t forked_pid = fork();
	if (forked_pid == -1) {
		exit(EXIT_FAILURE);
	}

	if (forked_pid == 0) {
		if (close(signaling_fd) == -1)
			err(EXIT_FAILURE, "close");

		if (close(pipefd[0]) == -1)  /* Close unused read end */
			err(EXIT_FAILURE, "close");

		if (dup2(pipefd[1], STDOUT_FILENO) == -1)
			err(EXIT_FAILURE, "dup2");
		if (close(pipefd[1]) == -1)
			err(EXIT_FAILURE, "close");

		// if (dup2(STDOUT_FILENO, STDERR_FILENO) == -1)
		// 	err(EXIT_FAILURE, "dup2");

		if (close(pipefd_in[1]) == -1)
			err(EXIT_FAILURE, "close");
		if (dup2(pipefd_in[0], STDIN_FILENO) == -1)
			err(EXIT_FAILURE, "dup2");	
		if (close(pipefd_in[0]) == -1)
			err(EXIT_FAILURE, "close");

		char str[STRING_LEN];
		ret = snprintf(str, STRING_LEN, "gdb -p %d -i=mi", program_pid);
		if (ret >= STRING_LEN) {
			fprintf(stderr, "Uwee\n");
			kill(program_pid, SIGTERM);
			return EXIT_FAILURE;
		}

		ret = execl("/bin/sh", "sh", "-c", str, (char *) NULL);
		perror("Oh shit\n");
		_exit(EXIT_FAILURE);
	} else {
		if (close(pipefd_in[0]) == -1)
			err(EXIT_FAILURE, "close");

		char buf = 0;

		if (close(pipefd[1]) == -1)  /* Close unused write end */
			err(EXIT_FAILURE, "close");

		int stops_ct = 0;

		FILE *pipe_stream = fdopen(pipefd[0], "r");
		char *lineptr = NULL;
		size_t line_cap = 0;
		ssize_t getline_readlen = 0;

		const char *GDB_STOPPED_PREFIX = "*stopped";
		const size_t STOPPED_PREF_LEN = strlen(GDB_STOPPED_PREFIX);

		const char *GDB_CONTINUE = "c\n";
		const char *GDB_INFO_LOCALS = "info locals\n";
		const char *GDB_INFO_LOCALS_OUT = "&\"info locals\\n\"";
		const size_t GDB_INFO_LOCALS_OUT_LEN = strlen(GDB_INFO_LOCALS);
		const char *GDB_CONSOLE_INTERPRETER = "new-ui console /dev/tty\n";

		const char *GDB_DONE_OUT = "^done";
		const size_t GDB_DONE_OUT_LEN = strlen(GDB_DONE_OUT);

		const char *GDB_DETACH = "detach\n";

		int info_locals_printing = 0;
		int print_gdb = 0;

		while ((getline_readlen = getline(&lineptr, &line_cap, pipe_stream)) != -1) {
			if (!strncmp(lineptr, GDB_STOPPED_PREFIX, STOPPED_PREF_LEN)) {
				if (stops_ct == 0) {
					printf("First continue: \n");
					if (close(signaling_fd) == -1) {
						err(EXIT_FAILURE, "close");
					}
					write(pipefd_in[1], GDB_CONTINUE, strlen(GDB_CONTINUE));
				} else if (stops_ct == 1) {
					write(pipefd_in[1], GDB_INFO_LOCALS, strlen(GDB_INFO_LOCALS));
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
							write(pipefd_in[1], GDB_DETACH, strlen(GDB_DETACH));
							break;
						case 'g':
							print_gdb = 1;
							break;
						case 'e':
						default:
							write(pipefd_in[1], GDB_DETACH, strlen(GDB_DETACH));
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
			write(pipefd_in[1], GDB_CONSOLE_INTERPRETER, strlen(GDB_CONSOLE_INTERPRETER));
		}

		if (fclose(pipe_stream) == EOF)
			err(EXIT_FAILURE, "fclose");

		if (wait(NULL) == -1)        /* Wait for child */
			err(EXIT_FAILURE, "wait");
	}

	return 0;
}

int fork_for_gdb(void) {
	pid_t pid = getpid();	

	int pipefd[2] = {0};
	if (pipe2(pipefd, O_CLOEXEC)) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	int pipefd2[2] = {0};
	if (pipe2(pipefd2, O_CLOEXEC)) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	printf("fd00: %d fd01: %d fd10: %d fd11: %d\n", pipefd[0], pipefd[1], pipefd2[0], pipefd2[1]);

	pid_t gdb_pid = fork();
	if (gdb_pid == -1) {
		perror("Cannot fork");
		exit(EXIT_FAILURE);
	}

	if (gdb_pid == 0) {
		if (close(pipefd[0]) == -1)
			err(EXIT_FAILURE, "close");
		if (close(pipefd2[1]) == -1)
			err(EXIT_FAILURE, "close");

		prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY);

		if (close(pipefd[1]) == -1)
			err(EXIT_FAILURE, "close");

		printf("Closed\n");

		struct pollfd pfds = {0};
		pfds.fd = pipefd2[0];
		pfds.events = POLLIN;
		int ret = poll(&pfds, 1, -1);
		printf("poll #2 ended <%d> <%d>\n", ret, pfds.revents);

		if (close(pipefd2[0]) == -1)
			err(EXIT_FAILURE, "close");	
	} else {
		if (close(pipefd[1]) == -1)
			err(EXIT_FAILURE, "close");
		if (close(pipefd2[0]) == -1)
			err(EXIT_FAILURE, "close");

		struct pollfd pfds = {0};
		pfds.fd = pipefd[0];
		pfds.events = POLLIN;
		int ret = poll(&pfds, 1, -1);
		printf("poll ended <%d> <%d>\n", ret, pfds.revents);	

		if (close(pipefd[0]) == -1)
			err(EXIT_FAILURE, "close");

		gdb_running(gdb_pid, pipefd2[1]);

		printf("GDB exited!\n");

		if (wait(NULL) == -1)	/* Wait for child */
			err(EXIT_FAILURE, "wait");

		exit (EXIT_SUCCESS);
	}

	return 0;
}

int main() {
	int ret = 0;

	printf("The program is running\n");	

	fork_for_gdb();

	asm ("int $3");
	for (;;) {
		printf("Uwu\n");
		sleep(1);
	}
	
	return 0;
}
