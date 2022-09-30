#include "shared.h"

static struct EXE_PATH EXE_PATH = {
   .length = 0x616168706c616f63,
   .chars = {[PATH_LIMIT] = 0},
};

enum CommandLineState {
   CLS_NORMAL,
   CLS_ESCAPED,
   CLS_QUOTED,
   CLS_QUOTED_ESCAPED,
};

void start(void) {
   LPWSTR commandline = GetCommandLineW();
   enum CommandLineState CLS = CLS_NORMAL;
   while (1) {
      WCHAR const c = *commandline;

      if (c == L'\0') {
         break;
      }

      if (CLS == CLS_NORMAL) {
         if (c == L'"') {
            CLS = CLS_QUOTED;
            goto loop_end;
         }
         if (c == L'\\') {
            CLS = CLS_ESCAPED;
            goto loop_end;
         }
         if (c == L' ') {
            break;
         }
         else {
            goto loop_end;
         }
      }

      if (CLS == CLS_ESCAPED) {
         if (c == L' ') {
            break;
         }
         else {
            CLS = CLS_NORMAL;
            goto loop_end;
         }
      }

      if (CLS == CLS_QUOTED) {
         if (c == L'"') {
            CLS = CLS_NORMAL;
            goto loop_end;
         }
         if (c == L'\\') {
            CLS = CLS_QUOTED_ESCAPED;
            goto loop_end;
         }
         else {
            goto loop_end;
         }
      }

      // "foo\ "
      //      ^
      if (CLS == CLS_QUOTED_ESCAPED) {
         CLS = CLS_QUOTED;
         goto loop_end;
      }

      loop_end: commandline++;
   }
   // now commandline is at the space before the second argument
   // we need to add that onto argv0 which is EXE_PATH.chars.
   // this requires finding the length.

   size_t remaining_arguments_length = 0;
   while (commandline[remaining_arguments_length] != '\0') {
      remaining_arguments_length++;
   }

   size_t new_command_line_length = 0
      + 1 // quotes
      + EXE_PATH.length
      + 1
      + remaining_arguments_length
      + 1; // null character

   WCHAR *new_command_line = __builtin_alloca(new_command_line_length * sizeof(WCHAR));
   WCHAR *new_command_line_wh = new_command_line;
   // yes, it's true.
   // instead of copying over two bytes at a time, I could be copying over four.
   // but honestly, I don't really care.
   *new_command_line_wh++ = L'"';
   for (size_t i = 0; i < EXE_PATH.length; i++) {
      *new_command_line_wh++ = EXE_PATH.chars[i];
   }
   *new_command_line_wh++ = L'"';

   for (size_t i = 0; i < new_command_line_length; i++) {
      *new_command_line_wh++ = commandline[i];
   }

   *new_command_line_wh = L'\0';

   STARTUPINFOW startupinfo;
   GetStartupInfoW(&startupinfo);
   PROCESS_INFORMATION new_proc_info;
   BOOL success = CreateProcessW(
      EXE_PATH.chars,
      new_command_line,
      NULL,              // lpProcessAttributes
      NULL,              // lpThreadAttributes
      TRUE,              // bInheritHandles
      0,                 // dwCreationFlags (inherit)
      NULL,              // use parent process's environment block
      NULL,              // lpCurrentDirectory (use current directory)
      &startupinfo,      // lpStartupInfo
      &new_proc_info     // lpProcessInformation [out]
   );

   if (success) {
      WaitForSingleObject(new_proc_info.hProcess, INFINITE);
      DWORD exit_code;
      GetExitCodeProcess(new_proc_info.hProcess, &exit_code);
      CloseHandle(new_proc_info.hProcess);
      CloseHandle(new_proc_info.hThread);
      ExitProcess(exit_code);
      __builtin_unreachable();
   }

   ExitProcess(GetLastError());
   __builtin_unreachable();
}
