#include <stdint.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#ifndef _WIN32
#include <dlfcn.h>
#endif

#ifdef __linux__
#include <linux/limits.h>
#elif __APPLE__
#include <sys/syslimits.h>
#include <crt_externs.h>
#include <mach-o/dyld.h>
#elif _WIN32
#include <windows.h>
#endif

#include "xtensaconfig/dynconfig.h"

#ifdef __linux__
#define PROC_PATH_MAX 32
#endif

#ifndef _WIN32
  #define LIB_SUFFIX_DIR "/lib/"
#else
  #define LIB_SUFFIX_DIR "\\lib\\"
#endif

#define ESP_LOG_FORMAT(format) "ESP_LOG %s:%d\t\t" format " \n"
#define ESP_LOG_LEVEL(level, format, ...) do { \
        esp_log_write(level, ESP_LOG_FORMAT(format), __func__, __LINE__, ##__VA_ARGS__); \
    } while(0)
#define ESP_LOG_ERR(format, ...)      ESP_LOG_LEVEL(0, format, ##__VA_ARGS__)
#define ESP_LOG_WARN(format, ...)     ESP_LOG_LEVEL(1, format, ##__VA_ARGS__)
#define ESP_LOG_INFO(format, ...)     ESP_LOG_LEVEL(2, format, ##__VA_ARGS__)
#define ESP_LOG_DBG(format, ...)      ESP_LOG_LEVEL(3, format, ##__VA_ARGS__)
#define ESP_LOG_TRACE(format, ...)    ESP_LOG_LEVEL(4, format, ##__VA_ARGS__)
// Print even if esp_log_level hasn't initialized yet
#define ESP_LOG_ANYWAY(format, ...)   ESP_LOG_LEVEL(0, format, ##__VA_ARGS__)

static int snprintf_or_abort(char *str, size_t size, const char *fmt, ...);
static void get_library_directory(char *libdir, size_t libdir_size);
static void get_path_to_executable(char *path, size_t path_size);
static void xtensa_load_shared_lib(void **s_handle, const char *xtensaconfig_option);

static const char *esp_log_proc(void);
static const char *esp_log_cmdline(void);
static void esp_log_write(int level, const char* format, ...) __attribute__ ((format (printf, 2, 3)));
#ifdef __APPLE__
static char *apple_dirname(char *path);
#endif

static struct xtensa_config *s_dynconfig = NULL;
extern const struct xtensa_config xtensa_default_config;

void xtensa_reset_config(void)
{
  s_dynconfig = NULL;
  ESP_LOG_TRACE("Reset dynconfig");
}

// Logging
static const char *esp_log_proc(void)
{
  static char s_path[PATH_MAX] = {0};

  if(s_path[0] != '\x0')
  {
    return s_path;
  }
  get_path_to_executable(s_path, sizeof(s_path));
  return s_path;
}

static const char *esp_log_cmdline(void)
{
  static char s_cmdline[PATH_MAX * 4] = {0};

  if(s_cmdline[0] != '\x0')
  {
    return s_cmdline;
  }

#ifdef __linux__
  char proc_path[PROC_PATH_MAX] = {0};
  FILE *fp = NULL;
  size_t len_used = 0;
  int ch = 0;

  snprintf_or_abort(proc_path, sizeof(proc_path), "/proc/%d/cmdline", getpid());
  fp = fopen(proc_path, "r");
  if(fp == NULL)
  {
    fprintf(stderr, "can't open process cmdline (\"%s\")\n", proc_path);
    abort();
  }

  while((ch = fgetc(fp)) != EOF)
  {
    if(len_used >= sizeof(s_cmdline) - 1)
    {
      break;
    }

    if(ch < 32)
    {
      ch = ' ';
    }
    s_cmdline[len_used] = (char)ch;
    ++len_used;
  }
  (void)fclose(fp);
#elif _WIN32
  char *tmp = GetCommandLineA();
  if (tmp == NULL)
  {
    fprintf(stderr, "can't get process command line (\"%s\")\n", strerror(errno));
    abort();
  }

  snprintf_or_abort(s_cmdline, sizeof(s_cmdline), "%s", tmp);
#elif __APPLE__
  int Argc = 0, i = 0;
  char** Argv = NULL;
  size_t len_used = 0;
  int *Argc_ptr = _NSGetArgc();
  char ***Argv_ptr = _NSGetArgv();

  if (Argc_ptr == NULL || Argv_ptr == NULL)
  {
     fprintf(stderr, "can't get running app argc and argv\n");
     abort();
  }

  Argc = *Argc_ptr;
  Argv = *Argv_ptr;

  for (i = 0; i < Argc; i++)
  {
    len_used += snprintf_or_abort(s_cmdline + len_used,
                                  sizeof(s_cmdline) - len_used,
                                  "%s%s",
                                  i == 0 ? "" : " ", Argv[i]);
  }
#endif // __linux__

  return s_cmdline;
}

