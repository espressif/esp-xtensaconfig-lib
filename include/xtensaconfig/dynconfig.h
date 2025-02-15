/* Xtensa configuration settings.
   Copyright (C) 2021 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

#ifndef XTENSA_DYNAMIC_CONFIG_H
#define XTENSA_DYNAMIC_CONFIG_H

#include <stdarg.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void xtensa_reset_config(void);
const char *xtensaconfig_get_option(void);

struct xtensa_config {
    unsigned long config_size;
    unsigned int xchal_have_be;
    unsigned int xchal_have_density;
    unsigned int xchal_have_const16;
    unsigned int xchal_have_abs;
    unsigned int xchal_have_addx;
    unsigned int xchal_have_l32r;
    unsigned int xshal_use_absolute_literals;
    unsigned int xshal_have_text_section_literals;
    unsigned int xchal_have_mac16;
    unsigned int xchal_have_mul16;
    unsigned int xchal_have_mul32;
    unsigned int xchal_have_mul32_high;
    unsigned int xchal_have_div32;
    unsigned int xchal_have_nsa;
    unsigned int xchal_have_minmax;
    unsigned int xchal_have_sext;
    unsigned int xchal_have_loops;
    unsigned int xchal_have_threadptr;
    unsigned int xchal_have_release_sync;
    unsigned int xchal_have_s32c1i;
    unsigned int xchal_have_booleans;
    unsigned int xchal_have_fp;
    unsigned int xchal_have_fp_div;
    unsigned int xchal_have_fp_recip;
    unsigned int xchal_have_fp_sqrt;
    unsigned int xchal_have_fp_rsqrt;
    unsigned int xchal_have_fp_postinc;
    unsigned int xchal_have_dfp;
    unsigned int xchal_have_dfp_div;
    unsigned int xchal_have_dfp_recip;
    unsigned int xchal_have_dfp_sqrt;
    unsigned int xchal_have_dfp_rsqrt;
    unsigned int xchal_have_windowed;
    unsigned int xchal_num_aregs;
    unsigned int xchal_have_wide_branches;
    unsigned int xchal_have_predicted_branches;
    unsigned int xchal_icache_size;
    unsigned int xchal_dcache_size;
    unsigned int xchal_icache_linesize;
    unsigned int xchal_dcache_linesize;
    unsigned int xchal_icache_linewidth;
    unsigned int xchal_dcache_linewidth;
    unsigned int xchal_dcache_is_writeback;
    unsigned int xchal_have_mmu;
    unsigned int xchal_mmu_min_pte_page_size;
    unsigned int xchal_have_debug;
    unsigned int xchal_num_ibreak;
    unsigned int xchal_num_dbreak;
    unsigned int xchal_debuglevel;
    unsigned int xchal_max_instruction_size;
    unsigned int xchal_inst_fetch_width;
    unsigned int xshal_abi;
    unsigned int xthal_abi_windowed;
    unsigned int xthal_abi_call0;
};

typedef struct xtensa_isa_internal_struct xtensa_isa_internal;

extern const void *xtensa_load_config (const char *name, const void *def);
extern struct xtensa_config *xtensa_get_config (int opt_dbg);

#ifdef XTENSA_CONFIG_DEFINITION

#ifndef XCHAL_HAVE_MUL32_HIGH
#define XCHAL_HAVE_MUL32_HIGH 0
#endif

#ifndef XCHAL_HAVE_RELEASE_SYNC
#define XCHAL_HAVE_RELEASE_SYNC 0
#endif

#ifndef XCHAL_HAVE_S32C1I
#define XCHAL_HAVE_S32C1I 0
#endif

#ifndef XCHAL_HAVE_THREADPTR
#define XCHAL_HAVE_THREADPTR 0
#endif

#ifndef XCHAL_HAVE_FP_POSTINC
#define XCHAL_HAVE_FP_POSTINC 0
#endif

#ifndef XCHAL_HAVE_DFP
#define XCHAL_HAVE_DFP 0
#endif

#ifndef XCHAL_HAVE_DFP_DIV
#define XCHAL_HAVE_DFP_DIV 0
#endif

#ifndef XCHAL_HAVE_DFP_RECIP
#define XCHAL_HAVE_DFP_RECIP 0
#endif

#ifndef XCHAL_HAVE_DFP_SQRT
#define XCHAL_HAVE_DFP_SQRT 0
#endif

#ifndef XCHAL_HAVE_DFP_RSQRT
#define XCHAL_HAVE_DFP_RSQRT 0
#endif

#ifndef XSHAL_HAVE_TEXT_SECTION_LITERALS
#define XSHAL_HAVE_TEXT_SECTION_LITERALS 0
#endif

#ifndef XCHAL_MMU_MIN_PTE_PAGE_SIZE
#define XCHAL_MMU_MIN_PTE_PAGE_SIZE 1
#endif

#define XTENSA_CONFIG_ENTRY(a) a

#define XTENSA_CONFIG_ENTRY_LIST \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_BE), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_DENSITY), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_CONST16), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_ABS), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_ADDX), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_L32R), \
    XTENSA_CONFIG_ENTRY(XSHAL_USE_ABSOLUTE_LITERALS), \
    XTENSA_CONFIG_ENTRY(XSHAL_HAVE_TEXT_SECTION_LITERALS), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_MAC16), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_MUL16), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_MUL32), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_MUL32_HIGH), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_DIV32), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_NSA), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_MINMAX), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_SEXT), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_LOOPS), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_THREADPTR), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_RELEASE_SYNC), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_S32C1I), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_BOOLEANS), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_FP), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_FP_DIV), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_FP_RECIP), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_FP_SQRT), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_FP_RSQRT), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_FP_POSTINC), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_DFP), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_DFP_DIV), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_DFP_RECIP), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_DFP_SQRT), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_DFP_RSQRT), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_WINDOWED), \
    XTENSA_CONFIG_ENTRY(XCHAL_NUM_AREGS), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_WIDE_BRANCHES), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_PREDICTED_BRANCHES), \
    XTENSA_CONFIG_ENTRY(XCHAL_ICACHE_SIZE), \
    XTENSA_CONFIG_ENTRY(XCHAL_DCACHE_SIZE), \
    XTENSA_CONFIG_ENTRY(XCHAL_ICACHE_LINESIZE), \
    XTENSA_CONFIG_ENTRY(XCHAL_DCACHE_LINESIZE), \
    XTENSA_CONFIG_ENTRY(XCHAL_ICACHE_LINEWIDTH), \
    XTENSA_CONFIG_ENTRY(XCHAL_DCACHE_LINEWIDTH), \
    XTENSA_CONFIG_ENTRY(XCHAL_DCACHE_IS_WRITEBACK), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_MMU), \
    XTENSA_CONFIG_ENTRY(XCHAL_MMU_MIN_PTE_PAGE_SIZE), \
    XTENSA_CONFIG_ENTRY(XCHAL_HAVE_DEBUG), \
    XTENSA_CONFIG_ENTRY(XCHAL_NUM_IBREAK), \
    XTENSA_CONFIG_ENTRY(XCHAL_NUM_DBREAK), \
    XTENSA_CONFIG_ENTRY(XCHAL_DEBUGLEVEL), \
    XTENSA_CONFIG_ENTRY(XCHAL_MAX_INSTRUCTION_SIZE), \
    XTENSA_CONFIG_ENTRY(XCHAL_INST_FETCH_WIDTH), \
    XTENSA_CONFIG_ENTRY(XSHAL_ABI), \
    XTENSA_CONFIG_ENTRY(XTHAL_ABI_WINDOWED), \
    XTENSA_CONFIG_ENTRY(XTHAL_ABI_CALL0)

#define XTENSA_CONFIG_INITIALIZER { \
    sizeof (struct xtensa_config), \
    XTENSA_CONFIG_ENTRY_LIST, \
}

#else /* XTENSA_CONFIG_DEFINITION */


