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
 *  @(#) $Id: tcp_echo_srv1.c,v 1.5 2009/12/24 05:44:56 abe Exp abe $
 */

/* 
 *  TCP ECHO サーバ #1
 *
 *    ・送受信タスク同一型
 *    ・ノンブロッキングコール
 *    ・省コピー API
 *    ・IPv4/IPv6
 *    ・緊急データの受信
 */

#include <string.h>

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
#include <t_syslog.h>
#include "kernel_cfg.h"
#include "tinet_cfg.h"

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <t_services.h>
#include "kernel_id.h"
#include "tinet_id.h"

#endif	/* of #ifdef TARGET_KERNEL_JSP */

#include <tinet_defs.h>
#include <tinet_config.h>

#include <net/if.h>
#include <net/if_ppp.h>
#include <net/if_loop.h>
#include <net/ethernet.h>
#include <net/net.h>
#include <net/net_timer.h>

#include <netinet/in.h>
#include <netinet/in_itron.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>

#include <netapp/netapp.h>
#include <netapp/netapp_var.h>
#include <netapp/echo.h>

#ifdef USE_TCP_ECHO_SRV1

/*
 *  表示
 */

//#define SHOW_RCV_RANGE

/*
 *  コネクション切断方法の指定
 */

#define USE_TCP_SHT_CEP

/*
 *  タイムアウト
 */

//#define CLS_TMO		TMO_FEVR	/* Close Wait は標準で 60秒 */
#define CLS_TMO		(70*SYSTIM_HZ+(net_rand()%SYSTIM_HZ)*10)
//#define RCV_TMO		TMO_FEVR
#define RCV_TMO		(30*SYSTIM_HZ+(net_rand()%SYSTIM_HZ)*30)
//#define SND_TMO		TMO_FEVR
#define SND_TMO		(40*SYSTIM_HZ+(net_rand()%SYSTIM_HZ)*20)

/*
 *  全域変数
 */

/* TCP 送受信ウィンドバッファ */

#ifndef TCP_CFG_SWBUF_CSAVE
uint8_t tcp_echo_srv_swbuf[NUM_TCP_ECHO_SRV_TASKS][TCP_ECHO_SRV_SWBUF_SIZE];
#endif

#ifndef TCP_CFG_RWBUF_CSAVE
uint8_t tcp_echo_srv_rwbuf[NUM_TCP_ECHO_SRV_TASKS][TCP_ECHO_SRV_RWBUF_SIZE];
#endif

/*
 *  変数
 */

/*
 *  注意:
 *
 *    BUF_SIZE は TCP の
 *    送信ウインドウバッファサイズ + 受信ウインドウバッファサイズの 
 *    3/2 倍以上の大きさがなければ、デッドロックする可能性がある。
 */

#define BUF_SIZE	((TCP_ECHO_SRV_SWBUF_SIZE + \
                          TCP_ECHO_SRV_RWBUF_SIZE) * 3 / 2)

static T_IPEP		dst;			/* 接続相手		*/

#ifdef USE_TCP_NON_BLOCKING

static char		buffer[BUF_SIZE];
static ER		nblk_error = E_OK;
static ER_UINT		nblk_slen  = 0;
static ER_UINT		nblk_rlen  = 0;

#else	/* of #ifdef USE_TCP_NON_BLOCKING */

#ifdef USE_COPYSAVE_API

#else	/* of #ifdef USE_COPYSAVE_API */

static char		buffer[BUF_SIZE];

#endif	/* of #ifdef USE_COPYSAVE_API */

#endif	/* of #ifdef USE_TCP_NON_BLOCKING */

#ifdef USE_TCP_NON_BLOCKING

/*
 *  ノンブロッキングコールのコールバック関数
 */

