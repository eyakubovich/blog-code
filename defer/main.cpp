#include <iostream>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "defer.h"

void foo(const char* path, bool cleanup) {
	deferred d;
	int fd;

	fd = creat(path, S_IRUSR);
	if( cleanup ) {
		d = defer([path, fd]{
			close(fd);
			unlink(path);
		});
	}
	else {
		d = defer([fd]{ close(fd); });
	}
}

int main() {
	deferred d;

	auto d1 = defer([] { std::cout << "deferred 1" << std::endl; });
	auto d2 = defer([] { std::cout << "deferred 2" << std::endl; });

	std::FILE* f = std::fopen("/etc/hosts", "rt");
	auto d3 = defer([f]() { std::fclose(f); });

	d = defer([] { std::cout << "all done" << std::endl; });

	return 0;
}

