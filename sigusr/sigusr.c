#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

void sig_handler(int signum, siginfo_t *info, void *ucp) {
	char *signame = (signum == SIGUSR1 ? "SIGUSR1" 
									   : "SIGUSR2");
	printf("%s from %d\n", signame, info->si_pid);
}

int main(int argc, char **argv) {
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = sig_handler;
	if (sigaction(SIGUSR1, &sa, NULL) || 
		sigaction(SIGUSR2, &sa, NULL)) {
		fprintf(stderr, "An error occured while setting handler\n");
		exit(1);
	}
	if (!sleep(10)) {
		puts("No signals were caught");
		exit(0);
	}
	return 0;
}