ER
callback_nblk_tcp_echo_srv (ID cepid, FN fncd, void *p_parblk)
{
	ER	error = E_OK;

	switch (fncd) {

	case TFN_TCP_ACP_CEP:
		nblk_error = *(ER*)p_parblk;
		syscall(sig_sem(SEM_TCP_ECHO_SRV_NBLK_READY));
		break;

	case TFN_TCP_RCV_DAT:
		if ((nblk_rlen = *(ER*)p_parblk) < 0)
			syslog(LOG_NOTICE, "[TES:%02d CBN] recv error: %s", cepid, itron_strerror(nblk_rlen));
		syscall(sig_sem(SEM_TCP_ECHO_SRV_NBLK_READY));
		break;

	case TFN_TCP_SND_DAT:
		if ((nblk_slen = *(ER*)p_parblk) < 0)
			syslog(LOG_NOTICE, "[TES:%02d CBN] send error: %s", cepid, itron_strerror(nblk_slen));
		syscall(sig_sem(SEM_TCP_ECHO_SRV_NBLK_READY));
		break;

	case TFN_TCP_CLS_CEP:
		if ((nblk_error = *(ER*)p_parblk) < 0)
			syslog(LOG_NOTICE, "[TES:%02d CBN] close error: %s", cepid, itron_strerror(nblk_error));
		syscall(sig_sem(SEM_TCP_ECHO_SRV_NBLK_READY));
		break;

	case TFN_TCP_RCV_BUF:
		if ((nblk_rlen = *(ER*)p_parblk) < 0)
			syslog(LOG_NOTICE, "[TES:%02d CBN] rbuf error: %s", cepid, itron_strerror(nblk_rlen));
		syscall(sig_sem(SEM_TCP_ECHO_SRV_NBLK_READY));
		break;

	case TFN_TCP_GET_BUF:
		if ((nblk_slen = *(ER*)p_parblk) < 0)
			syslog(LOG_NOTICE, "[TES:%02d CBN] sbuf error: %s", cepid, itron_strerror(nblk_slen));
		syscall(sig_sem(SEM_TCP_ECHO_SRV_NBLK_READY));
		break;

#ifdef USE_TCP_EXTENTIONS

	case TEV_TCP_RCV_OOB:
		if ((nblk_rlen = *(ER*)p_parblk) < 0)
			syslog(LOG_NOTICE, "[TES:%02d OOB] callback error: %s", cepid, itron_strerror(nblk_rlen));
		else if (nblk_rlen > 0) {
			char ch;

			if ((nblk_rlen = tcp_rcv_oob(cepid, &ch, sizeof(ch))) > 0)
				syslog(LOG_NOTICE, "[TES:%02d OOB] recv oob: 0x%02x", cepid, ch);
			else if (nblk_rlen < 0)
				syslog(LOG_NOTICE, "[TES:%02d OOB] recv error: %s", cepid, itron_strerror(nblk_rlen));
			}
		break;

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

	case TFN_TCP_CON_CEP:
	case TFN_TCP_SND_OOB:
	default:
		error = E_PAR;
		break;
		}
	return error;
	}

#ifdef USE_COPYSAVE_API

