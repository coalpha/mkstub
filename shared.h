#define WIN32_LEAN_AND_MEAN
#include <windows.h>

C_ASSERT(sizeof(size_t) == 8);

#define PATH_LIMIT 500
#define MAX_PATH_WARNING "Max path length is 500 characters.\n> "

size_t const coalphaa = 0x616168706c616f63; // coalphaa

struct counted_wstr {
   size_t length;
   WCHAR chars[];
};
