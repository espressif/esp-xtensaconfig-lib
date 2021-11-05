#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "target.h"

/* Returns GCC's CLI option value */
const char *xtensaconfig_get_option(void)
{
	//return xtensaconfig_string;
	return global_options.x_xtensaconfig_string;
}

#ifdef __cplusplus
} //extern "C"
#endif
