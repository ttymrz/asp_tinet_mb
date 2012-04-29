/*
 *  TINET (TCP/IP Protocol Stack)
 * 
 *  Copyright (C) 2001-2009 by Dep. of Computer Science and Engineering
 *                   Tomakomai National College of Technology, JAPAN
 *
 *  上記著作権者は，以下の (1)〜(4) の条件か，Free Software Foundation 
 *  によって公表されている GNU General Public License の Version 2 に記
 *  述されている条件を満たす場合に限り，本ソフトウェア（本ソフトウェア
 *  を改変したものを含む．以下同じ）を使用・複製・改変・再配布（以下，
 *  利用と呼ぶ）することを無償で許諾する．
 *  (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
 *      権表示，この利用条件および下記の無保証規定が，そのままの形でソー
 *      スコード中に含まれていること．
 *  (2) 本ソフトウェアを，ライブラリ形式など，他のソフトウェア開発に使
 *      用できる形で再配布する場合には，再配布に伴うドキュメント（利用
 *      者マニュアルなど）に，上記の著作権表示，この利用条件および下記
 *      の無保証規定を掲載すること．
 *  (3) 本ソフトウェアを，機器に組み込むなど，他のソフトウェア開発に使
 *      用できない形で再配布する場合には，次の条件を満たすこと．
 *    (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
 *        作権表示，この利用条件および下記の無保証規定を掲載すること．
 *  (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
 *      害からも，上記著作権者およびTOPPERSプロジェクトを免責すること．
 *
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，その適用可能性も
 *  含めて，いかなる保証も行わない．また，本ソフトウェアの利用により直
 *  接的または間接的に生じたいかなる損害に関しても，その責任を負わない．
 * 
 *  @(#) $Id: tcp_usrreq.c,v 1.5 2009/12/24 05:47:21 abe Exp abe $
 */

/*
 * Copyright (c) 1982, 1986, 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	From: @(#)tcp_usrreq.c	8.2 (Berkeley) 1/3/94
 * $FreeBSD: src/sys/netinet/tcp_usrreq.c,v 1.40.2.3 1999/08/29 16:29:57 peter Exp $
 */

#include <string.h>

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
#include "kernel_cfg.h"

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <s_services.h>
#include <t_services.h>
#include "kernel_id.h"

#endif	/* of #ifdef TARGET_KERNEL_JSP */

#include <tinet_defs.h>
#include <tinet_config.h>

#include <net/if.h>
#include <net/if_ppp.h>
#include <net/if_loop.h>
#include <net/ethernet.h>
#include <net/ppp_ipcp.h>
#include <net/net.h>
#include <net/net_var.h>
#include <net/net_buf.h>
#include <net/net_timer.h>
#include <net/net_count.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet6/in6.h>
#include <netinet6/in6_var.h>
#include <netinet/in_itron.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/ip6.h>
#include <netinet6/ip6_var.h>
#include <netinet/tcp.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_seq.h>

#include <net/if_var.h>

#ifdef SUPPORT_TCP

/*
 *  IPv4 と IPv6 の切り替えマクロ
 */

#if defined(SUPPORT_INET4)

#define TCP_CRE_REP	tcp_cre_rep
#define TCP_ACP_CEP	tcp_acp_cep
#define TCP_CON_CEP	tcp_con_cep

#endif	/* of #if defined(SUPPORT_INET4) */

#if defined(SUPPORT_INET6)

#define TCP_CRE_REP	tcp6_cre_rep
#define TCP_ACP_CEP	tcp6_acp_cep
#define TCP_CON_CEP	tcp6_con_cep

#endif	/* of #if defined(SUPPORT_INET6) */

/*
 *  TINET をライブラリ化しない場合は、全ての機能を
 *  オブジェクトファイルに出力するため、マクロを有効にする。
 */

#ifndef TCP_CFG_LIBRARY

#define __tcp_cre_rep
#define __tcp_del_rep
#define __tcp_cre_cep
#define __tcp_del_cep
#define __tcp_acp_cep
#define __tcp_con_cep
#define __tcp_sht_cep
#define __tcp_cls_cep
#define __tcp_snd_dat
#define __tcp_rcv_dat
#define __tcp_snd_oob
#define __tcp_rcv_oob
#define __tcp_get_buf
#define __tcp_snd_buf
#define __tcp_rcv_buf
#define __tcp_rel_buf
#define __tcp_can_cep
#define __tcp_set_opt
#define __tcp_get_opt

#endif	/* of #ifndef TCP_CFG_LIBRARY */

/*
 *  tcp_cre_rep -- TCP 受付口の生成【拡張機能】
 */

#ifdef __tcp_cre_rep

#ifdef TCP_CFG_EXTENTIONS

ER
TCP_CRE_REP (ID repid, T_TCP_CREP *pk_crep)
{
	T_TCP_REP	*rep;
	ER		error;

	/* TCP 受付口 ID をチェックする。*/
	if (!VAID_TCP_REPID(repid))
		return E_ID;

	/* pk_crep が NULL ならエラー */
	if (pk_crep == NULL)
		return E_PAR;

	/* TCP 受付口を得る。*/
	rep = GET_TCP_REP(repid);

	/* TCP 受付口が、動的生成用でなければエラー */
	if (!DYNAMIC_TCP_REP(rep))
		return E_ID;

	/* 受付口をロックする。*/
	syscall(wai_sem(rep->semid_lock));

	/*
	 * TCP 受付口をチェックする。生成済みであればエラー
	 */
	if (VALID_TCP_REP(rep))
		error = E_OBJ;
	else {

		/* TCP 受付口生成情報をコピーする。*/
		rep->repatr = pk_crep->repatr;		/* 受付口属性		*/
		rep->myaddr = pk_crep->myaddr;		/* 自分のアドレス	*/

		/* TCP 受付口を生成済みにする。*/
		rep->flags |= TCP_REP_FLG_VALID;
		error = E_OK;
		}

	/* 受付口のロックを解除する。*/
	syscall(sig_sem(rep->semid_lock));

	return error;
	}

#endif	/* of #ifdef TCP_CFG_EXTENTIONS */

#endif	/* of #ifdef __tcp_cre_cep */

#ifdef __tcp_del_rep

#ifdef TCP_CFG_EXTENTIONS

/*
 *  tcp_find_cep_rep -- TCP 受付口をリンクしている TCP 通信端点を得る。
 */

static T_TCP_CEP*
tcp_find_cep_rep (T_TCP_REP* rep)
{
	T_TCP_CEP*	cep;

	for (cep = &tcp_cep[tmax_tcp_cepid]; cep -- != tcp_cep; ) {
		if (cep->rep == rep)
			return cep;
		}

	return NULL;
	}

/*
 *  tcp_del_rep -- TCP 受付口の削除【拡張機能】
 */


