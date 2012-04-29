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
 *  @(#) $Id: tcp_discard_srv.c,v 1.5 2009/12/24 05:44:56 abe Exp abe $
 */

/* 
 *  DISCARD サーバ
 *
 *    ・省コピー API
 *    ・IPv4
 *    ・IPv6
 */

#include <string.h>

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
#include <t_syslog.h>
#include "tinet_cfg.h"

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <t_services.h>
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
#include <netapp/discard.h>

#ifdef USE_TCP_DISCARD_SRV

/*
 *  表示
 */

//#define SHOW_RCV_RANGE
//#define SHOW_RCV_DATA

/*
 *  全域変数
 */

/* TCP 送受信ウィンドバッファ */

#ifndef TCP_CFG_RWBUF_CSAVE_ONLY
uint8_t tcp_discard_srv_rwbuf[TCP_DISCARD_SRV_RWBUF_SIZE];
#endif

/*
 *  TCP DISCARD サーバタスク
 */

#define BUF_SIZE	TCP_DISCARD_SRV_RWBUF_SIZE

#ifdef USE_COPYSAVE_API

static ER
tcp_discard_srv (ID cepid, ID repid)
{
	T_IPEP		dst;
	ER_UINT		rlen;
	ER		error;
	SYSTIM		time;
	uint32_t	total;
	uint16_t	count;
	uint8_t		*buf;

#ifdef SHOW_RCV_DATA
	uint_t	ix;
#endif	/* of #ifdef SHOW_RCV_DATA */

	if ((error = TCP_ACP_CEP(cepid, repid, &dst, TMO_FEVR)) != E_OK) {
		syslog(LOG_NOTICE, "[TDS:%02d ACP] accept error: %s", cepid, itron_strerror(error));
		return error;
		}

#ifdef USE_TCP_EXTENTIONS
	if ((error = free_tcp_rep(repid, true)) != E_OK) {
		syslog(LOG_NOTICE, "[TDS:%02d DEL] REP delete error: %s", cepid, itron_strerror(error));
		return error;
		}
#endif	/* of #ifdef USE_TCP_EXTENTIONS */

	count = total = 0;
	get_tim(&time);
	syslog(LOG_NOTICE, "[TDS:%02d RCV] connected:  %6ld, from: %s.%d",
	                   cepid, time / SYSTIM_HZ, IP2STR(NULL, &dst.ipaddr), dst.portno);
	while ((rlen = tcp_rcv_buf(cepid, (void*)&buf, TMO_FEVR)) > 0) {
		count ++;

#ifdef SHOW_RCV_RANGE
		syslog(LOG_NOTICE, "[TDS:%02d RCV] count: %4d, len: %4d, data: %02x -> %02x",
		       cepid, count, (uint16_t)rlen, *buf, *(buf + rlen - 1));
#endif	/* of #ifdef SHOW_RCV_RANGE */

#ifdef SHOW_RCV_DATA
		for (ix = 0; ix < rlen; ix ++)
			cons_putchar(CONSOLE_PORTID,  *(buf + ix));
#endif	/* of #ifdef SHOW_RCV_DATA */

		if ((error = tcp_rel_buf(cepid, rlen)) != E_OK) {
			syslog(LOG_NOTICE, "[TDS:%02d RCV] rel buf error: %s",
			                    cepid, itron_strerror(error));
			rlen = 0;
			break;
			}
		total += rlen;
		}

	if (rlen != 0)
		syslog(LOG_NOTICE, "[TDS:%02d RCV] recv buf error: %s", cepid, itron_strerror(rlen));

	if ((error = tcp_sht_cep(cepid)) != E_OK)
		syslog(LOG_NOTICE, "[TDS:%02d RCV] shutdown error: %s", cepid, itron_strerror(error));

	if ((error = tcp_cls_cep(cepid, TMO_FEVR)) != E_OK)
		syslog(LOG_NOTICE, "[TDS:%02d RCV] close error: %s", cepid, itron_strerror(error));

	get_tim(&time);
	syslog(LOG_NOTICE, "[TDS:%02d RCV] finished:   %6ld,            rcv: %4d, len: %ld",
	                   cepid, time / SYSTIM_HZ, count, total);

	return error;
	}

#else	/* of #ifdef USE_COPYSAVE_API */

static ER
tcp_discard_srv (ID cepid, ID repid)
{
	static char buffer[BUF_SIZE];

	T_IPEP		dst;
	ER_UINT		rlen;
	ER		error;
	SYSTIM		time;
	uint32_t	total;
	uint16_t	count;
	uint8_t		*buf = buffer;

	if ((error = TCP_ACP_CEP(cepid, repid, &dst, TMO_FEVR)) != E_OK) {
		syslog(LOG_NOTICE, "[TDS:%02d RCV] accept error: %s", cepid, itron_strerror(error));
		return error;
		}

#ifdef USE_TCP_EXTENTIONS
	if ((error = free_tcp_rep(repid, true)) != E_OK) {
		syslog(LOG_NOTICE, "[TDS:%02d DEL] REP delete error: %s", cepid, itron_strerror(error));
		return error;
		}
#endif	/* of #ifdef USE_TCP_EXTENTIONS */

	count = total = 0;
	get_tim(&time);
	syslog(LOG_NOTICE, "[TDS:%02d RCV] connected:  %6ld, from: %s.%d",
	                   cepid, time / SYSTIM_HZ, IP2STR(NULL, &dst.ipaddr), dst.portno);
	while ((rlen = tcp_rcv_dat(cepid, buf, BUF_SIZE - 1, TMO_FEVR)) > 0) {
		count ++;

#ifdef SHOW_RCV_RANGE
		syslog(LOG_NOTICE, "[TDS:%02d RCV] count: %4d, len: %4d, data: %02x -> %02x",
		       cepid, count, (uint16_t)rlen, *buf, *(buf + rlen - 1));
#endif	/* of #ifdef SHOW_RCV_RANGE */

		total += rlen;
		}
	if (rlen != 0)
		syslog(LOG_NOTICE, "[TDS:%02d RCV] recv error: %s", cepid, itron_strerror(rlen));

	if ((error = tcp_sht_cep(cepid)) != E_OK)
		syslog(LOG_NOTICE, "[TDS:%02d RCV] shutdown error: %s", cepid, itron_strerror(error));

	if ((error = tcp_cls_cep(cepid, TMO_FEVR)) != E_OK)
		syslog(LOG_NOTICE, "[TDS:%02d RCV] close error: %s", cepid, itron_strerror(error));

	get_tim(&time);
	syslog(LOG_NOTICE, "[TDS:%02d RCV] finished:   %6ld,            rcv: %4d, len: %ld",
	                   cepid, time / SYSTIM_HZ, count, total);

	return error;
	}

#endif	/* of #ifdef USE_COPYSAVE_API */

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
	crep.myaddr.portno = UINT_C(9);

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
	ccep.sbuf = NADR;
	ccep.sbufsz = 0;
	ccep.rbufsz = TCP_DISCARD_SRV_RWBUF_SIZE;
	ccep.callback = NULL;

#ifdef TCP_CFG_RWBUF_CSAVE
	ccep.rbuf = NADR;
#else
	ccep.rbuf = tcp_discard_srv_rwbuf;
#endif

	return alloc_tcp_cep(cepid, tskid, &ccep);
	}