static void esp_log_write(int level, const char* format, ...)
{
  static int printed_once = 0;
  int trace = 0;
  char *trace_str = getenv ("ESP_DEBUG_TRACE");
  if(trace_str)
  {
    trace = atoi(trace_str);
  }

  if (level > trace)
  {
    return;
  }

  if (!printed_once)
  {
    fprintf(stderr, "Execution parameters:\nAPP: %s\nCMDLINE: %s\n\n", esp_log_proc(), esp_log_cmdline());
    printed_once = 1;
  }

  va_list list;
  va_start(list, format);
  vfprintf(stderr, format, list);
  va_end(list);

  fflush(stderr);
}


#if !defined (HAVE_DLFCN_H) && defined (_WIN32)

#define RTLD_NOW 0      /* Dummy value.  */

static struct {
    long lasterror;
    const char *err_rutin;
} var = {
    0,
    NULL
};

static void *dlopen (const char *filename, int flags);
static void *dlsym (void *handle, const char *name);
static const char *dlerror (void);

static void *dlopen (const char *filename, int flags)
{
    (void)flags; // make compiler happy
    HINSTANCE hInst;

    hInst = LoadLibrary(filename);
    if (hInst==NULL) {
        var.lasterror = GetLastError ();
        var.err_rutin = "dlopen";
    }
    return hInst;
}

static void *dlsym (void *handle, const char *name)
{
    FARPROC fp;

    fp = GetProcAddress ((HINSTANCE)handle, name);
    if (!fp) {
        var.lasterror = GetLastError ();
        var.err_rutin = "dlsym";
    }
    return (void *)(intptr_t)fp;
}

static const char *dlerror (void)
{
    static char errstr [PATH_MAX];

    if (!var.lasterror) {
        return NULL;
    }

    snprintf (errstr, sizeof(errstr), "%s error #%ld", var.err_rutin, var.lasterror);
    return errstr;
}

#endif /* !defined (HAVE_DLFCN_H) && defined (_WIN32)  */

static void xtensa_load_shared_lib(void **handle, const char *xtensaconfig_option)
{
  size_t curr_size = 0;
  char lib_file [PATH_MAX] = {0};

  get_library_directory(lib_file, PATH_MAX);
  curr_size = strlen(lib_file);
  snprintf_or_abort(&lib_file[curr_size], PATH_MAX - curr_size, "xtensaconfig-%s.so", xtensaconfig_option);
  *handle = dlopen (lib_file, RTLD_NOW);

  if (!(*handle))
  {
    ESP_LOG_ERR("Lib \"%s\" cannot be loaded: %s", lib_file, dlerror());
    abort ();
  }
  ESP_LOG_INFO("Lib \"%s\" loaded", lib_file);
}

const void *xtensa_load_config (const char *symbol, void *dummy_data)
{
  static void *s_handle = NULL;
  const char *xtensaconfig_option = xtensaconfig_get_option();
  const void *p = NULL;

  // While we are having an uninitialized command line option,
  // use dummy data to keep working,
  // but not initialize s_handle
  if (xtensaconfig_option == NULL)
  {
    ESP_LOG_INFO("Uninitialized DYN, symbol: %s", symbol);
    return dummy_data;
  }

  // GCC uses default value if command line option hasn't be set (see xtensa.opt)
  if (strcmp("default", xtensaconfig_option) == 0)
  {
    ESP_LOG_INFO("Default DYN, symbol: %s", symbol);
    return dummy_data;
  }

  // Load config from dynamic library
  if (s_handle == NULL)
  {
    xtensa_load_shared_lib(&s_handle, xtensaconfig_option);
  }

  ESP_LOG_INFO("Use \'%s\' config for \"%s\" symbol", xtensaconfig_option, symbol);

  p = dlsym (s_handle, symbol);
  if (!p)
  {
    ESP_LOG_ERR("Symbol \"%s\" cannot be found: %s", symbol, dlerror());
    abort ();
  }

  return p;
}