ER
tcp_del_rep (ID repid)
{
	T_TCP_CEP	*cep;
	T_TCP_REP	*rep;
	ER		error = E_OK;

	/* TCP 受付口 ID をチェックする。*/
	if (!VAID_TCP_REPID(repid))
		return E_ID;

	/* TCP 受付口を得る。*/
	rep = GET_TCP_REP(repid);

	/* TCP 受付口が、動的生成用でなければエラー */
	if (!DYNAMIC_TCP_REP(rep))
		return E_ID;

	/* 受付口をロックする。*/
	syscall(wai_sem(rep->semid_lock));

	/* TCP 受付口をチェックする。未生成の場合はエラー */
	if (!VALID_TCP_REP(rep))
		error = E_NOEXS;
	else {
		if ((cep = tcp_find_cep_rep(rep)) != NULL) {
			/*
			 * すでに受動オープンしているとき
			 *（tcp_acp_cep が呼ばれているとき）は、
			 * tcp_acp_cep を終了させる。
			 */

			/* 通信端点をロックする。*/
			syscall(wai_sem(cep->semid_lock));

			/*
			 *  通信端点から受付口を解放し、
			 *  イベントフラグをクローズに設定する。
			 */
			cep->rep = NULL;
			cep->fsm_state = TCP_FSM_CLOSED;
			cep->error = E_DLT;
			syscall(set_flg(cep->est_flgid, TCP_CEP_EVT_CLOSED));

#ifdef TCP_CFG_NON_BLOCKING

			if (cep->rcv_nblk_tfn != TFN_TCP_UNDEF) {
				if (IS_PTR_DEFINED(cep->callback)) {

#ifdef TCP_CFG_NON_BLOCKING_COMPAT14

					(*cep->callback)(GET_TCP_CEPID(cep), cep->rcv_nblk_tfn, (void*)E_DLT);

#else	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

					ER	error = E_DLT;

					(*cep->callback)(GET_TCP_CEPID(cep), cep->rcv_nblk_tfn, (void*)&error);

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

					}
				else
					error = E_OBJ;
				cep->rcv_nblk_tfn = TFN_TCP_UNDEF;
				}

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

			cep->rcv_tskid = TA_NULL;
			cep->rcv_tfn   = TFN_TCP_UNDEF;

			/* 通信端点のロックを解除する。*/
			syscall(sig_sem(cep->semid_lock));

			}
		else
			error = E_OK;

		/* TCP 受付口を未生成にする。*/
		rep->flags &= ~TCP_REP_FLG_VALID;
		}

	/* 受付口のロックを解除する。*/
	syscall(sig_sem(rep->semid_lock));

	return error;
	}

#endif	/* of #ifdef TCP_CFG_EXTENTIONS */

#endif	/* of #ifdef __tcp_del_rep */

/*
 *  tcp_cre_cep -- TCP 通信端点の生成【拡張機能】
 */

#ifdef __tcp_cre_cep

#ifdef TCP_CFG_EXTENTIONS

ER
tcp_cre_cep (ID cepid, T_TCP_CCEP *pk_ccep)
{
	T_TCP_CEP	*cep;
	ER		error;

	/* TCP 通信端点 ID をチェックする。*/
	if (!VAID_TCP_CEPID(cepid))
		return E_ID;

	/* pk_ccep が NULL ならエラー */
	if (pk_ccep == NULL)
		return E_PAR;

	/* TCP 通信端点を得る。*/
	cep = GET_TCP_CEP(cepid);

	/* TCP 通信端点が、動的生成用でなければエラー */
	if (!DYNAMIC_TCP_CEP(cep))
		return E_ID;

	/* 通信端点をロックする。*/
	syscall(wai_sem(cep->semid_lock));

	/*
	 * TCP 通信端点をチェックする。生成済みであればエラー
	 */
	if (VALID_TCP_CEP(cep))
		error = E_OBJ;
	else {

		/* TCP 通信端点生成情報をコピーする。*/
		cep->cepatr   = pk_ccep->cepatr;		/* 通信端点属性			*/
		cep->sbuf     = pk_ccep->sbuf;			/* 送信用ウィンドバッファ	*/
		cep->sbufsz   = pk_ccep->sbufsz;		/* 送信用ウィンドバッファサイズ	*/
		cep->rbuf     = pk_ccep->rbuf;			/* 受信用ウィンドバッファ	*/
		cep->rbufsz   = pk_ccep->rbufsz;		/* 受信用ウィンドバッファサイズ	*/
		cep->callback = (void*)pk_ccep->callback;	/* コールバック			*/

		/* TCP 通信端点を生成済みにする。*/
		cep->flags |= TCP_CEP_FLG_VALID;
		error = E_OK;
		}

	/* 通信端点のロックを解除する。*/
	syscall(sig_sem(cep->semid_lock));

	return error;
	}

#endif	/* of #ifdef TCP_CFG_EXTENTIONS */

#endif	/* of #ifdef __tcp_cre_cep */

/*
 *  tcp_del_cep -- TCP 通信端点の削除【拡張機能】
 */

#ifdef __tcp_del_cep

#ifdef TCP_CFG_EXTENTIONS

ER
tcp_del_cep (ID cepid)
{
	T_TCP_CEP	*cep;
	ER		error;

	/* TCP 通信端点 ID をチェックする。*/
	if (!VAID_TCP_CEPID(cepid))
		return E_ID;

	/* TCP 通信端点を得る。*/
	cep = GET_TCP_CEP(cepid);

	/* TCP 通信端点が、動的生成用でなければエラー */
	if (!DYNAMIC_TCP_CEP(cep))
		return E_ID;

	/* 通信端点をロックする。*/
	syscall(wai_sem(cep->semid_lock));

	/*
	 * TCP 通信端点をチェックする。以下の場合はエラー
	 * ・未生成。
	 * ・使用中。
	 */
	if (!VALID_TCP_CEP(cep))
		error = E_NOEXS;
	else if (cep->fsm_state != TCP_FSM_CLOSED)
		error = E_OBJ;
	else {

		/* TCP 通信端点を未生成にする。*/
		cep->flags &= ~TCP_CEP_FLG_VALID;
		error = E_OK;
		}

	/* 通信端点のロックを解除する。*/
	syscall(sig_sem(cep->semid_lock));

	return error;
	}

#endif	/* of #ifdef TCP_CFG_EXTENTIONS */

#endif	/* of #ifdef __tcp_del_cep */

#ifdef TCP_CFG_PASSIVE_OPEN

/*
 *  tcp_acp_cep -- 接続要求待ち (受動オープン)【標準機能】
 */

#ifdef __tcp_acp_cep

ER
TCP_ACP_CEP (ID cepid, ID repid, T_IPEP *p_dstaddr, TMO tmout)
{
	T_TCP_REP	*rep;
	T_TCP_CEP	*cep;
	ER		error;
	FLGPTN		flag;

	/* TCP 受付口をチェックする。*/
	if (!VAID_TCP_REPID(repid))
		return E_ID;

#ifdef TCP_CFG_NON_BLOCKING

	/* p_dstaddr が NULL ならエラー */
	if (p_dstaddr == NULL)
		return E_PAR;

#else	/* of #ifdef TCP_CFG_NON_BLOCKING */

	/* p_dstaddr が NULL か、tmout が TMO_NBLK ならエラー */
	if (p_dstaddr == NULL || tmout == TMO_NBLK)
		return E_PAR;

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

	/*
	 *  CEP をロックし、API 機能コードとタスク識別子を記録する。
	 *  すでに記録されていれば、ペンディング中なのでエラー
	 */
	if ((error = tcp_lock_cep(cepid, &cep, TFN_TCP_ACP_CEP)) != E_OK)
		return error;

	/* CEP の FSM がクローズ状態でなければエラー。*/
	if (cep->fsm_state != TCP_FSM_CLOSED) {
		error = E_OBJ;
		goto err_ret;
		}
	syscall(clr_flg(cep->est_flgid, (FLGPTN)(~TCP_CEP_EVT_CLOSED)));

	/* TCP 通信端点を初期化する。*/
	tcp_init_cep(cep);

	/* TCP 受付口を得る。*/
	rep = GET_TCP_REP(repid);

#ifdef TCP_CFG_EXTENTIONS

	/* TCP 受付口をロックする。*/
	syscall(wai_sem(rep->semid_lock));

	/* TCP 受付口をチェックする。*/
	if (!VALID_TCP_REP(rep)) {
		syscall(sig_sem(rep->semid_lock));
		error = E_NOEXS;
		goto err_ret;
		}

#endif	/* of #ifdef TCP_CFG_EXTENTIONS */

	/* TCP 通信端点にTCP受付口を記録する。*/
	cep->rep = rep;

#ifdef TCP_CFG_EXTENTIONS

	/* TCP 受付口のロックを解除する。*/
	syscall(sig_sem(rep->semid_lock));

#endif	/* of #ifdef TCP_CFG_EXTENTIONS */

	/* TCP 受付口のアドレスをコピーする。*/
	cep->myaddr = rep->myaddr;

	/* 通信端点を設定する。*/
	cep->fsm_state = TCP_FSM_LISTEN;

#ifdef TCP_CFG_NON_BLOCKING

	/* タイムアウトをチェックする。*/
	if (tmout == TMO_NBLK) {
		/* ノンブロッキングコール */
		cep->p_dstaddr    = p_dstaddr;
		cep->rcv_nblk_tfn = TFN_TCP_ACP_CEP;
		return E_WBLK;
		}
	else {

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

		/*
		 *  FSM が ESTABLISHED になるまで待つ。
		 *  FSM が CLOSED になった場合は、エラーが発生したことを意味している。
		 */
		error = twai_flg(cep->est_flgid, (TCP_CEP_EVT_CLOSED |
		                                  TCP_CEP_EVT_ESTABLISHED), TWF_ORW, &flag, tmout);
		if (error == E_OK) {
			if (cep->error != E_OK)
				error = cep->error;
			else if (cep->fsm_state == TCP_FSM_CLOSED)
				error = E_TMOUT;
			}

		syscall(clr_flg(cep->est_flgid, (FLGPTN)(~TCP_CEP_EVT_ESTABLISHED)));

		if (error == E_OK) {
			/* 相手のアドレスをコピーする。*/
			*p_dstaddr = cep->dstaddr;
			}
		else {
			/*
			 *  通信端点から受付口を解放し、
			 *  イベントフラグをクローズに設定する。
			 */
			cep->rep = NULL;
			cep->fsm_state = TCP_FSM_CLOSED;
			syscall(set_flg(cep->est_flgid, TCP_CEP_EVT_CLOSED));
			}

#ifdef TCP_CFG_NON_BLOCKING

		}

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

err_ret:
	cep->rcv_tskid = TA_NULL;
	cep->rcv_tfn   = TFN_TCP_UNDEF;
	return error;
	}

