/* This file is generated from prc_rename.def by genrename. */

#ifndef TOPPERS_PRC_RENAME_H
#define TOPPERS_PRC_RENAME_H

/*
 *  prc_config.c
 */
#define idf							_kernel_idf
#define iipm						_kernel_iipm
#define inh_tbl						_kernel_inh_tbl
#define iipm_mask_tbl				_kernel_iipm_mask_tbl
#define inh_iipm_tbl				_kernel_inh_iipm_tbl
#define bitpat_cfgint				_kernel_bitpat_cfgint
#define x_config_int				_kernel_x_config_int
#define exch_tbl					_kernel_exch_tbl
#define intatr_tbl					_kernel_intatr_tbl

#define prc_initialize				_kernel_prc_initialize
#define prc_terminate				_kernel_prc_terminate
#define default_int_handler			_kernel_default_int_handler
#define default_exc_handler			_kernel_default_exc_handler

/*
 *  prc_support.S
 */

#define usrexcept_entry				_kernel_usrexcept_entry
#define hwexcept_entry				_kernel_hwexcept_entry
#define interrupt_entry				_kernel_interrupt_entry

#ifdef TOPPERS_LABEL_ASM

/*
 *  prc_config.c
 */
#define _idf						__kernel_idf
#define _iipm						__kernel_iipm
#define _inh_tbl					__kernel_inh_tbl
#define _iipm_mask_tbl				__kernel_iipm_mask_tbl
#define _inh_iipm_tbl				__kernel_inh_iipm_tbl
#define _bitpat_cfgint				__kernel_bitpat_cfgint
#define _x_config_int				__kernel_x_config_int
#define _exch_tbl					__kernel_exch_tbl
#define _intatr_tbl					__kernel_intatr_tbl

#define _prc_initialize				__kernel_prc_initialize
#define _prc_terminate				__kernel_prc_terminate
#define _default_int_handler		__kernel_default_int_handler
#define _default_exc_handler		__kernel_default_exc_handler

/*
 *  prc_support.S
 */

#define _usrexcept_entry			__kernel_usrexcept_entry
#define _hwexcept_entry				__kernel_hwexcept_entry
#define _interrupt_entry			__kernel_interrupt_entry

#endif /* TOPPERS_LABEL_ASM */


#endif /* TOPPERS_PRC_RENAME_H */
