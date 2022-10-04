#define WIN32_LEAN_AND_MEAN
#include <windows.h>

C_ASSERT(sizeof(size_t) == 8);

#define PATH_LIMIT 500
#define PATH_LIMIT_S "500"
#define MAX_PATH_WARNING \
   "Quotes at the start and end of the string will be removed.\n" \
   "Max path length is 500 characters.\n" \
   "target> " \

#define coalphaa 0x616168706c616f63

#define qwordptr(x) *((size_t *) (x))

struct counted_wstr {
   size_t length;
   WCHAR chars[];
};
