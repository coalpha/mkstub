#include "shared.h"

struct counted_wstr exe_path = {
   .length = coalphaa,
   .chars = {[PATH_LIMIT] = 0},
};

enum cl_state {
   CLS_NORMAL,
   CLS_ESCAPED,
   CLS_QUOTED,
   CLS_QUOTED_ESCAPED,
};

void start(void) {
   // When we get the command line string for the stub, the first (index = 0)
   // argument is always going to be the path of the current module (exe).
   // We definitely don't want that to be passed along to our target executable
   // so we need to find the end of the first argument.
   // This means that command line parsing has to happen.
   // Luckily, this is pretty simple. All we need is a small state machine.

   // See CommandLineToArgvW
   // https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-commandlinetoargvw

   // See "Parsing C++ Command-Line Arguments "
   // https://learn.microsoft.com/en-us/previous-versions/17w5ykft(v=vs.85)

   LPWSTR commandline = GetCommandLineW();
   enum cl_state CLS = CLS_NORMAL;
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

   // Now commandline is at the space before the second (index = 1) argument.
   // To create the command line string for the target executable, we need to
   // concatenate the rest of `commandline' with `exe_path.chars'.
   // exe_path.chars + args[1..].join(' ');

   // In order to allocate the nessesary amount of space for the new command
   // line string, we need to figure out how long the rest of `commandline' is.

   size_t remaining_arguments_length = 0;
   while (commandline[remaining_arguments_length] != '\0') {
      remaining_arguments_length++;
   }

   size_t new_command_line_length = 0
      + 1 // quotes
      + exe_path.length
      + 1
      + remaining_arguments_length
      + 1; // null character

   WCHAR *new_command_line = __builtin_alloca(new_command_line_length * sizeof(WCHAR));
   WCHAR *new_command_line_wh = new_command_line;
   // yes, it's true.
   // instead of copying over two bytes at a time, I could be copying over four.
   // but honestly, I don't really care.
   *new_command_line_wh++ = L'"';
   for (size_t i = 0; i < exe_path.length; i++) {
      *new_command_line_wh++ = exe_path.chars[i];
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
      exe_path.chars,
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
      // I forgot why we need to do all this waiting stuff but I think things go
      // fucky if we don't.
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
