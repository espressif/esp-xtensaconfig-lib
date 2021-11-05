// GDB related things

#ifdef __cplusplus
extern "C" {
#endif

const char *xtensaconfig_string;

const char *xtensaconfig_get_option(void)
{
	// return gcc_opt_xtensa_config;
	return xtensaconfig_string;
}

#ifdef __cplusplus
} //extern "C"
#endif
