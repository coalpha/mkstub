#include "shared.h"
#include "./bin/shrub.h"

size_t const sig = 0x616168706c616f63; // coalphaa

static char const prompt[] = "Generate a stub for a path.\nTruncates to 260 characters.\n> ";

void start(void) {
   unsigned char *pos = bin_shrub_exe;
   while (1) {
      // yes, unaligned memory reads
      size_t const next_8 = *((size_t *) pos);
      if (next_8 == sig) break;
      pos++;
   }
   // pos points to size_t .length

   // write prompt
   HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
   WriteConsoleA(hStdout, prompt, sizeof(prompt) - 1, ((DWORD[1]) {}), NULL);

   // get response
   LPWSTR input = __builtin_alloca(260 * sizeof(WCHAR));
   HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
   DWORD chars_left;
   ReadConsoleW(hStdin, input, 260, &chars_left, NULL);

   struct EXE_PATH *editable = (struct EXE_PATH *) pos;
   editable->length = chars_left;

   WCHAR *chars_wh = editable->chars;
   // copy response into binary
   while (chars_left --> 0) {
      *chars_wh++ = *input++;
   }
   *chars_wh = L'\0';

   HANDLE hStub = CreateFile(
      "stub.exe",
      GENERIC_WRITE,
      0,
      NULL,
      CREATE_ALWAYS,
      FILE_ATTRIBUTE_NORMAL,
      NULL
   );
   WriteFile(hStub, bin_shrub_exe, bin_shrub_exe_len, ((DWORD[1]) {}), NULL);
   ExitProcess(0);
}
