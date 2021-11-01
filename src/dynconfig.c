#include "config.h"

#ifdef __linux__
#include <linux/limits.h>
#elif __APPLE__
#include <sys/syslimits.h>
#include <crt_externs.h>
#include <mach-o/dyld.h>
#endif

#include <stdint.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define _STRINGIFY(a) #a
#define STRINGIFY(a) _STRINGIFY(a)

#define XTENSA_CONFIG_DEFINITION
#include "xtensa-config.h"
#include "xtensaconfig/dynconfig.h"

#if defined (HAVE_DLFCN_H)
#include <dlfcn.h>
#elif defined (_WIN32)
#include <windows.h>
#define ENABLE_PLUGIN
#endif

#ifdef __linux__
#define PROC_PATH_MAX 32
#endif

#ifndef _WIN32
  #define LIB_SUFFIX_DIR "/../lib/"
#else
  #define LIB_SUFFIX_DIR "\\..\\lib\\"
#endif

static struct xtensa_config s_dummy_config = XTENSA_CONFIG_INITIALIZER;

#undef XTENSA_CONFIG_ENTRY
#define XTENSA_CONFIG_ENTRY(a) "__" #a "=" STRINGIFY(a)

static const char *s_dummy_strings[] = {
    XTENSA_CONFIG_ENTRY_LIST,
    NULL,
};

static int snprintf_or_abort(char *str, size_t size, const char *fmt, ...);
static void get_library_directory(char *libdir, size_t libdir_size);
static void get_path_to_executable(char *path, size_t path_size);
static void xtensa_load_shared_lib(void **s_handle, const char *xtensaconfig_option);

static struct xtensa_config *dynconfig = NULL;

void xtensa_reset_config(void)
{
  dynconfig = NULL;
}

//TODO just for researching, when we don't have the ENABLE_PLUGIN define
//#if !defined(ENABLE_PLUGIN) && !defined(BFD_SUPPORTS_PLUGINS)
//#error "ENABLE_PLUGIN is not defined"
//#endif

// Logging
const char *esp_log_proc(void)
{
  static char s_path[PATH_MAX] = {0};

  if(s_path[0] != '\x0')
  {
    return s_path;
  }
  get_path_to_executable(s_path, sizeof(s_path));
  return s_path;
}

const char *esp_log_cmdline(void)
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
    perror("fopen()");
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
    perror("GetCommandLineA()");
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

void esp_log_write(int level, const char* format, ...)
{
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

  va_list list;
  va_start(list, format);
  vfprintf(stderr, format, list);
  va_end(list);
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

static void xtensa_load_shared_lib(void **s_handle, const char *xtensaconfig_option)
{
  size_t curr_size = 0;
  char s_lib_file [PATH_MAX] = {0};

  get_library_directory(s_lib_file, PATH_MAX);
  curr_size = strlen(s_lib_file);
  snprintf_or_abort(&s_lib_file[curr_size], PATH_MAX - curr_size, "xtensaconfig-%s.so", xtensaconfig_option);
  *s_handle = dlopen (s_lib_file, RTLD_NOW);

  if (!(*s_handle))
  {
    ESP_LOG_ERR("Lib \"%s\" cannot be loaded: %s", s_lib_file, dlerror());
    abort ();
  }
  ESP_LOG_INFO("Lib \"%s\" loaded", s_lib_file);
}

const void *xtensa_load_config (const char *symbol, void *dummy_data)
{
  // If initialized, it means that:
  //    s_handle - was loaded correctly,
  static int s_init = 0;
  static void *s_handle = 0;
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
  if (!s_init)
  {
    ESP_LOG_INFO("Use \'%s\' config for %s symbol", xtensaconfig_option, symbol);
    xtensa_load_shared_lib(&s_handle, xtensaconfig_option);
    s_init = 1;
  }

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
  if (dynconfig)
  {
    return dynconfig;
  }

  dynconfig = (struct xtensa_config *) xtensa_load_config ("xtensa_config", &s_dummy_config);
  if (dynconfig->config_size < sizeof(struct xtensa_config))
  {
    ESP_LOG_ERR("Old or incompatible configuration is loaded: config_size = %lu, expected: %u",
      dynconfig->config_size, (uint32_t) sizeof (struct xtensa_config));
    abort ();
  }

  return dynconfig;
}

static void get_library_directory(char *libdir, size_t libdir_size)
{
    char *tmp = NULL;
    char *dname = NULL;

    get_path_to_executable(libdir, libdir_size);

    // an executable file directory
    tmp = strdup((const char *)libdir);
    if (tmp == NULL)
    {
        fprintf(stderr, "strdup failed\n");
        abort();
    }

    dname = dirname(tmp);
    // append with "/../lib/" for linux; "\..\lib\" for win
    snprintf_or_abort(libdir, libdir_size, "%s%s", dname, LIB_SUFFIX_DIR);
    free(tmp);
}

static void get_path_to_executable(char *path, size_t path_size) {
#ifdef __linux__
  char proc_path[PROC_PATH_MAX] = {0};
  ssize_t written = 0;

  snprintf_or_abort(proc_path, sizeof(proc_path), "/proc/%d/exe", getpid());
  written = readlink(proc_path, path, path_size);
  if(written < 1)
  {
      perror("readlink()");
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
      perror("GetModuleFileName()");
      abort();
  }
#elif __APPLE__
  if (_NSGetExecutablePath(path, (uint32_t*) &path_size) != 0)
  {
    perror("_NSGetExecutablePath()");
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
