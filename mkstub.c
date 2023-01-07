#include "shared.h"
#include "./bin/template.h"

char const header[] =
   "mkstub.exe\n"
   "An interactive program to create stubs.\n"
   "The path limit is " to_str(PATH_LIMIT) " characters.\n"
   "Leading and trailing quotes are ignored.\n"
   "\n"
   "target> ";
char const stub_[] = "stub> ";

HANDLE hStdout;
HANDLE hStdin;
DWORD before_program_console_mode;

void wait(void) {
   SetConsoleMode(hStdin, ENABLE_PROCESSED_INPUT);
   char const press_any_key[] = "\nPress any key to continue...\n";
   WriteConsoleA(hStdout, press_any_key, sizeof(press_any_key) - 1, IGNORE_WRITE);
   ReadConsoleA(hStdin, ((char[1]) {}), 1, IGNORE_WRITE);
   // assuming that after you wait you die
   SetConsoleMode(hStdin, before_program_console_mode);
}

BOOL WINAPI sigint_handler(DWORD signal) {
   if (signal == CTRL_C_EVENT) {
      char const interrupt[] = "\nInterrupt. Exiting.\n";
      WriteConsoleA(hStdout, interrupt, sizeof(interrupt) - 1, IGNORE_WRITE);
      SetConsoleMode(hStdin, before_program_console_mode);
   }
   return 1;
}

void start(void) {
   // so for whatever reason, the data section always seems to be aligned
   // on 16 bytes. probably something to do with the /align:16
   size_t template_offset = 0;
   while (qwordptr(bin_template_exe + template_offset) != coalphaa)
      template_offset += sizeof(size_t);

   hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
   hStdin = GetStdHandle(STD_INPUT_HANDLE);

   if (!GetConsoleMode(hStdin, &before_program_console_mode)) {
      __builtin_trap();
   }

   if (!SetConsoleCtrlHandler(sigint_handler, 1)) {
      __builtin_trap();
   }

   SetConsoleMode(hStdin, 0
      | ENABLE_ECHO_INPUT
      | ENABLE_LINE_INPUT
      | ENABLE_PROCESSED_INPUT
      | ENABLE_QUICK_EDIT_MODE
      | ENABLE_EXTENDED_FLAGS
   );

   // write prompt
   WriteConsoleA(hStdout, header, sizeof(header) - 1, IGNORE_WRITE);
   // get response
   LPWSTR target = __builtin_alloca(PATH_LIMIT + 2 * sizeof(WCHAR));
   DWORD target_length;
   ReadConsoleW(hStdin, target, PATH_LIMIT + 2, &target_length, NULL);
   // console should be line buffered so remove the crlf
   target_length -= 2;

   if (target_length == 0) {
      WriteConsoleA(hStdout, "No input", sizeof("No input") - 1, IGNORE_WRITE);
      wait();
      ExitProcess(0);
      __builtin_unreachable();
   }

   // remove leading quote
   if (target[0] == L'"') {
      target++;
      target_length--;
   }
   // remove trailing quote
   if (target[target_length - 1] == L'"') {
      target_length--;
   }

   // copy all the stuff to the wstr
   struct counted_wstr *const editable =
      (struct counted_wstr *) (bin_template_exe + template_offset);
   editable->length = target_length;
   _Bool invalid_characters = 0;
   for (size_t i = 0; i < target_length; i++) {
      WCHAR const c = target[i];
      if (0
         || c < 32
         || c == L'"'
         || c == L'*'
         || c == L'<'
         || c == L'>'
      )
      {
         target[i] = '^';
         invalid_characters = 1;
      }
      else {
         target[i] = ' ';
      }
      editable->chars[i] = c;
   }

   if (invalid_characters) {
      WriteConsoleW(hStdout, editable->chars, editable->length, IGNORE_WRITE);
      WriteConsoleA(hStdout, "\n", 1, IGNORE_WRITE);
      WriteConsoleW(hStdout, target, target_length, IGNORE_WRITE);
      WriteConsoleA(hStdout, "\n", 1, IGNORE_WRITE);
      static char const msg[] = "Invalid characters found in path string!";
      WriteConsoleA(hStdout, msg, sizeof(msg) - 1, IGNORE_WRITE);
      wait();
      ExitProcess(1);
      __builtin_unreachable();
   }
   // and bump the offset
   template_offset += sizeof(editable->length);
   template_offset += target_length * sizeof(WCHAR);

   // now we can trim all the remaining zeros off the string after a 16 byte
   // alignment of course
   size_t const final_file_size = ((template_offset + 15) / 16) * 16;

   WriteConsoleA(hStdout, stub_, sizeof(stub_) - 1, IGNORE_WRITE);
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
      goto DIE_with_last_error;
   }

   BOOL success = WriteFile(
      hStub,
      bin_template_exe,
      final_file_size,
      ((DWORD[1]) {}),
      NULL
   );

   if (success) {
      WriteConsoleA(hStdout, "Done", sizeof("Done") - 1, IGNORE_WRITE);
      wait();
      ExitProcess(0);
      __builtin_unreachable();
   }

   DIE_with_last_error: {
      DWORD const err = GetLastError();
      char const *msg;
      DWORD const msg_sz = FormatMessage(0
         | FORMAT_MESSAGE_ALLOCATE_BUFFER
         | FORMAT_MESSAGE_FROM_SYSTEM
         | FORMAT_MESSAGE_IGNORE_INSERTS
         | (FORMAT_MESSAGE_MAX_WIDTH_MASK & 80),
         NULL,
         err,
         LANG_ENGLISH,
         (LPSTR) &msg, // so I have to cast this even though what I'm doing
         0,            // should technically not requrie any casting.
         NULL
      );
      WriteConsoleA(hStdout, msg, msg_sz, IGNORE_WRITE);
      wait();
      ExitProcess(err);
      __builtin_unreachable();
   }
}