#endif	/* of #ifdef __tcp_acp_cep */

#endif	/* of #ifdef TCP_CFG_PASSIVE_OPEN */

/*
 *  tcp_con_cep -- 接続要求 (能動オープン)【標準機能】
 */

#ifdef __tcp_con_cep

ER
TCP_CON_CEP (ID cepid, T_IPEP *p_myaddr, T_IPEP *p_dstaddr, TMO tmout)
{
	T_TCP_CEP	*cep;
	ER		error;
	FLGPTN		flag;

#ifdef TCP_CFG_NON_BLOCKING

	/*
	 *  p_dstaddr または p_myaddr が NULL か、
	 *  あて先がマルチキャストアドレスならエラー
	 */
	if (p_myaddr == NULL || p_dstaddr == NULL || IN_IS_ADDR_MULTICAST(&p_dstaddr->ipaddr))
		return E_PAR;

#else	/* of #ifdef TCP_CFG_NON_BLOCKING */

	/*
	 *  p_dstaddr または p_myaddr が NULL 、
	 *  あて先がマルチキャストアドレスか、
	 *  tmout が TMO_NBLK ならエラー
	 */
	if (p_myaddr == NULL || p_dstaddr == NULL ||
	    IN_IS_ADDR_MULTICAST(&p_dstaddr->ipaddr) || tmout == TMO_NBLK)
		return E_PAR;

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

	/*
	 *  CEP をロックし、API 機能コードとタスク識別子を記録する。
	 *  すでに記録されていれば、ペンディング中なのでエラー
	 */
	if ((error = tcp_lock_cep(cepid, &cep, TFN_TCP_CON_CEP)) != E_OK)
		return error;

	/* CEP の FSM がクローズ状態でなければエラー。*/
	if (cep->fsm_state != TCP_FSM_CLOSED) {
		error = E_OBJ;
		goto err_ret;
		}
	syscall(clr_flg(cep->est_flgid, (FLGPTN)(~TCP_CEP_EVT_CLOSED)));

	/* シーケンス番号を初期化する。*/
	if (tcp_iss == 0)
		tcp_init_iss();

	/* 通信端点を初期化する。*/
	tcp_init_cep(cep);

	/*
	 *  p_myaddr が NADR (-1) ではなく、自 IP アドレスが ANY でなければ、
	 *  指定された IP アドレスを割り当てる。
	 */
	if (p_myaddr != NADR && !IN_IS_ADDR_ANY(&p_myaddr->ipaddr))
		cep->myaddr.ipaddr = p_myaddr->ipaddr;
	else {
		T_IN_IFADDR	*ia;

		if ((ia = IN_IFAWITHIFP(IF_GET_IFNET(), &p_dstaddr->ipaddr)) == NULL) {
			error = E_PAR;
			goto err_ret;
			}
		cep->myaddr.ipaddr = ia->addr;
		}

	/* 通信端点を設定する。*/
	cep->fsm_state = TCP_FSM_SYN_SENT;
	cep->dstaddr   = *p_dstaddr;
	cep->iss       = tcp_iss;
	cep->timer[TCP_TIM_KEEP] = TCP_TVAL_KEEP_INIT;
	tcp_iss += TCP_ISS_INCR() / 2;
	init_send_seq(cep);

#ifdef TCP_CFG_NON_BLOCKING

	/* タイムアウトをチェックする。*/
	if (tmout == TMO_NBLK) {
		/* ノンブロッキングコール */
		cep->p_dstaddr    = p_dstaddr;
		cep->p_myaddr     = p_myaddr;
		cep->snd_nblk_tfn = TFN_TCP_CON_CEP;

		/* コネクションの開設をポストする。*/
		cep->flags |= TCP_CEP_FLG_POST_OUTPUT;
		sig_sem(SEM_TCP_POST_OUTPUT);
		return E_WBLK;
		}
	else {

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

		/*
		 *  p_myaddr が NADR (-1) か、
		 *  自ポート番号が TCP_PORTANY なら、自動で割り当てる。
		 */
		if (p_myaddr == NADR || p_myaddr->portno == TCP_PORTANY)
			tcp_alloc_auto_port(cep);
		else if ((error = tcp_alloc_port(cep, p_myaddr->portno)) != E_OK)
			goto err_ret;

		/* コネクションの開設をポストする。*/
		cep->flags |= TCP_CEP_FLG_POST_OUTPUT;
		sig_sem(SEM_TCP_POST_OUTPUT);

		/*
		 *  イベントが ESTABLISHED になるまで待つ。
		 *  イベントが CLOSED になった場合は、何らかのエラーが発生したか、
		 *  接続要求が拒否されたことを意味している。
		 */
		error = twai_flg(cep->est_flgid, (TCP_CEP_EVT_CLOSED |
		                                  TCP_CEP_EVT_ESTABLISHED), TWF_ORW, &flag, tmout);
		if (error == E_OK) {
			if (cep->error != E_OK)
				error = cep->error;
			else if (cep->fsm_state == TCP_FSM_CLOSED)
				error = E_CLS;
			}

		syscall(clr_flg(cep->est_flgid, (FLGPTN)(~TCP_CEP_EVT_ESTABLISHED)));

		if (error != E_OK) {
			/*
			 *  通信端点から受付口を解放し、
			 *  イベントフラグをクローズに設定する。
			 */
			cep->rep = NULL;
			cep->fsm_state = TCP_FSM_CLOSED;
			syscall(set_flg(cep->est_flgid, TCP_CEP_EVT_CLOSED));
			}

#ifdef TCP_CFG_NON_BLOCKING

		}

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

err_ret:
	cep->snd_tskid = TA_NULL;
	cep->snd_tfn   = TFN_TCP_UNDEF;
	return error;
	}

#endif	/* of #ifdef __tcp_con_cep */

#ifdef __tcp_cls_cep

/*
 *  tcp_user_closed -- ユーザからのコネクションの開放
 */

