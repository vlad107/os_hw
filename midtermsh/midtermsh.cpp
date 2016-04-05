#include <stdio.h>
#include <assert.h>
#include <vector>
#include <signal.h>
#include <string.h>
#include <cstring>
#include <iostream>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int curC = 0;
std::vector<std::string> cmds;
bool finished;
int lastPid;

void run_cmd(std::string cmd, int ppid = -1) {
	std::string cur_param;
	int cnt_params = 0;
	std::vector<std::string> args;
	cmd += ' ';
	bool quotes1 = false;
	bool quotes2 = false;
	for (int i = 0; i < (int)cmd.size(); i++) {
		if ((cmd[i] == ' ') && (!quotes1) && (!quotes2)) {
			if (cur_param != "") {
				if (cur_param[0] == '\'') {
					assert(cur_param[(int)cur_param.size() - 1] == '\'');
					cur_param = cur_param.substr(1, (int)cur_param.size() - 2);
				}
				args.push_back(cur_param);
				cur_param = "";
			} 
		} else {
			if ((cmd[i] == '"') && (!quotes2)) quotes1 ^= 1;
			else if ((cmd[i] == '\'') && (!quotes1)) quotes2 ^= 1;
			cur_param += cmd[i];
		}
	}
	char **argv = new char*[(int)args.size() + 1];
	for (int i = 0; i < (int)args.size(); i++) {
		argv[i] = new char[(int)args[i].size() + 1];
		strcpy(argv[i], args[i].c_str());
		argv[i][args[i].size()] = 0;
	}
	argv[args.size()] = 0;
//	for (int i = 0; i < (int)args.size(); i++) {
//		std::cerr << argv[i] << std::endl;
//	}
	execvp(argv[0], argv);
}

void handler_end(int signum, siginfo_t *info, void *ucp) {
//	std::cerr << info->si_pid << std::endl;
	if (info->si_pid == lastPid) {
		finished = true;
	}
}

int main(int argc, char **argv) {
	while (1) {
		cmds.clear();
		struct sigaction sa;
		sigemptyset(&sa.sa_mask);
		sigaddset(&sa.sa_mask, SIGCHLD);
		sigaddset(&sa.sa_mask, SIGINT);
		sa.sa_flags = SA_SIGINFO;
		sa.sa_sigaction = handler_end;
		if (sigaction(SIGCHLD, &sa, NULL) || (sigaction(SIGINT, &sa, NULL))) {
			fprintf(stderr, "sigaction failed");
			exit(1);
		}
		puts("$");
		fflush(stdout);
		const int SIZE_BUFF = 1024;
		char buff[SIZE_BUFF];
		bool quotes1 = false;
		bool quotes2 = false;
		std::string cur_cmd = "";
		bool first = 1;
		int _read;
		std::vector< std::pair<int, int> > pipes(cmds.size());
		while ((_read = read(0, buff, SIZE_BUFF)) > 0) {
			bool finished = false;
			for (int i = 0; i < _read; i++) {
				if ((buff[i] == '"') && (!quotes2)) { 
					cur_cmd += buff[i];
					quotes1 ^= 1;
				} else if ((buff[i] == '\'') && (!quotes1)) {
					cur_cmd += buff[i];
					quotes2 ^= 1;
				} else if (((buff[i] == '\n') || (buff[i] == '|')) && (!quotes1) && (!quotes2)) {
					int bp[2];
					pipe2(bp, O_CLOEXEC);
					pipes.push_back(std::make_pair(bp[0], bp[1]));
					cmds.push_back(cur_cmd);
					cur_cmd = "";
					if (buff[i] == '\n') { 
						finished = true;
						int pnt = 0;
						while (i+1+pnt < _read) {
							pnt += write(pipes[0].second, buff+i+1+pnt, _read-i-1-pnt);
						}
						break;
					}
				} else {
					cur_cmd += buff[i];
				}
			}
			if (finished) break;
		}
		std::vector<int> pids(cmds.size());
		for (int i = 0; i < (int)cmds.size(); i++) {
			pids[i] = fork();
			if (pids[i] < 0) {
				fprintf(stderr, "Fork failed");
				return 1;
			}
			if (pids[i] == 0) {
				dup2(pipes[i].first, STDIN_FILENO);
				close(pipes[i].first);
				if (i + 1 < (int)cmds.size()) {
					dup2(pipes[i + 1].second, STDOUT_FILENO);
					close(pipes[i + 1].second);
				}
				run_cmd(cmds[i], (i ? pids[i - 1] : -1));
				return 0;
			} else {
				close(pipes[i].first);
				if (i + 1 < (int)cmds.size()) {
					close(pipes[i + 1].second);
				}
			}
		}
		for (int i = 0; i < (int)cmds.size(); i++) std::cerr << pids[i] << std::endl;
		finished = false;
		lastPid = pids.back();
		while (!finished) {
		    _read = read(0, buff, SIZE_BUFF); 
			if (_read > 0) {
				int pnt = 0;
				while (pnt < _read) {
					int _write = write(pipes[0].second, buff+pnt, _read-pnt);
					if (_write < 0) continue;
					pnt += _write;
				}
			}
		}
	}
}

