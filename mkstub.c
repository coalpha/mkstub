#include "shared.h"
#include "./bin/template.h"
#include <errhandlingapi.h>

static char const prompt0[] =
   "mkstub.exe (by coalpha)\n"
   "The path limit is " PATH_LIMIT_S " characters.\n"
   "Leading and trailing quotes are ignored.\n"
   "\n"
   "target> ";
static char const prompt1[] = "stub> ";

void start(void) {
   // so for whatever reason, the data section always seems to be aligned
   // on 16 bytes. probably something to do with the /align:16
   size_t *pos = (size_t *) bin_template_exe;
   while (*pos != coalphaa) pos++;

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

   // remove trailing quote
   if (input[chars_left - 1] == L'"') {
      chars_left--;
   }

   struct counted_wstr *editable = (struct counted_wstr *) pos;
   editable->length = chars_left;

   WCHAR *chars_wh = editable->chars;
   // remove leading quote
   if (*input == L'"') {
      input++;
      chars_left--;
   }
   // copy response into binary
   while (chars_left --> 0) {
      *chars_wh++ = *input++;
   }
   // *chars_wh = L'\0'; don't actually need to add the null character

   WriteConsoleA(hStdout, prompt1, sizeof(prompt1) - 1, ((DWORD[1]) {}), NULL);
   char *path_input = __builtin_alloca(PATH_LIMIT + 2);
   DWORD read_amount;
   ReadConsoleA(hStdin, path_input, PATH_LIMIT + 2, &read_amount, NULL);
   if (path_input[read_amount - 3] == '"') {
      // remove trailing quote and crlf
      path_input[read_amount - 3] = '\0';
   } else {
      path_input[read_amount - 2] = '\0'; // remove crlf
   }
   // remove leading quote
   if (*path_input == '"') {
      path_input++;
   }

   HANDLE hStub = CreateFile(
      path_input,
      GENERIC_WRITE,
      0,
      NULL,
      CREATE_ALWAYS,
      FILE_ATTRIBUTE_NORMAL,
      NULL
   );
   if (hStub == INVALID_HANDLE_VALUE) {
      ExitProcess(GetLastError());
      __builtin_unreachable();
   }
   
   BOOL success = WriteFile(
      hStub,
      bin_template_exe,
      bin_template_exe_len,
      ((DWORD[1]) {}),
      NULL
   );

   if (success) {}
   else {
      ExitProcess(GetLastError());
      __builtin_unreachable();
   }

   ExitProcess(0);
   __builtin_unreachable();
}
