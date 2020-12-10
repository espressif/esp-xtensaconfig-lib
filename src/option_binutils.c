// binutils related things

//const char *xtensa_config_path;

const char *xtensaconfig_string;

const char *xtensaconfig_get_option(void)
{
	// return gcc_opt_xtensa_config;
	return xtensaconfig_string;
}