T_TCP_CEP *
tcp_user_closed (T_TCP_CEP *cep)
{
	switch (cep->fsm_state) {

	case TCP_FSM_CLOSED:		/* クローズ	*/
	case TCP_FSM_LISTEN:		/* 受動オープン	*/
		cep->fsm_state = TCP_FSM_CLOSED;
		cep = tcp_close(cep);
		break;

	case TCP_FSM_SYN_SENT:		/* 能動オープン、SYN 送信済み	*/
	case TCP_FSM_SYN_RECVD:		/* SYN を受信し、SYN 送信済み	*/
		cep->flags |= TCP_CEP_FLG_NEED_FIN;
		break;

	case TCP_FSM_ESTABLISHED:	/* コネクション開設完了	*/
		cep->fsm_state = TCP_FSM_FIN_WAIT_1;
		break;

	case TCP_FSM_CLOSE_WAIT:	/* 相手から FIN 受信、APP の終了待ち */
		cep->fsm_state = TCP_FSM_LAST_ACK;
		break;
		}

	if (cep != NULL && cep->fsm_state == TCP_FSM_FIN_WAIT_2)
		cep->timer[TCP_TIM_2MSL] = TCP_TVAL_KEEP_COUNT * TCP_TVAL_KEEP_INTERVAL;
	return cep;
	}

/*
 *  tcp_cls_cep -- 通信端点のクローズ【標準機能】
 */

ER
tcp_cls_cep (ID cepid, TMO tmout)
{
	T_TCP_CEP	*cep;
	ER		error = E_OK;
	FLGPTN		flag;

#ifndef TCP_CFG_NON_BLOCKING

	/* tmout が TMO_NBLK ならエラー */
	if (tmout == TMO_NBLK)
		return E_PAR;

#endif	/* of #ifndef TCP_CFG_NON_BLOCKING */

	/*
	 *  CEP をロックし、API 機能コードとタスク識別子を記録する。
	 *  すでに記録されていれば、ペンディング中なのでエラー
	 */
	if ((error = tcp_lock_cep(cepid, &cep, TFN_TCP_CLS_CEP)) != E_OK)
		return error;

#ifdef TCP_CFG_NON_BLOCKING

	/* タイムアウトをチェックする。*/
	if (tmout == TMO_NBLK) {				/* ノンブロッキングコール */

		if (!IS_PTR_DEFINED(cep->callback))
			error = E_OBJ;
		else if (cep->fsm_state == TCP_FSM_CLOSED) {	/* すでにクローズされているとき */

#ifdef TCP_CFG_NON_BLOCKING_COMPAT14

			(*cep->callback)(GET_TCP_CEPID(cep), TFN_TCP_CLS_CEP, E_OK);
			error = E_WBLK;

#else	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

			ER	error = E_OK;

			(*cep->callback)(GET_TCP_CEPID(cep), TFN_TCP_CLS_CEP, (void*)&error);
			error = E_WBLK;

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

			}
		else {
			/* NBLK のAPI 機能コードを設定する。*/
			cep->rcv_nblk_tfn = TFN_TCP_CLS_CEP;

			if ((cep = tcp_user_closed(cep)) != NULL) {	/* コネクションを切断する。*/
				/* 切断セグメント出力をポストする。*/
				cep->flags |= TCP_CEP_FLG_POST_OUTPUT;
				sig_sem(SEM_TCP_POST_OUTPUT);
				}

			/*  cep が NULL で戻ってきた場合は、
			 *  既にコネクションが切断されていることを意味している。
			 *  また、この場合コールバック関数が呼び出されている。
			 */
			return E_WBLK;
			}
		}
	else {							/* 非ノンブロッキングコール */

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

		if ((cep = tcp_user_closed(cep)) == NULL) {	/* コネクションを切断する。*/

			/*  cep が NULL で戻ってきた場合は、
			 *  既にコネクションが切断されていることを意味している。
			 */
			return error;
			}
		else {
			/* 切断セグメント出力をポストする。*/
			cep->flags |= TCP_CEP_FLG_POST_OUTPUT;
			sig_sem(SEM_TCP_POST_OUTPUT);

			/* イベントフラグが CLOSED になるまで待つ。*/
			error = twai_flg(cep->est_flgid, TCP_CEP_EVT_CLOSED, TWF_ORW, &flag, tmout);
			if (error == E_OK && cep->error != E_OK)
				error = cep->error;

			if (error != E_OK) {
				if (error == E_RLWAI) {
					/* tcp_cls_cep がキャンセルされたときは、RST を送信する。*/
					tcp_respond(NULL, cep, cep->rcv_nxt, cep->snd_una - 1, 
					            cep->rbufsz - cep->rwbuf_count, TCP_FLG_RST);
					}

				/* タイマーを停止する。*/
				tcp_cancel_timers(cep);

				/* 
				 *  通信端点をロックし、
				 *  受信再構成キューのネットワークバッファを解放する。
				 */
				syscall(wai_sem(cep->semid_lock));
				tcp_free_reassq(cep);
				syscall(sig_sem(cep->semid_lock));

				/* 状態を未使用にする。*/
				cep->fsm_state = TCP_FSM_CLOSED;

				/*
				 * 以下に関係しないフラグをクリアーする。
				 * ・送受信ウィンドバッファの省コピー機能
				 * ・動的な通信端点の生成・削除機能
				 */
				cep->flags &= (TCP_CEP_FLG_WBCS_NBUF_REQ | TCP_CEP_FLG_WBCS_MASK | 
				               TCP_CEP_FLG_DYNAMIC       | TCP_CEP_FLG_VALID);

				/* 
				 *  通信端点をロックし、
				 *  送受信ウィンドバッファキューのネットワークバッファを解放する。
				 */
				cep->rwbuf_count = 0;
				syscall(wai_sem(cep->semid_lock));
				TCP_FREE_RWBUFQ(cep);
				TCP_FREE_SWBUFQ(cep);
				syscall(sig_sem(cep->semid_lock));

				syscall(set_flg(cep->est_flgid, TCP_CEP_EVT_CLOSED));
				}
			}

#ifdef TCP_CFG_NON_BLOCKING

		}

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

	cep->rcv_tskid = TA_NULL;
	cep->rcv_tfn   = TFN_TCP_UNDEF;
	return error;
	}

#endif	/* of #ifdef __tcp_cls_cep */

#ifdef __tcp_sht_cep

/*
 *  tcp_sht_cep  -- データ送信の終了【標準機能】
 */

ER
tcp_sht_cep (ID cepid)
{
	T_TCP_CEP	*cep;
	ER		error = E_OK;

	/*
	 *  CEP をロックし、API 機能コードとタスク識別子を記録する。
	 *  すでに記録されていれば、ペンディング中なのでエラー
	 */
	if ((error = tcp_lock_cep(cepid, &cep, TFN_TCP_SHT_CEP)) != E_OK)
		return error;

	/* TCP 通信端点のコネクションが確立状態でなければエラー */
	if (!TCP_FSM_HAVE_ESTABLISHED(cep->fsm_state)) {
		if ((error = cep->error) == E_OK)
			error = E_OBJ;
		}

	else if ((cep = tcp_user_closed(cep)) != NULL) {		/* コネクションを切断する。*/

		/* 切断セグメント出力をポストする。*/
		cep->flags |= TCP_CEP_FLG_POST_OUTPUT;
		sig_sem(SEM_TCP_POST_OUTPUT);
		}

	cep->snd_tskid = TA_NULL;
	cep->snd_tfn   = TFN_TCP_UNDEF;
	return error;
	}

#endif	/* of #ifdef __tcp_sht_cep */

#ifdef __tcp_snd_dat

/*
 *  tcp_snd_dat -- パケットの送信【標準機能】
 */

