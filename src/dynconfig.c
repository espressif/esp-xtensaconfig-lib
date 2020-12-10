#include "config.h"

#include <linux/limits.h>
#include <unistd.h>
#include <libgen.h>
#include <alloca.h>
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

#define PROC_PATH_MAX 32

//TODO just for researching, when we don't have the ENABLE_PLUGIN define
//#if !defined(ENABLE_PLUGIN) && !defined(BFD_SUPPORTS_PLUGINS)
//#error "ENABLE_PLUGIN is not defined"
//#endif

// Logging

const char *esp_log_proc(void)
{
  static char s_path[PATH_MAX] = {};

  if(s_path[0] != '\x0')
  {
    return s_path;
  }

  char proc_path[PROC_PATH_MAX];
  sprintf(proc_path, "/proc/%d/exe", getpid());
  ssize_t written = readlink(proc_path, s_path, sizeof(s_path));
  if(written < 1)
  {
      perror("readlink()");
      abort();
  }
  if((size_t)written >= sizeof(s_path))
  {
      written = sizeof(s_path) - 1;
  }
  s_path[written] = '\0';

  return s_path;
}

const char *esp_log_cmdline(void)
{
  static char s_cmdline[PATH_MAX * 4] = {};

  if(s_cmdline[0] != '\x0')
  {
    return s_cmdline;
  }

  char proc_path[PROC_PATH_MAX];
  sprintf(proc_path, "/proc/%d/cmdline", getpid());
  //sprintf(proc_path, "/proc/%d/cmdline", 21684);
  FILE *fp = fopen(proc_path, "r");
  if(fp == NULL)
  {
    perror("fopen()");
    abort();
  }

  int ch = 0;
  size_t len_used = 0;
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

static struct xtensa_config s_dummy_config = XTENSA_CONFIG_INITIALIZER;

#undef XTENSA_CONFIG_ENTRY
#define XTENSA_CONFIG_ENTRY(a) "__" #a "=" STRINGIFY(a)

static const char *s_dummy_strings[] = {
    XTENSA_CONFIG_ENTRY_LIST,
    NULL,
};

#if !defined (HAVE_DLFCN_H) && defined (_WIN32)

#define RTLD_LAZY 0      /* Dummy value.  */

static void *
dlopen (const char *file, int mode ATTRIBUTE_UNUSED)
{
  return LoadLibrary (file);
}

static void *
dlsym (void *handle, const char *name)
{
  return (void *) GetProcAddress ((HMODULE) handle, name);
}

static int ATTRIBUTE_UNUSED
dlclose (void *handle)
{
  FreeLibrary ((HMODULE) handle);
  return 0;
}

static const char *
dlerror (void)
{
  return "Unable to load DLL.";
}

#endif /* !defined (HAVE_DLFCN_H) && defined (_WIN32)  */

const char *xtensaconfig_get_option(void);

static void get_library_directory(char * const libdir, const size_t libdir_bufsz);

const void *xtensa_load_config (const char *symbol, void *dummy_data)
{
  // If initialized, it means that:
  //    s_handle - was loaded correctly,
  //    s_lib_file - has a correct path
  static int s_init = 0;
  static void *s_handle = 0;
  static char s_lib_file[PATH_MAX] = {};

  // While we are having an uninitialized command line option,
  // use dummy data to keep working,
  // but not initialize s_handle
  if (xtensaconfig_get_option() == NULL)
  {
    ESP_LOG_INFO("Uninitialized DYN, symbol: %s", symbol);
    return dummy_data;
  }

  // GCC uses default value if command line option hasn't be set (see xtensa.opt)
  if (strcmp("default", xtensaconfig_get_option()) == 0)
  {
    ESP_LOG_INFO("Default DYN, symbol: %s", symbol);
    return dummy_data;
  }

  // Load config from dynamic library
  if (!s_init)
  {
    //TODO window's 'DLL'

    ESP_LOG_INFO("Use \'%s\' config for %s symbol", xtensaconfig_get_option(), symbol);

    get_library_directory(s_lib_file, sizeof(s_lib_file));
    //"xtensaconfig-esp32.so", see xtensaconfig/Makefile
    strcat(s_lib_file, "xtensaconfig-");
    strcat(s_lib_file, xtensaconfig_get_option());
    strcat(s_lib_file, ".so");

    s_handle = dlopen (s_lib_file, RTLD_NOW);
    if (!s_handle)
    {
      ESP_LOG_ERR("Lib \"%s\" cannot be loaded: %s", s_lib_file, dlerror());
      abort ();
    }

    ESP_LOG_INFO("Use lib: \"%s\" for a symbol %s", s_lib_file, symbol);
    s_init = 1;
  }

  const void *p = dlsym (s_handle, symbol);
  if (!p)
  {
    ESP_LOG_ERR("Symbol \"%s\" cannot be found in \"%s\": %s", symbol, s_lib_file, dlerror());
    abort ();
  }

  return p;
}

struct xtensa_config *xtensa_get_config (int opt_dbg)
{
  static struct xtensa_config *config;

  ESP_LOG_TRACE("DYN: %s, OPT: %3d", xtensaconfig_get_option(), opt_dbg);
  if (config)
  {
    return config;
  }

  struct xtensa_config *vals = (struct xtensa_config *) xtensa_load_config ("xtensa_config", &s_dummy_config);
  if (vals->config_size < sizeof(struct xtensa_config))
  {
    ESP_LOG_ERR("Old or incompatible configuration is loaded: config_size = %ld, expected: %ld",
      config->config_size, sizeof (struct xtensa_config));
    abort ();
  }

  return vals;
}

const char **xtensa_get_config_strings (void)
{
  static const char **config_strings;

  ESP_LOG_INFO("DYN: %s", xtensaconfig_get_option());
  if (config_strings)
  {
    return config_strings;
  }

  const char ** strs = (const char **) xtensa_load_config ("xtensa_config_strings", &s_dummy_strings);
  if(strs != s_dummy_strings)
  {
    config_strings = strs;
  }

  return strs;
}

static void get_library_directory(char * const libdir, const size_t libdir_bufsz)
{
    char proc_path[PROC_PATH_MAX];
    sprintf(proc_path, "/proc/%d/exe", getpid());

    // an executable file path
    ssize_t written = readlink(proc_path, libdir, libdir_bufsz);
    if(written < 1)
    {
        perror("readlink()");
        abort();
    }
    if((size_t)written >= libdir_bufsz)
    {
        written = libdir_bufsz - 1;
    }
    libdir[written] = '\0';

    // an executable file directory
    char *tmp = strdupa(libdir); //todo warning: stack usage might be unbounded
    char *dname = dirname(tmp);
    if(strlen(dname) > libdir_bufsz - 1)
    {
        fprintf(stderr, "insufficient buffer size\n");
        abort();
    }
    strcpy(libdir, dname);

    // a relative library directory
    static const char lib_suffix[] = "/../lib/";
    if(strlen(libdir) + sizeof(lib_suffix) > libdir_bufsz)
    {
        fprintf(stderr, "insufficient buffer size\n");
        abort();
    }
    strcat(libdir, "/../lib/");
}

