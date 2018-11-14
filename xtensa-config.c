/*
 * Copyright (c) 2017 Cadence Design Systems Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdlib.h>
#define XTENSA_CONFIG_DEFINITION
#include <xtensa-config.h>
#include <xtensa-dynconfig.h>

#undef XTENSA_CONFIG_ENTRY
#define XTENSA_CONFIG_ENTRY(a) a
struct xtensa_config xtensa_config = XTENSA_CONFIG_INITIALIZER;

#define STRINGIFY1(a) #a
#define STRINGIFY(a) STRINGIFY1(a)

#undef XTENSA_CONFIG_ENTRY
#define XTENSA_CONFIG_ENTRY(a) "__" #a "=" STRINGIFY(a)
const char *xtensa_config_strings[] = {
	XTENSA_CONFIG_ENTRY_LIST,
	NULL,
};

int plugin_is_GPL_compatible;