ER_UINT
tcp_snd_dat (ID cepid, void *data, int_t len, TMO tmout)
{
	T_TCP_CEP	*cep;
	ER_UINT		error;

#ifdef TCP_CFG_NON_BLOCKING

	/* data が NULL か、len < 0 ならエラー */
	if (data == NULL || len < 0)
		return E_PAR;

#else	/* of #ifdef TCP_CFG_NON_BLOCKING */

	/* data が NULL、len < 0 か、tmout が TMO_NBLK ならエラー */
	if (data == NULL || len < 0 || tmout == TMO_NBLK)
		return E_PAR;

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

	/*
	 *  CEP をロックし、API 機能コードとタスク識別子を記録する。
	 *  すでに記録されていれば、ペンディング中なのでエラー
	 */
	if ((error = tcp_lock_cep(cepid, &cep, TFN_TCP_SND_DAT)) != E_OK)
		return error;

	/* 送信できるか、通信端点の状態を見る。*/
	if ((error = tcp_can_send_more(cep, TFN_TCP_SND_DAT, tmout)) != E_OK)
		goto err_ret;

#ifdef TCP_CFG_NON_BLOCKING

	/* タイムアウトをチェックする。*/
	if (tmout == TMO_NBLK) {		/* ノンブロッキングコール */

		/* 送信ウィンドバッファに空きがあればコールバック関数を呼び出す。*/
		if (!TCP_IS_SWBUF_FULL(cep)) {

		 	/* 送信ウィンドバッファにデータを書き込む。*/
			error = TCP_WRITE_SWBUF(cep, data, (uint_t)len);

			/* 出力をポストする。*/
			cep->flags |= TCP_CEP_FLG_POST_OUTPUT;
			sig_sem(SEM_TCP_POST_OUTPUT);

#ifdef TCP_CFG_NON_BLOCKING_COMPAT14

			/* コールバック関数を呼び出す。*/
			(*cep->callback)(GET_TCP_CEPID(cep), TFN_TCP_SND_DAT, (void*)error);

#else	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

			/* コールバック関数を呼び出す。*/
			(*cep->callback)(GET_TCP_CEPID(cep), TFN_TCP_SND_DAT, (void*)&error);

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

			error = E_WBLK;
			goto err_ret;
			}
		else {
			cep->snd_data     = data;
			cep->snd_len      = len;
			cep->snd_nblk_tfn = TFN_TCP_SND_DAT;
			TCP_ALLOC_SWBUF(cep);

			return E_WBLK;
			}
		}
	else {		/* 非ノンブロッキングコール */

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

	 	/* 送信ウィンドバッファが空くのを待つ。*/
		if ((error = TCP_WAIT_SWBUF(cep, tmout)) != E_OK)
			goto err_ret;

	 	/* 送信ウィンドバッファにデータを書き込む。*/
		if ((error = TCP_WRITE_SWBUF(cep, data, (uint_t)len)) > 0) {

			/* データを送信する。送信ウィンドバッファがフルのときは強制的に送信する。*/
			if (TCP_IS_SWBUF_FULL(cep))
				cep->flags |= TCP_CEP_FLG_FORCE | TCP_CEP_FLG_FORCE_CLEAR;

			/* 出力をポストする。*/
			cep->flags |= TCP_CEP_FLG_POST_OUTPUT;
			sig_sem(SEM_TCP_POST_OUTPUT);
			}

#ifdef TCP_CFG_NON_BLOCKING

		}

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

err_ret:
	cep->snd_tskid = TA_NULL;
	cep->snd_tfn   = TFN_TCP_UNDEF;
	return error;
	}

#endif	/* of #ifdef __tcp_snd_dat */

#ifdef __tcp_snd_oob

#ifdef TCP_CFG_EXTENTIONS

/*
 *  tcp_snd_oob -- 緊急データの送信【拡張機能】
 */

ER_UINT
tcp_snd_oob (ID cepid, void *data, int_t len, TMO tmout)
{
	T_TCP_CEP	*cep;
	ER_UINT		error;

#ifdef TCP_CFG_NON_BLOCKING

	/* data が NULL か、len < 0 ならエラー */
	if (data == NULL || len < 0)
		return E_PAR;

#else	/* of #ifdef TCP_CFG_NON_BLOCKING */

	/* data が NULL、len < 0 か、tmout が TMO_NBLK ならエラー */
	if (data == NULL || len < 0 || tmout == TMO_NBLK)
		return E_PAR;

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

	/*
	 *  CEP をロックし、API 機能コードとタスク識別子を記録する。
	 *  すでに記録されていれば、ペンディング中なのでエラー
	 */
	if ((error = tcp_lock_cep(cepid, &cep, TFN_TCP_SND_OOB)) != E_OK)
		return error;

	/* 送信できるか、通信端点の状態を見る。*/
	if ((error = tcp_can_send_more(cep, TFN_TCP_SND_OOB, tmout)) != E_OK)
		goto err_ret;

#ifdef TCP_CFG_NON_BLOCKING

	/* タイムアウトをチェックする。*/
	if (tmout == TMO_NBLK) {		/* ノンブロッキングコール */

		/* 送信ウィンドバッファに空きがあればコールバック関数を呼び出す。*/
		if (!TCP_IS_SWBUF_FULL(cep)) {

		 	/* 送信ウィンドバッファにデータを書き込む。*/
			error = TCP_WRITE_SWBUF(cep, data, (uint_t)len);

			/* 送信緊急ポインタを設定する。*/
			cep->snd_up = cep->snd_una + cep->swbuf_count;

			/* 出力をポストする。*/
			cep->flags |= TCP_CEP_FLG_POST_OUTPUT;
			sig_sem(SEM_TCP_POST_OUTPUT);

#ifdef TCP_CFG_NON_BLOCKING_COMPAT14

			/* コールバック関数を呼び出す。*/
			(*cep->callback)(GET_TCP_CEPID(cep), TFN_TCP_SND_OOB, (void*)error);

#else	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

			/* コールバック関数を呼び出す。*/
			(*cep->callback)(GET_TCP_CEPID(cep), TFN_TCP_SND_OOB, (void*)&error);

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

			error = E_WBLK;
			goto err_ret;
			}
		else {
			cep->snd_data     = data;
			cep->snd_len      = len;
			cep->snd_nblk_tfn = TFN_TCP_SND_OOB;
			TCP_ALLOC_SWBUF(cep);

			return E_WBLK;
			}
		}
	else {		/* 非ノンブロッキングコール */

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

	 	/* 送信ウィンドバッファが空くのを待つ。*/
		if ((error = TCP_WAIT_SWBUF(cep, tmout)) != E_OK)
			goto err_ret;

	 	/* 送信ウィンドバッファにデータを書き込む。*/
		if ((error = TCP_WRITE_SWBUF(cep, data, (uint_t)len)) > 0) {

			/* 送信緊急ポインタを設定する。*/
			cep->snd_up = cep->snd_una + cep->swbuf_count;

			/* データを送信する。送信ウィンドバッファがフルのときは強制的に送信する。*/
			if (TCP_IS_SWBUF_FULL(cep))
				cep->flags |= TCP_CEP_FLG_FORCE | TCP_CEP_FLG_FORCE_CLEAR;

			/* 出力をポストする。*/
			cep->flags |= TCP_CEP_FLG_POST_OUTPUT;
			sig_sem(SEM_TCP_POST_OUTPUT);
			}

#ifdef TCP_CFG_NON_BLOCKING

		}

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

err_ret:
	cep->snd_tskid = TA_NULL;
	cep->snd_tfn   = TFN_TCP_UNDEF;
	return error;
	}

#endif	/* of #ifdef TCP_CFG_EXTENTIONS */

#endif	/* of #ifdef __tcp_snd_oob */

#ifdef __tcp_get_buf

/*
 *  tcp_get_buf -- 送信用バッファの獲得【標準機能】
 */

