#include "guicall.h"

unsigned long long len(const signed char *s) {
	unsigned long long c = 0;
	while (s[c] != '\0') { ++c; }
	return c;
}

int main() {
	const char *msg = "Hello, world!\n";
	guicall(1, 1, msg, len(msg));
	guicall(60, 0);
}
