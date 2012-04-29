/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2002-2008 by Y.D.K Co.,Ltd
 *  Copyright (C) 2012 by Tetsuya Morizumi
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
 *  @(#) $Id: prc_config.c 5 2012-01-14 09:35:55Z tetsuya $
 */

/*
 *  プロセッサ依存モジュール（Microbalze用）
 */
#include "kernel_impl.h"
#include "check.h"
#include "task.h"
#include "microblaze.h"

/*
 * 例外（割込み/CPU例外）のネスト回数のカウント
 * コンテキスト参照のために使用
 */
uint32_t except_nest_count;

/*
 * 各割込みの割込み要求禁止フラグの状態
 */
uint32_t idf;

/*
 * 現在の割込み優先度マスクの値
 */
uint8_t iipm;
 
/*
 *  割込み属性が設定されているかを判別するための変数
 */
uint32_t bitpat_cfgint;
uint32_t bitpat_intedge;

/*
 *  プロセッサ依存の初期化
 */
void
prc_initialize()
{
	/*
	 *  カーネル起動時は非タスクコンテキストとして動作させるため1に
	 */ 
	except_nest_count = 1;

	/*
	 * 各割込みの割込み要求禁止フラグ全禁止
	 */
	idf = ~0U;

	/*
	 * 割込み優先度マスクの初期値
	 */ 
	iipm = 0U;

	/*
	 * 全ての割込みをマスク
	 */ 
	xintc_disable_int(~0U);

	/*
	 *  割込み属性が設定されているかを判別するための変数を初期化する．
	 */
	bitpat_cfgint = 0U;
    bitpat_intedge = 0U;

	/*
	 *  XINTCを開始
	 */
	xintc_start();
}

/*
 *  プロセッサ依存の終了処理
 */
void
prc_terminate(void)
{
	/*
	 *  すべての割込みをマスクする．
	 */
	xintc_disable_int(~0U);
}

/*
 *  割込み要求ラインの属性の設定
 *
 *  ASPカーネルでの利用を想定して，パラメータエラーはアサーションでチェッ
 *  クしている．FI4カーネルに利用する場合には，エラーを返すようにすべき
 *  であろう．
 */
void
x_config_int(INTNO intno, ATR intatr, PRI intpri)
{
	assert(VALID_INTNO(intno));
	assert(TMIN_INTPRI <= intpri && intpri <= TMAX_INTPRI);

	/*
	 *  割込み属性が設定されているかを判別するための変数の設定
	 */
	bitpat_cfgint |= INTNO_BITPAT(intno);
	if ((intatr & TA_EDGE) != 0U)
    {
        bitpat_intedge |= INTNO_BITPAT(intno);
    }

	/* 
	 * いったん割込みを禁止する
	 */    
	x_disable_int(intno);

	if ((intatr & TA_ENAINT) != 0U){
		(void)x_enable_int(intno);
	}
}

/*
 *  CPU例外の発生状況のログ出力
 *
 *  CPU例外ハンドラの中から，CPU例外情報ポインタ（p_excinf）を引数とし
 *  て呼び出すことで，CPU例外の発生状況をシステムログに出力する．
 */
#ifdef SUPPORT_XLOG_SYS

void
xlog_sys(void *p_excinf)
{    
}

#endif /* SUPPORT_XLOG_SYS */

#ifndef OMIT_DEFAULT_EXC_HANDLER

/*
 * 未定義の例外が入った場合の処理
 */
void
default_exc_handler(void *p_excinf){
	syslog(LOG_EMERG, "Unregistered Exception occurs.");
	syslog(LOG_EMERG, " MSR = 0x%x, PC = 0x%x, IPM = %d.",
		   *(((uint32_t*)p_excinf) + CPUEXC_FLM_MSR),
		   *(((uint32_t*)p_excinf) + CPUEXC_FLM_PC),
		   *(((uint32_t*)p_excinf) + CPUEXC_FLM_IPM));
	syslog(LOG_EMERG, " ESR = 0x%x. EAR = 0x%x",
			 *(((uint32_t*)p_excinf) + CPUEXC_FLM_ESR),
			 *(((uint32_t*)p_excinf) + CPUEXC_FLM_EAR));
	syslog(LOG_EMERG, " Interrupt/Exception Nest Count = %d",
			 *(((uint32_t*)p_excinf) + CPUEXC_FLM_NEST));

	target_exit();
}

#endif /* OMIT_DEFAULT_EXC_HANDLER */

#ifndef OMIT_DEFAULT_INT_HANDLER

/*
 * 未定義の割込みが入った場合の処理
 */
void
default_int_handler(void *p_excinf){
	syslog(LOG_EMERG, "Unregistered Interrupt 0x%x occurs.",
		   *((int*)p_excinf));
	target_exit();
}

#endif /* OMIT_DEFAULT_INT_HANDLER */
