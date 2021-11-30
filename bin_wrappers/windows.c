#include <stdio.h>
#include <windows.h>

#define MCPU_MAX_LEN 16
#define MCPU_PREFIX "--mcpu="
#define MCPU_BASE "esp"

#define GDB_FILENAME_PREFIX "xtensa-" MCPU_BASE
#define GDB_BASE_FILENAME "xtensa-" MCPU_BASE "-elf-gdb.exe"
#define GDB_FILENAME_EXAMPLE "xtensa-" MCPU_BASE "XXX-elf-gdb.exe"

#define PYTHON_VERSION_BUFFER_SIZE 32
#define PYTHON_SCRIPT_CMD_OPTION " -c "
#define PYTHON_SCRIPT_GET_VERSION "\"import sys; print(\\\"{}.{}\\\".format(sys.version_info.major, sys.version_info.minor))\""
#define REDIRECT_STDERR_TO_NULL " 2>nul"
#define PYTHON_MAJOR_WITH_DOT "3."

static char * getModuleFileName(size_t append_memory_size);
static char *get_filename_ptr(const char *exe_path);
static void set_mcpu_option(const char *filename, char *mcpu, const size_t mcpu_size);
static char *get_exe_path_and_mcpu_option(char *mcpu_option, const size_t mcpu_option_size);
static char *get_cmdline(const int argc, const char **argv, const char *exe_path, const char * mcpu_option);
static char *get_python_version(void);
static void execute_cmdline(const char *cmdline);

const char *python_exe_arr[] = {"python", "python3"};

int main (int argc, char **argv) {
  char *cmdline = NULL;
  char *exe_path = NULL;
  char mcpu_option[MCPU_MAX_LEN] = {0};

  exe_path = get_exe_path_and_mcpu_option(mcpu_option, sizeof(mcpu_option));
  cmdline = get_cmdline(argc, (const char **) argv, exe_path, mcpu_option);
  execute_cmdline(cmdline);

  free(exe_path);
  free(cmdline);
  return 0;
}

static char * getModuleFileName(size_t append_memory_size) {
  LPTSTR exe_path;
  DWORD exe_path_size;
  DWORD res;

  exe_path_size = PATH_MAX;
  exe_path = malloc(exe_path_size);
  if (exe_path == NULL) {
    perror("malloc()");
    abort();
  }

  // For some reasons it could be a path greater than PATH_MAX,
  // so allocate more memory until it fits to the buffer
  while ((res = GetModuleFileName(0, exe_path, exe_path_size)) &&
       (GetLastError() == ERROR_INSUFFICIENT_BUFFER)) {
      exe_path_size *= 2;
      exe_path = realloc(exe_path, exe_path_size);
      if (exe_path == NULL) {
         perror("realloc()");
         abort();
      }
  }

  if (!res) {
    perror("GetModuleFileName()");
    abort();
  }

  // resize buffer if need for future usage
  if (exe_path_size < strlen(exe_path) + append_memory_size + 1) {
    exe_path_size = strlen(exe_path) + append_memory_size + 1;
    exe_path = realloc(exe_path, exe_path_size);
    if (exe_path == NULL) {
      perror("realloc()");
      abort();
    }
  }
  return exe_path;
}

static char *get_filename_ptr(const char *exe_path) {
  char *filename = strrchr(exe_path, '\\');
  if(filename == NULL) {
    fprintf(stderr, "Wrong path, can't extract filename (\"%s\")", exe_path);
    abort();
  }

  if (strlen(filename) - 1 < strlen(GDB_BASE_FILENAME)) {
    fprintf(stderr, "Filename is too short. Expected \"%s\"", GDB_FILENAME_EXAMPLE);
    abort();
  }

  return filename++;
}

