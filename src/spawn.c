/* spawn.c - fork/exec ssh, provide pipes to stdout/in; handled by sc
 * swh - switch ssh front-end - (C) 2017, Timo Buhrmester
 * See README for contact-, COPYING for license information. */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#define LOG_MOD MOD_SPAWN

#include "spawn.h"

#include <stdio.h>

#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "common/log.h"


/* these are ssh's. [0] is read end, [1] is write end */
static int s_stdin[2];
static int s_stdout[2];
static int s_stderr[2];

static char s_host[256];
static pid_t s_childpid;

static bool s_sigchld;

static char **s_env;


static void dowait(void);
static void sigchld(int s);


void
spawn_init(char **envp)
{
	s_env = envp;
	signal(SIGCHLD, sigchld);
	I("spawn initialized");
	return;
}

void
spawn_operate(void)
{
	if (s_sigchld) {
		I("SIGCHLD seen");
		s_sigchld = false;
		dowait();
	}
	return;
}

int
spawn_launch(const char *host, int *stin, int *stout, int *sterr)
{
	D("spawn called for host '%s'", host);
	if (pipe(s_stdin) != 0) {
		EE("pipe in");
		return -1;
	}
	if (pipe(s_stdout) != 0) {
		EE("pipe out");
		return -1;
	}
	if (pipe(s_stderr) != 0) {
		EE("pipe err");
		return -1;
	}

	snprintf(s_host, sizeof s_host, "%s", host);
	I("forking");

	int r = fork();
	if (r == -1) {
		EE("fork");
		return -1;
	} else if (r == 0) {
		close(s_stdin[1]); close(s_stdout[0]); close(s_stderr[0]);
		close(0); close(1); close(2);

		dup2(s_stdin[0], 0);
		dup2(s_stdout[1], 1);
		dup2(s_stderr[1], 2);

		char binary[] = "/usr/bin/ssh";
		char arg[] = "-T";
		char *argv[] = { binary, arg, s_host, NULL };
		int e = execve(binary, argv, s_env);
		fprintf(stderr, "child: execve: %d\n", e);
		_exit(1);
	} else {
		close(s_stdin[0]); close(s_stdout[1]); close(s_stderr[1]);

		*stin = s_stdin[1];
		*stout = s_stdout[0];
		*sterr = s_stderr[0];

		s_childpid = r;
		I("ssh spawned, pid %d", (int)s_childpid);
		return 0;
	}
}

void
spawn_kill(void)
{
	if (!s_childpid)
		return;
	D("closing our pipe to ssh's stdin");
	close(s_stdin[1]);
	s_stdin[1] = -1;
	return;
}



static void
dowait(void)
{
	int st = -1;
	D("waiting for child");
	pid_t p = wait(&st);
	if (p != s_childpid)
		C("waited for %d but got %d (with exit status %d)",
		    (int)s_childpid, (int)p, WEXITSTATUS(st));
	I("child %d ded, ec %d", (int)s_childpid, (int)WEXITSTATUS(st));
	s_childpid = 0;
	close(s_stdout[0]);
	close(s_stderr[0]);
	if (s_stdin[1] >= 0)
		close(s_stdin[1]);

	return;
}

static void
sigchld(int s)
{
	(void)s;
	s_sigchld = true;
}
