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
 *  @(#) $Id: tcp_echo_cli.c,v 1.5 2009/12/24 05:44:56 abe Exp abe $
 */

/* 
 *  TCP ECHO クライアント
 *
 *    ・送受信タスク分離型
 *    ・ノンブロッキングコール
 *    ・省コピー API
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
#include <net/net.h>
#include <net/ethernet.h>
#include <net/net_var.h>
#include <net/net_timer.h>

#include <netinet/in.h>
#include <netinet/in_itron.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>

#include <netapp/netapp.h>
#include <netapp/netapp_var.h>
#include <netapp/echo.h>

#ifdef USE_TCP_ECHO_CLI

/* echo サーバのポート番号 */

#define ECHO_SRV_PORTNO		UINT_C(7)

/* 緊急データの送信 */

#ifdef USE_TCP_EXTENTIONS

//#define SND_URG_DATA_SIZE	4
#define SND_URG_COUNT		10

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

/* 表示 */

//#define SHOW_RCV_RANGE

/* タイムアウト */

#define CON_TMO		TMO_FEVR
//#define CON_TMO		(60*SYSTIM_HZ)
//#define CLS_TMO		TMO_FEVR				/* Close Wait は標準で 60秒 */
#define CLS_TMO		(70*SYSTIM_HZ+(net_rand()%SYSTIM_HZ)*10)
//#define RCV_TMO		TMO_FEVR
#define RCV_TMO		(40*SYSTIM_HZ+(net_rand()%SYSTIM_HZ)*20)
//#define SND_TMO		TMO_FEVR
#define SND_TMO		(30*SYSTIM_HZ+(net_rand()%SYSTIM_HZ)*30)

/* 送信間隔 */

//#define SND_DLY			ULONG_C(500)
#define SND_ITV		(5*SYSTM_HZ)
#define SLP_ITV		(60*SYSTIM_HZ)

/* 自動実行 */

#if 0
#if defined(SUPPORT_INET6)
#define AUTO_RUN_STR	"fe80::211:2fff:fe8a:e8c0 - 0"
#else
#define AUTO_RUN_STR	"172.25.193.140 - 0"
#endif
#endif

/*
 *  バッファサイズの定義
 */

#define NUM_ECHO		20
#define NUM_REP_PAT		20
#define PAT_BEGIN		' '
#define PAT_END			'~'
#define SND_BUF_SIZE		((PAT_END - PAT_BEGIN + 1) * NUM_REP_PAT)
#define RCV_BUF_SIZE		(SND_BUF_SIZE*15/10)

/*
 *  全域変数
 */

bool_t tcp_echo_cli_valid;

/* TCP 送受信ウィンドバッファ */

#ifndef TCP_CFG_SWBUF_CSAVE
uint8_t tcp_echo_cli_swbuf[TCP_ECHO_CLI_SWBUF_SIZE];
#endif

#ifndef TCP_CFG_RWBUF_CSAVE
uint8_t tcp_echo_cli_rwbuf[TCP_ECHO_CLI_RWBUF_SIZE];
#endif

/*
 *  変数
 */

static uint_t	scount;
static uint_t	rcount;

#ifdef USE_TCP_NON_BLOCKING

static T_IPEP	nblk_src = {
			IP_ADDRANY,
			TCP_PORTANY,
			};
static T_IPEP	nblk_dst;
static ER_UINT	nblk_error;

/*
 *  ノンブロッキングコールのコールバック関数
 */

ER
callback_nblk_tcp_echo_cli (ID cepid, FN fncd, void *p_parblk)
{
	ER	error = E_OK;

	nblk_error = *(ER*)p_parblk;

	switch (fncd) {

	case TFN_TCP_CON_CEP:
		syscall(sig_sem(SEM_TCP_ECHO_CLI_NBLK_READY));
		break;

	case TFN_TCP_RCV_BUF:
		if (nblk_error < 0)
			syslog(LOG_NOTICE, "[TEC:%02d CBN] rcv buf error: %s", cepid, itron_strerror(nblk_error));
		syscall(sig_sem(SEM_TCP_ECHO_CLI_NBLK_READY));
		break;

	case TFN_TCP_CLS_CEP:
		if (nblk_error < 0)
			syslog(LOG_NOTICE, "[TEC:%02d CBN] close error: %s", cepid, itron_strerror(nblk_error));
		syscall(sig_sem(SEM_TCP_ECHO_CLI_NBLK_READY));
		break;

	case TFN_TCP_RCV_DAT:
		if (nblk_error < 0)
			syslog(LOG_NOTICE, "[TEC:%02d CBN] rcv dat error: %s", cepid, itron_strerror(nblk_error));
		syscall(sig_sem(SEM_TCP_ECHO_CLI_NBLK_READY));
		break;

	case TFN_TCP_SND_OOB:
		if (nblk_error > 0)
			syslog(LOG_NOTICE, "[TEC:%02d CBN] snd urg: %4d", cepid, nblk_error);
		else
			syslog(LOG_NOTICE, "[TEC:%02d CBN] snd urg error: %s", cepid, itron_strerror(nblk_error));
		break;

	case TFN_TCP_ACP_CEP:
	case TFN_TCP_SND_DAT:
	case TFN_TCP_GET_BUF:
	default:
		error = E_PAR;
		break;
		}
	return error;
	}

/*
 *  send_tcp_echo -- ECHO/TCP サーバにメッセージを送信する。
 */

static ER
send_tcp_echo (ID cepid, T_IN_ADDR *ipaddr, uint16_t portno)
{
	static char smsg[SND_BUF_SIZE];

	ER_UINT		slen;
	ER		error;
	SYSTIM		time;
	uint32_t	total;
	uint16_t	soff, echo, rep;
	char		*p, pat;

#ifdef SND_URG_DATA_SIZE
	int_t	urg = SND_URG_COUNT;
#endif

	nblk_dst.ipaddr = *ipaddr;
	nblk_dst.portno = portno;

	p = smsg;
	for (rep = NUM_REP_PAT; rep -- > 0; )
		for (pat = PAT_BEGIN; pat <= PAT_END; pat ++)
			*p ++ = pat;

	if ((error = TCP_CON_CEP(cepid, &nblk_src, &nblk_dst, TMO_NBLK)) != E_WBLK) {
		syslog(LOG_NOTICE, "[TEC:%02d SND] connect error: %s", cepid, itron_strerror(error));
		return error;
		}

	/* 接続が完了するまで待つ。*/
	syscall(wai_sem(SEM_TCP_ECHO_CLI_NBLK_READY));

	if (nblk_error != E_OK)
		syslog(LOG_NOTICE, "[TEC:%02d SND] connect error: %s", cepid, itron_strerror(nblk_error));
	else {
		get_tim(&time);
		syslog(LOG_NOTICE, "[TEC:%02d SND] connecting: %6lu, to:   %s.%u",
		                   cepid, time / SYSTIM_HZ, IP2STR(NULL, ipaddr), nblk_dst.portno);

		if ((error = psnd_dtq(DTQ_TCP_ECHO_CLI_RCV, (intptr_t)cepid)) != E_OK) {
			syslog(LOG_NOTICE, "[TEC:%02d SND] sync error: %s", cepid, itron_strerror(error));
			goto cls_ret;
			}

		scount = total = 0;
		for (echo = NUM_ECHO; echo -- > 0; ) {
			soff = 0;
			while (soff < SND_BUF_SIZE) {
				if ((slen = tcp_snd_dat(cepid, smsg + soff, sizeof(smsg) - soff, SND_TMO)) < 0) {
					syslog(LOG_NOTICE, "[TEC:%02d SND] snd dat error: %s",
					                   cepid, itron_strerror(slen));
					goto cls_ret;
					}
				soff  += (uint16_t)slen;
				total +=     slen;
				scount ++;
#ifdef SHOW_RCV_RANGE
				syslog(LOG_NOTICE, "[TEC:%02d SND] send cnt: %2d, len: %4d, off: %4d",
				                   cepid, scount, (uint16_t)slen, soff);
#endif	/* of #ifdef SHOW_RCV_RANGE */

#ifdef SND_URG_DATA_SIZE
				if (urg) {
					if (urg == 1) {
						if ((error = tcp_snd_oob(cepid, smsg, SND_URG_DATA_SIZE, TMO_NBLK)) != E_WBLK)
							syslog(LOG_NOTICE, "[TEC:%02d SND] snd urg error: %s",
							                   cepid, itron_strerror(error));
						}
					urg --;
					}
#endif	/* of #ifdef SND_URG_DATA_SIZE */

#if SND_DLY > 0
				syscall(dly_tsk(SND_DLY + net_rand() % SYSTIM_HZ));
#endif
				}
			}

	cls_ret:
		if ((error = tcp_sht_cep(cepid)) < 0)
			syslog(LOG_NOTICE, "[TEC:%02d SND] shutdown error: %s", cepid, itron_strerror(error));

		/* 受信が完了するまで待つ。*/
		syscall(slp_tsk());
		}

	return error;
	}

/*
 *  TCP ECHO クライアント受信タスク
 */

#ifdef USE_COPYSAVE_API

void
tcp_echo_cli_rcv_task (intptr_t exinf)
{
	ID		tskid, cepid;
	ER		error;
	SYSTIM		time;
	uint32_t	total;
	uint16_t	roff, rlen;
	char		*rmsg, head, tail;

	get_tid(&tskid);
	syslog(LOG_NOTICE, "[TCP ECHO CLI (NBLK,CS) RCV:%d] started.", tskid);
	while (true) {
		if ((error = rcv_dtq(DTQ_TCP_ECHO_CLI_RCV, (intptr_t*)&cepid)) != E_OK) {
			syslog(LOG_NOTICE, "[TEC:%02d RCV] sync error: %s",
			                   cepid, itron_strerror(error));
			}
                else {
			roff = rcount = total = 0;
			while (true) {
				if ((error = tcp_rcv_buf(cepid, (void**)&rmsg, TMO_NBLK)) != E_WBLK) {
					syslog(LOG_NOTICE, "[TEC:%02d RCV] rcv buf error: %s", cepid, itron_strerror(error));
					break;
					}

				/* 受信バッファの獲得が完了するまで待つ。*/
				syscall(wai_sem(SEM_TCP_ECHO_CLI_NBLK_READY));
				if (nblk_error < 0)
					break;

				rlen = nblk_error;
				head = *rmsg;
				tail = *(rmsg + rlen - 1);
				if (rlen > 0) {
					roff  += rlen;
					total += rlen;
					rcount ++;
#ifdef SHOW_RCV_RANGE
					syslog(LOG_NOTICE, "[TEC:%02d RCV] "
					                   "recv cnt: %2d, len: %4d, off: %4d, data: %02x -> %02x",
					                   cepid, rcount, rlen, roff, head, tail);
#endif	/* of #ifdef SHOW_RCV_RANGE */
					}
				else
					break;

				if ((error = tcp_rel_buf(cepid, rlen)) != E_OK)
					syslog(LOG_NOTICE, "[TEC:%02d RCV] rel buf error: %s", 
					                   cepid, itron_strerror(error));
				}

			if ((error = tcp_cls_cep(cepid, TMO_NBLK)) != E_WBLK && error != E_OK)
				syslog(LOG_NOTICE, "[TEC:%02d SND] close error: %s", cepid, itron_strerror(error));

			/* 切断が完了するまで待つ。*/
			syscall(wai_sem(SEM_TCP_ECHO_CLI_NBLK_READY));

			get_tim(&time);
			syslog(LOG_NOTICE, "[TEC:%02u RCV] finished:   %6lu, snd: %4u, rcv: %4u, len: %u",
			                   cepid, time / SYSTIM_HZ, scount, rcount, total);
			}

		syscall(wup_tsk(TCP_ECHO_CLI_SND_TASK));
		}
	}

#else	/* of #ifdef USE_COPYSAVE_API */

void
tcp_echo_cli_rcv_task (intptr_t exinf)
{
	static char rmsg[RCV_BUF_SIZE];

	ID		tskid, cepid;
	ER		error;
	SYSTIM		time;
	uint32_t	total;
	uint16_t	roff, rlen;
	char		head, tail;

	get_tid(&tskid);
	syslog(LOG_NOTICE, "[TCP ECHO CLI (NBLK) RCV:%d] started.", tskid);
	while (true) {
		if ((error = rcv_dtq(DTQ_TCP_ECHO_CLI_RCV, (intptr_t*)&cepid)) != E_OK) {
			syslog(LOG_NOTICE, "[TEC:%02d RCV] sync error: %s",
			                   cepid, itron_strerror(error));
			}
                else {
			roff = rcount = total = 0;
			while (true) {
				if ((error = tcp_rcv_dat(cepid, rmsg, sizeof(rmsg), TMO_NBLK)) != E_WBLK) {
					syslog(LOG_NOTICE, "[TEC:%02d RCV] rcv error: %s", cepid, itron_strerror(error));
					break;
					}

				/* 受信が完了するまで待つ。*/
				syscall(wai_sem(SEM_TCP_ECHO_CLI_NBLK_READY));
				if (nblk_error < 0)
					break;

				rlen = nblk_error;
				head = *rmsg;
				tail = *(rmsg + rlen - 1);
				rcount ++;
				if (rlen > 0) {
					roff  += rlen;
					total += rlen;
#ifdef SHOW_RCV_RANGE
					syslog(LOG_NOTICE, "[TEC:%02d RCV] "
					                   "recv cnt: %2d, len: %4d, off: %4d, data: %02x -> %02x",
					                   cepid, rcount, rlen, roff, head, tail);
#endif	/* of #ifdef SHOW_RCV_RANGE */
					}
				else
					break;
				}

			if ((error = tcp_cls_cep(cepid, TMO_NBLK)) != E_WBLK && error != E_OK)
				syslog(LOG_NOTICE, "[TEC:%02d SND] close error: %s", cepid, itron_strerror(error));

			/* 切断が完了するまで待つ。*/
			syscall(wai_sem(SEM_TCP_ECHO_CLI_NBLK_READY));

			get_tim(&time);
			syslog(LOG_NOTICE, "[TEC:%02u RCV] finished:   %6lu, snd: %4u, rcv: %4u, len: %u",
			                   cepid, time / SYSTIM_HZ, scount, rcount, total);
			}

		syscall(wup_tsk(TCP_ECHO_CLI_SND_TASK));
		}
	}

#endif	/* of #ifdef USE_COPYSAVE_API */

#else	/* of #ifdef USE_TCP_NON_BLOCKING */

/*
 *  send_tcp_echo -- ECHO/TCP サーバにメッセージを送信する。
 */

static ER
send_tcp_echo (ID cepid, T_IN_ADDR *ipaddr, uint16_t portno)
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
	uint32_t	total;
	uint16_t	soff, echo, rep;
	char 		pat, *p;

#ifdef SND_URG_DATA_SIZE
	int_t	urg = SND_URG_COUNT;
#endif

	dst.ipaddr = *ipaddr;
	dst.portno = portno;

	p = smsg;
	for (rep = NUM_REP_PAT; rep -- > 0; )
		for (pat = PAT_BEGIN; pat <= PAT_END; pat ++)
			*p ++ = pat;

	if ((error = TCP_CON_CEP(cepid, &src, &dst, CON_TMO)) != E_OK) {
		syslog(LOG_NOTICE, "[TEC:%02d SND] connect error: %s", cepid, itron_strerror(error));
		return error;
		}

	get_tim(&time);
	syslog(LOG_NOTICE, "[TEC:%02u SND] connecting: %6lu, to:   %s.%u",
	                   cepid, time / SYSTIM_HZ, IP2STR(NULL, &dst.ipaddr), dst.portno);

	if ((error = psnd_dtq(DTQ_TCP_ECHO_CLI_RCV, (intptr_t)cepid)) != E_OK) {
		syslog(LOG_NOTICE, "[TEC:%02d SND] sync error: %s", cepid, itron_strerror(error));
		goto cls_ret;
		}

	scount = total = 0;
	for (echo = NUM_ECHO; echo -- > 0; ) {
		soff = 0;
		while (soff < SND_BUF_SIZE) {
			if ((slen = tcp_snd_dat(cepid, smsg + soff, sizeof(smsg) - soff, SND_TMO)) < 0) {
				syslog(LOG_NOTICE, "[TEC:%02d SND] snd dat error: %s", cepid, itron_strerror(slen));
				goto cls_ret;
				}
			soff  += (uint16_t)slen;
			total +=     slen;
			scount ++;
#ifdef SHOW_RCV_RANGE
			syslog(LOG_NOTICE, "[TEC:%02d SND] send count: %3d, len: %4d, offset: %4d",
			                   cepid, scount, (uint16_t)slen, soff);
#endif	/* of #ifdef SHOW_RCV_RANGE */

#ifdef SND_URG_DATA_SIZE
			if (urg > 0) {
				if (urg == 1) {
					if ((slen = tcp_snd_oob(cepid, smsg, SND_URG_DATA_SIZE, SND_TMO)) >= 0)
						syslog(LOG_NOTICE, "[TEC:%02d SND] snd urg: %4d",
						                   cepid, (uint16_6)slen);
					else
						syslog(LOG_NOTICE, "[TEC:%02d SND] snd urg error: %s",
						                   cepid, itron_strerror(slen));
					}
				urg --;
				}
#endif	/* of #ifdef SND_URG_DATA_SIZE */

#if SND_DLY > 0
			syscall(dly_tsk(SND_DLY + net_rand() % SYSTIM_HZ));
#endif
			}
		}

cls_ret:
	if ((error = tcp_sht_cep(cepid)) < 0)
		syslog(LOG_NOTICE, "[TEC:%02d SND] shutdown error: %s", cepid, itron_strerror(error));

	/* 受信が完了するまで待つ。*/
	syscall(slp_tsk());

	return error;
	}