static ER
tcp_echo_srv (ID cepid, ID repid)
{
	ER		error;
	SYSTIM		now;
	uint32_t	total;
	uint16_t	rblen, sblen, rlen, slen, soff, scount, rcount;
	char		*rbuf, *sbuf, head, tail;

	if ((error = TCP_ACP_CEP(cepid, repid, &dst, TMO_NBLK)) != E_WBLK) {
		syslog(LOG_NOTICE, "[TES:%02d ACP] error: %s", cepid, itron_strerror(error));
		return error;
		}

	/* 相手から接続されるまで待つ。*/
	syscall(wai_sem(SEM_TCP_ECHO_SRV_NBLK_READY));

	if (nblk_error == E_OK) {
		syscall(get_tim(&now));
		syslog(LOG_NOTICE, "[TES:%02d ACP] connected:  %6lu, from: %s.%u",
		                   cepid, now / SYSTIM_HZ, IP2STR(NULL, &dst.ipaddr), dst.portno);
		}
	else {
		syslog(LOG_NOTICE, "[TES:%02d ACP] error: %s", cepid, itron_strerror(nblk_error));
		return nblk_error;
		}

#ifdef USE_TCP_EXTENTIONS
	if ((error = free_tcp_rep(repid, true)) != E_OK)
		syslog(LOG_NOTICE, "[TES:%02d DEL] REP delete error: %s", cepid, itron_strerror(error));
#endif	/* of #ifdef USE_TCP_EXTENTIONS */

	rlen = scount = rcount = total = 0;
	while (true) {
		if ((error = tcp_rcv_buf(cepid, (void**)&rbuf, TMO_NBLK)) != E_WBLK) {
			syslog(LOG_NOTICE, "[TES:%02d RCV] error: %s", cepid, itron_strerror(error));
			break;
			}

		/* 受信するまで待つ。*/
		syscall(wai_sem(SEM_TCP_ECHO_SRV_NBLK_READY));

		if (nblk_rlen < 0) {	/* エラー */
			syslog(LOG_NOTICE, "[TES:%02d RCV] error: %s", 
			                   cepid, itron_strerror(nblk_rlen));
			break;
			}
		else if (nblk_rlen == 0)	/* 受信終了 */
			break;

		rblen = nblk_rlen;

		/* バッファの残りにより、受信長を調整する。*/
		if (rblen > BUF_SIZE - rlen)
			rblen = BUF_SIZE - rlen;
		total += rblen;
		rlen   = rblen;

		head = *rbuf;
		tail = *(rbuf + rblen - 1);
		rcount ++;

#ifdef SHOW_RCV_RANGE
		syslog(LOG_NOTICE, "[TES:%02d RCV] "
		                   "rcount: %4d, len: %4d, data %02x -> %02x",
		                   cepid, rcount, rblen, head, tail);
#endif	/* of #ifdef SHOW_RCV_RANGE */

		memcpy(buffer, rbuf, rblen);

		if ((error = tcp_rel_buf(cepid, rlen)) < 0) {
			syslog(LOG_NOTICE, "[TES:%02d REL] error: %s",
			                   cepid, itron_strerror(error));
			break;
			}

		soff = 0;
		while (rlen > 0) {

			if ((error = tcp_get_buf(cepid, (void**)&sbuf, TMO_NBLK)) != E_WBLK) {
				syslog(LOG_NOTICE, "[TES:%02d GET] error: %s",
				                   cepid, itron_strerror(error));
				goto err_fin;
				}

			/* 送信バッファの獲得が完了するまで待つ。*/
			syscall(wai_sem(SEM_TCP_ECHO_SRV_NBLK_READY));

			if (nblk_slen < 0) {
				syslog(LOG_NOTICE, "[TES:%02d GET] error: %s",
				                   cepid, itron_strerror(nblk_slen));
				goto err_fin;
				}

			sblen = nblk_slen;
			scount ++;
			slen = sblen < rlen ? sblen : rlen;
			memcpy(sbuf, buffer + soff, slen);

			if ((error = tcp_snd_buf(cepid, slen)) != E_OK) {
				syslog(LOG_NOTICE, "[TES:%02d SND] error: %s",
				                   cepid, itron_strerror(error));
				goto err_fin;
				}
#ifdef SHOW_RCV_RANGE
			syslog(LOG_NOTICE, "[TES:%02d SND] scount: %4d, len: %4d",
			                   cepid, scount, slen);
#endif	/* of #ifdef SHOW_RCV_RANGE */

			rlen -= slen;
			soff += slen;
			}
		}
err_fin:

#ifdef USE_TCP_SHT_CEP
	if ((error = tcp_sht_cep(cepid)) != E_OK)
		syslog(LOG_NOTICE, "[TES:%02d SHT] error: %s", cepid, itron_strerror(error));
#endif	/* of #ifdef USE_TCP_SHT_CEP */

	if ((error = tcp_cls_cep(cepid, TMO_NBLK)) != E_WBLK)
		syslog(LOG_NOTICE, "[TES:%02d CLS] error: %s", cepid, itron_strerror(error));

	/* 開放が完了するまで待つ。*/
	syscall(wai_sem(SEM_TCP_ECHO_SRV_NBLK_READY));

	syscall(get_tim(&now));
	syslog(LOG_NOTICE, "[TES:%02d FIN] finished:   %6lu, snd: %4u, rcv: %4u, len: %lu",
	                   cepid, now / SYSTIM_HZ, scount, rcount, total);

	return error == E_WBLK ? E_OK : error;
	}

#else	/* of #ifdef USE_COPYSAVE_API */