#undef XCHAL_HAVE_BE
#define XCHAL_HAVE_BE				(xtensa_get_config (0)->xchal_have_be)

#undef XCHAL_HAVE_DENSITY
#define XCHAL_HAVE_DENSITY			(xtensa_get_config (1)->xchal_have_density)

#undef XCHAL_HAVE_CONST16
#define XCHAL_HAVE_CONST16			(xtensa_get_config (2)->xchal_have_const16)

#undef XCHAL_HAVE_ABS
#define XCHAL_HAVE_ABS				(xtensa_get_config (3)->xchal_have_abs)

#undef XCHAL_HAVE_ADDX
#define XCHAL_HAVE_ADDX			(xtensa_get_config (4)->xchal_have_addx)

#undef XCHAL_HAVE_L32R
#define XCHAL_HAVE_L32R			(xtensa_get_config (5)->xchal_have_l32r)

#undef XSHAL_USE_ABSOLUTE_LITERALS
#define XSHAL_USE_ABSOLUTE_LITERALS		(xtensa_get_config (6)->xshal_use_absolute_literals)

#undef XSHAL_HAVE_TEXT_SECTION_LITERALS
#define XSHAL_HAVE_TEXT_SECTION_LITERALS 	(xtensa_get_config (7)->xshal_have_text_section_literals)

