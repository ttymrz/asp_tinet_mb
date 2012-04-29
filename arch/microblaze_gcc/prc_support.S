/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2002-2008 by Y.D.K Co.,Ltd
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
 *  @(#) $Id: prc_support.S 1776 2010-04-05 11:12:59Z ertl-honda $
 */

/*
 *  プロセッサ依存モジュール アセンブリ言語部（Microblaze用）
 */

#define TOPPERS_MACRO_ONLY
#define UINT_C(val)		(val)		/* uint_t型の定数を作るマクロ */
#define ULONG_C(val)	(val)		/* ulong_t型の定数を作るマクロ */
#define CAST(type, val)	(val)		/* 型キャストを行うマクロ */

/*
 *  アセンブラ用のマクロを有効にする
 */
#define TOPPERS_ASM_MACRO
#include "kernel_impl.h"
#include "offset.h"
#include "microblaze_asm.inc"

/*
 * ベクタテーブル
 */
	.section .vectors.reset,"a"
	.global vectors_reset
vectors_reset:
	brai start           /* 0x00-0x04 Reset       */

	.section .vectors.sw_exception,"a"
	.global vectors_sw_exception
vectors_sw_exception:
	brai usrexcept_entry /* 0x08-0x0C User Vector */

	.section .vectors.interrupt,"a"
	.global vectors_interrupt
vectors_interrupt:
	brai interrupt_entry /* 0x10-14 Interrupt     */

	.section .vectors.hw_exception,"a"
	.global vectors_hw_exception
vectors_hw_exception:
	brai hwexcept_entry  /* 0x20-0x24 Break       */

	.section .vectors.reserve,"a"
	.global vectors_reserve
vectors_reserve:
	nop                  /* 0x28 Reserved         */
	nop                  /* 0x2C Reserved         */
	nop                  /* 0x30 Reserved         */
	nop                  /* 0x34 Reserved         */
	nop                  /* 0x38 Reserved         */
	nop                  /* 0x3C Reserved         */
	nop                  /* 0x40 Reserved         */
	nop                  /* 0x44 Reserved         */
	nop                  /* 0x48 Reserved         */
	nop                  /* 0x4C Reserved         */


/*
 *  ユーザー例外エントリ
 *  戻り値はr15に入っているとしてリターン
 */
	.text
	.global usrexcept_entry
	.align 2
usrexcept_entry:
	rtsd r15, 8
	 nop


/*
 *  例外エントリ
 */
	.text
	.global hwexcept_entry
	.align 2
hwexcept_entry:
	/*
	 * コンテキストの保存
	 * 戻り先アドレスはR17もしくはBTRに入っている．
	 * 出口のコードを割込みと共通化するため，R14の格納場所に
	 * 保存する．
	 */
	addik r1,r1,-64
	swi   r3,  r1, 60
	swi   r4,  r1, 56
	swi   r5,  r1, 52
	swi   r6,  r1, 48
	swi   r7,  r1, 44
	swi   r8,  r1, 40
	swi   r9,  r1, 36
	swi   r10, r1, 32
	swi   r11, r1, 28
	swi   r12, r1, 24
	swi   r17, r1, 20
	swi   r15, r1, 16
	swi   r16, r1, 12
	swi   r14, r1, 8
	swi   r18, r1, 4
	mfs   r3,  rmsr
	swi   r3,  r1, 0       /* 復帰時用に保存 */

	/*
	 *  遅延スロットで割込みが発生した場合には，戻り先アドレスは，
	 *  BTRに格納される．  
	 */
	mfs   r7,  resr        /* ESR を取得 */
	andi  r4,  r7, ESR_DS  /* DSビットがセットされていれば */
	beqi  r4,  no_dslot    /* ディレイスロットではない     */
	mfs   r5,  rbtr        /* BTR を取得 */
	swi   r5,  r1, 20      /* 戻り先アドレスとして保存     */
	Mov   r17, r5          /* 例外フレームに入れるためr17に保存 */

no_dslot:
	/* 
	 * 割込み・例外のネスト回数が1以上ならスタックを切り替えない 
	 */
	lwi   r5,  r13, except_nest_count
	bgti  r5,  nest_int_exc      

	/*
	 * スタックポインタ切り替え
	 */
	lwi   r4,  r2, _kernel_istkpt /* 非タスクコンテキスト用のスタックの読み込み */
	addik r4,  r4, -4             /* スタックポインタをデクリメント   */
	sw    r1,  r0, r4             /* タスクスタックの保存             */
	Mov   r1,  r4                 /* スタックポインタ切り替え         */

nest_int_exc:
	/* 
	 * 割込み・例外のネスト回数のインクリメント
	 */
	addi  r6,  r5,  1 
	swi   r6,  r13, except_nest_count

	/*
	 * 例外フレームの作成    
	 *     内容                       オフセット
	 *    MSR(r3)                         0
	 *    ESR(r7)                         4
	 *    EAR(r6)                         8
	 *    割込み・例外ネストカウント(r5) 12
	 *    戻りアドレス(r17)              16
	 *    割込み優先度マスク             20
	 */
	mfs   r4,  rear
	lbui  r6,  r13, iipm
	addik r1,  r1,  -24
	swi   r6,  r1,   20 
	swi   r17, r1,   16 
	swi   r5,  r1,   12 
	swi   r4,  r1,    8
	swi   r7,  r1,    4
	swi   r3,  r1,    0

	/*
	 *  例外フレームの先頭番地をCPU例外ハンドラの引数に設定
	 */
	Mov   r5,  r1         

	/*
	 *  CPU例外番号の取得
	 */
	andi  r6,  r7, ESR_EC

	/*
	 *  例外ハンドラアドレスの取得
	 */
	add   r8,  r6, r6              /* 4倍してオフセットを生成    */
	add   r8,  r8, r8
	lwi   r7,  r8, exch_tbl

	/*
	 * 例外許可(EE=1), 例外処理中クリア(EIP=0)とする
	 */   
	ori   r8,  r3, MSR_EE
	andi  r8,  r8, ~(MSR_EIP)
	mts   rmsr,r8

#ifdef LOG_EXC_ENTER
	Push  r6         /* CPU例外番号保存               */
	Push  r7         /* CPU例外ハンドラアドレス保存   */
	Push  r5         /* CPU例外フレームの先頭番地保存 */
	brlid r15, log_exc_enter
	 Mov   r5, r6    /* CPU例外番号を引数にすふ       */
	Pop   r5         /* CPU例外フレームの先頭番地復帰 */
	Pop   r7        /* CPU例外ハンドラアドレス復帰   */
#endif /* LOG_EXC_ENTER */

	/* 
	 *  CPU例外ハンドラ呼び出し 
	 */
	brald r15, r7             
	 nop

#ifdef LOG_EXC_LEAVE
	brlid r15, log_exc_leave
	 Pop   r5         /* CPU例外番号復帰               */
#endif /* LOG_EXC_LEAVE */

	/*
	 *  割込み禁止
	 */
	mfs   r3,  rmsr            
	andi  r3,  r3, ~MSR_IE
	mts   rmsr,r3

	/*
	 *  例外フレームを捨てる
	 */
	addik r1,  r1,  20
	bri   ret_exc


/*
 *  割込みエントリ
 */
	.text
	.global interrupt_entry
	.align 2
interrupt_entry:
	/*
	 * コンテキストの保存
	 */
	addik r1,r1,-64
	swi   r3,  r1, 60
	swi   r4,  r1, 56
	swi   r5,  r1, 52
	swi   r6,  r1, 48
	swi   r7,  r1, 44
	swi   r8,  r1, 40
	swi   r9,  r1, 36
	swi   r10, r1, 32
	swi   r11, r1, 28
	swi   r12, r1, 24
	swi   r14, r1, 20
	swi   r15, r1, 16
	swi   r16, r1, 12
	swi   r17, r1, 8
	swi   r18, r1, 4

	/* 
	 * msrの保存 
	 */
	mfs   r3,  rmsr
	swi   r3,  r1, 0

	/* 
	 * 割込み・例外のネスト回数が1以上ならスタックを切り替えない 
	 */
	lwi   r5,  r13, except_nest_count
	bgti  r5,  nest_int      

	/*
	 * スタックポインタ切り替え
	 */
	lwi   r4,  r2, _kernel_istkpt /* 非タスクコンテキスト用のスタックの読み込み */
	addik r4,  r4, -4             /* スタックポインタをデクリメント   */
	sw    r1,  r0, r4             /* タスクスタックの保存             */
	Mov   r1,  r4                 /* スタックポインタ切り替え         */


nest_int:
	/* 
	 * 割込み・例外のネスト回数のインクリメント
	 */
	addi  r5,  r5,  1 
	swi   r5,  r13, except_nest_count
                    
	/*
	 *  割り込み要因の取得
	 *  R5に入れ，default_int_handler() の引数とする．
	 */
	lwi   r5,  r0, XINTC_IVR       /* 割込み番号の読み込み             */

	/*
	 *  割込み要因の割込み優先度を求め(モデル上の)割込み優先度マスクをセット
	 *  する．またその際，ハンドラ実行前の(モデル上の)割込み優先度マスクを
	 *  保存する．
	 */
	lbui  r3,  r5,  inh_iipm_tbl  /* 受け付けた割込みの割込み優先度マスクを取得 */
	lbui  r6,  r13, iipm          /* 割込み発生前の割込み優先度マスクをスタックに入れる */
	Push  r6
	sbi   r3,  r13, iipm          /* (モデル上の)割込み優先度マスクをセット */

	/*
	 *  XINTCを受け付けた割込みの割込み優先度に対応する
	 *  マスクビットでセットする．
	 */
	add   r6,  r3, r3                /* 割込み優先度マスクを4倍してオフセットを生成    */
	add   r6,  r6, r6
	lwi   r7,  r6, iipm_mask_tbl     /* 割込み優先度マスクに対応した割込み禁止パターン */
	lwi   r8,  r13, idf              /* 各割込みの割込み要求禁止フラグの状態を取得     */
	or    r7,  r7,  r8               /* 両者のORを生成                                 */
	xori  r7,  r7, -1                /* 反転                                           */
	swi   r7,  r0, XINTC_IER         /* 指定した割込みのみ許可                         */

	/*
	 *  割込みハンドラのアドレスを読み込む
	 */
	add   r4,  r5,  r5               /* 割込み番号を4倍してオフセットを生成  */
	add   r4,  r4,  r4
	lwi   r6,  r4, inh_tbl           

	/*
	 *  割込み許可
	 */
	mfs   r3,  rmsr
	ori   r3,  r3, MSR_IE
	mts   rmsr,r3

#ifdef LOG_INH_ENTER
	Push  r6           /* ハンドラアドレスを保存 */
	Push  r5           /* 割込み番号を保存 */
	brlid r15, log_inh_enter
	 nop   
	Pop   r5           /* 割込み番号を復帰 */
	Pop   r6           /* ハンドラアドレスを復帰 */
	Push  r5           /* 割込み番号を保存 */
#endif /* LOG_INH_ENTER */

	/* 
	 *  割込みハンドラ呼び出し 
	 */
	brald r15, r6
	 nop

#ifdef LOG_INH_LEAVE
	Pop  r5           /* 割込み番号を復帰 */     
	brlid r15, log_inh_leave
	 nop
#endif /* LOG_INH_LEAVE */

	/*
	 *  割込み禁止
	 */
	mfs   r3,  rmsr            
	andi  r3,  r3, ~MSR_IE
	mts   rmsr,r3

/*
 *  割込みハンドラ/CPU例外ハンドラ出口処理
 *
 *  ret_intは，割込みハンドラから戻った直後に実行するルーチンで，
 *  割込みハンドラ終了後，ターゲット依存の処理を実行した後，
 *  カーネル管理の割込みを禁止，スタックを割込み前のスタックにした
 *  状態で呼び出される．
 */
	.global ret_int
	.global ret_exc
rer_int:
ret_exc:
	/*
	 *  (モデル上の)割込み優先度マスクを元に戻す
	 */
	Pop   r6
	sbi   r6,  r13, iipm             /* (モデル上の)割込み優先度マスクをセット */
	add   r5,  r6,  r6               /* 割込み優先度マスクを4倍してオフセットを生成  */
	add   r5,  r5,  r5
	lwi   r7,  r5,  iipm_mask_tbl    /* 割込み優先度マスクに対応した割込み禁止パターン */
	lwi   r8,  r13, idf              /* 各割込みの割込み要求禁止フラグの状態を取得 */
	or    r8,  r7,  r8               /* 両者のORを生成                             */
	xori  r8,  r8,  -1               /* IERにセットするために反転する              */
	swi   r8,  r0, XINTC_IER         /* 指定した割込みのみ許可                     */

	/*
	 *  割込みネスト回数のデクリメント
	 */
	lwi   r5,  r13, except_nest_count
	addi  r5,  r5,  -1 
	swi   r5,  r13, except_nest_count

	bgti  r5,  ret_int_1            /* ネスト回数が1以上なら戻る */

	lw    r1,  r0, r1               /* スタックポインタを戻す    */  
	lwi   r4,  r13, reqflg          /* reqflg のチェック         */
	beqi  r4,  ret_int_1            /* false なら タスクに戻る   */
	bri   ret_int_2                 /* true なら ret_int_2 に    */

ret_int_1:
	lwi  r3,  r1, 0
	lwi  r18, r1, 4
	lwi  r17, r1, 8
	lwi  r16, r1, 12
	lwi  r15, r1, 16
	lwi  r14, r1, 20
	lwi  r12, r1, 24
	lwi  r11, r1, 28
	lwi  r10, r1, 32
	lwi  r9,  r1, 36
	lwi  r8,  r1, 40
	lwi  r7,  r1, 44
	lwi  r6,  r1, 48
	lwi  r5,  r1, 52
	lwi  r4,  r1, 56

	/*
	 *  CPU例外からのリターンかを判定
	 */
	andi r3,  r3, MSR_EIP
	bnei r3,  ret_int_1_EIP

	/*
	 *  割込みからのリターン
	 */
	lwi  r3,  r1, 0
	mts  rmsr,r3
	lwi  r3,  r1, 60
	rtid r14,0 
	 addik r1,r1,64

	/*
	 *  CPU例外からのリターン
	 */
ret_int_1_EIP:
	lwi  r3,  r1, 0
	mts  rmsr,r3
	lwi  r3,  r1, 60
	rted r14,0 
	 addik r1,r1,64

	/*
	 *  割込みハンドラ/CPU例外ハンドラ出口処理1
	 * 
	 *  ここでは，戻り先がタスクであり，スタックは，タスクスタックの上
	 *  にスクラッチレジスタのみが保存された状態になっている．また，
	 *  プロセッサは，スーパーバイザーモード・カーネル管理の割込みを禁止
	 *  した状態となっている．
	 */  
ret_int_2:
	swi  r0,  r13,  reqflg   /* reqflg を false に */

	/*
	 *  CPUロック状態に移行する．
	 *
	 *  この時点でCPUロック状態とするのは，dispatcherへ分岐する時と，
	 *  call_texrtnを呼び出す時に，CPUロック状態になっている必要がある
	 *  ためである．
	 *
	 *  Microbalzeでは，カーネル管理外の割込みを設けないため，ここでは，
	 *  特に何も行わない．
	 */

	/*
	 *  dspflgがfalseである場合と，p_runtskとp_schedtskが同じ場合には，
	 *  ディスパッチを行わない．このチェックが必要なのは，タスク例外処
	 *  理ルーチンの呼出しが必要な場合に，ディスパッチが必要なくても，
	 *  reqflgをtrueにするためである．
	 */
	lwi  r4,  r13,  p_runtsk   /* r4 <- p_runtsk        */
	lwi  r6,  r13,  dspflg     /* r6 <- dspflg          */
	beqi r6,  ret_int_3        /* dspflg が false なら ret_int_3 へ */
	lwi  r5,  r13,  p_schedtsk /* r5 <- p_schedtsk                  */
	sub  r6,  r5,   r4         /* p_runtsk と p_schedtsk が同じなら */
	beqi r6,  ret_int_3        /* ret_int_3 へ                      */
	addi  r1,  r1,  -52        /* 残りのレジスタを保存する          */
	swi   r19, r1,  48
	swi   r20, r1,  44
	swi   r21, r1,  40
	swi   r22, r1,  36
	swi   r23, r1,  32
	swi   r24, r1,  28
	swi   r25, r1,  24
	swi   r26, r1,  20
	swi   r27, r1,  16
	swi   r28, r1,  12
	swi   r29, r1,  8
	swi   r30, r1,  4
	swi   r31, r1,  0
	swi   r1 , r4,  TCB_sp     /* タスクスタックをTCBに保存 */
	la    r6 , r0,  ret_int_r  /* 実行再開番地を保存 */
	swi   r6 , r4,  TCB_pc     /* 実行再開番地をTCBに保存   */
	bri   dispatcher

	/*
	 *  割込み・例外でコンテキスト保存した場合の復帰ルーチン
	 */
ret_int_r:
	lwi  r31,  r1,  0      
	lwi  r30,  r1,  4
	lwi  r29,  r1,  8
	lwi  r28,  r1,  12
	lwi  r27,  r1,  16
	lwi  r26,  r1,  20
	lwi  r25,  r1,  24
	lwi  r24,  r1,  28
	lwi  r23,  r1,  32
	lwi  r22,  r1,  36
	lwi  r21,  r1,  40
	lwi  r20,  r1,  44
	lwi  r19,  r1,  48
	addi r1,   r1,  52
	lwi  r3,   r1,   0
	mts  rmsr, r3                

ret_int_3:
	/*
	 * タスク例外ルーチンの起動
	 * ret_int_r は dispatcher から呼び出されるため，
	 * tcb のアドレスは r4 に入っている
	 */
	lbui r5,  r4,  TCB_enatex  /* r5 <- enatex                 */
	andi r6,  r5,  TCB_enatex_mask  
	beqi r6,  ret_int_4        /* enatex が false ならリターン */
	lwi  r7,  r4,  TCB_texptn  /* r5 <- texptn                 */
	beqi r7,  ret_int_4        /* texptn が 0 でなければ       */
	brlid r15  call_texrtn     /* タスク例外ルーチンの呼び出し */
	 nop

ret_int_4:
	lwi  r3,  r1, 0    /* MSR を戻す（割込みIE=0で割込み禁止になる）*/
	lwi  r18, r1, 4
	lwi  r17, r1, 8
	lwi  r16, r1, 12
	lwi  r15, r1, 16
	lwi  r14, r1, 20
	lwi  r12, r1, 24
	lwi  r11, r1, 28
	lwi  r10, r1, 32
	lwi  r9,  r1, 36
	lwi  r8,  r1, 40
	lwi  r7,  r1, 44
	lwi  r6,  r1, 48
	lwi  r5,  r1, 52
	lwi  r4,  r1, 56

	/*
	 *  CPU例外からのリターンかを判定
	 */
	andi r3,  r3, MSR_EIP
	bnei r3,  ret_int_4_EIP

	/*
	 *  割込みからのリターン
	 */
	lwi  r3,  r1, 0
	mts  rmsr,r3
	lwi  r3,  r1, 60
	rtid r14,0 
	 addik r1,r1,64

	/*
	 *  CPU例外からのリターン
	 */
ret_int_4_EIP:
	lwi  r3,  r1, 0
	mts  rmsr,r3
	lwi  r3,  r1, 60
	rted r14,0 
	 addik r1,r1,64


/*
 * タスクディスパッチャ
 */
	.global dispatch
	.align  2
dispatch:
	/*
	 *  このルーチンは，タスクコンテキスト・CPUロック状態・ディスパッチ
	 *  許可状態・（モデル上の）割込み優先度マスク全解除状態で呼び出さ
	 *  れる．
	 */
	addi  r1,  r1,  -64  /* スクラッチレジスタ以外を保存する */
	swi   r15, r1,  60
	mfs   r3,  rmsr      /* msrの保存                        */
	swi   r3,  r1,  56                
	swi   r18, r1,  52   
	swi   r19, r1,  48
	swi   r20, r1,  44
	swi   r21, r1,  40
	swi   r22, r1,  36
	swi   r23, r1,  32
	swi   r24, r1,  28
	swi   r25, r1,  24
	swi   r26, r1,  20
	swi   r27, r1,  16
	swi   r28, r1,  12
	swi   r29, r1,  8
	swi   r30, r1,  4
	swi   r31, r1,  0
	lwi   r4 , r13, p_runtsk   /* r4 <- p_runtsk */
	swi   r1 , r4,  TCB_sp 	   /* タスクスタックをTCBに保存 */
	la    r5 , r0,  dispatch_r /* 実行再開番地を保存 */
	swi   r5 , r4,  TCB_pc     /* 実行再開番地をTCBに保存   */
	bri   dispatcher

dispatch_r:
	lwi  r31,  r1, 0      /* レジスタを復帰 */
	lwi  r30,  r1, 4
	lwi  r29,  r1, 8
	lwi  r28,  r1, 12
	lwi  r27,  r1, 16
	lwi  r26,  r1, 20
	lwi  r25,  r1, 24
	lwi  r24,  r1, 28
	lwi  r23,  r1, 32
	lwi  r22,  r1, 36
	lwi  r21,  r1, 40
	lwi  r20,  r1, 44
	lwi  r19,  r1, 48
	lwi  r18,  r1, 52
	lwi  r3,   r1, 56   
	mts  rmsr,r3            
	addi r1,   r1, 60   /* スタックポインタを戻す */

	/*
	 *  タスク例外処理ルーチンの起動
	 *  dispatch_r は dispatcher から呼び出されるため，
	 *  tcb のアドレスは r4 に入っている
	 */
	lbui r5,  r4,  TCB_enatex  /* r5 <- enatex                 */
	andi r6,  r5,  TCB_enatex_mask  
	beqi r6,  dispatch_r_1     /* enatex が false ならリターン */
	lwi  r7,  r4,  TCB_texptn  /* r5 <- texptn                 */
	beqi r7,  dispatch_r_1     /* texptn が 0 でなければ       */
	brlid r15  call_texrtn     /* タスク例外ルーチンの呼び出し */
	 nop
dispatch_r_1:
	Pop     r15
	rtsd	r15, 8
	 nop

/*
 *  ディスパッチャの動作開始
 */
    .globl start_dispatch
start_dispatch:
	/*
	 *  このルーチンは，カーネル起動時に，すべての割込みを禁止した状態
	 * （割込みロック状態と同等）で呼び出される．また，割込みモード（非
	 *  タスクコンテキストと同等）で呼び出されることを想定している．
	 *
	 *  dispatcherは，CPUロック状態，（モデル上の）割込み優先度マスク全
	 *  解除状態，例外（割込み/CPU例外）のネスト回数0で呼び出す．     
	 *  target_initializeでは，（モデル上の）割込み優先度マスク全解除
	 *  とし，カーネル管理外の割込みであるFIQを許可することで，
	 *  CPUロック状態・（モデル上の）割込み優先度マスク全解除状態になる．
	 *  また，task_initializeでdisdspをfalseに初期化しているため，ディ
	 *  スパッチ許可状態になっている．
	 */ 
	/* 例外（割込み/CPU例外）のネスト回数をクリア */
	swi   r0,  r13, except_nest_count

/*
 *  現在のコンテキストを捨ててディスパッチ
 */
    .globl exit_and_dispatch
exit_and_dispatch:
	/* ディスパッチャ本体（dispatcher）へ */  

/*
 *  ディスパッチャ本体
 */
dispatcher:
	/*
	 *  このルーチンは，タスクコンテキスト・CPUロック状態・ディスパッチ
	 *  許可状態・（モデル上の）割込み優先度マスク全解除状態で呼び出さ
	 *  れる．実行再開番地へもこの状態のまま分岐する．
	 */
#ifdef LOG_DSP_ENTER
	brlid r15, log_dsp_enter
	 lwi  r5,  r13, p_runtsk  /* r5 <- p_schedtsk            */
#endif /* LOG_DSP_ENTER */

dispatcher_0:
	lwi  r4,  r13, p_schedtsk /* r4 <- p_schedtsk            */
	swi  r4,  r13, p_runtsk   /* p_schedtsk を runtsk に     */        
	beqi r4,  dispatcher_1    /* p_schedtsk があるか         */
	lwi  r1,  r4,  TCB_sp     /* TCBからタスクスタックを復帰 */

#ifdef LOG_DSP_ENTER
	Push r4
	brlid r15, log_dsp_leave
	 Mov   r5, r4
	Pop  r4
#endif /* LOG_DSP_ENTER */

	lwi  r5,  r4,  TCB_pc     /* TCBから実行再開番地を復帰   */
	bra  r5
dispatcher_1:
	/*
	 * CPUロック状態の解除と，非タスクコンテキスト実行状態への
	 * 準備をする
	 */
	lwi   r1,   r2,  _kernel_istkpt   /* 非タスクコンテキスト用のスタックの読み込み */
	la    r6,   r0,  1                /* except_nest_count を1に                    */
	swi   r6,   r13, except_nest_count
	andi  r4,   r3, ~MSR_IE           /* IE = '0'               */
dispatcher_2:
	/*
	 *  割込みを許可し，非タスクコンテキスト実行状態とし割込みを待つ．
	 *
	 *  ここで非タスクコンテキスト実行状態に切り換えるのは，ここで発生
	 *  する割込み処理にどのスタックを使うかという問題の解決と，割込み
	 *  ハンドラ内でのタスクディスパッチの防止という2つの意味がある．
	 *
	 *  プロセッサを割込み待ちに移行させる処理と，割込み許可とは，不可
	 *  分に行なう必要がある．
	 *  これを不可分に行なわない場合，割込みを許可した直後に割込
	 *  みが入り，その中でタスクが実行可能状態になると，実行すべきタス
	 *  クがあるにもかかわらずプロセッサが割込み待ちになってしまう．
	 *
	 *  割込み待ちの間は，p_runtskをNULL（＝0）に設定しなければならな
	 *  い．このように設定しないと，割込みハンドラからiget_tidを呼び出
	 *  した際の動作が仕様に合致しなくなる．
	 *
	 */
	mfs   r3,   rmsr                 /* 割込み許可(MSR(IE)をセット)      */
	ori   r5,   r3, MSR_IE            
	mts   rmsr, r5
	nop
	nop
	mfs   r3,   rmsr                 /* 割込み禁止(MSR(IE)をクリア)      */
	andi  r5,   r3, ~MSR_IE            
	mts   rmsr, r5
	lwi   r6,  r13,  reqflg           /* r6 <- reqflg                     */
	beqi  r6,  dispatcher_2           /* reqflg が false なら             */
	swi   r0,  r13, except_nest_count /* except_nest_count をクリア       */
	swi   r0,  r13,  reqflg           /* reqflg を false に               */
	bri   dispatcher_0

/*
 *  カーネルの終了処理の呼出し
 *
 *  モードとスタックを非タスクコンテキスト用に切り替え．
 */
    .globl call_exit_kernel
call_exit_kernel:
	lwi   r1,  r2,  _kernel_istkpt /* 非タスクコンテキスト用のスタックの読み込み */
	brlid r15  exit_kernel         
	 nop

/*
 *  タスク開始時処理
 *
 *  スタックの取り方
 *  Microblaze Processor Reference Guide 52〜53より
 *  関数呼出し時の引数は，r5〜r10 に格納する．
 *  それと同時に呼出し元はスタックフレームに引数の格納場所を確保する必要
 *  がある．さらにその上にはリンクレジスタ(R15)の分の領域が必要となる．
 *  呼出し先は更に他の関数を呼び出す場合等に r5〜r10 をこの領域に保存す
 * る．
 *  Low Address
 *             --------------------
 *  new_sp -> | Link Register(R15) |
 *             --------------------
 *            |   Arg1用の領域     |
 *             --------------------
 *            |   Arg2用の領域     |
 *             --------------------        
 *            |      ....          |
 *             --------------------
 *  High Address
 *  dispatcherから呼び出されるため，TCBのアドレスは r4 に入っている
 */
	.text
	.global start_r
	.align 2
start_r:
	mfs   r3,   rmsr              /* 割込み許可(MSR(IE)をセット)   */
	ori   r5,   r3, MSR_IE            
	mts   rmsr, r5
	lwi   r3,  r4, TCB_p_tinib    /* r3  <- p_runtsk->p_tinib      */
	lwi   r5,  r3, TINIB_exinf    /* r5  <- 引数（exinf）          */
	lwi   r11, r3, TINIB_task     /* r11 <- タスクの実行番地       */
	la    r15, r0, (ext_tsk - 8)  /* タスクからの戻り先(ext_tsk)   */
	Push  r5                      /* スタック上の領域を確保        */
	Push  r15
	bra   r11                     /* タスクの実行開始              */

/*
 * 微少時間待ち
 */
	.global sil_dly_nse
sil_dly_nse:
	addi   r5, r5, -SIL_DLY_TIM1
	bgti   r5, sil_dly_nse_1
	rtsd   r15, 8
	nop  
sil_dly_nse_1:
	addi   r5, r5, -SIL_DLY_TIM2
	bgti   r5, sil_dly_nse_1
	rtsd   r15, 8
	 nop          
