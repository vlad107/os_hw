#include <stdio.h>
#include <string.h>
#include <errno.h>

int main() {
	const int BUFF_SIZE = 2048;

	char buffer[BUFF_SIZE];
	int _read = 0;
	while ((_read = read(0, buffer, BUFF_SIZE)) > 0) {
		int cur = 0;
		while (cur < _read) {
			int _write = write(1, buffer + cur, _read - cur);
			if (_write < 0) {
				printf("An error occured while writing: %s", strerror(errno));
				return 1;
			}
			cur += _write;
		}
	}
	if (_read != 0) {
		write(2, "An error occured while reading: %s", strerror(errno));
		return 1;
	}
	return 0;
}