/*
 *  TCP ECHO クライアント受信タスク
 */

#ifdef USE_COPYSAVE_API

void
tcp_echo_cli_rcv_task (intptr_t exinf)
{
	ID		tskid, cepid;
	ER_UINT		rlen;
	ER		error;
	SYSTIM		time;
	uint32_t	total;
	uint16_t	roff;
	char		*rmsg;

	get_tid(&tskid);
	syslog(LOG_NOTICE, "[TCP ECHO CLI (CS) RCV:%d] started.", tskid);
	while (true) {
		if ((error = rcv_dtq(DTQ_TCP_ECHO_CLI_RCV, (intptr_t*)&cepid)) != E_OK) {
			syslog(LOG_NOTICE, "[TEC:%02d RCV] sync error: %s",
			                   cepid, itron_strerror(error));
			}
		else {
			roff = rcount = total = 0;
			while (true) {
				if ((rlen = tcp_rcv_buf(cepid, (void**)&rmsg, RCV_TMO)) < 0) {
					syslog(LOG_NOTICE, "[TEC:%02d RCV] rcv buf error: %s",
					                   cepid, itron_strerror(rlen));
					break;
					}
				else if (rlen > 0) {
					roff  += (uint16_t)rlen;
					total +=     rlen;
					rcount ++;
#ifdef SHOW_RCV_RANGE
					syslog(LOG_NOTICE, "[TEC:%02d RCV] recv count: %3d, len: %4d, data: %02x -> %02x",
					                   cepid, rcount, (uint16_t)rlen, *rmsg, *(rmsg + rlen - 1));
#endif	/* of #ifdef SHOW_RCV_RANGE */
					}
				else
					break;
				if ((error = tcp_rel_buf(cepid, rlen)) != E_OK)
					syslog(LOG_NOTICE, "[TEC:%02d RCV] rel buf error: %s", cepid, itron_strerror(error));
				}

			if ((error = tcp_cls_cep(cepid, CLS_TMO)) < 0)
				syslog(LOG_NOTICE, "[TEC:%02d RCV] close error: %s", cepid, itron_strerror(error));

			get_tim(&time);
			syslog(LOG_NOTICE, "[TEC:%02u RCV] finished:   %6lu, snd: %4u, rcv: %4u, len: %u",
			                   cepid, time / SYSTIM_HZ, scount, rcount, total);
			}

		syscall(wup_tsk(TCP_ECHO_CLI_SND_TASK));
		}
	}

