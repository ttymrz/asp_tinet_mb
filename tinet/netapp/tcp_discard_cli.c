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
 *  @(#) $Id: tcp_discard_cli.c,v 1.5 2009/12/24 05:44:56 abe Exp abe $
 */

/* 
 *  TCP DISCARD クライアント
 *
 *    ・ノンブロッキングコール
 *    ・IPv4
 *    ・IPv6
 */

#include <stdlib.h>

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
#include <t_syslog.h>
#include "kernel_cfg.h"

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <t_services.h>
#include "kernel_id.h"

#endif	/* of #ifdef TARGET_KERNEL_JSP */

#include <tinet_defs.h>
#include <tinet_config.h>

#include <net/if.h>
#include <net/if_ppp.h>
#include <net/if_loop.h>
#include <net/ethernet.h>
#include <net/net.h>
#include <net/net_var.h>
#include <net/net_timer.h>

#include <netinet/in.h>
#include <netinet/in_itron.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>

#include <netapp/netapp.h>
#include <netapp/netapp_var.h>
#include <netapp/discard.h>

#ifdef USE_TCP_DISCARD_CLI

/* discard サーバのポート番号 */

#define DISCARD_SRV_PORTNO	UINT_C(9)

/* 表示 */

/*#define SHOW_RCV_RANGE*/

/*
 *  バッファサイズの定義
 */

#define NUM_DISCARD		5
#define NUM_REP_PAT		40
#define PAT_BEGIN		' '
#define PAT_END			'~'
#define SND_BUF_SIZE		((PAT_END - PAT_BEGIN + 1) * NUM_REP_PAT)

/*
 *  全域変数
 */

bool_t tcp_discard_cli_valid;

/* TCP 送受信ウィンドバッファ */

#ifndef TCP_CFG_SWBUF_CSAVE_ONLY
uint8_t tcp_discard_cli_swbuf[TCP_DISCARD_CLI_SWBUF_SIZE];
#endif

#ifdef USE_TCP_NON_BLOCKING

/*
 *  変数
 */

static T_IPEP		nblk_src = {
				IP_ADDRANY,
				TCP_PORTANY,
				};
static T_IPEP		nblk_dst;
static ER_UINT		nblk_error;

/*
 *  ノンブロッキングコールのコールバック関数
 */

ER
callback_nblk_tcp_discard_cli (ID cepid, FN fncd, void *p_parblk)
{
	ER	error = E_OK;

	nblk_error = *(ER*)p_parblk;

	switch (fncd) {

	case TFN_TCP_CON_CEP:
		syscall(sig_sem(SEM_TCP_DISCARD_CLI_NBLK_READY));
		break;

	case TFN_TCP_CLS_CEP:
		if (nblk_error < 0)
			syslog(LOG_NOTICE, "[TDC:%02d CBN] close error: %s", cepid, itron_strerror(nblk_error));
		syscall(sig_sem(SEM_TCP_DISCARD_CLI_NBLK_READY));
		break;

	case TFN_TCP_ACP_CEP:
	case TFN_TCP_SND_DAT:
	case TFN_TCP_GET_BUF:
	case TFN_TCP_SND_OOB:
	default:
		error = E_PAR;
		break;
		}
	return error;
	}

/*
 *  send_tcp_discard -- DISCARD/TCP サーバにメッセージを送信する。
 */

static void
send_tcp_discard (ID cepid, T_IN_ADDR *ipaddr, uint16_t portno)
{
	static char smsg[SND_BUF_SIZE];

	ER_UINT		slen;
	ER		error;
	SYSTIM		time;
	uint32_t	total;
	uint16_t	soff, discard, rep, scount;
	uint8_t		pat, *p;

	nblk_dst.ipaddr = *ipaddr;
	nblk_dst.portno = portno;

	p = smsg;
	for (rep = NUM_REP_PAT; rep -- > 0; )
		for (pat = PAT_BEGIN; pat <= PAT_END; pat ++)
			*p ++ = pat;

	if ((error = TCP_CON_CEP(cepid, &nblk_src, &nblk_dst, TMO_NBLK)) != E_WBLK) {
		syslog(LOG_NOTICE, "[TDC:%02d SND] connect error: %s", cepid, itron_strerror(error));
		return;
		}

	/* 接続が完了するまで待つ。*/
	syscall(wai_sem(SEM_TCP_DISCARD_CLI_NBLK_READY));

	if (nblk_error != E_OK)
		syslog(LOG_NOTICE, "[TDC:%02d SND] connect error: %s", cepid, itron_strerror(nblk_error));
	else {
		get_tim(&time);
		syslog(LOG_NOTICE, "[TDC:%02d SND] connecting: %6ld, to:   %s.%d",
		                   cepid, time / SYSTIM_HZ, IP2STR(NULL, ipaddr), nblk_dst.portno);

		scount = total = 0;
		for (discard = NUM_DISCARD; discard -- > 0; ) {
			soff = 0;
			while (soff < SND_BUF_SIZE) {
				if ((slen = tcp_snd_dat(cepid, smsg + soff, sizeof(smsg) - soff, TMO_FEVR)) < 0) {
					syslog(LOG_NOTICE, "[TDC:%02d SND] send error: %s",
					                   cepid, itron_strerror(slen));
					goto cls_ret;
					}
				soff  += (uint16_t)slen;
				total +=     slen;
				scount ++;

#ifdef SHOW_RCV_RANGE
				syslog(LOG_NOTICE, "[TDC:%02d SND] snd: %2d, len: %4d, off: %4d",
				                   cepid, scount, (uint16_t)slen, soff);
#endif	/* of #ifdef SHOW_RCV_RANGE */

				syscall(dly_tsk(500 + net_rand() % SYSTIM_HZ));
				}
			}

	cls_ret:
		get_tim(&time);
		syslog(LOG_NOTICE, "[TDC:%02d SND] finished:   %6ld, snd: %4d,            len: %ld",
		                   cepid, time / SYSTIM_HZ, scount, total);

		if ((error = tcp_sht_cep(cepid)) < 0)
			syslog(LOG_NOTICE, "[TDC:%02d SND] shutdown error: %s", cepid, itron_strerror(error));

		if ((error = tcp_cls_cep(cepid, TMO_NBLK)) != E_WBLK)
			syslog(LOG_NOTICE, "[TDC:%02d SND] close error: %s", cepid, itron_strerror(error));

		/* 開放が完了するまで待つ。*/
		syscall(wai_sem(SEM_TCP_DISCARD_CLI_NBLK_READY));
		}
	}

#else	/* of #ifdef USE_TCP_NON_BLOCKING */

/*
 *  send_tcp_discard -- DISCARD/TCP サーバにメッセージを送信する。
 */

static void
send_tcp_discard (ID cepid, T_IN_ADDR *ipaddr, uint16_t portno)
{
	static char smsg[SND_BUF_SIZE];
	static T_IPEP src = {
		IP_ADDRANY,
		TCP_PORTANY,
		};

	T_IPEP		dst;
	ER_UINT		slen;
	ER		error;
	SYSTIM		time;
	uint16_t	pat, soff, discard, rep, scount, total;
	uint8_t		*p;

	dst.ipaddr = *ipaddr;
	dst.portno = portno;

	p = smsg;
	for (rep = NUM_REP_PAT; rep -- > 0; )
		for (pat = PAT_BEGIN; pat <= PAT_END; pat ++)
			*p ++ = pat;

	if ((error = TCP_CON_CEP(cepid, &src, &dst, 60 * 1000)) != E_OK) {
		syslog(LOG_NOTICE, "[TDC:%02d SND] connect error: %s", cepid, itron_strerror(error));
		return;
		}

	get_tim(&time);
	syslog(LOG_NOTICE, "[TDC:%02d SND] connecting: %6ld, to:   %s.%d",
	                   cepid, time / SYSTIM_HZ, IP2STR(NULL, &dst.ipaddr), dst.portno);

	scount = total = 0;
	for (discard = NUM_DISCARD; discard -- > 0; ) {
		soff = 0;
		while (soff < SND_BUF_SIZE) {
			if ((slen = tcp_snd_dat(cepid, smsg + soff, sizeof(smsg) - soff, TMO_FEVR)) < 0) {
				syslog(LOG_NOTICE, "[TDC:%02d SND] send error: %s", cepid, itron_strerror(slen));
				goto cls_ret;
				}
			soff  += (uint16_t)slen;
			total +=     slen;
			scount ++;

#ifdef SHOW_RCV_RANGE
			syslog(LOG_NOTICE, "[TDC:%02d SND] snd: %3d, len: %4d, off: %4d",
			                   cepid, scount, (uint16_t)slen, soff);
#endif	/* of #ifdef SHOW_RCV_RANGE */

			syscall(dly_tsk(500 + net_rand() % SYSTIM_HZ));
			}
		}

cls_ret:
	get_tim(&time);
	syslog(LOG_NOTICE, "[TDC:%02d SND] finished:   %6ld, snd: %4d,            len: %ld",
	                   cepid, time / SYSTIM_HZ, scount, total);

	if ((error = tcp_sht_cep(cepid)) < 0)
		syslog(LOG_NOTICE, "[TDC:%02d SND] shutdown error: %s", cepid, itron_strerror(error));

	if ((error = tcp_cls_cep(cepid, TMO_FEVR)) < 0)
		syslog(LOG_NOTICE, "[TDC:%02d SND] close error: %s", cepid, itron_strerror(error));
	}

#endif	/* of #ifdef USE_TCP_NON_BLOCKING */

void
tcp_discard_cli_task (intptr_t exinf)
{
	T_IN_ADDR	addr;
	ID		tskid, cepid;
	int_t		rep;
	uint32_t	count;
	uint16_t	portno;
	uint8_t		*line;

#ifdef USE_TCP_EXTENTIONS

	ER		error;
	T_TCP_CCEP	ccep;

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

	get_tid(&tskid);
	syslog(LOG_NOTICE, "[TCP DISCARD CLI:%d,%d] started.", tskid, (ID)exinf);
	while (true) {
		if (rcv_dtq(DTQ_TCP_DISCARD_CLI, (intptr_t*)&line) == E_OK) {
			line = skip_blanks(GET_IPADDR(&addr, skip_blanks(line)));	/* IP Address */

			if ('0' <= *line && *line <= '9') {				/* Port No */
				line = get_int(&rep, line);
				portno = (uint16_t)rep;
				}
			else {
				line ++;
				portno = DISCARD_SRV_PORTNO;
				}

			line = skip_blanks(line);
			if ('0' <= *line && *line <= '9')				/* Repeat */
				line = get_int(&rep, line);
			else
				rep = 1;

#ifdef USE_TCP_EXTENTIONS

			ccep.cepatr = 0;
			ccep.sbufsz = TCP_DISCARD_CLI_SWBUF_SIZE;
			ccep.rbufsz = 0;
			ccep.rbuf = NADR;

#ifdef TCP_CFG_SWBUF_CSAVE
			ccep.sbuf = NADR;
#else
			ccep.sbuf = tcp_discard_cli_swbuf;
#endif
#ifdef USE_TCP_NON_BLOCKING
			ccep.callback = (FP)callback_nblk_tcp_discard_cli;
#else
			ccep.callback = NULL;
#endif

			if ((error = alloc_tcp_cep(&cepid, tskid, &ccep)) != E_OK) {
				syslog(LOG_NOTICE, "[TDC:%02d TSK] CEP create error: %s", cepid, itron_strerror(error));
				continue;
				}

#else	/* of #ifdef USE_TCP_EXTENTIONS */

			cepid = (ID)exinf;

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

			tcp_discard_cli_valid = true;
			for (count = rep; count -- > 0; ) {
				if (!tcp_discard_cli_valid) {
					syslog(LOG_NOTICE, "[TDC:%02d TSK] canceled.", (ID)exinf);
					break;
					}
				syslog(LOG_NOTICE, "[TDC:%02d TSK] start:              repeat: %d/%d",
				                   (ID)exinf, rep - count, rep);
				send_tcp_discard(cepid, &addr, portno);
				syslog(LOG_NOTICE, "[TDC:%02d TSK] finished:           repeat: %d/%d",
				                   (ID)exinf, rep - count, rep);
				syscall(dly_tsk(500 + net_rand() % SYSTIM_HZ));
				}

#ifdef USE_TCP_EXTENTIONS

			if ((error = free_tcp_cep(cepid)) != E_OK)
				syslog(LOG_NOTICE, "[TDC:%02d TSK] CEP delete error: %s", cepid, itron_strerror(error));

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

			}
		}
	}

#endif	/* of #ifdef USE_TCP_DISCARD_CLI */
