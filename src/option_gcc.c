#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "target.h"

/* Returns GCC's CLI option value */
const char *xtensaconfig_get_option(void)
{
    return global_options.x_xtensaconfig_string;
}