ER_UINT
tcp_get_buf (ID cepid, void **p_buf, TMO tmout)
{
	T_TCP_CEP	*cep;
	ER_UINT		error;

#ifdef TCP_CFG_NON_BLOCKING

	/* p_buf が NULL ならエラー */
	if (p_buf == NULL)
		return E_PAR;

#else	/* of #ifdef TCP_CFG_NON_BLOCKING */

	/* p_buf が NULL か、tmout が TMO_NBLK ならエラー */
	if (p_buf == NULL || tmout == TMO_NBLK)
		return E_PAR;

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

	/*
	 *  CEP をロックし、API 機能コードとタスク識別子を記録する。
	 *  すでに記録されていれば、ペンディング中なのでエラー
	 */
	if ((error = tcp_lock_cep(cepid, &cep, TFN_TCP_GET_BUF)) != E_OK)
		return error;

	/* 送信できるか、通信端点の状態を見る。*/
	if ((error = tcp_can_send_more(cep, TFN_TCP_GET_BUF, tmout)) != E_OK)
		goto err_ret;

#ifdef TCP_CFG_NON_BLOCKING

	/* タイムアウトをチェックする。*/
	if (tmout == TMO_NBLK) {		/* ノンブロッキングコール */

		if (!TCP_IS_SWBUF_FULL(cep)) {

			/* 送信ウィンドバッファに空きがあればコールバック関数を呼び出す。*/
			error = TCP_GET_SWBUF_ADDR(cep, p_buf);

#ifdef TCP_CFG_NON_BLOCKING_COMPAT14

			/* コールバック関数を呼び出す。*/
			(*cep->callback)(GET_TCP_CEPID(cep), TFN_TCP_GET_BUF, (void*)error);

#else	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

			/* コールバック関数を呼び出す。*/
			(*cep->callback)(GET_TCP_CEPID(cep), TFN_TCP_GET_BUF, (void*)&error);

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

			error = E_WBLK;
			goto err_ret;
			}
		else {
			cep->snd_p_buf    = p_buf;
			cep->snd_nblk_tfn = TFN_TCP_GET_BUF;
			TCP_ALLOC_SWBUF(cep);
			return E_WBLK;
			}
		}

	else {		/* 非ノンブロッキングコール */

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

	 	/* 送信ウィンドバッファが空くのを待つ。*/
		if ((error = TCP_WAIT_SWBUF(cep, tmout)) != E_OK)
			goto err_ret;

	 	/* 送信ウィンドバッファの空アドレスを獲得する。*/
		error = TCP_GET_SWBUF_ADDR(cep, p_buf);

#ifdef TCP_CFG_NON_BLOCKING

		}

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

err_ret:
	cep->snd_tskid = TA_NULL;
	cep->snd_tfn   = TFN_TCP_UNDEF;
	return error;
	}

#endif	/* of #ifdef __tcp_get_buf */

#ifdef __tcp_snd_buf

/*
 *  tcp_snd_buf -- バッファ内のデータの送信【標準機能】
 */

ER
tcp_snd_buf (ID cepid, int_t len)
{
	T_TCP_CEP	*cep;
	ER		error;

	/* 送信する長さ len をチェックする。*/
	if (len < 0)
		return E_PAR;

	/* 送信する長さlen が 0 であれば、何もしないで戻る。*/
	if (len == 0)
		return E_OK;

	/*
	 *  CEP をロックし、API 機能コードとタスク識別子を記録する。
	 *  すでに記録されていれば、ペンディング中なのでエラー
	 */
	if ((error = tcp_lock_cep(cepid, &cep, TFN_TCP_SND_BUF)) != E_OK)
		return error;

	/* 送信する長さ len をチェックする。*/
	if (len > cep->get_buf_len) {
		error = E_OBJ;
		goto err_ret;
		}

	/* 送信できるか、CEP の FSM 状態を見る。*/
	if (!TCP_FSM_CAN_SEND_MORE(cep->fsm_state)) {
		if ((error = cep->error) == E_OK)
			error = E_OBJ;
		goto err_ret;
		}

	/* 送信ウィンドバッファのデータを送信可能にする。*/
	TCP_SEND_SWBUF(cep, (uint_t)len);

	/* 出力をポストする。*/
	cep->flags |= TCP_CEP_FLG_POST_OUTPUT;
	sig_sem(SEM_TCP_POST_OUTPUT);

err_ret:
	cep->snd_tskid = TA_NULL;
	cep->snd_tfn   = TFN_TCP_UNDEF;
	return error;
	}

#endif	/* of #ifdef __tcp_snd_buf */

#ifdef __tcp_rcv_dat

/*
 *  tcp_rcv_dat -- パケットの受信【標準機能】
 */

ER_UINT
tcp_rcv_dat (ID cepid, void *data, int_t len, TMO tmout)
{
	T_TCP_CEP	*cep;
	ER_UINT		error;

#ifdef TCP_CFG_NON_BLOCKING

	/* data が NULL か、len < 0 ならエラー */
	if (data == NULL || len < 0)
		return E_PAR;

#else	/* of #ifdef TCP_CFG_NON_BLOCKING */

	/* data が NULL、len < 0 か、tmout が TMO_NBLK ならエラー */
	if (data == NULL || len < 0 || tmout == TMO_NBLK)
		return E_PAR;

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

	/*
	 *  CEP をロックし、API 機能コードとタスク識別子を記録する。
	 *  すでに記録されていれば、ペンディング中なのでエラー
	 */
	if ((error = tcp_lock_cep(cepid, &cep, TFN_TCP_RCV_DAT)) != E_OK)
		return error;

	/* 受信できるか、通信端点の状態を見る。*/
	if (tcp_can_recv_more(&error, cep, TFN_TCP_RCV_DAT, tmout) != E_OK)
		goto err_ret;

#ifdef TCP_CFG_NON_BLOCKING

	/* タイムアウトをチェックする。*/
	if (tmout == TMO_NBLK) {		/* ノンブロッキングコール */

		/* 受信ウィンドバッファにデータがあればコールバック関数を呼び出す。*/
		if (cep->rwbuf_count > 0) {

			/* 受信ウィンドバッファからデータを取り出す。*/
			len = TCP_READ_RWBUF(cep, data, (uint_t)len);

#ifdef TCP_CFG_NON_BLOCKING_COMPAT14

			(*cep->callback)(GET_TCP_CEPID(cep), TFN_TCP_RCV_DAT, (void*)(uint32_t)len);

#else	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

			(*cep->callback)(GET_TCP_CEPID(cep), TFN_TCP_RCV_DAT, (void*)&len);

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

			error = E_WBLK;
			goto err_ret;
			}
		else {
			cep->rcv_data     = data;
			cep->rcv_len      = len;
			cep->rcv_nblk_tfn = TFN_TCP_RCV_DAT;
			return E_WBLK;
			}
		}
	else {		/* 非ノンブロッキングコール */

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

		/* 受信ウィンドバッファにデータがなければ、入力があるまで待つ。*/
		if ((error = tcp_wait_rwbuf(cep, tmout)) != E_OK)
			goto err_ret;

		/* 受信ウィンドバッファからデータを取り出す。*/
		error = TCP_READ_RWBUF(cep, data, (uint_t)len);

		/* 相手にウィンドウサイズが変わったことを知らせるため出力をポストする。*/
		cep->flags |= TCP_CEP_FLG_POST_OUTPUT;
		sig_sem(SEM_TCP_POST_OUTPUT);

#ifdef TCP_CFG_NON_BLOCKING

		}

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

err_ret:
	cep->rcv_tskid = TA_NULL;
	cep->rcv_tfn   = TFN_TCP_UNDEF;
	return error;
	}

#endif	/* of #ifdef __tcp_rcv_dat */

#ifdef __tcp_rcv_oob

#ifdef TCP_CFG_EXTENTIONS

/*
 *  tcp_rcv_oob -- 緊急データの受信【拡張機能】
 *
 *    注意: 送信側が複数オクテットのデータを送信しても、
 *          緊急ポインタが指す 1 オクテットのデータのみ受信する。
 */

