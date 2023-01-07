#define WIN32_LEAN_AND_MEAN
#include <windows.h>

C_ASSERT(sizeof(size_t) == 8);

#define to_str(x) str(x)
#define str(x) #x
#define PATH_LIMIT 500
#define coalphaa 0x616168706c616f63

#define qwordptr(x) *((size_t *) (x))

struct counted_wstr {
   size_t length;
   WCHAR chars[];
};

#define IGNORE_WRITE ((DWORD[1]) {}), NULL
