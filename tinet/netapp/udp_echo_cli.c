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
 *  @(#) $Id: udp_echo_cli.c,v 1.5 2009/12/24 05:44:56 abe Exp abe $
 */

/* 
 *  UDP ECHO クライアント
 */

#include <string.h>

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
#include <t_syslog.h>
#include "kernel_cfg.h"

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <t_services.h>
#include "kernel_id.h"
#include "tinet_id.h"

#endif	/* of #ifdef TARGET_KERNEL_JSP */

#include <tinet_defs.h>
#include <tinet_config.h>

#include <net/net.h>
#include <net/net_var.h>
#include <net/net_timer.h>

#include <netinet/in.h>
#include <netinet/in_itron.h>

#include <netapp/netapp.h>
#include <netapp/netapp_var.h>
#include <netapp/echo.h>

#ifdef USE_UDP_ECHO_CLI

/* echo サーバのポート番号 */

#define ECHO_SRV_PORTNO		UINT_C(7)

/*
 *  全域変数
 */

bool_t udp_echo_cli_valid;

/*
 * 変数
 */

#define BUF_SIZE	2048

static char		udp_buf[BUF_SIZE];
static T_IPEP		dst;

#ifdef USE_UDP_NON_BLOCKING

static ER_UINT		nblk_len;

/*
 *  ノンブロッキングコールのコールバック関数
 */

ER
callback_nblk_udp_echo_cli (ID cepid, FN fncd, void *p_parblk)
{
	nblk_len = *(ER_UINT*)p_parblk;
	syscall(wup_tsk(UDP_ECHO_CLI_TASK));
	return E_OK;
	}

/*
 *  send_udp_echo -- ECHO/UDP サーバにメッセージを送信する (ノンブロッキングコール使用)。
 */

static ER
send_udp_echo (ID cepid, T_IN_ADDR *ipaddr, uint16_t portno, char *line)
{
	ER_UINT	error;
	uint_t	len;

	dst.ipaddr = *ipaddr;
	dst.portno = portno;
	len         = strlen(line);
	if ((error = UDP_SND_DAT(cepid, &dst, line, len, TMO_NBLK)) != E_WBLK) {
		syslog(LOG_NOTICE, "[UEC:%02d SND] error: %s", cepid, itron_strerror(error));
		return error;
		}
	else
		syslog(LOG_NOTICE, "[UEC:%02d SND] sending:            to:   %s.%d: %s",
		                   cepid, IP2STR(NULL, &dst.ipaddr), dst.portno, line);

	syscall(slp_tsk());
	if (nblk_len < 0) {	/* 0 以下の場合は、エラーコード */
		syslog(LOG_NOTICE, "[UEC:%02d CBR] error: %s", cepid, itron_strerror(nblk_len));
		return nblk_len;
		}

	if ((error = UDP_RCV_DAT(cepid, &dst, udp_buf, sizeof(udp_buf), TMO_NBLK)) != E_WBLK) {
		syslog(LOG_NOTICE, "[UEC:%02d RCV] error: %s", cepid, itron_strerror(error));
		return error;
		}

	syscall(slp_tsk());
	if (nblk_len < 0) {	/* 0 以下の場合は、エラーコード */
		syslog(LOG_NOTICE, "[UEC:%02d RCV] error: %s", cepid, itron_strerror(nblk_len));
		return nblk_len;
		}
	else {
		udp_buf[nblk_len] = '\0';
		syslog(LOG_NOTICE, "[UEC:%02d RCV] received:           from: %s.%d: %s",
		                   cepid, IP2STR(NULL, &dst.ipaddr), dst.portno, udp_buf);
		nblk_len = E_OK;
		}

	return nblk_len;
	}

#else	/* of #ifdef USE_UDP_NON_BLOCKING */

#ifdef USE_UDP_CALL_BACK

/*
 *  コールバック関数
 */

ER
callback_udp_echo_cli (ID cepid, FN fncd, void *p_parblk)
{
	ER_UINT	len;

	if ((len = UDP_RCV_DAT(cepid, &dst, udp_buf, sizeof(udp_buf), TMO_FEVR)) >= 0) {
		*(udp_buf + len) = '\0';
		syslog(LOG_NOTICE, "[UEC:%02d RCV] recv: addr: %s.%d: %s",
		                   cepid, IP2STR(NULL, &dst.ipaddr), dst.portno, udp_buf);
		}
	else
		syslog(LOG_NOTICE, "[UEC:%02d RCV] recv: error: %s", cepid, itron_strerror(len));

	return E_OK;
	}

/*
 *  send_udp_echo -- ECHO/UDP サーバにメッセージを送信する (コールバック関数使用)。
 */

static ER
send_udp_echo (ID cepid, T_IN_ADDR *ipaddr, uint16_t portno, char *line)
{
	ER_UINT	len;

	dst.ipaddr = *ipaddr;
	dst.portno = portno;
	len        = strlen(line);
	syslog(LOG_NOTICE, "[UEC:%02d SND] sending:            to:   %s.%d: %s",
	                   cepid, IP2STR(NULL, &dst.ipaddr), dst.portno, line);
	if ((len = UDP_SND_DAT(cepid, &dst, line, len, TMO_FEVR)) < 0)
		syslog(LOG_NOTICE, "[UEC:%02d SND] error: %s", cepid, itron_strerror(len));

	return E_OK;
	}

#else	/* of #ifdef USE_UDP_CALL_BACK */

/*
 *  send_udp_echo -- ECHO/UDP サーバにメッセージを送信する (コールバック関数未使用)。
 */

static ER
send_udp_echo (ID cepid, T_IN_ADDR *ipaddr, uint16_t portno, char *line)
{
	ER_UINT	len;

	dst.ipaddr = *ipaddr;
	dst.portno = portno;
	len         = strlen(line);
	syslog(LOG_NOTICE, "[UEC:%02d SND] sending:            to:   %s.%d: %s",
	                   cepid, IP2STR(NULL, &dst.ipaddr), dst.portno, line);
	if ((len = UDP_SND_DAT(cepid, &dst, line, len, TMO_FEVR)) < 0) {
		syslog(LOG_NOTICE, "[UEC:%02d SND] error: %s", cepid, itron_strerror(len));
		return len;
		}

	if ((len = UDP_RCV_DAT(cepid, &dst, udp_buf, sizeof(udp_buf), 10*1000)) >= 0) {
		*(udp_buf + len) = '\0';
		syslog(LOG_NOTICE, "[UEC:%02d RCV] received:           from: %s.%d: %s",
		                   cepid, IP2STR(NULL, &dst.ipaddr), dst.portno, line);
		return E_OK;
		}
	else {
		syslog(LOG_NOTICE, "[UEC:%02d RCV] error: %s", cepid, itron_strerror(len));
		return len;
		}
	}

#endif	/* of #ifdef USE_UDP_CALL_BACK */

#endif	/* of #ifdef USE_UDP_NON_BLOCKING */

#define MESSAGE_FORMAT	"MSG 1000000"

/*
 *  UDP ECHO クライアント送信タスク
 */

void
udp_echo_cli_task (intptr_t exinf)
{
	static char msg[sizeof(MESSAGE_FORMAT)] = MESSAGE_FORMAT;

	T_IN_ADDR	addr;
	ID		tskid, cepid;
	ER		error = E_OK;
	char		*line, *p;
	int_t		rep;
	uint32_t	count, msgno;
	uint16_t	portno;

#ifdef USE_UDP_EXTENTIONS

	T_UDP_CCEP	ccep;

#endif	/* of #ifdef USE_UDP_EXTENTIONS */

	get_tid(&tskid);
	syslog(LOG_NOTICE, "[UDP ECHO CLI:%d,%d] started.", tskid, (ID)exinf);
	while (true) {
		if (rcv_dtq(DTQ_UDP_ECHO_CLI, (intptr_t*)&line) == E_OK) {

#ifdef USE_UDP_EXTENTIONS

			ccep.cepatr = UINT_C(0);
			ccep.myaddr.portno = UDP_PORTANY;

#if defined(SUPPORT_INET4)
			ccep.myaddr.ipaddr = IPV4_ADDRANY;
#endif

#if defined(SUPPORT_INET6)
			memcpy(&ccep.myaddr.ipaddr, &ipv6_addrany, sizeof(T_IN6_ADDR));
#endif

#ifdef USE_UDP_NON_BLOCKING
			ccep.callback = (FP)callback_nblk_udp_echo_cli;
#else
			ccep.callback = NULL;
#endif

			if ((error = alloc_udp_cep(&cepid, tskid, &ccep)) != E_OK) {
				syslog(LOG_NOTICE, "[UEC:%02d TSK] CEP create error: %s", cepid, itron_strerror(error));
				continue;
				}

#else	/* of #ifdef USE_UDP_EXTENTIONS */

			cepid = (ID)exinf;

#endif	/* of #ifdef USE_UDP_EXTENTIONS */

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
			if ('0' <= *line && *line <= '9') {				/* Repeat */
				line = get_int(&rep, line);
				if (rep > 1000000)
					rep = 1000000;
				udp_echo_cli_valid = true;
				for (count = 0;  count ++ < rep; ) {
					if (!udp_echo_cli_valid) {
						syslog(LOG_NOTICE, "[UEC:%02d TSK] canceled.", cepid);
						break;
						}
						
					p = &msg[sizeof(MESSAGE_FORMAT)] - 1;
					for (msgno = count; msgno > 0; msgno /= 10)
						*(-- p) = msgno % 10 + '0';
					while (*(p - 2) != 'G')
						*(-- p) = ' ';

					if ((error = send_udp_echo(cepid, &addr, portno, msg)) != E_OK)
						break;

					dly_tsk(10 * SYSTIM_HZ + net_rand() % (10 * SYSTIM_HZ));
					}
				}

			else								/* Single Message */
				error = send_udp_echo(cepid, &addr, portno, line);

			if (error != E_OK)
				syslog(LOG_NOTICE, "[UEC:%02d TSK] error: %s", cepid, itron_strerror(error));

#ifdef USE_UDP_EXTENTIONS

			if ((error = free_udp_cep(cepid, !(error == E_NOEXS || error == E_DLT))) != E_OK)
				syslog(LOG_NOTICE, "[UEC:%02d TSK] CEP delete error: %s", cepid, itron_strerror(error));

#endif	/* of #ifdef USE_UDP_EXTENTIONS */

			}
		}
	}

#endif	/* of #ifdef USE_UDP_ECHO_CLI */