#undef XCHAL_HAVE_MAC16
#define XCHAL_HAVE_MAC16			(xtensa_get_config (8)->xchal_have_mac16)

#undef XCHAL_HAVE_MUL16
#define XCHAL_HAVE_MUL16			(xtensa_get_config (9)->xchal_have_mul16)

#undef XCHAL_HAVE_MUL32
#define XCHAL_HAVE_MUL32			(xtensa_get_config (10)->xchal_have_mul32)

#undef XCHAL_HAVE_MUL32_HIGH
#define XCHAL_HAVE_MUL32_HIGH			(xtensa_get_config (11)->xchal_have_mul32_high)

#undef XCHAL_HAVE_DIV32
#define XCHAL_HAVE_DIV32			(xtensa_get_config (12)->xchal_have_div32)

#undef XCHAL_HAVE_NSA
#define XCHAL_HAVE_NSA				(xtensa_get_config (13)->xchal_have_nsa)

#undef XCHAL_HAVE_MINMAX
#define XCHAL_HAVE_MINMAX			(xtensa_get_config (14)->xchal_have_minmax)

#undef XCHAL_HAVE_SEXT
#define XCHAL_HAVE_SEXT			(xtensa_get_config (15)->xchal_have_sext)

#undef XCHAL_HAVE_LOOPS
#define XCHAL_HAVE_LOOPS			(xtensa_get_config (16)->xchal_have_loops)

#undef XCHAL_HAVE_THREADPTR
#define XCHAL_HAVE_THREADPTR			(xtensa_get_config (17)->xchal_have_threadptr)

#undef XCHAL_HAVE_RELEASE_SYNC
#define XCHAL_HAVE_RELEASE_SYNC		(xtensa_get_config (18)->xchal_have_release_sync)

#undef XCHAL_HAVE_S32C1I
#define XCHAL_HAVE_S32C1I			(xtensa_get_config (19)->xchal_have_s32c1i)

#undef XCHAL_HAVE_BOOLEANS
#define XCHAL_HAVE_BOOLEANS			(xtensa_get_config (20)->xchal_have_booleans)

#undef XCHAL_HAVE_FP
#define XCHAL_HAVE_FP				(xtensa_get_config (21)->xchal_have_fp)

#undef XCHAL_HAVE_FP_DIV
#define XCHAL_HAVE_FP_DIV			(xtensa_get_config (22)->xchal_have_fp_div)

#undef XCHAL_HAVE_FP_RECIP
#define XCHAL_HAVE_FP_RECIP			(xtensa_get_config (23)->xchal_have_fp_recip)

#undef XCHAL_HAVE_FP_SQRT
#define XCHAL_HAVE_FP_SQRT			(xtensa_get_config (24)->xchal_have_fp_sqrt)

#undef XCHAL_HAVE_FP_RSQRT
#define XCHAL_HAVE_FP_RSQRT			(xtensa_get_config (25)->xchal_have_fp_rsqrt)

#undef XCHAL_HAVE_FP_POSTINC
#define XCHAL_HAVE_FP_POSTINC			(xtensa_get_config (26)->xchal_have_fp_postinc)

#undef XCHAL_HAVE_DFP
#define XCHAL_HAVE_DFP				(xtensa_get_config (27)->xchal_have_dfp)

#undef XCHAL_HAVE_DFP_DIV
#define XCHAL_HAVE_DFP_DIV			(xtensa_get_config (28)->xchal_have_dfp_div)

#undef XCHAL_HAVE_DFP_RECIP
#define XCHAL_HAVE_DFP_RECIP			(xtensa_get_config (29)->xchal_have_dfp_recip)