#else	/* of #ifdef USE_COPYSAVE_API */

void
tcp_echo_cli_rcv_task (intptr_t exinf)
{
	static char rmsg[RCV_BUF_SIZE];

	ID		tskid, cepid;
	ER_UINT		rlen;
	ER		error;
	SYSTIM		time;
	uint32_t	total;
	uint16_t	roff;

	get_tid(&tskid);
	syslog(LOG_NOTICE, "[TCP ECHO CLI RCV:%d] started.", tskid);
	while (true) {
		if ((error = rcv_dtq(DTQ_TCP_ECHO_CLI_RCV, (intptr_t*)&cepid)) != E_OK) {
			syslog(LOG_NOTICE, "[TEC:%02d RCV] sync error: %s",
			                   cepid, itron_strerror(error));
			}
                else {
			roff = rcount = total = 0;
			while (true) {
				if ((rlen = tcp_rcv_dat(cepid, rmsg, sizeof(rmsg), RCV_TMO)) < 0) {
					syslog(LOG_NOTICE, "[TEC:%02d RCV] rcv dat error: %s", cepid, itron_strerror(rlen));
					tcp_can_cep(cepid, TFN_TCP_ALL);
					break;
					}
				else if (rlen > 0) {
					roff  += (uint16_t)rlen;
					total +=     rlen;
					rcount ++;
#ifdef SHOW_RCV_RANGE
					syslog(LOG_NOTICE, "[TEC:%02d RCV] recv count: %3d, len: %4d, data: %02x -> %02x",
					                   cepid, rcount, (uint16_t)rlen, *rmsg, *(rmsg + rlen - 1));
#endif	/* of #ifdef SHOW_RCV_RANGE */
					}
				else
					break;
				}

			if ((error = tcp_cls_cep(cepid, CLS_TMO)) < 0)
				syslog(LOG_NOTICE, "[TEC:%02d RCV] close error: %s", cepid, itron_strerror(error));

			get_tim(&time);
			syslog(LOG_NOTICE, "[TEC:%02u RCV] received:   %6lu, snd: %4u, rcv: %4u, len: %u",
			                   cepid, time / SYSTIM_HZ, scount, rcount, total);
			}

		syscall(wup_tsk(TCP_ECHO_CLI_SND_TASK));
		}
	}