ER_UINT
tcp_rcv_oob (ID cepid, void *data, int_t len)
{
	T_TCP_CEP	*cep;
	uint8_t		*urg;

	/* TCP 通信端点 ID をチェックする。*/
	if (!VAID_TCP_CEPID(cepid))
		return E_ID;

	/* data が NULL か、len < 0 ならエラー */
	if (data == NULL || len < 0)
		return E_PAR;

	/* TCP 通信端点を得る。*/
	cep = GET_TCP_CEP(cepid);

	/* 受信できるか、通信端点の状態を見る。*/
	/* 受信できるか、fsm_state を見る。*/
	if (!TCP_FSM_CAN_RECV_MORE(cep->fsm_state))
		return E_OBJ;

	/*
	 *  緊急データ入りのセグメントの TCP ヘッダが
	 *  設定されていなければ、緊急データを受信していない。
	 */
	if (cep->urg_tcph == NULL)
		return E_OBJ;

	/* len == 0 ならバッファオーバーフロー */
	if (len == 0)
		return E_BOVR;

	/* 緊急ポインタが指す 1 オクテットのデータを読み取る。*/
	urg = (uint8_t*)cep->urg_tcph + TCP_DATA_OFF(cep->urg_tcph->doff) + cep->urg_tcph->urp + TCP_CFG_URG_OFFSET;
	*(uint8_t*)data = *urg;

	/* 読み取ったデータから後ろの SDU を前に詰める。*/
	memcpy(urg, urg + 1, cep->urg_tcph->sum - (cep->urg_tcph->urp + TCP_CFG_URG_OFFSET) - 1);

	/* tcp_rcv_oob() が呼出されたこと知らせるために、NULL を設定する。*/
	cep->urg_tcph = NULL;

	return 1;
	}

#endif	/* of #ifdef TCP_CFG_EXTENTIONS */

#endif	/* of #ifdef __tcp_rcv_oob */

#ifdef __tcp_rcv_buf

/*
 *  tcp_rcv_buf -- 受信したデータの入ったバッファの獲得【標準機能】
 */

ER_UINT
tcp_rcv_buf (ID cepid, void **p_buf, TMO tmout)
{
	T_TCP_CEP	*cep;
	ER_UINT		error;

#ifdef TCP_CFG_NON_BLOCKING

	/* p_buf が NULL ならエラー */
	if (p_buf == NULL)
		return E_PAR;

#else	/* of #ifdef TCP_CFG_NON_BLOCKING */

	/* p_buf が NULL か、tmout が TMO_NBLK ならエラー */
	if (p_buf == NULL || tmout == TMO_NBLK)
		return E_PAR;

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

	/*
	 *  CEP をロックし、API 機能コードとタスク識別子を記録する。
	 *  すでに記録されていれば、ペンディング中なのでエラー
	 */
	if ((error = tcp_lock_cep(cepid, &cep, TFN_TCP_RCV_BUF)) != E_OK)
		return error;

	/* 受信できるか、通信端点の状態を見る。*/
	if (tcp_can_recv_more(&error, cep, TFN_TCP_RCV_DAT, tmout) != E_OK)
		goto err_ret;

#ifdef TCP_CFG_NON_BLOCKING

	/* タイムアウトをチェックする。*/
	if (tmout == TMO_NBLK) {		/* ノンブロッキングコール */

		/* 受信ウィンドバッファにデータがあればコールバック関数を呼び出す。*/
		if (cep->rwbuf_count > 0) {

		 	/* 受信ウィンドバッファの空アドレスを獲得する。*/
			error = TCP_GET_RWBUF_ADDR(cep, p_buf);

#ifdef TCP_CFG_NON_BLOCKING_COMPAT14

			/* コールバック関数を呼び出す。*/
			(*cep->callback)(GET_TCP_CEPID(cep), TFN_TCP_RCV_BUF, (void*)error);

#else	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

			/* コールバック関数を呼び出す。*/
			(*cep->callback)(GET_TCP_CEPID(cep), TFN_TCP_RCV_BUF, (void*)&error);

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

			error = E_WBLK;
			goto err_ret;
			}
		else {
			cep->rcv_p_buf    = p_buf;
			cep->rcv_nblk_tfn = TFN_TCP_RCV_BUF;
			return E_WBLK;
			}
		}
	else {		/* 非ノンブロッキングコール */

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

		/* 受信ウィンドバッファにデータがなければ、入力があるまで待つ。*/
		if ((error = tcp_wait_rwbuf(cep, tmout)) != E_OK) {
			cep->rwbuf_count = 0;
			goto err_ret;
			}

	 	/* 受信ウィンドバッファのアドレスを獲得する。*/
		error = TCP_GET_RWBUF_ADDR(cep, p_buf);

#ifdef TCP_CFG_NON_BLOCKING

		}

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

err_ret:
	cep->rcv_tskid = TA_NULL;
	cep->rcv_tfn   = TFN_TCP_UNDEF;
	return error;
	}

#endif	/* of #ifdef __tcp_rcv_buf */

#ifdef __tcp_rel_buf

/*
 *  tcp_rel_buf -- 受信用バッファの解放【標準機能】
 */

ER
tcp_rel_buf (ID cepid, int_t len)
{
	T_TCP_CEP	*cep;
	ER		error;

	/* 解放する長さ len をチェックする。*/
	if (len < 0)
		return E_PAR;

	/*
	 *  CEP をロックし、API 機能コードとタスク識別子を記録する。
	 *  すでに記録されていれば、ペンディング中なのでエラー
	 */
	if ((error = tcp_lock_cep(cepid, &cep, TFN_TCP_REL_BUF)) != E_OK)
		return error;

	/* 解放する長さ len をチェックする。*/
	if (len > cep->rcv_buf_len) {
		error = E_OBJ;
		goto err_ret;
		}

	/* 通信端点をロックする。*/
	syscall(wai_sem(cep->semid_lock));

	/* 受信ウィンドバッファから受信したオクテットを削除する。*/
	TCP_DROP_RWBUF(cep, (uint_t)len);

	/* tcp_rcv_buf の割当て長をリセットする。*/
	cep->rcv_buf_len = 0;

	/* 通信端点のロックを解除する。*/
	syscall(sig_sem(cep->semid_lock));

	/* 相手にウィンドウサイズが変わったことを知らせるため出力をポストする。*/
	cep->flags |= TCP_CEP_FLG_POST_OUTPUT;
	sig_sem(SEM_TCP_POST_OUTPUT);

err_ret:
	cep->rcv_tskid = TA_NULL;
	cep->rcv_tfn   = TFN_TCP_UNDEF;
	return error;
	}

#endif	/* of #ifdef __tcp_rel_buf */

#ifdef __tcp_can_cep

/*
 *  tcp_can_snd -- ペンディングしている送信のキャンセル
 */

static ER
tcp_can_snd (T_TCP_CEP *cep, FN fncd)
{
	ER	error = E_OK;

	/* 通信端点をロックする。*/
	syscall(wai_sem(cep->semid_lock));

	/*
	 *  snd_tskid が TA_NULL なら、
	 *  ペンディングしていないのでエラー
	 */
	if (cep->snd_tskid == TA_NULL)
		error = EV_NOPND;

	/* ペンディング中の API 機能コードと一致しなければエラー */
	else if (fncd != TFN_TCP_ALL && fncd != cep->snd_tfn)
		error = E_OBJ;

	/* 処理をキャンセルする。*/
	else {

		/* 受信再構成キューのネットワークバッファを解放する。*/
		tcp_free_reassq(cep);

		/* 受信ウィンドバッファキューのネットワークバッファを解放する。*/
		cep->rwbuf_count = 0;
		TCP_FREE_RWBUFQ(cep);

		/* 送信ウィンドバッファキューのネットワークバッファを解放する。*/
		TCP_FREE_SWBUFQ(cep);

#ifdef TCP_CFG_NON_BLOCKING

		if (cep->snd_nblk_tfn != TFN_TCP_UNDEF) {	/* ノンブロッキングコール */

			switch (cep->snd_nblk_tfn) {

			case TFN_TCP_CON_CEP:
				/*  通信端点から受付口を解放する。*/
				cep->rep = NULL;
				cep->fsm_state = TCP_FSM_CLOSED;
				syscall(set_flg(cep->est_flgid, TCP_CEP_EVT_CLOSED));
				break;

			case TFN_TCP_SND_DAT:
			case TFN_TCP_GET_BUF:
			case TFN_TCP_SND_OOB:
				break;
				}

			if (IS_PTR_DEFINED(cep->callback)) {

#ifdef TCP_CFG_NON_BLOCKING_COMPAT14

				(*cep->callback)(GET_TCP_CEPID(cep), cep->snd_nblk_tfn, (void*)E_RLWAI);

#else	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

				ER	error = E_RLWAI;

				(*cep->callback)(GET_TCP_CEPID(cep), cep->snd_nblk_tfn, (void*)&error);

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

				}
			else
				error = E_OBJ;
			cep->snd_nblk_tfn = TFN_TCP_UNDEF;
			}
		else

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

			error = rel_wai(cep->snd_tskid);

		cep->snd_tskid = TA_NULL;
		cep->snd_tfn   = TFN_TCP_UNDEF;
		}

	/* 通信端点のロックを解除する。*/
	syscall(sig_sem(cep->semid_lock));

	return error;
	}