struct xtensa_config *xtensa_get_config (int opt_dbg)
{
  ESP_LOG_TRACE("DYN: %s, OPT: %3d", xtensaconfig_get_option(), opt_dbg);
  if (s_dynconfig)
  {
    ESP_LOG_TRACE("DYN: s_dynconfig->xchal_have_be %u", s_dynconfig->xchal_have_be);
    return s_dynconfig;
  }

  s_dynconfig = (struct xtensa_config *) xtensa_load_config ("xtensa_config", &xtensa_default_config);
  ESP_LOG_TRACE("Setup %s xtensa_config", (&xtensa_default_config == s_dynconfig) ? "default" : "custom");

  if (s_dynconfig->config_size < sizeof(struct xtensa_config))
  {
    ESP_LOG_ERR("Old or incompatible configuration is loaded: config_size = %lu, expected: %u",
      s_dynconfig->config_size, (uint32_t) sizeof (struct xtensa_config));
    abort ();
  }

  return s_dynconfig;
}

#ifdef __APPLE__
static char *apple_dirname(char *path)
{
  // The dirname() function returns a pointer to internal static storage space
  // that will be overwritten by subsequent calls (each function has its own
  // separate storage).
  char *dir = dirname(path);
  if (dir == NULL)
  {
    fprintf(stderr, "can't get process's (\"%s\") dirname (\"%s\")\n", path, strerror(errno));
    abort();
  }
  return dir;
}
#endif

static void get_library_directory(char *libdir, size_t libdir_size)
{
    size_t libdir_len = 0;

    get_path_to_executable(libdir, libdir_size);

    // NOTE: we assume that the executable located in /.../binutils-gdb-root-dir/bin
    //               and the libraries are located in /.../binutils-gdb-root-dir/lib
    // get /.../binutils-gdb-root-dir
#ifdef __APPLE__
    char *subdir = apple_dirname(apple_dirname(libdir));
    snprintf_or_abort(libdir, libdir_size, "%s", subdir);
#else
    libdir = dirname(dirname(libdir));
#endif
    libdir_len = strlen(libdir);
    // append with "/lib/" for linux; "\lib\" for win
    snprintf_or_abort(&libdir[libdir_len], libdir_size - libdir_len, "%s", LIB_SUFFIX_DIR);
}

static void get_path_to_executable(char *path, size_t path_size) {
#ifdef __linux__
  char proc_path[PROC_PATH_MAX] = {0};
  ssize_t written = 0;

  snprintf_or_abort(proc_path, sizeof(proc_path), "/proc/%d/exe", getpid());
  written = readlink(proc_path, path, path_size);
  if(written < 1)
  {
      fprintf(stderr, "can't open process exe (\"%s\")\n", proc_path);
      abort();
  }
  if((size_t)written >= path_size)
  {
      written = path_size - 1;
  }
  path[written] = '\0';
#elif _WIN32
  if (GetModuleFileName(0, path, path_size) == 0)
  {
      fprintf(stderr, "can't get process's exec path (\"%s\")\n", strerror(errno));
      abort();
  }
#elif __APPLE__
  if (_NSGetExecutablePath(path, (uint32_t*) &path_size) != 0)
  {
    fprintf(stderr, "can't get process's exec path (\"%s\")\n", strerror(errno));
    abort();
  }
#endif // __linux__
}

static int snprintf_or_abort(char *str, size_t size, const char *fmt, ...)
{
    int res = 0;
    va_list va;
    va_start (va, fmt);
    res = vsnprintf (str, size, fmt, va);
    va_end (va);

    if (res < 0 || (size_t) res >= size)
    {
        fprintf(stderr, "insufficient buffer size\n");
        abort();
    }
    return res;
}