static void set_mcpu_option(const char *filename, char *mcpu, const size_t mcpu_option_size) {
  char *mcpu_start = strchr(filename, '-');
  char *mcpu_end = strchr(&filename[strlen(GDB_FILENAME_PREFIX)], '-');
  size_t len_to_write = 0;

  if (mcpu_start == NULL || mcpu_end == NULL) {
    fprintf(stderr, "Wrong filename format. Expected \"%s\"", GDB_FILENAME_EXAMPLE);
    abort();
  }
  mcpu_start++;

  len_to_write = mcpu_end - mcpu_start + strlen(MCPU_PREFIX);
  if (mcpu_option_size < len_to_write) {
    fprintf(stderr, "insufficient buffer size\n");
    abort();
  }
  snprintf(mcpu, len_to_write, "%s%s", MCPU_PREFIX, mcpu_start);

  return;
}

static char * get_exe_path_and_mcpu_option(char *mcpu_option, const size_t mcpu_option_size) {
  char *exe_path;
  char *filename = NULL;
  char *python_version = NULL;
  size_t chars_to_remove = 0;
  char *start = NULL;
  size_t chars_to_move = 0;

  python_version = get_python_version();
  exe_path = getModuleFileName(strlen(python_version));
  filename = get_filename_ptr(exe_path);
  set_mcpu_option(filename, mcpu_option, mcpu_option_size);

  // Remove esp mcpu
  chars_to_remove = strlen(mcpu_option) - strlen(MCPU_PREFIX) - strlen(MCPU_BASE);
  start = filename + strlen(GDB_FILENAME_PREFIX) + 1;
  chars_to_move = strlen(start) - chars_to_remove + 1;
  memmove(start, start + chars_to_remove, chars_to_move);

  // Add python version postfix if exists
  if (strlen(python_version) == 0) {
    // Notify user he/she is using GDB without python support
    printf("Without python\r\n");
    fflush(stdout);
    return exe_path;
  }

  // insert python_version to the filename
  // don't worry about buffer overflow, additionall memory for python version
  // was allocated in getModuleFileName()
  start = strrchr(filename, '.');
  chars_to_move = strlen(python_version) + 1;
  memmove(start + chars_to_move, start, strlen(filename) - strlen(start) + 1);
  snprintf(start, chars_to_move, "-%s", python_version);

  // Notify user he/she is using GDB with python support
  printf("With python-%s\r\n", python_version);
  fflush(stdout);
  return exe_path;
}

static char *get_cmdline(const int argc, const char **argv, const char *exe_path, const char * mcpu_option) {
  char * cmdline = (char *)malloc(strlen(exe_path) + strlen(mcpu_option) + 2);
  if (cmdline == NULL) {
    perror("malloc");
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

static char *get_python_version(void) {
  static char python_version[PYTHON_VERSION_BUFFER_SIZE] = {0};
  char *tmp = NULL;
  const size_t python_exe_arr_size = sizeof(python_exe_arr) / sizeof(python_exe_arr[0]);
  size_t i = 0;
  for (i = 0; i < python_exe_arr_size; i++) {
    FILE* pipe = NULL;
    char *python_cmd = malloc(strlen(python_exe_arr[i]) +
                              strlen(PYTHON_SCRIPT_CMD_OPTION) +
                              strlen(PYTHON_SCRIPT_GET_VERSION) +
                              strlen(REDIRECT_STDERR_TO_NULL) +
                              1);
    sprintf(python_cmd, "%s%s%s%s", python_exe_arr[i],
                                    PYTHON_SCRIPT_CMD_OPTION,
                                    PYTHON_SCRIPT_GET_VERSION,
                                    REDIRECT_STDERR_TO_NULL);

    pipe = popen(python_cmd, "r");
    if (pipe == NULL) {
      free(python_cmd);
      continue;
    }

    fgets(python_version, sizeof(python_version), pipe);
    pclose(pipe);
    free(python_cmd);

    if (strncmp(python_version, PYTHON_MAJOR_WITH_DOT, strlen(PYTHON_MAJOR_WITH_DOT)) == 0) {
      break;
    } else {
      python_version[0] = 0;
    }
  }

  // find newline character and override it with \0
  tmp = strchr(python_version, '\n');
  if (tmp != NULL) {
    *tmp = '\0';
  }

  return python_version;
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
