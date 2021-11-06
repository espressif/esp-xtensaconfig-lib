#include <stdio.h>
#include <windows.h>

#define MCPU_MAX_LEN 16
#define MCPU_PREFIX "--mcpu="
#define MCPU_BASE "esp"

#define GDB_FILENAME_PREFIX "xtensa-" MCPU_BASE
#define GDB_BASE_FILENAME "xtensa-" MCPU_BASE "-elf-gdb.exe"
#define GDB_FILENAME_EXAMPLE "xtensa-" MCPU_BASE "XXX-elf-gdb.exe"

static char *get_filename_ptr(const char *exe_path);
static void set_mcpu_option(const char *filename, char *mcpu, const size_t mcpu_size);
static void change_exec_path_filename(char *filename, const char * mcpu_option);
static char *get_cmdline(const int argc, const char **argv, const char *exe_path, const char * mcpu_option);
static void execute_cmdline(const char *cmdline);

int main (int argc, char **argv) {
  char exe_path[PATH_MAX] = {0};
  char mcpu_option[MCPU_MAX_LEN] = {0};
  char *filename = NULL;
  char *cmdline = NULL;

  if (GetModuleFileName(0, exe_path, PATH_MAX) == 0)
  {
      perror("GetModuleFileName()");
      abort();
  }

  filename = get_filename_ptr(exe_path);
  set_mcpu_option(filename, mcpu_option, sizeof(mcpu_option));
  change_exec_path_filename(filename, mcpu_option);
  cmdline = get_cmdline(argc, (const char **) argv, exe_path, mcpu_option);
  execute_cmdline(cmdline);

  if (cmdline) {
    free(cmdline);
  }
  return 0;
}

static char *get_filename_ptr(const char *exe_path) {
  char *filename = strrchr(exe_path, '\\');
  if(filename == NULL) {
    fprintf(stderr, "Wrong path, can't extract filename");
    abort();
  }

  if (strlen(filename) < strlen(GDB_BASE_FILENAME)) {
    fprintf(stderr, "Filename is too short. Expected \"%s\"", GDB_FILENAME_EXAMPLE);
    abort();
  }

  return filename++;
}

static void set_mcpu_option(const char *filename, char *mcpu, const size_t mcpu_size) {
  char *mcpu_start = strchr(filename, '-');;
  char *mcpu_end = strchr(&filename[strlen(GDB_FILENAME_PREFIX)], '-');
  size_t len_to_write = 0;

  if (mcpu_start == NULL || mcpu_end == NULL) {
    fprintf(stderr, "Wrong filename format. Expected \"%s\"", GDB_FILENAME_EXAMPLE);
    abort();
  }
  mcpu_start++;

  len_to_write = mcpu_end - mcpu_start + strlen(MCPU_PREFIX);
  if (mcpu_size < len_to_write) {
    fprintf(stderr, "insufficient buffer size\n");
    abort();
  }
  snprintf(mcpu, len_to_write, "%s%s", MCPU_PREFIX, mcpu_start);

  return;
}

// Expecting that filename length is grater than filename which we are going to
// execute, because original gdb name is like "xtensa-esp-elf-gdb.exe" but
// we have "xtensa-espWHATEVER-elf-gdb.exe"
static void change_exec_path_filename(char *filename, const char const * mcpu_option) {
  size_t chars_to_remove = strlen(mcpu_option) - strlen(MCPU_PREFIX) - strlen(MCPU_BASE);
  char *start = filename + strlen(GDB_FILENAME_PREFIX) + 1;
  size_t chars_to_move = strlen(start) - chars_to_remove + 1;
  memmove(start, start + chars_to_remove, chars_to_move);
}

static char *get_cmdline(const int argc, const char **argv, const char *exe_path, const char * mcpu_option) {
  char * cmdline = calloc(1, strlen(exe_path) + strlen(mcpu_option) + 2);
  if (cmdline == NULL) {
    perror("calloc");
    abort();
  }
  sprintf(cmdline, "%s %s", exe_path, mcpu_option);

  for (int i = 1; i < argc; i++) {
    size_t cur_len = strlen(cmdline);
    cmdline = realloc(cmdline, cur_len + strlen(argv[i]) + 2);
    if (cmdline == NULL) {
      perror("realloc");
      abort();
    }
    sprintf(&cmdline[cur_len], " %s", argv[i]);
  }
  return cmdline;
}

static void execute_cmdline(const char *cmdline) {
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  if (!CreateProcessA(NULL, // No module name (use command line)
       (char *) cmdline,    // Command line
       NULL,                // Process handle not inheritable
       NULL,                // Thread handle not inheritable
       FALSE,               // Set handle inheritance to FALSE
       0,                   // No creation flags
       NULL,                // Use parent's environment block
       NULL,                // Use parent's starting directory
       &si,                 // Pointer to STARTUPINFO structure
       &pi)                 // Pointer to PROCESS_INFORMATION structure
     ) {
    fprintf(stderr, "Can't execute \"%s\"\nError: %s", cmdline, GetLastError());
    abort();
  }

  // Wait until child process exits.
  WaitForSingleObject(pi.hProcess, INFINITE);

  // Close process and thread handles.
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
}