#endif	/* of #ifdef USE_COPYSAVE_API */

#endif	/* of #ifdef USE_TCP_NON_BLOCKING */

/*
 *  getcomd -- コマンドを得る。
 */

#ifdef AUTO_RUN_STR

static char *
getcomd (ID cepid)
{
	ER		error;
	char		*line = NULL;
	static char	auto_run_str[] = AUTO_RUN_STR;
	static int_t	count = 0;

	if (count == 0) {
		line = auto_run_str;
		dly_tsk(3 * 1000);
		}
	else {
		while ((error = rcv_dtq(DTQ_TCP_ECHO_CLI_SND, (intptr_t*)&line)) != E_OK) {
			syslog(LOG_NOTICE, "[TEC:%02d TSK] error: %s", cepid, itron_strerror(error));
			dly_tsk(SLP_ITV);
			}
		}

	count ++;
	return line;
	}

#else	/* of #ifdef AUTO_RUN_STR */

static char *
getcomd (ID cepid)
{
	ER	error;
	char	*line = NULL;

	while ((error = rcv_dtq(DTQ_TCP_ECHO_CLI_SND, (intptr_t*)&line)) != E_OK) {
		syslog(LOG_NOTICE, "[TEC:%02d TSK] error: %s", cepid, itron_strerror(error));
		dly_tsk(SLP_ITV);
		}
	return line;
	}

