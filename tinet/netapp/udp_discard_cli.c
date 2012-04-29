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
 *  @(#) $Id: udp_discard_cli.c,v 1.5 2009/12/24 05:44:56 abe Exp abe $
 */

/* 
 *  UDP DISCARD クライアント
 */

#include <stdlib.h>
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

#include <net/if.h>
#include <net/if_ppp.h>
#include <net/if_loop.h>
#include <net/ethernet.h>
#include <net/net.h>
#include <net/net_var.h>
#include <net/net_timer.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <netinet/in_itron.h>

#include <netapp/netapp.h>
#include <netapp/netapp_var.h>
#include <netapp/discard.h>

#ifdef USE_UDP_DISCARD_CLI

/* discard サーバのポート番号 */

#define DISCARD_SRV_PORTNO	UINT_C(9)

/*
 * 変数
 */

#define PAT_BEGIN	' '
#define PAT_END		'~'

/*
 *  全域変数
 */

bool_t udp_discard_cli_valid;

/*
 *  send_udp_discard -- DISCARD/UDP サーバにメッセージを送信する。
 */

static ER
send_udp_discard (ID cepid, T_IN_ADDR *ipaddr, uint16_t portno)
{
	static char smsg[IF_MTU - IP_UDP_HDR_SIZE];

	T_IPEP		dst;
	ER_UINT		len;
	SYSTIM		time;
	char		*p;
	uint32_t	total;
	uint16_t	pat, slen, scount;

	dst.ipaddr = *ipaddr;
	dst.portno = portno;

	p = smsg;
	for (slen = IF_MTU - IP_UDP_HDR_SIZE; slen > 0; )
		for (pat = PAT_BEGIN; slen > 0 && pat <= PAT_END; pat ++, slen --)
			*p ++ = pat;

	get_tim(&time);
	syslog(LOG_NOTICE, "[UDC:%02d SND] sending:    %6ld, to:   %s.%d",
	                   cepid, time / SYSTIM_HZ, IP2STR(NULL, &dst.ipaddr), dst.portno);

	scount = total = 0;
	udp_discard_cli_valid = true;
	for (slen = IF_MTU - IP_UDP_HDR_SIZE; udp_discard_cli_valid && slen > 0; slen --) {
		scount ++;
		if ((len = UDP_SND_DAT(cepid, &dst, smsg, slen, TMO_FEVR)) < 0) {
			syslog(LOG_NOTICE, "[UDC:%02d SND] send error: %s", cepid, itron_strerror(len));
			return len;
			}
		else
			syslog(LOG_NOTICE, "[UDC:%02d SND] send: %4d, %4d", cepid, scount, slen);
		total += len;
		dly_tsk(10 * SYSTIM_HZ + net_rand() % (10 * SYSTIM_HZ));
		}

	get_tim(&time);
	syslog(LOG_NOTICE, "[UDC:%02d SND] finished:   %6ld, snd: %4d,            len: %ld",
		           cepid, time / SYSTIM_HZ, scount, total);
	return E_OK;
	}

/*
 *  UDP DISCARD クライアント送信タスク
 */

void
udp_discard_cli_task (intptr_t exinf)
{
	T_IN_ADDR	addr;
	ID		tskid, cepid;
	ER		error;
	char		*line;
	int_t		no;
	uint16_t	portno;

#ifdef USE_UDP_EXTENTIONS

	T_UDP_CCEP	ccep;

#endif	/* of #ifdef USE_UDP_EXTENTIONS */

	get_tid(&tskid);
	syslog(LOG_NOTICE, "[UDP DISCARD CLI:%d,%d] started.", tskid, (ID)exinf);
	while (true) {
		if (rcv_dtq(DTQ_UDP_DISCARD_CLI, (intptr_t*)&line) == E_OK) {
			line = skip_blanks(GET_IPADDR(&addr, skip_blanks(line)));	/* IP Address */

			if ('0' <= *line && *line <= '9') {				/* Port No */
				line = get_int(&no, line);
				portno = (uint16_t)no;
				}
			else {
				line ++;
				portno = DISCARD_SRV_PORTNO;
				}

#ifdef USE_UDP_EXTENTIONS

			ccep.cepatr = UINT_C(0);
			ccep.callback = NULL;
			ccep.myaddr.portno = UDP_PORTANY;

#if defined(SUPPORT_INET4)
			ccep.myaddr.ipaddr = IPV4_ADDRANY;
#endif

#if defined(SUPPORT_INET6)
			memcpy(&ccep.myaddr.ipaddr, &ipv6_addrany, sizeof(T_IN6_ADDR));
#endif

			if ((error = alloc_udp_cep(&cepid, tskid, &ccep)) != E_OK) {
				syslog(LOG_NOTICE, "[UDC:%02d TSK] CEP create error: %s", cepid, itron_strerror(error));
				continue;
				}

#else	/* of #ifdef USE_UDP_EXTENTIONS */

			cepid = (ID)exinf;

#endif	/* of #ifdef USE_UDP_EXTENTIONS */

			if ((error = send_udp_discard(cepid, &addr, portno)) != E_OK)
				syslog(LOG_NOTICE, "[UDC:%02d TSK] error: %s", cepid, itron_strerror(error));

#ifdef USE_UDP_EXTENTIONS

			if ((error = free_udp_cep(cepid, !(error == E_NOEXS || error == E_DLT))) != E_OK)
				syslog(LOG_NOTICE, "[UDC:%02d TSK] CEP delete error: %s", cepid, itron_strerror(error));

#endif	/* of #ifdef USE_UDP_EXTENTIONS */

			}
		}
	}

#endif	/* of #ifdef USE_UDP_DISCARD_CLI */
