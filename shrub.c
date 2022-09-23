#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// static WCHAR EXE_PATH[MAX_PATH] =
// L"\xc0\xa1\xff\x1e*************************************************************************************************************************************************************************************************************************************************************";

static WCHAR EXE_PATH[] = L"C:\\Windows\\cmd.exe";

void start(void) {
   // need to ignore first argument in GetCommandLineW.
   STARTUPINFOW startupinfo;
   GetStartupInfoW(&startupinfo);
   size_t const current_directory_sz = GetCurrentDirectoryW(0, NULL); // in characters
   WCHAR const *current_directory = __builtin_alloca(current_directory_sz * sizeof(WCHAR));
   PROCESS_INFORMATION new_proc_info;
   CreateProcessW(
      EXE_PATH,
      GetCommandLineW(),
      NULL,              // lpProcessAttributes
      NULL,              // lpThreadAttributes
      TRUE,              // bInheritHandles
      0,                 // dwCreationFlags (inherit)
      NULL,              // use parent process's environment block
      current_directory, // lpCurrentDirectory
      &startupinfo,      // lpStartupInfo
      &new_proc_info     // lpProcessInformation [out]
   );
}