#endif	/* of #ifdef AUTO_RUN_STR */

/*
 *  TCP ECHO クライアント送信タスク
 */

void
tcp_echo_cli_snd_task (intptr_t exinf)
{
	ID		tskid, cepid;
	ER		error;
	T_IN_ADDR	addr;
	uint16_t	portno;
	int_t		rep, count;
	char		*line;

#ifdef USE_TCP_EXTENTIONS

	T_TCP_CCEP	ccep;

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

	get_tid(&tskid);
	syslog(LOG_NOTICE, "[TCP ECHO CLI SND:%d,%d] started.", tskid, (ID)exinf);
	while (true) {
		line = getcomd((ID)exinf);
		line = skip_blanks(GET_IPADDR(&addr, skip_blanks(line)));	/* IP Address */

		if ('0' <= *line && *line <= '9') {				/* Port No */
			line = get_int(&rep, line);
			portno = (uint16_t)rep;
			}
		else {
			line ++;
			portno = ECHO_SRV_PORTNO;
			}

		line = skip_blanks(line);
		if ('0' <= *line && *line <= '9')				/* Repeat */
			line = get_int(&rep, line);
		else
			rep = 1;

#ifdef USE_TCP_EXTENTIONS

		ccep.cepatr = 0;
		ccep.sbufsz = TCP_ECHO_CLI_SWBUF_SIZE;
		ccep.rbufsz = TCP_ECHO_CLI_RWBUF_SIZE;

#ifdef TCP_CFG_SWBUF_CSAVE
		ccep.sbuf = NADR;
#else
		ccep.sbuf = tcp_echo_cli_swbuf;
#endif
#ifdef TCP_CFG_RWBUF_CSAVE
		ccep.rbuf = NADR;
#else
		ccep.rbuf = tcp_echo_cli_rwbuf;
#endif
#ifdef USE_TCP_NON_BLOCKING
		ccep.callback = (FP)callback_nblk_tcp_echo_cli;
#else
		ccep.callback = NULL;
#endif

		if ((error = alloc_tcp_cep(&cepid, tskid, &ccep)) != E_OK) {
			syslog(LOG_NOTICE, "[TEC:%02d TSK] CEP create error: %s", cepid, itron_strerror(error));
			continue;
			}

#else	/* of #ifdef USE_TCP_EXTENTIONS */

		cepid = (ID)exinf;

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

		tcp_echo_cli_valid = true;
		count = 0;
		while (rep == 0 || count < rep) {
			if (!tcp_echo_cli_valid) {
				syslog(LOG_NOTICE, "[TEC:%02d TSK] canceled.", cepid);
				break;
				}

			count ++;
			if (rep == 0) {
				syslog(LOG_NOTICE, "[TEC:%02d TSK] start:              repeat: %d", cepid, count);
				error = send_tcp_echo(cepid, &addr, portno);
				syslog(LOG_NOTICE, "[TEC:%02d TSK] finished:           repeat: %d", cepid, count);
				}
			else {
				syslog(LOG_NOTICE, "[TEC:%02d TSK] start:              repeat: %d/%d", cepid, count, rep);
				error = send_tcp_echo(cepid, &addr, portno);
				syslog(LOG_NOTICE, "[TEC:%02d TSK] finished:           repeat: %d/%d", cepid, count, rep);
				}

			if (error != E_OK) {
				syslog(LOG_NOTICE, "[TEC:%02d TSK] sleep %d[ms], error: %s",
				                   cepid, SLP_ITV, itron_strerror(error));
				tslp_tsk(SLP_ITV);
				syslog(LOG_NOTICE, "[TEC:%02d TSK] resume.", cepid);
				}

#if defined(SND_ITV)
#if SND_ITV > 0
			if (count > 0) {
				uint_t itv;
				
				itv = SND_ITV;
				syslog(LOG_NOTICE, "[TEC:%02d TSK] interval: %d[ms].", cepid, itv);
				syscall(dly_tsk(SND_ITV));
				}
#endif
#endif
			}

#ifdef USE_TCP_EXTENTIONS

		if ((error = free_tcp_cep(cepid)) != E_OK)
			syslog(LOG_NOTICE, "[TEC:%02d TSK] CEP delete error: %s", cepid, itron_strerror(error));

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

		}
	}

#endif	/* of #ifdef USE_TCP_ECHO_CLI */
