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
 *  @(#) $Id: start.S 6 2012-03-15 12:51:23Z tetsuya $
 */

/*
 *  カーネル対応のスタートアップモジュール（Microblaze用）
 */

#define TOPPERS_MACRO_ONLY
#define UINT_C(val)		(val)		/* uint_t型の定数を作るマクロ */
#define ULONG_C(val)	(val)		/* ulong_t型の定数を作るマクロ */
#define CAST(type, val)	(val)		/* 型キャストを行うマクロ */
#include "kernel_impl.h"
        
	.text
	.align  2
	.global start
start:
	/*
	 *  msrを初期化
	 *  例外は許可
	 */
	la   r1, r0, (MSR_INIT_VAL|MSR_EE)
	mts  rmsr,r1

	/*
	 * R2とR13の設定
	 *・R2  : Read-only small data area anchor
	 *   _SDA2_BASE_ .sdata2の中心
	 *・R13 : Read-write small data area anchor
	 *   .sdataとsbssを連続して置き，この中間に_SDA_BASE_ を設定する
	 */
	la   r13, r0, _SDA_BASE_
	la   r2,  r0, _SDA2_BASE_

	/*
	 * スタックポインタの初期化
	 */
	lwi  r1,  r2, _kernel_istkpt

	/*
	 *  _hardware_init_hook を呼び出す。(0でない場合)
	 *
	 *  ハードウェア依存に必要な初期化処理がある場合は，
	 *  hardware_init_hook という関数を用意すればよい．
	 *  具体的にはROM化の際、RAMを使用可能にするための
	 *  バスコントローラの初期化等を行う。
	 *  sys_support.S 内で _hardware_init_hook を定義してい
	 *  る．
	 */
start_0:
	la    r3,  r0,  hardware_init_hook
	beqi  r3,  start_01
	brald r15, r3
	 nop

start_01:
	/*
	 * ベクタのコピー
     * カーネルのベクタを 0番地にコピーする
	 */
#ifdef VECTOR_COPY
	addk	r5,r0,r0
start_02:
	addik	r3,r5,vectors_reset
	lwi	    r4,r3,0
	swi	    r4,r5,0
	addik	r5,r5,4
	xori	r18,r5,80
	bnei	r18,start_02
#endif /* VECTOR_COPY */

	/*
	 *  bssセクションをクリア
	 */
start_s1:
    la      r3, r0, __sbss_start
    la      r4, r0, __sbss_end
    sub     r5, r4, r3
    beqi    r5, start_1
start_s2:
    sb      r0, r0, r3
    addi    r3, r3, 1
    sub     r5, r4, r3
    bgti    r5, start_s2

start_1:
	la    r3,  r0,  __bss_start
	la    r4,  r0,  __bss_end
	sub   r5,  r4,  r3
	beqi  r5,  start_3
start_2:
	sb    r0,  r0,  r3
	addi  r3,  r3,  1
	sub   r5,  r4,  r3
	bgti  r5,  start_2

	/*
	 *  dataセクションの初期化（ROM化対応）
	 *
	 *  __idata_start から __idata_end までを，__data_start 以降に
	 *  コピーする．
	 */
start_3:
#ifndef OMIT_DATA_IN_ROM
	la    r3,  r0,  __idata_start
	la    r4,  r0,  __idata_end
	sub   r5,  r4,  r3     /* サイズをチェック */
	beqi  r5,  start_5
	la    r6,  r0,  __data_start
start_4:
	lbu   r7,  r0,  r3
	sb    r7,  r0,  r6
	addi  r3,  r3,  1
	addi  r6,  r6,  1
	sub   r5,  r4,  r3
	bgti  r5,  start_4

start_5:
	la    r3,  r0,  __isdata_start
	la    r4,  r0,  __isdata_end
	sub   r5,  r4,  r3     /* サイズをチェック */
	beqi  r5,  start_9
	la    r6,  r0,  __sdata_start
start_6:
	lbu   r7,  r0,  r3
	sb    r7,  r0,  r6
	addi  r3,  r3,  1
	addi  r6,  r6,  1
	sub   r5,  r4,  r3
	bgti  r5,  start_6
#endif /* OMIT_DATA_IN_ROM */
	/*
	 *  _software_init_hook を呼び出す（0 でない場合）．
	 *
	 *  ソフトウェア環境（特にライブラリ）に依存して必要な初期化処
	 *  理がある場合は，software_init_hook という関数を用意すれば
	 *  よい．
	 */
start_9:
	la    r3,  r0,  software_init_hook
	beqi  r3,  start_10
	brald r15, r3
	nop

start_10:
	brlid	r15, sta_ker
	nop