/*
 *  tcp_can_rcv -- ペンディングしている受信のキャンセル
 */

static ER
tcp_can_rcv (T_TCP_CEP *cep, FN fncd)
{
	ER	error = E_OK;

	/* 通信端点をロックする。*/
	syscall(wai_sem(cep->semid_lock));

	/*
	 *  rcv_tskid が TA_NULL なら、
	 *  ペンディングしていないのでエラー
	 */
	if (cep->rcv_tskid == TA_NULL)
		error = EV_NOPND;

	/* ペンディング中の API 機能コードと一致しなければエラー */
	else if (fncd != TFN_TCP_ALL && fncd != cep->rcv_tfn)
		error = E_OBJ;

	/* 処理をキャンセルする。*/
	else {

		/* 受信再構成キューのネットワークバッファを解放する。*/
		tcp_free_reassq(cep);

		/* 受信ウィンドバッファキューのネットワークバッファを解放する。*/
		cep->rwbuf_count = 0;
		TCP_FREE_RWBUFQ(cep);

		/* 送信ウィンドバッファキューのネットワークバッファを解放する。*/
		TCP_FREE_SWBUFQ(cep);

#ifdef TCP_CFG_NON_BLOCKING

		if (cep->rcv_nblk_tfn != TFN_TCP_UNDEF) {	/* ノンブロッキングコール */

			switch (cep->rcv_nblk_tfn) {

			case TFN_TCP_ACP_CEP:
				/*
				 *  通信端点から受付口を解放し、
				 *  イベントフラグをクローズに設定する。
				 */
				cep->rep = NULL;
				cep->fsm_state = TCP_FSM_CLOSED;
				syscall(set_flg(cep->est_flgid, TCP_CEP_EVT_CLOSED));
				break;

			case TFN_TCP_RCV_DAT:
			case TFN_TCP_RCV_BUF:
				break;

			case TFN_TCP_CLS_CEP:
				cep->fsm_state = TCP_FSM_CLOSED;
				tcp_respond(NULL, cep, cep->rcv_nxt, cep->snd_una - 1,
				            cep->rbufsz - cep->rwbuf_count, TCP_FLG_RST);
				syscall(set_flg(cep->est_flgid, TCP_CEP_EVT_CLOSED));
				break;
				}

			if (IS_PTR_DEFINED(cep->callback)) {

#ifdef TCP_CFG_NON_BLOCKING_COMPAT14

				(*cep->callback)(GET_TCP_CEPID(cep), cep->rcv_nblk_tfn, (void*)E_RLWAI);

#else	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

				ER	error = E_RLWAI;

				(*cep->callback)(GET_TCP_CEPID(cep), cep->rcv_nblk_tfn, (void*)&error);

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

				}
			else
				error = E_OBJ;
			cep->rcv_nblk_tfn = TFN_TCP_UNDEF;
			}
		else

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

			error = rel_wai(cep->rcv_tskid);

		cep->rcv_tskid = TA_NULL;
		cep->rcv_tfn   = TFN_TCP_UNDEF;
		}

	/* 通信端点のロックを解除する。*/
	syscall(sig_sem(cep->semid_lock));

	return error;
	}

/*
 *  tcp_can_cep -- ペンディングしている処理のキャンセル【標準機能】
 */

ER
tcp_can_cep (ID cepid, FN fncd)
{
	T_TCP_CEP	*cep;
	ER		error = E_OK, snd_err, rcv_err;

	/* API 機能コードをチェックする。*/
	if (!VALID_TFN_TCP_CAN(fncd))
		return E_PAR;

	/* TCP 通信端点 ID をチェックする。*/
	if (!VAID_TCP_CEPID(cepid))
		return E_ID;

	/* TCP 通信端点を得る。*/
	cep = GET_TCP_CEP(cepid);

	/* TCP 通信端点をチェックする。*/
	if (!VALID_TCP_CEP(cep))
		return E_NOEXS;

	if (fncd == TFN_TCP_ALL) {		/* TFN_TCP_ALL の処理 */
		snd_err = tcp_can_snd(cep, fncd);
		rcv_err = tcp_can_rcv(cep, fncd);

		/*
		 *  snd_err と rcv_err のどちらも EV_NOPND
		 *  なら、ペンディングしていないのでエラー
		 */
		if (snd_err == EV_NOPND && rcv_err == EV_NOPND)
			error = E_OBJ;
		else {
			if (snd_err == EV_NOPND)
				snd_err = E_OK;
			if (rcv_err == EV_NOPND)
				rcv_err = E_OK;

			if (snd_err != E_OK)
				error = snd_err;
			else if (rcv_err != E_OK)
				error = rcv_err;
			}
		}

	else if (IS_TFN_TCP_RCV(fncd)) {	/* 受信処理のキャンセル */
		if ((error = tcp_can_rcv(cep, fncd)) == EV_NOPND)
			error = E_OBJ;
		}

	else {					/* 送信処理のキャンセル */
		if ((error = tcp_can_snd(cep, fncd)) == EV_NOPND)
			error = E_OBJ;
		}

	return error;
	}

#endif	/* of #ifdef __tcp_can_cep */

/*
 *  tcp_set_opt -- TCP 通信端点オプションの設定【拡張機能】
 *
 *  注意: 設定可能な TCP 通信端点オプションは無いため、E_PAR が返される。
 */

#ifdef __tcp_set_opt

#ifdef TCP_CFG_EXTENTIONS

ER
tcp_set_opt (ID cepid, int_t optname, void *optval, int_t optlen)
{
	T_TCP_CEP	*cep;

	/* TCP 通信端点 ID をチェックする。*/
	if (!VAID_TCP_CEPID(cepid))
		return E_ID;

	/* TCP 通信端点を得る。*/
	cep = GET_TCP_CEP(cepid);

	/* TCP 通信端点をチェックする。*/
	if (!VALID_TCP_CEP(cep))
		return E_NOEXS;

	return E_PAR;
	}

#endif	/* of #ifdef TCP_CFG_EXTENTIONS */

#endif	/* of #ifdef __tcp_set_opt */

/*
 *  tcp_get_opt -- TCP 通信端点オプションの設定【拡張機能】
 *
 *  注意: 設定可能な TCP 通信端点オプションは無いため、E_PAR が返される。
 */

#ifdef __tcp_get_opt

#ifdef TCP_CFG_EXTENTIONS

ER
tcp_get_opt (ID cepid, int_t optname, void *optval, int_t optlen)
{
	T_TCP_CEP	*cep;

	/* TCP 通信端点 ID をチェックする。*/
	if (!VAID_TCP_CEPID(cepid))
		return E_ID;

	/* TCP 通信端点を得る。*/
	cep = GET_TCP_CEP(cepid);

	/* TCP 通信端点をチェックする。*/
	if (!VALID_TCP_CEP(cep))
		return E_NOEXS;

	return E_PAR;
	}

#endif	/* of #ifdef TCP_CFG_EXTENTIONS */

#endif	/* of #ifdef __tcp_get_opt */

#endif	/* of #ifdef SUPPORT_TCP */