static ER
tcp_echo_srv (ID cepid, ID repid)
{
	SYSTIM		now;
	ER		error;
	uint32_t	total;
	uint16_t	rlen, slen, soff, scount, rcount;
	char		head, tail;

	if ((error = TCP_ACP_CEP(cepid, repid, &dst, TMO_NBLK)) != E_WBLK) {
		syslog(LOG_NOTICE, "[TES:%02d ACP] error: %s", cepid, itron_strerror(error));
		return error;
		}

	/* 相手から接続されるまで待つ。*/
	syscall(wai_sem(SEM_TCP_ECHO_SRV_NBLK_READY));

	if (nblk_error == E_OK) {
		syscall(get_tim(&now));
		syslog(LOG_NOTICE, "[TES:%02d ACP] connected:  %6lu, from: %s.%u",
		                   cepid, now / SYSTIM_HZ, IP2STR(NULL, &dst.ipaddr), dst.portno);
		}
	else {
		syslog(LOG_NOTICE, "[TES:%02d ACP] error: %s", cepid, itron_strerror(nblk_error));
		return nblk_error;
		}

#ifdef USE_TCP_EXTENTIONS
	if ((error = free_tcp_rep(repid, true)) != E_OK)
		syslog(LOG_NOTICE, "[TES:%02d DEL] REP delete error: %s", cepid, itron_strerror(error));
#endif	/* of #ifdef USE_TCP_EXTENTIONS */

	scount = rcount = total = 0;
	while (true) {
		if ((error = tcp_rcv_dat(cepid, buffer, BUF_SIZE - 1, TMO_NBLK)) != E_WBLK) {
			syslog(LOG_NOTICE, "[TES:%02d RCV] error: %s",
			                   cepid, itron_strerror(error));
			break;
			}

		/* 受信完了まで待つ。*/
		syscall(wai_sem(SEM_TCP_ECHO_SRV_NBLK_READY));

		if (nblk_rlen < 0) {
			syslog(LOG_NOTICE, "[TES:%02d RCV] error: %s",
			                   cepid, itron_strerror(nblk_rlen));
			break;
			}
		else if (nblk_rlen == 0)
			break;

		rlen = nblk_rlen;
		head = *buffer;
		tail = *(buffer + rlen - 1);
		rcount ++;

#ifdef SHOW_RCV_RANGE
		syslog(LOG_NOTICE, "[TES:%02d RCV] rcount: %4d, len: %4d, data %02x -> %02x",
		                   cepid, rcount, rlen, head, tail);
#endif	/* of #ifdef SHOW_RCV_RANGE */

		total += rlen;
		soff = 0;
		while (rlen > 0) {
			scount ++;
			if ((error = tcp_snd_dat(cepid, &buffer[soff], rlen, TMO_NBLK)) != E_WBLK) {
				syslog(LOG_NOTICE, "[TES:%02d SND] error: %s",
				                   cepid, itron_strerror(error));
				goto err_fin;
				}

			/* 送信完了まで待つ。*/
			syscall(wai_sem(SEM_TCP_ECHO_SRV_NBLK_READY));

			if (nblk_slen < 0) {
				syslog(LOG_NOTICE, "[TES:%02d SND] error: %s",
				                   cepid, itron_strerror(nblk_slen));
				goto err_fin;
				}

			slen = nblk_slen;

#ifdef SHOW_RCV_RANGE
			syslog(LOG_NOTICE, "[TES:%02d SND] scount: %4d, len: %4d",
			                   cepid, scount, slen);
#endif	/* of #ifdef SHOW_RCV_RANGE */

			rlen -= slen;
			soff += slen;
			}
		}
err_fin:

#ifdef USE_TCP_SHT_CEP
	if ((error = tcp_sht_cep(cepid)) != E_OK)
		syslog(LOG_NOTICE, "[TES:%02d SHT] error: %s", cepid, itron_strerror(error));
#endif	/* of #ifdef USE_TCP_SHT_CEP */

	if ((error = tcp_cls_cep(cepid, TMO_NBLK)) != E_WBLK)
		syslog(LOG_NOTICE, "[TES:%02d CLS] error: %s", cepid, itron_strerror(error));

	/* 開放が完了するまで待つ。*/
	syscall(wai_sem(SEM_TCP_ECHO_SRV_NBLK_READY));

	syscall(get_tim(&now));
	syslog(LOG_NOTICE, "[TES:%02d FIN] finished:   %6lu, snd: %4u, rcv: %4u, len: %lu",
	                   cepid, now / SYSTIM_HZ, scount, rcount, total);

	return error == E_WBLK ? E_OK : error;
	}

