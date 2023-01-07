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
char const continuing_anyways[] = "Continuing anyways...\n";

void start(void) {
   // so for whatever reason, the data section always seems to be aligned
   // on 16 bytes. probably something to do with the /align:16
   size_t template_offset = 0;
   while (qwordptr(bin_template_exe + template_offset) != coalphaa)
      template_offset += sizeof(size_t);

   // write prompt
   HANDLE const hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
   WriteConsoleA(hStdout, header, sizeof(header) - 1, ((DWORD[1]) {}), NULL);
   // get response
   LPWSTR target = __builtin_alloca(PATH_LIMIT + 2 * sizeof(WCHAR));
   HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
   DWORD target_length;
   ReadConsoleW(hStdin, target, PATH_LIMIT + 2, &target_length, NULL);
   // console should be line buffered so remove the crlf
   target_length -= 2;

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
   for (size_t i = 0; i < target_length; i++) {
      WCHAR const c = target[i];
      if (c < 32) {
         // control character
      }
      if (c == L'"') {}
      if (c == L'*') {}
      if (c == L'.') {}
      if (c == L'/') {}
      if (c == L':') {}
      if (c == L'<') {}
      if (c == L'>') {}
      if (c == L'?') {}
      if (c == L'\\') {}
      editable->chars[i] = c;
   }
   // and bump the offset
   template_offset += sizeof(editable->length);
   template_offset += target_length * sizeof(WCHAR);

   // now we can trim all the remaining zeros off the string after a 16 byte
   // alignment of course
   size_t const final_file_size = ((template_offset + 15) / 16) * 16;

   WriteConsoleA(hStdout, stub_, sizeof(stub_) - 1, ((DWORD[1]) {}), NULL);
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
      final_file_size,
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
