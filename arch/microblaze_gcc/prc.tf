$ 
$     パス2のアーキテクチャ依存テンプレート（Microblaze用）
$ 

$ 
$ 有効なCPU例外ハンドラ番号
$ 
$EXCNO_VALID = { 0,1,2,3,4,6,7,16,17,18,19 }$

$ 
$  DEF_EXCで使用できるCPU例外ハンドラ番号
$ 
$EXCNO_DEFEXC_VALID = EXCNO_VALID$

$ 
$  ターゲット非依存部に含まれる標準の割込み管理機能の初期化処理を用いない
$ 
$OMIT_INITIALIZE_EXCEPTION = 1$

$ 
$  有効な割込み番号，割込みハンドラ番号，CPU例外ハンドラ番号
$ 
$INTNO_VALID = { 0,1,...,31 }$
$INHNO_VALID = INTNO_VALID$

$ 
$  ATT_ISRで使用できる割込み番号とそれに対応する割込みハンドラ番号
$ 
$INTNO_ATTISR_VALID = INTNO_VALID$
$INHNO_ATTISR_VALID = INHNO_VALID$

$ 
$  DEF_INTで使用できる割込みハンドラ番号
$ 
$INHNO_DEFINH_VALID = INHNO_VALID$

$ 
$  チェック方法の指定
$ 
$CHECK_STKSZ_ALIGN = 4$

$ 
$  CFG_INTで使用できる割込み番号と割込み優先度
$ 
$INTNO_CFGINT_VALID  = INTNO_VALID$
$INTPRI_CFGINT_VALID = { -1,-2,...,-7 }$

$ 
$  標準テンプレートファイルのインクルード
$ 
$INCLUDE "kernel/kernel.tf"$

$ 
$  例外ハンドラテーブル
$ 
$FILE "kernel_cfg.c"$
$NL$
const FP _kernel_exch_tbl[TNUM_EXCH] = {$NL$
$FOREACH excno {0,1,...,6}$ 
	$IF LENGTH(EXC.EXCNO[excno])$
		$TAB$(FP)($EXC.EXCHDR[excno]$),
	$ELSE$
		$TAB$(FP)(_kernel_default_exc_handler),
	$END$
	$SPC$$FORMAT("/* %d */", +excno)$$NL$
$END$
};$NL$
$NL$

$ 
$  割込み優先度テーブル（内部表現）
$ 
$FILE "kernel_cfg.c"$
$NL$
const uint8_t _kernel_inh_iipm_tbl[TNUM_INH] = {$NL$
$FOREACH inhno INHNO_VALID$ 
	$IF LENGTH(INH.INHNO[inhno])$
	  $TAB$UINT8_C($-INT.INTPRI[inhno]$),
	$ELSE$
	  $TAB$UINT8_C(0),
	$END$
	$SPC$$FORMAT("/* %d */", +inhno)$$NL$
$END$
};$NL$
$NL$

$ 
$  割込みマスクテーブル
$ 
const uint32_t _kernel_iipm_mask_tbl[TNUM_INT_PRI]={$NL$
$FOREACH intpri { 0,-1,...,-7 }$
 $intmask = 0$
 $FOREACH intno (INT.ID_LIST)$
  $IF INT.INTPRI[intno] >= intpri $
	$intmask = intmask | (1 << (INT.INTNO[intno]))$
  $END$
 $END$
 $TAB$UINT32_C($FORMAT("0x%08x", intmask)$),/* Priority $+intpri$ */$NL$
$END$
$NL$
};$NL$


$ 
$  割込みハンドラテーブル
$ 
$NL$
const FP _kernel_inh_tbl[TNUM_INH] = {$NL$
$FOREACH inhno INHNO_VALID$
	$IF LENGTH(INH.INHNO[inhno])$
		$TAB$(FP)($INH.INTHDR[inhno]$),
	$ELSE$
		$TAB$(FP)(_kernel_default_int_handler),
	$END$
	$SPC$$FORMAT("/* %d */", +inhno)$$NL$
$END$
};$NL$
$NL$

$ 
$  割込み属性テーブル
$ 
$NL$
ATR _kernel_intatr_tbl[TNUM_INT] = {$NL$
$FOREACH intno INTNO_VALID$
	$IF LENGTH(INT.INTNO[intno])$
		$TAB$($INT.INTATR[intno]$),
	$ELSE$
		$TAB$0U,
	$END$
	$SPC$$FORMAT("/* %d */", +intno)$$NL$
$END$
};$NL$
$NL$
