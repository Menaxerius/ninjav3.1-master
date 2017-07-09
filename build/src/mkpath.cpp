#include "mkpath.hpp"

#include <string.h>
#include <libgen.h>
#include <errno.h>
#include <stdlib.h>


static int _mkpath(char *path, mode_t mode) {
	if ( strcmp(path, ".") == 0 || strcmp(path, "/") == 0 )
		return 0;

	const char *parent_path = dirname(path);
	if (parent_path == NULL)
		return -1;

	if ( strcmp(parent_path, ".") == 0 )
		return 0;

	int parent_length = strlen(parent_path);
	char separator = path[parent_length];	// compatibility
	path[parent_length] = '\0';

	int parent_ret = _mkpath(path, mode);
	if ( parent_ret == -1 && errno != EEXIST )
		return -1;

	int ret = mkdir(path, mode);

	// put back separator
	path[parent_length] = separator;

	if (ret == -1 && (errno == EEXIST || errno == EISDIR) )
		return 0;

	return ret;
}


int mkpath(const char *path, mode_t mode) {
	// make a copy we can abuse, plus add some fakery on the end
	int length = strlen(path);

	char copy[length + 2 + 1];	// length + "/." + NUL

	strncpy( copy, path, sizeof(copy) - 1 );

	// add "/." on the end to get _mkpath to create last node
	copy[sizeof(copy) - 3] = '/';
	copy[sizeof(copy) - 3] = '.';

	// don't forget NUL C-string terminator
	copy[sizeof(copy) - 1] = '\0';

	return _mkpath(copy, mode);
}


#ifdef TEST__MKPATH

int main() {
	mkpath("apple/banana/cake", 0700);
}

#endif