#undef XCHAL_HAVE_DFP_SQRT
#define XCHAL_HAVE_DFP_SQRT			(xtensa_get_config (30)->xchal_have_dfp_sqrt)

#undef XCHAL_HAVE_DFP_RSQRT
#define XCHAL_HAVE_DFP_RSQRT			(xtensa_get_config (31)->xchal_have_dfp_rsqrt)

#undef XCHAL_HAVE_WINDOWED
#define XCHAL_HAVE_WINDOWED			(xtensa_get_config (32)->xchal_have_windowed)

#undef XCHAL_NUM_AREGS
#define XCHAL_NUM_AREGS			(xtensa_get_config (33)->xchal_num_aregs)

#undef XCHAL_HAVE_WIDE_BRANCHES
#define XCHAL_HAVE_WIDE_BRANCHES		(xtensa_get_config (34)->xchal_have_wide_branches)

#undef XCHAL_HAVE_PREDICTED_BRANCHES
#define XCHAL_HAVE_PREDICTED_BRANCHES		(xtensa_get_config (35)->xchal_have_predicted_branches)


#undef XCHAL_ICACHE_SIZE
#define XCHAL_ICACHE_SIZE			(xtensa_get_config (36)->xchal_icache_size)

#undef XCHAL_DCACHE_SIZE
#define XCHAL_DCACHE_SIZE			(xtensa_get_config (37)->xchal_dcache_size)

#undef XCHAL_ICACHE_LINESIZE
#define XCHAL_ICACHE_LINESIZE			(xtensa_get_config (38)->xchal_icache_linesize)

#undef XCHAL_DCACHE_LINESIZE
#define XCHAL_DCACHE_LINESIZE			(xtensa_get_config (39)->xchal_dcache_linesize)

#undef XCHAL_ICACHE_LINEWIDTH
#define XCHAL_ICACHE_LINEWIDTH		(xtensa_get_config (40)->xchal_icache_linewidth)

#undef XCHAL_DCACHE_LINEWIDTH
#define XCHAL_DCACHE_LINEWIDTH		(xtensa_get_config (41)->xchal_dcache_linewidth)

#undef XCHAL_DCACHE_IS_WRITEBACK
#define XCHAL_DCACHE_IS_WRITEBACK		(xtensa_get_config (42)->xchal_dcache_is_writeback)


#undef XCHAL_HAVE_MMU
#define XCHAL_HAVE_MMU				(xtensa_get_config (43)->xchal_have_mmu)

#undef XCHAL_MMU_MIN_PTE_PAGE_SIZE
#define XCHAL_MMU_MIN_PTE_PAGE_SIZE		(xtensa_get_config (44)->xchal_mmu_min_pte_page_size)


#undef XCHAL_HAVE_DEBUG
#define XCHAL_HAVE_DEBUG			(xtensa_get_config (45)->xchal_have_debug)

#undef XCHAL_NUM_IBREAK
#define XCHAL_NUM_IBREAK			(xtensa_get_config (46)->xchal_num_ibreak)

#undef XCHAL_NUM_DBREAK
#define XCHAL_NUM_DBREAK			(xtensa_get_config (47)->xchal_num_dbreak)

#undef XCHAL_DEBUGLEVEL
#define XCHAL_DEBUGLEVEL			(xtensa_get_config (48)->xchal_debuglevel)


#undef XCHAL_MAX_INSTRUCTION_SIZE
#define XCHAL_MAX_INSTRUCTION_SIZE		(xtensa_get_config (49)->xchal_max_instruction_size)

#undef XCHAL_INST_FETCH_WIDTH
#define XCHAL_INST_FETCH_WIDTH		(xtensa_get_config (50)->xchal_inst_fetch_width)


#undef XSHAL_ABI
#define XSHAL_ABI				(xtensa_get_config (51)->xshal_abi)

#undef XTHAL_ABI_WINDOWED
#define XTHAL_ABI_WINDOWED			(xtensa_get_config (52)->xthal_abi_windowed)

#undef XTHAL_ABI_CALL0
#define XTHAL_ABI_CALL0			(xtensa_get_config (53)->xthal_abi_call0)

#endif /* XTENSA_CONFIG_DEFINITION */

#ifdef __cplusplus
}
#endif
#endif /* !XTENSA_DYNAMIC_CONFIG_H */
