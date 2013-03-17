/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2002-2008 by Y.D.K Co.,Ltd
 *  Copyright (C) 2010 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN 
 *  Copyright (C) 2012-2013 by Tetsuya Morizumi
 * 
 *  上記著作権者は，以下の(1)〜(4)の条件を満たす場合に限り，本ソフトウェ
 *  ア（本ソフトウェアを改変したものを含む．以下同じ）を使用・複製・改
 *  変・再配布（以下，利用と呼ぶ）することを無償で許諾する．
 *  (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
 *      権表示，この利用条件および下記の無保証規定が，そのままの形でソー
 *      スコード中に含まれていること．
 *  (2) 本ソフトウェアを，ライブラリ形式など，他のソフトウェア開発に使
 *      用できる形で再配布する場合には，再配布に伴うドキュメント（利用
 *      者マニュアルなど）に，上記の著作権表示，この利用条件および下記
 *      の無保証規定を掲載すること．
 *  (3) 本ソフトウェアを，機器に組み込むなど，他のソフトウェア開発に使
 *      用できない形で再配布する場合には，次のいずれかの条件を満たすこ
 *      と．
 *    (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
 *        作権表示，この利用条件および下記の無保証規定を掲載すること．
 *    (b) 再配布の形態を，別に定める方法によって，TOPPERSプロジェクトに
 *        報告すること．
 *  (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
 *      害からも，上記著作権者およびTOPPERSプロジェクトを免責すること．
 *      また，本ソフトウェアのユーザまたはエンドユーザからのいかなる理
 *      由に基づく請求からも，上記著作権者およびTOPPERSプロジェクトを
 *      免責すること．
 * 
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，特定の使用目的
 *  に対する適合性も含めて，いかなる保証も行わない．また，本ソフトウェ
 *  アの利用により直接的または間接的に生じたいかなる損害に関しても，そ
 *  の責任を負わない．
 * 
 *  @(#) $Id: prc_config.h 5 2012-01-14 09:35:55Z tetsuya $
 */

/*
 *		プロセッサ依存モジュール（SH34用）
 *
 *  このインクルードファイルは，target_config.h（または，そこからインク
 *  ルードされるファイル）のみからインクルードされる．他のファイルから
 *  直接インクルードしてはならない．
 */

#ifndef TOPPERS_PRC_CONFIG_H
#define TOPPERS_PRC_CONFIG_H

#include "microblaze.h"

/*
 *  例外の個数
 */  
#define TNUM_EXCH  20U

/*
 *  割込みハンドラ番号に関する定義
 */ 
#define TMIN_INHNO 0U
#define TMAX_INHNO 31U
#define TNUM_INH   32U

/*
 *  割込み番号に関する定義
 */ 
#define TMIN_INTNO 0U
#define TMAX_INTNO 31U
#define TNUM_INT   32U

/*
 *  割込み優先度の個数
 */
#define  TNUM_INT_PRI  8U

/*
 *  割込み番号の範囲の判定
 *
 *  ビットパターンを求めるのを容易にするために，8は欠番になっている．
 */
#define VALID_INTNO(intno) (TMIN_INTNO <= (intno) && (intno) <= TMAX_INTNO)
#define VALID_INTNO_DISINT(intno)	VALID_INTNO(intno)
#define VALID_INTNO_CFGINT(intno)	VALID_INTNO(intno)
#define VALID_INTNO_ATTISR(intno)   VALID_INTNO(intno)

#ifndef TOPPERS_MACRO_ONLY

/*
 *  非タスクコンテキスト用のスタック開始アドレス設定マクロ
 */
#define TOPPERS_ISTKPT(istk, istksz) ((STK_T *)((char *)(istk) + (istksz)))

/*
 *  タスクコンテキストブロックの定義
 */
typedef struct task_context_block {
	void	*sp;		/* スタックポインタ */
	FP		pc;			/* プログラムカウンタ */
} TSKCTXB;
 
/*
 *  プロセッサの特殊命令のインライン関数定義
 */
#include "prc_insn.h"

/*
 *  TOPPERS標準割込み処理モデルの実現
 *
 *  カーネル管理外の割込みは設けない．
 */

/*
 * 例外（割込み/CPU例外）のネスト回数のカウント
 * コンテキスト参照のために使用
 */
extern uint32_t except_nest_count; /* 例外（割込み/CPU例外）のネスト回数のカウント */

/*
 *  コンテキストの参照
 *
 *   例外（割込み/CPU例外）のネスト回数をカウントする変数（except_nest_count）を
 *   用意し，例外の入り口でインクリメントすることで，コンテキストを判定する．
 */
Inline bool_t
sense_context(void)
{
    return(except_nest_count > 0U);
}

/*
 *  CPUロック状態への移行
 */
Inline void
x_lock_cpu(void)
{
	set_msr(current_msr() & ~MSR_IE);
	/* クリティカルセクションの前後でメモリが書き換わる可能性がある */
	Asm("":::"memory");
}

#define t_lock_cpu()   x_lock_cpu()
#define i_lock_cpu()   x_lock_cpu()

/*
 *  CPUロック状態の解除
 */
Inline void
x_unlock_cpu(void)
{
	/* クリティカルセクションの前後でメモリが書き換わる可能性がある */
	Asm("":::"memory");
	set_msr(current_msr() | MSR_IE);
}

#define t_unlock_cpu() x_unlock_cpu()
#define i_unlock_cpu() x_unlock_cpu()

/*
 *  CPUロック状態の参照
 */
Inline bool_t
x_sense_lock(void)
{
	return((current_msr() & MSR_IE) == 0U);
}

#define t_sense_lock()    x_sense_lock()
#define i_sense_lock()    x_sense_lock()

/*
 *  割込みハンドラの登録用テーブル
 *   実態はコンフィギュレータで生成する 
 */
extern const FP inh_tbl[TNUM_INH];

/*
 *  割込みハンドラの設定
 */
Inline void
x_define_inh(INHNO inhno, FP int_entry)
{
}

/*
 *  割込みハンドラの出入口処理の生成マクロ
 */
#define INT_ENTRY(inhno, inthdr)    inthdr
#define INTHDR_ENTRY(inhno, inhno_num, inthdr) extern void inthdr(void);

/*
 * 各割込みの割込み要求禁止フラグの状態
 */
extern uint32_t idf;

/*
 *  割込み優先度マスク操作ライブラリ
 *
 *  XPS Interrupt Controller は割込み優先度の概念がないため，
 *  ソフトウェアで擬似的に実現する． 
 */

/*
 *  現在の割込み優先度マスクの値（内部表現）
 */
extern uint8_t iipm;

/*
 *  割込み優先度マスク毎にセットする，割込み要求禁止フラグの値
 *  のテーブル（kernel_cfg.c）
 */
extern const uint32_t iipm_mask_tbl[TNUM_INT_PRI];

/*
 *  IPMをimp_mask_tblのインデックスに変換するマクロ
 */
#define INDEX_IPM(ipm)  (-(ipm))

/*
 *  割込み優先度マスクの外部表現と内部表現の変換
 *
 */
#define EXT_IPM(iipm)    (-((PRI)(iipm)))       /* 内部表現を外部表現に */
#define INT_IPM(ipm)     ((uint8_t)(-(ipm)))    /* 外部表現を内部表現に */

/*
 *  (モデル上の)割込み優先度マスクの設定
 * 
 *  指定された優先度に設定された割込み要求禁止フラグのテーブルの値と（モデ
 *  ル上の）各割込みの割込み要求禁止フラグの状態を保持した変数の値との
 *  ORをIRCの割込み要求禁止フラグにセットし，設定した優先度を内部変数
 *  ipmに保存する．
 */
Inline void
x_set_ipm(PRI intpri)
{
	uint32_t ipm_mask = iipm_mask_tbl[INT_IPM(intpri)];

	/*
	 *  XPS INTC の割込みコントローラはイネーブルレジスタと
	 *  クリアーレジスタがあるため，一旦全ての割込みを禁止してから，
	 *  特定の割込みのみ許可する必要がある
	 */
	/* 全割込み禁止 */
	xintc_disable_int(~0U);

	/* マスク指定されていない割込みのみ許可 */
	xintc_enable_int(~(ipm_mask|idf));

	iipm = INT_IPM(intpri);
}

#define t_set_ipm(intpri) x_set_ipm(intpri)
#define i_set_ipm(intpri) x_set_ipm(intpri)

/*
 *  (モデル上の)割込み優先度マスクの参照
 *
 *  ipmの値を返す
 */
Inline PRI
x_get_ipm(void)
{
    return(EXT_IPM(iipm));
}

#define t_get_ipm() x_get_ipm()
#define i_get_ipm() x_get_ipm()

/*
 *  割込み属性が設定されているかを判別するための変数
 */
extern uint32_t	bitpat_cfgint;

/*
 * （モデル上の）割込み要求禁止フラグのセット
 *
 *  指定された，割込み番号の割込み要求禁止フラグのセットして，割込みを
 *  禁止する．また，（モデル上の）割込み要求禁止フラグを管理するidfの対
 *  応するビットををセットする．
 *  割込み要求をマスクする機能をサポートしていない場合には，falseを返す
 */
Inline bool_t
x_disable_int(INTNO intno)
{
    if ((bitpat_cfgint & INTNO_BITPAT(intno)) == 0U) {
        return(false);
    }
    xintc_disable_int(INTNO_BITPAT(intno));
    idf |= INTNO_BITPAT(intno);
    return(true);
}

#define t_disable_int(intno)  x_disable_int(intno)
#define i_disable_int(intno)  t_disable_int(intno)

/* 
 * (モデル上の)割り要求禁止フラグの解除
 *
 * 指定された，割込み番号の割込み要求禁止フラグのクリアして，割込みを
 * 許可する．また，（モデル上の）割込み要求禁止フラグを管理するidfの対
 * 応するビットををクリアする．
 * 割込み要求をマスクする機能をサポートしていない場合には，falseを返す
 */
Inline bool_t
x_enable_int(INTNO intno)
{
    if ((bitpat_cfgint & INTNO_BITPAT(intno)) == 0U) {
        return(false);
    }    
    xintc_enable_int(INTNO_BITPAT(intno));
    idf &= ~INTNO_BITPAT(intno);
    return(true);
}

#define t_enable_int(intno) x_enable_int(intno)
#define i_enable_int(intno) x_enable_int(intno)

/*
 * 割込み要求のクリア
 */
Inline void
x_clear_int(INTNO intno)
{
	xintc_ack_int(INTNO_BITPAT(intno));
}

#define t_clear_int(intno) x_clear_int(intno) 
#define i_clear_int(intno) x_clear_int(intno) 


/*
 *  割込み要求のチェック
 */
Inline bool_t
x_probe_int(INTNO intno)
{
    return(xintc_probe_int(INTNO_BITPAT(intno)));
}

#define t_probe_int(intno) x_probe_int(intno)
#define i_probe_int(intno) x_probe_int(intno)

/*
 *  割込み要求ラインの属性の設定
 */
extern void x_config_int(INTNO intno, ATR intatr, PRI intpri);

/*
 *  割込み属性テーブル（kernel_cfg.c）
 */
extern ATR intatr_tbl[TNUM_INT];

/*
 *  割込みハンドラの入り口で必要なIRC操作
 */
Inline void
i_begin_int(INTNO intno)
{
    /* エッジトリガ(TA_EDGE)なら割込みをクリア */
    if((intatr_tbl[intno] & TA_EDGE) == TA_EDGE)
    {
        xintc_ack_int(INTNO_BITPAT(intno));    
	}
}

/*
 *  割込みハンドラの出口で必要なIRC操作
 */
Inline void
i_end_int(INTNO intno)
{
	/* レベルトリガなら割込みをクリア */
	if((intatr_tbl[intno] & TA_EDGE) != TA_EDGE)
    {
		xintc_ack_int(INTNO_BITPAT(intno));    
	}
}

/*
 *  未定義の割込みが入った場合の処理
 */
extern void default_int_handler(void *p_exinf);


/*
 *  CPU例外機能
 */

/*
 *  CPU例外フレームのオフセット
 */
#define CPUEXC_FLM_MSR  0x00U
#define CPUEXC_FLM_ESR  0x01U
#define CPUEXC_FLM_EAR  0x02U
#define CPUEXC_FLM_NEST 0x03U
#define CPUEXC_FLM_PC   0x04U
#define CPUEXC_FLM_IPM  0x05U

/*
 *  CPU例外ハンドラの登録用テーブル
 */
extern const FP exch_tbl[TNUM_EXCH];

/*
 * ターゲット非依存部に含まれる標準の例外管理機能の初期化処理を用いない
 */
#define OMIT_INITIALIZE_EXCEPTION

/*
 * CPU例外ハンドラの初期化
 */
Inline void
initialize_exception(void)
{
}

/*
 *  CPU例外の発生した時のシステム状態の参照
 */

/*
 *  CPU例外の発生した時のコンテキストの参照
 *
 *  CPU例外の発生した時のコンテキストが，タスクコンテキストの時にfalse，
 *  そうでない時にtrueを返す．
 */
Inline bool_t
exc_sense_context(void *p_excinf)
{
	return((*(((uint32_t *)p_excinf) + CPUEXC_FLM_NEST) != 0U));
}

/*
 *  CPU例外の発生した時の(モデル上の)割込み優先度マスクの参照
 */
Inline uint8_t
exc_get_ipm(void *p_excinf)
{
	return((uint8_t)(*(((uint32_t *)p_excinf) + CPUEXC_FLM_IPM)));
}

/*
 * CPU例外の発生時した時のCPUロック状態の参照
 *
 *  CPUロック状態の場合はtrue，CPUロック解除状態の場合にはfalseを返す
 */
Inline bool_t
exc_sense_lock(void *p_excinf){
	return((*(((uint32_t *)p_excinf) + CPUEXC_FLM_MSR) & MSR_IE) != MSR_IE);
}

/*
 * 割込みロック状態か
 */
Inline bool_t
exc_sense_int_lock(void *p_excinf){
	return false;
}

/*
 *  CPU例外の発生した時のコンテキストと割込みのマスク状態の参照
 *
 *  CPU例外の発生した時のシステム状態が，カーネル実行中でなく，タスクコ
 *  ンテキストであり，割込みロック状態でなく，CPUロック状態でなく，（モ
 *  デル上の）割込み優先度マスク全解除状態である時にtrue，そうでない時
 *  にfalseを返す（CPU例外がカーネル管理外の割込み処理中で発生した場合
 *  にもfalseを返す）．
 *
 */
Inline bool_t
exc_sense_intmask(void *p_excinf)
{
	return(!exc_sense_context(p_excinf)
		   && (exc_get_ipm(p_excinf) == 0U)
		   && !exc_sense_lock(p_excinf)
		   && !exc_sense_int_lock(p_excinf));
}

/*
 *  CPU例外の発生した時のコンテキストと割込み／CPUロック状態の参照
 *
 *  CPU例外の発生した時のシステム状態が，カーネル実行中でなく，タスクコ
 *  ンテキストであり，割込みロック状態でなく，CPUロック状態でない時に
 *  true，そうでない時にfalseを返す（CPU例外がカーネル管理外の割込み処
 *  理中で発生した場合にもfalseを返す）．
 *
 */
Inline bool_t
exc_sense_unlock(void *p_excinf)
{
	return(!exc_sense_context(p_excinf)
		   && !exc_sense_lock(p_excinf)
		   && !exc_sense_int_lock(p_excinf));
}

/*
 *  プロセッサ依存の初期化
 */
extern void prc_initialize(void);

/*
 *  プロセッサ依存の終了時処理
 */
extern void prc_terminate(void);

/*
 *  未定義の例外が入った場合の処理
 */
extern void default_exc_handler(void *p_excinf);

/*
 *  タスクディスパッチャ
 */

/*
 *  最高優先順位タスクへのディスパッチ（core_support.S）
 *
 *  dispatchは，タスクコンテキストから呼び出されたサービスコール処理か
 *  ら呼び出すべきもので，タスクコンテキスト・CPUロック状態・ディスパッ
 *  チ許可状態・（モデル上の）割込み優先度マスク全解除状態で呼び出さな
 *  ければならない．
 */
extern void dispatch(void);

/*
 *  ディスパッチャの動作開始（core_support.S）
 *
 *  start_dispatchは，カーネル起動時に呼び出すべきもので，すべての割込
 *  みを禁止した状態（割込みロック状態と同等の状態）で呼び出さなければ
 *  ならない．
 */
extern void start_dispatch(void) NoReturn;

/*
 *  現在のコンテキストを捨ててディスパッチ（core_support.S）
 *
 *  exit_and_dispatchは，ext_tskから呼び出すべきもので，タスクコンテキ
 *  スト・CPUロック状態・ディスパッチ許可状態・（モデル上の）割込み優先
 *  度マスク全解除状態で呼び出さなければならない．
 */
extern void exit_and_dispatch(void) NoReturn;

/*
 *  カーネルの終了処理の呼出し（core_support.S）
 *
 *  call_exit_kernelは，カーネルの終了時に呼び出すべきもので，非タスク
 *  コンテキストに切り換えて，カーネルの終了処理（exit_kernel）を呼び出
 *  す．
 */
extern void call_exit_kernel(void) NoReturn;

/*
 *  タスクコンテキストの初期化
 *
 *  タスクが休止状態から実行できる状態に移行する時に呼ばれる．この時点
 *  でスタック領域を使ってはならない．
 *
 *  activate_contextを，インライン関数ではなくマクロ定義としているのは，
 *  この時点ではTCBが定義されていないためである．
 */
extern void    start_r(void);

#define activate_context(p_tcb)                                         \
{                                                                       \
    (p_tcb)->tskctxb.sp = (void *)((char *)((p_tcb)->p_tinib->stk)      \
                                        + (p_tcb)->p_tinib->stksz);     \
    (p_tcb)->tskctxb.pc = (FP)start_r;                                  \
}

/*
 *  calltexは使用しない
 */
#define OMIT_CALLTEX

/*
 *  atexitの処理とデストラクタの実行
 */
Inline void
call_atexit(void)
{
    extern void    software_term_hook(void);
    void (*volatile fp)(void) = software_term_hook;

    /*
     *  software_term_hookへのポインタを，一旦volatile指定のあるfpに代
     *  入してから使うのは，0との比較が最適化で削除されないようにするた
     *  めである．
     */
    if (fp != 0) {
        (*fp)();
    }
}
#endif /* TOPPERS_MACRO_ONLY */
#endif /* TOPPERS_PRC_CONFIG_H */