#endif	/* of #ifdef USE_COPYSAVE_API */

#else	/* of #ifdef USE_TCP_NON_BLOCKING */

#ifdef USE_COPYSAVE_API

static ER
tcp_echo_srv (ID cepid, ID repid)
{
	ER_UINT		rblen, sblen;
	SYSTIM		now;
	ER		error;
	uint32_t	total;
	uint16_t	rlen, slen, soff, scount, rcount;
	char		*rbuf, *sbuf, head, tail;

	if ((error = TCP_ACP_CEP(cepid, repid, &dst, TMO_FEVR)) != E_OK) {
		syslog(LOG_NOTICE, "[TES:%02d ACP] error: %s", cepid, itron_strerror(error));
		return error;
		}

#ifdef USE_TCP_EXTENTIONS
	if ((error = free_tcp_rep(repid, true)) != E_OK)
		syslog(LOG_NOTICE, "[TES:%02d DEL] REP delete error: %s", cepid, itron_strerror(error));
#endif	/* of #ifdef USE_TCP_EXTENTIONS */

	scount = rcount = total = 0;
	syscall(get_tim(&now));
	syslog(LOG_NOTICE, "[TES:%02d ACP] connected:  %6lu, from: %s.%u",
	                   cepid, now / SYSTIM_HZ, IP2STR(NULL, &dst.ipaddr), dst.portno);
	while (true) {
		if ((rblen = tcp_rcv_buf(cepid, (void**)&rbuf, RCV_TMO)) <= 0) {
			if (rblen != E_OK)
				syslog(LOG_NOTICE, "[TES:%02d RCV] error: %s",
				                   cepid, itron_strerror(rblen));
			break;
			}

		head = *rbuf;
		tail = *(rbuf + rblen - 1);
		rcount ++;

		//syslog(LOG_NOTICE, "[TES:%02d RCV] len: %4d", cepid, (uint16_t)rblen);
#ifdef SHOW_RCV_RANGE
		syslog(LOG_NOTICE, "[TES:%02d RCV] rcount: %4d, len: %4d, data %02x -> %02x",
		       cepid, rcount, (uint16_t)rblen, head, tail);
#endif	/* of #ifdef SHOW_RCV_RANGE */

		rlen   = (uint16_t)rblen;
		total +=     rblen;
		soff = 0;
		while (rlen > 0) {

			if ((sblen = tcp_get_buf(cepid, (void**)&sbuf, SND_TMO)) < 0) {
				syslog(LOG_NOTICE, "[TES:%02d GET] error: %s",
				                   cepid, itron_strerror(sblen));
				goto err_fin;
				}

			//syslog(LOG_NOTICE, "[TES:%02d SND] len: %4d", cepid, (uint16_t)sblen);
			scount ++;
			slen = rlen < (uint16_t)sblen ? rlen : (uint16_t)sblen;
			memcpy(sbuf, rbuf + soff, slen);
			if ((error = tcp_snd_buf(cepid, slen)) != E_OK) {
				syslog(LOG_NOTICE, "[TES:%02d SND] error: %s",
				                   cepid, itron_strerror(error));
				goto err_fin;
				}
#ifdef SHOW_RCV_RANGE
			syslog(LOG_NOTICE, "[TES:%02d SND] scount: %4d, len: %4d", cepid, scount, slen);
#endif	/* of #ifdef SHOW_RCV_RANGE */

			rlen -= slen;
			soff += slen;
			}

		if ((error = tcp_rel_buf(cepid, rblen)) < 0) {
			syslog(LOG_NOTICE, "[TES:%02d REL] error: %s", cepid, itron_strerror(error));
			break;
			}
		}
err_fin:

#ifdef USE_TCP_SHT_CEP
	if ((error = tcp_sht_cep(cepid)) != E_OK)
		syslog(LOG_NOTICE, "[TES:%02d SHT] error: %s", cepid, itron_strerror(error));
#endif	/* of #ifdef USE_TCP_SHT_CEP */

	if ((error = tcp_cls_cep(cepid, CLS_TMO)) != E_OK)
		syslog(LOG_NOTICE, "[TES:%02d CLS] error: %s", cepid, itron_strerror(error));

	syscall(get_tim(&now));
	syslog(LOG_NOTICE, "[TES:%02d FIN] finished:   %6lu, snd: %4u, rcv: %4u, len: %lu",
	                   cepid, now / SYSTIM_HZ, scount, rcount, total);

	return error;
	}

