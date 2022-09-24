#include "shared.h"
#include "./bin/template.h"

size_t const sig = 0x616168706c616f63; // coalphaa

static char const prompt0[] =
   "Generate a stub for a path.\n"
   MAX_PATH_WARNING;
static char const prompt1[] =
   "Where should the stub be written to?\n"
   MAX_PATH_WARNING;

void start(void) {
   // so for whatever reason, the data section always seems to be aligned
   // on 16 bytes.
   size_t *pos = (size_t *) bin_template_exe;
   while (*pos != sig) pos++;
   // pos points to struct EXE_PATH

   // write prompt
   HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
   WriteConsoleA(hStdout, prompt0, sizeof(prompt0) - 1, ((DWORD[1]) {}), NULL);

   // get response
   LPWSTR input = __builtin_alloca(PATH_LIMIT + 2 * sizeof(WCHAR));
   HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
   DWORD chars_left;
   ReadConsoleW(hStdin, input, PATH_LIMIT + 2, &chars_left, NULL);
   // console should be line buffered so remove the crlf
   chars_left -= 2;

   struct EXE_PATH *editable = (struct EXE_PATH *) pos;
   editable->length = chars_left;

   WCHAR *chars_wh = editable->chars;
   // copy response into binary
   while (chars_left --> 0) {
      *chars_wh++ = *input++;
   }
   *chars_wh = L'\0';

   WriteConsoleA(hStdout, prompt1, sizeof(prompt1), ((DWORD[1]) {}), NULL);
   char *path_input = __builtin_alloca(PATH_LIMIT + 2);
   DWORD read_amount;
   ReadConsoleA(hStdin, path_input, PATH_LIMIT + 2, &read_amount, NULL);
   path_input[read_amount - 2] = '\0'; // remove the crlf

   HANDLE hStub = CreateFile(
      path_input,
      GENERIC_WRITE,
      0,
      NULL,
      CREATE_ALWAYS,
      FILE_ATTRIBUTE_NORMAL,
      NULL
   );
   WriteFile(hStub, bin_template_exe, bin_template_exe_len, ((DWORD[1]) {}), NULL);
   ExitProcess(0);
   __builtin_unreachable();
}
