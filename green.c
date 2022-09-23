#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "./bin/shrub.h"

size_t const sig = *L"\xc0\xa1\xff\x1e";

static char const prompt[] = "Generate a stub!\n> ";

void start(void) {
   unsigned char *pos = bin_shrub_exe;
   while (1) {
      size_t const next_8 = *((size_t *) pos);
      if (next_8 == sig) break;
      pos++;
   }
   // we've met a pointer to the beginning of the string
   HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
   DWORD dummy;
   WriteConsoleA(hStdout, prompt, sizeof(prompt) - 1, &dummy, NULL);
   CONSOLE_SCREEN_BUFFER_INFO buffer_info = {0};
   if (GetConsoleScreenBufferInfo(hStdout, &buffer_info) != FALSE) {
      ExitProcess(GetLastError());
   }
   DWORD input_length = 0
      - (buffer_info.dwSize.X * (buffer_info.dwCursorPosition.Y + 1))
      - (buffer_info.dwSize.X - (buffer_info.dwCursorPosition.X + 0));
   LPWSTR input = __builtin_alloca(input_length * sizeof(WCHAR));
   HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
   ReadConsoleW(hStdin, input, input_length, &dummy, NULL);
   WCHAR *next_pos = (WCHAR *) pos;
   while (input_length --> 0) {
      *next_pos++ = *input++;
   }
   *next_pos = L'\0';
   HANDLE hStub = CreateFile(
      "stub.exe",
      GENERIC_WRITE,
      0,
      NULL,
      CREATE_ALWAYS,
      FILE_ATTRIBUTE_NORMAL,
      NULL
   );
   WriteFile(hStub, bin_shrub_exe, bin_shrub_exe_len, &dummy, NULL);
}