#else	/* of #ifdef USE_COPYSAVE_API */

/*
 *  tcp_echo_srv -- TCP エコーサーバ
 */

static ER
tcp_echo_srv (ID cepid, ID repid)
{
	ER_UINT		rlen, slen;
	ER		error = E_OK;
	SYSTIM		now;
	uint32_t	total;
	uint16_t	soff, scount, rcount;

	if ((error = TCP_ACP_CEP(cepid, repid, &dst, TMO_FEVR)) != E_OK) {
		syslog(LOG_NOTICE, "[TES:%02d ACP] error: %s", cepid, itron_strerror(error));
		return error;
		}

#ifdef USE_TCP_EXTENTIONS
	if ((error = free_tcp_rep(repid, true)) != E_OK)
		syslog(LOG_NOTICE, "[TES:%02d DEL] REP delete error: %s", cepid, itron_strerror(error));
#endif	/* of #ifdef USE_TCP_EXTENTIONS */

	scount = rcount = total = 0;
	syscall(get_tim(&now));
	syslog(LOG_NOTICE, "[TES:%02d ACP] connected:  %6lu, from: %s.%u",
	                   cepid, now / SYSTIM_HZ, IP2STR(NULL, &dst.ipaddr), dst.portno);
	while (true) {
		if ((rlen = tcp_rcv_dat(cepid, buffer, BUF_SIZE - 1, RCV_TMO)) <= 0) {
			if (rlen != E_OK)
				syslog(LOG_NOTICE, "[TES:%02d RCV] error: %s",
				                   cepid, itron_strerror(rlen));
			break;
			}

		rcount ++;
#ifdef SHOW_RCV_RANGE
		syslog(LOG_NOTICE, "[TES:%02d RCV] rcount: %4d, len: %4d, data %02x -> %02x",
		       cepid, rcount, (uint16_t)rlen, *buffer, *(buffer + rlen - 1));
#endif	/* of #ifdef SHOW_RCV_RANGE */

		total += rlen;
		soff = 0;
		while (rlen > 0) {
			scount ++;
			if ((slen = tcp_snd_dat(cepid, &buffer[soff], rlen, SND_TMO)) < 0) {
				syslog(LOG_NOTICE, "[TES:%02d SND] error: %s",
				                   cepid, itron_strerror(slen));
				goto err_fin;
				}
#ifdef SHOW_RCV_RANGE
			syslog(LOG_NOTICE, "[TES:%02d SND] scount: %4d, len: %4d", cepid, scount, (uint16_t)slen);
#endif	/* of #ifdef SHOW_RCV_RANGE */

			rlen -= slen;
			soff += slen;
			}
		}
err_fin:

#ifdef USE_TCP_SHT_CEP
	if ((error = tcp_sht_cep(cepid)) != E_OK)
		syslog(LOG_NOTICE, "[TES:%02d SHT] error: %s", cepid, itron_strerror(error));
#endif	/* of #ifdef USE_TCP_SHT_CEP */

	if ((error = tcp_cls_cep(cepid, CLS_TMO)) != E_OK)
		syslog(LOG_NOTICE, "[TES:%02d CLS] error: %s", cepid, itron_strerror(error));

	syscall(get_tim(&now));
	syslog(LOG_NOTICE, "[TES:%02d FIN] finished:   %6lu, snd: %4u, rcv: %4u, len: %lu",
	                   cepid, now / SYSTIM_HZ, scount, rcount, total);

	return error;
	}

