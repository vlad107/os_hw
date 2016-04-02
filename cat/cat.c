int main() {
	const int BUFF_SIZE = 2048;
	const char* ERR_WRITE = "An error occured while writing\n";
	const int ERR_WRITE_LEN = 31;
	const char* ERR_READ = "An error occured while reading\n";
	const int ERR_READ_LEN = 31;

	char buffer[BUFF_SIZE];
	int _read = 0;
	while ((_read = read(0, buffer, BUFF_SIZE)) > 0) {
		int cur = 0;
		while (cur < _read) {
			int _write = write(1, buffer + cur, _read - cur);
			if (_write < 0) {
				write(2, ERR_WRITE, ERR_WRITE_LEN);
				return 1;
			}
			cur += _write;
		}
	}
	if (_read != 0) {
		write(2, ERR_READ, ERR_READ_LEN);
		return 1;
	}
	return 0;
}