/*
 *  tcp_discard_srv_task -- TCP エコーサーバタスク
 */

void
tcp_discard_srv_task (intptr_t exinf)
{
	ID	tskid, cepid, repid;
	ER	error = E_OK;

	syscall(get_tid(&tskid));
	syslog(LOG_NOTICE, "[TCP ECHO SRV:%d] started.", tskid);
	while (true) {

		syscall(slp_tsk());
		if ((error = get_tcp_cep (&cepid)) != E_OK) {
			syslog(LOG_NOTICE, "[TDS:00 EXT] CEP create error: %s", itron_strerror(error));
			continue;
			}

		while (true) {

			if ((error = get_tcp_rep (&repid)) != E_OK) {
				syslog(LOG_NOTICE, "[TDS:00 EXT] REP create error: %s", itron_strerror(error));
				break;
				}
			else if ((error = tcp_discard_srv(cepid, repid)) != E_OK) {
				error = free_tcp_rep(repid, error != E_DLT);
				break;
				}
			}

		if ((error = free_tcp_cep(cepid)) != E_OK)
			syslog(LOG_NOTICE, "[TDS:%02d EXT] CEP delete error: %s", cepid, itron_strerror(error));

		}
	}

#else	/* of #ifdef USE_TCP_EXTENTIONS */

void
tcp_discard_srv_task(intptr_t exinf)
{
	ID	tskid;

	get_tid(&tskid);
	syslog(LOG_NOTICE, "[TCP DISCARD SRV:%d,%d] started.", tskid, (ID)exinf);
	while (true) {
		while (tcp_discard_srv((ID)exinf, TCP_DISCARD_SRV_REPID) == E_OK)
			;
		}
	}

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

#endif	/* of #ifdef USE_TCP_DISCARD_SRV */
