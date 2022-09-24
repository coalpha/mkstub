#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define PATH_LIMIT 500
#define MAX_PATH_WARNING "Max path length is 500 characters.\n> "

struct EXE_PATH {size_t length; WCHAR chars[];};