#endif	/* of #ifdef USE_COPYSAVE_API */

#endif	/* of #ifdef USE_TCP_NON_BLOCKING */

#ifdef USE_TCP_EXTENTIONS

/*
 *  get_tcp_rep -- TCP 受付口を獲得する。
 */

static ER
get_tcp_rep (ID *repid)
{
	ID		tskid;
	T_TCP_CREP	crep;

	get_tid(&tskid);

	crep.repatr = UINT_C(0);
	crep.myaddr.portno = UINT_C(7);

#if defined(SUPPORT_INET4)
	crep.myaddr.ipaddr = IPV4_ADDRANY;
#endif

#if defined(SUPPORT_INET6)
	memcpy(&crep.myaddr.ipaddr, &ipv6_addrany, sizeof(T_IN6_ADDR));
#endif

	return alloc_tcp_rep(repid, tskid, &crep);
	}

/*
 *  get_tcp_cep -- TCP 通信端点とを獲得する。
 */

static ER
get_tcp_cep (ID *cepid)
{
	ID		tskid;
	T_TCP_CCEP	ccep;

	get_tid(&tskid);

	ccep.cepatr = UINT_C(0);
	ccep.sbufsz = TCP_ECHO_SRV_SWBUF_SIZE;
	ccep.rbufsz = TCP_ECHO_SRV_RWBUF_SIZE;

#ifdef TCP_CFG_SWBUF_CSAVE
	ccep.sbuf = NADR;
#else
	ccep.sbuf = tcp_echo_srv_swbuf;
#endif
#ifdef TCP_CFG_RWBUF_CSAVE
	ccep.rbuf = NADR;
#else
	ccep.rbuf = tcp_echo_srv_rwbuf;
#endif
#ifdef USE_TCP_NON_BLOCKING
	ccep.callback = (FP)callback_nblk_tcp_echo_srv;
#else
	ccep.callback = NULL;
#endif

	return alloc_tcp_cep(cepid, tskid, &ccep);
	}

/*
 *  tcp_echo_srv_task -- TCP エコーサーバタスク
 */

void
tcp_echo_srv_task (intptr_t exinf)
{
	ID	tskid, cepid, repid;
	ER	error = E_OK;

	syscall(get_tid(&tskid));
	syslog(LOG_NOTICE, "[TCP ECHO SRV:%d] started.", tskid);
	while (true) {

		syscall(slp_tsk());
		if ((error = get_tcp_cep (&cepid)) != E_OK) {
			syslog(LOG_NOTICE, "[TES:00 EXT] CEP create error: %s", itron_strerror(error));
			continue;
			}

		while (true) {

			if ((error = get_tcp_rep (&repid)) != E_OK) {
				syslog(LOG_NOTICE, "[TES:00 EXT] REP create error: %s", itron_strerror(error));
				break;
				}
			else if ((error = tcp_echo_srv(cepid, repid)) != E_OK) {
				error = free_tcp_rep(repid, error != E_DLT);
				break;
				}
			}

		if ((error = free_tcp_cep(cepid)) != E_OK)
			syslog(LOG_NOTICE, "[TES:%02d EXT] CEP delete error: %s", cepid, itron_strerror(error));

		}
	}

#else	/* of #ifdef USE_TCP_EXTENTIONS */

/*
 *  tcp_echo_srv_task -- TCP エコーサーバタスク
 */

void
tcp_echo_srv_task(intptr_t exinf)
{
	ID	tskid;
	ER	error;

	syscall(get_tid(&tskid));
	syslog(LOG_NOTICE, "[TCP ECHO SRV:%d,%d] started.", tskid, (ID)exinf);
	while (true) {
		while ((error = tcp_echo_srv((ID)exinf, TCP_ECHO_SRV_REPID)) == E_OK)
			;
		syslog(LOG_NOTICE, "[TES:%02d] goto sleep 60[s], error: %s", (ID)exinf, itron_strerror(error));
		tslp_tsk(60 * 1000);
		}
	}

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

#endif	/* of #ifdef USE_TCP_ECHO_SRV1 */
