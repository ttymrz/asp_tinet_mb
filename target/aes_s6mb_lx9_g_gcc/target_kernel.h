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
 *  @(#) $Id: target_kernel.h 5 2012-01-14 09:35:55Z tetsuya $
 */

/*
 *  kernel.hのターゲット依存部（AES-S6MB-LX9-G）
 *
 *  このインクルードファイルは，kernel.hでインクルードされる．他のファ
 *  イルから直接インクルードすることはない．このファイルをインクルード
 *  する前に，t_stddef.hがインクルードされるので，それらに依存してもよ
 *  い．
 */

#ifndef TOPPERS_TARGET_KERNEL_H
#define TOPPERS_TARGET_KERNEL_H

/*
 *  タイムティックの定義
 */
#define TIC_NUME   1U            /* タイムティックの周期の分子 */
#define TIC_DENO   1U            /* タイムティックの周期の分母 */

/*
 *  MSRの初期値
 */
#define MSR_INIT_VAL 0xA0

/*
 *  ベクタコピーを行う
 */
#define VECTOR_COPY

/*
 *  データセクションの初期化を行わない
 */
#define OMIT_DATA_IN_ROM

/*
 *  ペリフェラルのベースアドレス
 */

/*
 *  XPS Interrupt Controller
 */
#define XINTC_BASE  0x41200000

/*
 *  XPS Timer/Counter
 */
#define XTIMER_BASE 0x41c00000

/*
 *  XPS UART Lite
 */
#define XUARTL_PORT1_BASE 0x40600000
//#define XUARTL_PORT2_BASE 0x40600000

/*
 *  カーネル低レベル出力用の XPS UART Lite 
 */
#define KENEL_LOW_OUT_XUARTL_BASE 0x40600000

/*
 *  プロセッサで共通な定義
 */
#include "microblaze_gcc/prc_kernel.h"

#endif /* TOPPERS_TARGET_KERNEL_H */
