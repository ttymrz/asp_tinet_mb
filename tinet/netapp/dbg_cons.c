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
 *  @(#) $Id: dbg_cons.c,v 1.5 2009/12/24 05:44:56 abe Exp abe $
 */

/* 
 *  デバッグコンソール
 */

#include <stdlib.h>

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
#include <syssvc/serial.h>
#include <syssvc/logtask.h>
#include "kernel_cfg.h"
#include "kernel/kernel_impl.h"
#include "kernel/task.h"

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <s_services.h>
#include <t_services.h>
#include "kernel_id.h"

#include "../kernel/jsp_kernel.h"
#include "../kernel/task.h"

#endif	/* of #ifdef TARGET_KERNEL_JSP */

#include <tinet_defs.h>
#include <tinet_config.h>

#include <net/if.h>
#include <net/if_ppp.h>
#include <net/if_loop.h>
#include <net/ethernet.h>
#include <net/net.h>
#include <net/net_buf.h>
#include <net/net_timer.h>
#include <net/ppp_var.h>
#include <net/ppp_fsm.h>
#include <net/ppp_lcp.h>
#include <net/ppp_ipcp.h>
#include <net/ppp_modem.h>
#include <net/net_count.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/in_itron.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/ip6.h>
#include <netinet6/ip6_var.h>
#include <netinet6/nd6.h>
#include <netinet/if_ether.h>
#include <netinet/tcp.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <netinet/tcp_fsm.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>

#include <net/if_var.h>
#include <net/if6_var.h>

#include <netapp/netapp.h>
#include <netapp/netapp_var.h>
#include <netapp/echo.h>
#include <netapp/discard.h>
#include <netapp/dbg_cons.h>

#ifdef USE_LCD

#include "sc1602/lcd.h"

#endif	/* of #ifdef USE_LCD */

#ifdef USE_DBG_CONS

#define DBG_LINE_SIZE	63

/*
 *  task_status -- タスクの状態の出力
 */

static const char task_stat_str[][sizeof("SUSPENDED")] = {
	"DORMANT",		/* 休止状態		*/
	"RUNNABLE",		/* 実行できる状態	*/
	"WAINTING",		/* 待ち状態		*/
	"SUSPENDED",		/* 強制待ち状態		*/
	"SLEEP",		/* 起床待ち状態		*/
	"WOBJ",			/* 同期・通信オブジェクトに対する待ち状態	*/
	"WOBJCB",		/* 共通部分の待ちキューにつながっている。	*/
	};

static void
task_status (ID portid, char *line)
{
	TCB	*tcb;
	int_t	ix, st, sx;

	WAI_NET_CONS_PRINTF();
	cons_printf(portid,
	            "TASK Status\n"
	            "ID PI State\n");
	for (ix = TMIN_TSKID; ix <= tmax_tskid; ix ++) {
		tcb = get_tcb(ix);
		cons_printf(portid, "%2d %2d ", ix, tcb->priority);
		st = tcb->tstat;
		for (sx = 0; st; sx ++) {
			if (st & 0x01) {
				cons_printf(portid, "%s", task_stat_str[sx]);
				if (st & 0xfe)
					cons_printf(portid, "|");
				}
			st >>= 1;
			}
		cons_printf(portid, "\n");
		}

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();
	}

#ifdef SUPPORT_TCP

/*
 *  tcp_status -- TCP の状態の出力
 */

static const char tcp_fsm_str[][sizeof("ESTABLISHED")] = {
	"CLOSED",		/* クローズ			*/
	"LISTEN",		/* 受動オープン			*/
	"SYN_SENT",		/* 能動オープン、SYN 送信済み	*/
	"SYN_RECVD",		/* 受動オープン、SYN 受信済み	*/
	"ESTABLISHED",		/* コネクション開設完了		*/
	"CLOSE_WAIT",		/* 相手から FIN 受信、APP の終了待ち	*/
	"FIN_WAIT_1",		/* APP が終了、FIN 送信済み、ACK 待ち	*/
	"CLOSING",		/* 同時クローズ、FIN 交換済み、ACK 待ち	*/
	"LAST_ACK",		/* APP が終了、ACK 待ち			*/
	"FIN_WAIT_2",		/* 相手からの FIN 待ち			*/
	"TIME_WAIT",		/* 相手からの FIN 受信済み、時間待ち	*/
	};

#if defined(SUPPORT_INET4)

static void
tcp_status (ID portid, char *line)
{
	T_TCP_CEP*	cep;
	uint_t		len;

#if defined(NUM_TCP_TW_CEP_ENTRY) && NUM_TCP_TW_CEP_ENTRY > 0

	T_TCP_TWCEP*	twcep;

#endif	/* of #if defined(NUM_TCP_TW_CEP_ENTRY) && NUM_TCP_TW_CEP_ENTRY > 0 */

#ifdef TCP_CFG_PASSIVE_OPEN

	T_TCP_REP*	rep;
	int_t		cnt;

#endif	/* of #ifdef TCP_CFG_PASSIVE_OPEN */

	WAI_NET_CONS_PRINTF();
	cons_printf(portid,
	            "TCP CEP Status\n"
	            "ID  Snd-Q Rcv-Q Local Address         Foreign Address       State\n");

	for (cep = tcp_cep; cep < &tcp_cep[tmax_tcp_cepid]; cep ++)
		if (!VALID_TCP_CEP(cep)) {
			cons_printf(portid,
			            "%2d%c     0     0"
			            " -                    "
			            " -                    "
			            " INVALID\n",
			            GET_TCP_CEPID(cep),
			            DYNAMIC_TCP_CEP(cep) ? 'D' : ' ');
			}
		else if (cep->fsm_state == TCP_FSM_CLOSED) {
			cons_printf(portid,
			            "%2d%c     0     0"
			            " -                    "
			            " -                    "
			            " CLOSED\n",
			            GET_TCP_CEPID(cep),
			            DYNAMIC_TCP_CEP(cep) ? 'D' : ' ');
			}
		else if (cep->fsm_state == TCP_FSM_LISTEN) {
			cons_printf(portid,
			            "%2d%c     0     0 ",
			            GET_TCP_CEPID(cep),
			            DYNAMIC_TCP_CEP(cep) ? 'D' : ' ');
			len  = PUT_IPADDR(portid, &cep->myaddr.ipaddr, 0);
			cons_putchar(portid, ':');
			len += cons_putnumber(portid, cep->myaddr.portno, 10, radhex, 0, false, ' ');
			for (len = 20 - len; len -- > 0; )
				cons_putchar(portid, ' ');
			cons_printf(portid,
			            " -                    "
			            " LISTEN\n");
			}
		else {
			cons_printf(portid,
			            "%2d%c %5d %5d ",
			            GET_TCP_CEPID(cep),
			            DYNAMIC_TCP_CEP(cep) ? 'D' : ' ',
			            cep->swbuf_count, cep->rwbuf_count);
			len  = PUT_IPADDR(portid, &cep->myaddr.ipaddr, 0);
			cons_putchar(portid, ':');
			len += cons_putnumber(portid, cep->myaddr.portno, 10, radhex, 0, false, ' ');
			for (len = 21 - len; len -- > 0; )
				cons_putchar(portid, ' ');
			len  = PUT_IPADDR(portid, &cep->dstaddr.ipaddr, 0);
			cons_putchar(portid, ':');
			len += cons_putnumber(portid, cep->dstaddr.portno, 10, radhex, 0, false, ' ');
			for (len = 20 - len; len -- > 0; )
				cons_putchar(portid, ' ');
			if (cep->fsm_state == TCP_FSM_TIME_WAIT)
				cons_printf(portid, " %s(%d)\n", tcp_fsm_str[cep->fsm_state],
				                    cep->timer[TCP_TIM_2MSL] / TCP_SLOW_HZ);
			else
				cons_printf(portid, " %s\n", tcp_fsm_str[cep->fsm_state]);
			}

#if defined(NUM_TCP_TW_CEP_ENTRY) && NUM_TCP_TW_CEP_ENTRY > 0

	for (twcep = tcp_twcep; twcep < &tcp_twcep[NUM_TCP_TW_CEP_ENTRY]; twcep ++) {
		if (twcep->fsm_state == TCP_FSM_CLOSED) {
			cons_printf(portid,
			            "%2dTW    0     0"
			            " -                    "
			            " -                    "
			            " CLOSED\n",
			            GET_TCP_TWCEPID(twcep));
			}
		else {
			cons_printf(portid, "%2dTW    0     0 ", GET_TCP_TWCEPID(twcep));
			len  = PUT_IPADDR(portid, &twcep->myaddr.ipaddr, 0);
			cons_putchar(portid, ':');
			len += cons_putnumber(portid, twcep->myaddr.portno, 10, radhex, 0, false, ' ');
			for (len = 21 - len; len -- > 0; )
				cons_putchar(portid, ' ');
			len  = PUT_IPADDR(portid, &twcep->dstaddr.ipaddr, 0);
			cons_putchar(portid, ':');
			len += cons_putnumber(portid, twcep->dstaddr.portno, 10, radhex, 0, false, ' ');
			for (len = 20 - len; len -- > 0; )
				cons_putchar(portid, ' ');
			if (twcep->fsm_state == TCP_FSM_TIME_WAIT)
				cons_printf(portid, " %s(%d)\n", tcp_fsm_str[twcep->fsm_state],
				                    twcep->timer_2msl / TCP_SLOW_HZ);
			else
				cons_printf(portid, " %s\n", tcp_fsm_str[twcep->fsm_state]);
			}
		}

#endif	/* of #if defined(NUM_TCP_TW_CEP_ENTRY) && NUM_TCP_TW_CEP_ENTRY > 0 */

#ifdef TCP_CFG_PASSIVE_OPEN

	cons_printf(portid,
	            "TCP REP Status\n"
	            "ID  Local Address         CEP\n");
	for (rep = tcp_rep; rep < &tcp_rep[tmax_tcp_repid]; rep ++) {
		cons_printf(portid, "%2d%c ", GET_TCP_REPID(rep), DYNAMIC_TCP_REP(rep) ? 'D' : ' ');
		if (VALID_TCP_REP(rep)) {
			len  = PUT_IPADDR(portid, &rep->myaddr.ipaddr, 0);
			cons_putchar(portid, ':');
			len += cons_putnumber(portid, rep->myaddr.portno, 10, radhex, 0, false, ' ');
			for (len = 21 - len; len -- > 0; )
				cons_putchar(portid, ' ');
			cnt = 0;
			for (cep = tcp_cep; cep < &tcp_cep[tmax_tcp_cepid]; cep ++) {
				if (cep->rep == rep) {
					if (cnt > 0)
						cons_printf(portid, ",%d", GET_TCP_CEPID(cep));
					else
						cons_printf(portid, "%d", GET_TCP_CEPID(cep));
					cnt ++;
					}
				}
			}
		else
			cons_printf(portid, "INVALID");
		cons_putchar(portid, '\n');
		}

#endif	/* of #ifdef TCP_CFG_PASSIVE_OPEN */

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();
	}

#endif	/* of #if defined(SUPPORT_INET4) */

#if defined(SUPPORT_INET6)

static void
tcp_status (ID portid, char *line)
{
	T_TCP_CEP*	cep;
	uint_t		len;

#if defined(NUM_TCP_TW_CEP_ENTRY) && NUM_TCP_TW_CEP_ENTRY > 0

	T_TCP_TWCEP*	twcep;

#endif	/* of #if defined(NUM_TCP_TW_CEP_ENTRY) && NUM_TCP_TW_CEP_ENTRY > 0 */

#ifdef TCP_CFG_PASSIVE_OPEN

	T_TCP_REP*	rep;
	int_t		cnt;

#endif	/* of #ifdef TCP_CFG_PASSIVE_OPEN */

	WAI_NET_CONS_PRINTF();
	cons_printf(portid,
	            "TCP CEP Status\n"
	            "ID  Snd-Q Rcv-Q Foreign Address                             State\n");

	for (cep = tcp_cep; cep < &tcp_cep[tmax_tcp_cepid]; cep ++)
		if (!VALID_TCP_CEP(cep)) {
			cons_printf(portid,
			            "%2d%c     0     0"
			            " -                                          "
			            " INVALID\n",
			            GET_TCP_CEPID(cep),
			            DYNAMIC_TCP_CEP(cep) ? 'D' : ' ');
			}
		else if (cep->fsm_state == TCP_FSM_CLOSED) {
			cons_printf(portid,
			            "%2d%c     0     0"
			            " -                                          "
			            " CLOSED\n",
			            GET_TCP_CEPID(cep),
			            DYNAMIC_TCP_CEP(cep) ? 'D' : ' ');
			}
		else if (cep->fsm_state == TCP_FSM_LISTEN) {
			cons_printf(portid,
			            "%2d%c     0     0"
			            " -                                          "
			            " LISTEN\n",
			            GET_TCP_CEPID(cep),
			            DYNAMIC_TCP_CEP(cep) ? 'D' : ' ');
			}
		else {
			cons_printf(portid,
			            "%2d%c %5d %5d ",
			            GET_TCP_CEPID(cep),
			            DYNAMIC_TCP_CEP(cep) ? 'D' : ' ',
			            cep->swbuf_count, cep->rwbuf_count);
			len  = PUT_IPADDR(portid, &cep->dstaddr.ipaddr, 0);
			cons_putchar(portid, '.');
			len += cons_putnumber(portid, cep->dstaddr.portno, 10, radhex, 0, false, ' ');
			for (len = 42 - len; len -- > 0; )
				cons_putchar(portid, ' ');
			if (cep->fsm_state == TCP_FSM_TIME_WAIT)
				cons_printf(portid, " %s(%d)\n", tcp_fsm_str[cep->fsm_state],
				                    cep->timer[TCP_TIM_2MSL] / TCP_SLOW_HZ);
			else
				cons_printf(portid, " %s\n", tcp_fsm_str[cep->fsm_state]);
			}

#if defined(NUM_TCP_TW_CEP_ENTRY) && NUM_TCP_TW_CEP_ENTRY > 0

	for (twcep = tcp_twcep; twcep < &tcp_twcep[NUM_TCP_TW_CEP_ENTRY]; twcep ++) {
		if (twcep->fsm_state == TCP_FSM_CLOSED) {
			cons_printf(portid,
			            "%2dTW    0     0"
			            " -                                          "
			            " CLOSED\n",
			            GET_TCP_TWCEPID(twcep));
			}
		else {
			cons_printf(portid, "%2dTW    0     0 ", GET_TCP_TWCEPID(twcep));
			len  = PUT_IPADDR(portid, &twcep->dstaddr.ipaddr, 0);
			cons_putchar(portid, '.');
			len += cons_putnumber(portid, twcep->dstaddr.portno, 10, radhex, 0, false, ' ');
			for (len = 42 - len; len -- > 0; )
				cons_putchar(portid, ' ');
			if (twcep->fsm_state == TCP_FSM_TIME_WAIT)
				cons_printf(portid, " %s(%d)\n", tcp_fsm_str[twcep->fsm_state],
				                    twcep->timer_2msl / TCP_SLOW_HZ);
			else
				cons_printf(portid, " %s\n", tcp_fsm_str[twcep->fsm_state]);
			}
		}

#endif	/* of #if defined(NUM_TCP_TW_CEP_ENTRY) && NUM_TCP_TW_CEP_ENTRY > 0 */

#ifdef TCP_CFG_PASSIVE_OPEN

	cons_printf(portid,
	            "TCP REP Status\n"
	            "ID  Local Address         CEP\n");
	for (rep = tcp_rep; rep < &tcp_rep[tmax_tcp_repid]; rep ++) {
		cons_printf(portid, "%2d%c ", GET_TCP_REPID(rep), DYNAMIC_TCP_REP(rep) ? 'D' : ' ');
		if (VALID_TCP_REP(rep)) {
			len  = PUT_IPADDR(portid, &rep->myaddr.ipaddr, 0);
			cons_putchar(portid, '.');
			len += cons_putnumber(portid, rep->myaddr.portno, 10, radhex, 0, false, ' ');
			for (len = 21 - len; len -- > 0; )
				cons_putchar(portid, ' ');
			cnt = 0;
			for (cep = tcp_cep; cep < &tcp_cep[tmax_tcp_cepid]; cep ++) {
				if (cep->rep == rep) {
					if (cnt > 0)
						cons_printf(portid, ",%d", GET_TCP_CEPID(cep));
					else
						cons_printf(portid, "%d", GET_TCP_CEPID(cep));
					cnt ++;
					}
				}
			}
		else
			cons_printf(portid, "INVALID");
		cons_putchar(portid, '\n');
		}

#endif	/* of #ifdef TCP_CFG_PASSIVE_OPEN */

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();
	}

#endif	/* of #if defined(SUPPORT_INET6) */

#endif	/* of #ifdef SUPPORT_TCP */

#ifdef SUPPORT_UDP

/*
 *  udp_status -- UDP の状態の出力
 */

static void
udp_status (ID portid, char *line)
{
	T_UDP_CEP*	cep;
	uint_t		len;

	WAI_NET_CONS_PRINTF();
	cons_printf(portid,
	            "UDP CEP Status\n"
	            "ID  Local Address\n");

	for (cep = udp_cep; cep < &udp_cep[tmax_udp_ccepid]; cep ++) {
		cons_printf(portid, "%2d%c ", GET_UDP_CEPID(cep), DYNAMIC_UDP_CEP(cep) ? 'D' : ' ');
		if (VALID_UDP_CEP(cep)) {
			len  = PUT_IPADDR(portid, &cep->myaddr.ipaddr, 0);
			cons_putchar(portid, '.');
			len += cons_putnumber(portid, cep->myaddr.portno, 10, radhex, 0, false, ' ');
			}
		else
			cons_printf(portid, "INVALID");
		cons_putchar(portid, '\n');
		}

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();
	}

#endif	/* of #ifdef SUPPORT_UDP */

#ifdef SUPPORT_ETHER

#if defined(SUPPORT_INET4)

/*
 *  ifa_status -- ARP キャッシュ出力
 */

static void
ifa_status (ID portid, char *line)
{
	const T_ARP_ENTRY	*cache;
	int_t			ix;

	WAI_NET_CONS_PRINTF();
	cons_printf(portid,
	            "ARP Cache\n"
	            "IX Expire IP Address      MAC Address\n");

	/* expire の単位は [ms]。*/
	cache = arp_get_cache();
	for (ix = 0; ix < NUM_ARP_ENTRY; ix ++) {
		if (cache[ix].expire != 0)
			cons_printf(portid, "%2d %6d %15I %M\n",
			                    ix, cache[ix].expire / NET_TIMER_HZ,
			                    &cache[ix].ip_addr, cache[ix].mac_addr);
		}

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();
	}

#endif	/* of #if defined(SUPPORT_INET4) */

#if defined(SUPPORT_INET6)

/*
 * ifa_status -- 近隣キャッシュ状態の出力
 */

static const char nd_state_str[][sizeof("INCOMPLETE")] = {
	"FREE",			/* 未使用		*/
	"NO STATE",		/* 状態未定義		*/
	"INCOMPLETE",		/* 未解決		*/
	"REACHABLE",		/* 到達可能		*/
	"STALE",		/* 到達可能性は未確認	*/
	"DELAY",		/* 到達可能性の確認待機	*/
	"PROBE",		/* 到達可能性を確認中	*/
	};

static void
ifa_status (ID portid, char *line)
{
	const T_LLINFO_ND6	*cache;
	SYSTIM			now;
	int_t			ix;

	WAI_NET_CONS_PRINTF();
	cons_printf(portid,
	            "ND Cache Status\n"
	            "IX Expire State      MAC Address       IP Address\n");

	/* expire の単位は [ms]。*/
	get_tim(&now);
	cache = nd6_get_cache();
	for (ix = 0; ix < NUM_ND6_CACHE_ENTRY; ix ++) {
		if (cache[ix].state != ND6_LLINFO_FREE) {
			cons_printf(portid, "%2d %6d %10s %M %I\n",
			            ix, 
			            cache[ix].state == ND6_LLINFO_STALE ||
			            (uint32_t)(cache[ix].expire - now) <= 0
			                ? 0 : (cache[ix].expire - now) / SYSTIM_HZ,
			            nd_state_str[cache[ix].state],
			            &cache[ix].ifaddr,
			            &cache[ix].addr);
			}
		}

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();
	}

#endif	/* of #if defined(SUPPORT_INET6) */

#endif	/* of #ifdef SUPPORT_ETHER */

#if NET_COUNT_ENABLE

/*
 *  net_count_struct -- ネットワーク統計情報
 */

static void
net_count_struct (ID portid, char *name, T_NET_COUNT *item)
{
	cons_printf(portid, "\n%s\n", name);
	cons_printf(portid, "\t受信オクテット数\t%lu\n",	item->in_octets);
	cons_printf(portid, "\t送信オクテット数\t%lu\n",	item->out_octets);
	cons_printf(portid, "\t受信バケット数\t%lu\n",		item->in_packets);
	cons_printf(portid, "\t送信バケット数\t%lu\n",		item->out_packets);
	cons_printf(portid, "\t受信エラーバケット数\t%lu\n",	item->in_err_packets);
	cons_printf(portid, "\t送信エラーバケット数\t%lu\n",	item->out_err_packets);
	}

/*
 *  net_count -- ネットワーク統計情報
 */

static void
net_count (ID portid, char *line)
{
	const T_NET_BUF_ENTRY	*tbl;
	SYSTIM			now;
	int_t			ix;

	WAI_NET_CONS_PRINTF();
	get_tim(&now);
	cons_printf(portid, "ネットワーク統計情報\t経過時間[ms]\t%lu\t", now);
	if (now > (1000 * 3600 * 24))
		cons_printf(portid, "%3lu日 %2lu時間 %2lu分 %2lu秒\n",
		             now / (1000 * 3600 * 24),
		            (now / (1000 * 3600)) % 24,
		            (now / (1000 * 60  )) % 60,
		            (now /  1000        ) % 60);
	else
		cons_printf(portid, "%2lu時間 %2lu分 %2lu秒\n",
		            (now / (1000 * 3600)) % 24,
		            (now / (1000 * 60  )) % 60,
		            (now /  1000        ) % 60);

#ifdef SUPPORT_PPP

	net_count_struct(portid, "HDLC", &net_count_hdlc);
	cons_printf(portid, "\nPPP\n");
	cons_printf(portid, "\t受信オクテット数\t%lu\n",	net_count_ppp.in_octets);
	cons_printf(portid, "\t送信オクテット数\t%lu\n",	net_count_ppp.out_octets);
	cons_printf(portid, "\t受信バケット数\t%lu\n",		net_count_ppp.in_packets);
	cons_printf(portid, "\t送信バケット数\t%lu\n",		net_count_ppp.out_packets);
	cons_printf(portid, "\t受信エラーバケット数\t%lu\n",	net_count_ppp.in_err_packets);
	cons_printf(portid, "\t送信エラーバケット数\t%lu\n",	net_count_ppp.out_err_packets);
	cons_printf(portid, "\tnet_buf 割り当て失敗数\t%lu\n",	net_count_ppp_no_bufs);

	cons_printf(portid, "\nLCP\n");
	cons_printf(portid, "\t受信オクテット数\t%lu\n",	net_count_ppp_lcp_in_octets);
	cons_printf(portid, "\t受信バケット数\t%lu\n",		net_count_ppp_lcp_in_packets);

	cons_printf(portid, "\nIPCP\n");
	cons_printf(portid, "\t受信オクテット数\t%lu\n",	net_count_ppp_ipcp_in_octets);
	cons_printf(portid, "\t受信バケット数\t%lu\n",		net_count_ppp_ipcp_in_packets);

	cons_printf(portid, "\nPAP\n");
	cons_printf(portid, "\t受信オクテット数\t%lu\n",	net_count_ppp_upap_in_octets);
	cons_printf(portid, "\t受信バケット数\t%lu\n",		net_count_ppp_upap_in_packets);

#endif	/* of #ifdef SUPPORT_PPP */

#ifdef SUPPORT_ETHER

	net_count_struct(portid, "イーサネット", &net_count_ether);

	cons_printf(portid, "\nイーサネット・ネットワークインタフェース\n");
	cons_printf(portid, "\t受信オクテット数\t%lu\n",			net_count_ether_nic[NC_ETHER_NIC_IN_OCTETS]);
	cons_printf(portid, "\t受信バケット数\t%lu\n",				net_count_ether_nic[NC_ETHER_NIC_IN_PACKETS]);
	cons_printf(portid, "\t受信エラーバケット数\t%lu\n",			net_count_ether_nic[NC_ETHER_NIC_IN_ERR_PACKETS]);
	cons_printf(portid, "\tnet_buf 割り当て失敗数\t%lu\n",			net_count_ether_nic[NC_ETHER_NIC_NO_BUFS]);
	cons_printf(portid, "\t受信セマフォ資源返却上限オーバー数\t%lu\n",	net_count_ether_nic[NC_ETHER_NIC_RXB_QOVRS]);
	cons_printf(portid, "\t送信オクテット数\t%lu\n",			net_count_ether_nic[NC_ETHER_NIC_OUT_OCTETS]);
	cons_printf(portid, "\t送信バケット数\t%lu\n",				net_count_ether_nic[NC_ETHER_NIC_OUT_PACKETS]);
	cons_printf(portid, "\t送信エラーバケット数\t%lu\n",			net_count_ether_nic[NC_ETHER_NIC_OUT_ERR_PACKETS]);
	cons_printf(portid, "\tコリジョン数\t%lu\n",				net_count_ether_nic[NC_ETHER_NIC_COLS]);
	cons_printf(portid, "\t送信セマフォ資源返却上限オーバー数\t%lu\n",	net_count_ether_nic[NC_ETHER_NIC_TXB_QOVRS]);
	cons_printf(portid, "\t送信タイムアウト数	\t%lu\n",		net_count_ether_nic[NC_ETHER_NIC_TIMEOUTS]);
	cons_printf(portid, "\tリセット数\t%lu\n",				net_count_ether_nic[NC_ETHER_NIC_RESETS]);

#endif	/* of #ifdef SUPPORT_ETHER */

#if defined(SUPPORT_INET4)

#ifdef SUPPORT_ETHER

	net_count_struct(portid, "ARP", &net_count_arp);

#endif	/* of #ifdef SUPPORT_ETHER */

	cons_printf(portid, "\nIPv4\n");
	cons_printf(portid, "\t受信オクテット数\t%lu\n",		net_count_ip4[NC_IP4_IN_OCTETS]);
	cons_printf(portid, "\t受信データグラム数\t%lu\n",		net_count_ip4[NC_IP4_IN_PACKETS]);
	cons_printf(portid, "\t受信エラーデータグラム数\t%lu\n",	net_count_ip4[NC_IP4_IN_ERR_PACKETS]);
	cons_printf(portid, "\tチェックサムエラー数\t%lu\n",		net_count_ip4[NC_IP4_IN_ERR_CKSUM]);
	cons_printf(portid, "\t長さエラー数\t%lu\n",			net_count_ip4[NC_IP4_IN_ERR_SHORT]);
	cons_printf(portid, "\tバージョンエラー数\t%lu\n",		net_count_ip4[NC_IP4_IN_ERR_VER]);
	cons_printf(portid, "\tアドレスエラー数\t%lu\n",		net_count_ip4[NC_IP4_IN_ERR_ADDR]);
	cons_printf(portid, "\tプロトコルエラー数\t%lu\n",		net_count_ip4[NC_IP4_IN_ERR_PROTO]);
	cons_printf(portid, "\tオプション入力数\t%lu\n",		net_count_ip4[NC_IP4_OPTS]);
	cons_printf(portid, "\t分割受信数\t%lu\n",			net_count_ip4[NC_IP4_FRAG_IN]);
	cons_printf(portid, "\t分割受信フラグメント数\t%lu\n",		net_count_ip4[NC_IP4_FRAG_IN_FRAGS]);
	cons_printf(portid, "\t分割受信再構成成功数\t%lu\n",		net_count_ip4[NC_IP4_FRAG_IN_OK]);
	cons_printf(portid, "\t分割受信破棄数\t%lu\n",			net_count_ip4[NC_IP4_FRAG_IN_DROP]);
	cons_printf(portid, "\t分割受信バッファり当て失敗数\t%lu\n",	net_count_ip4[NC_IP4_FRAG_IN_NO_BUF]);
	cons_printf(portid, "\t分割受信タイムアウト数\t%lu\n",		net_count_ip4[NC_IP4_FRAG_IN_TMOUT]);
	cons_printf(portid, "\t送信オクテット数\t%lu\n",		net_count_ip4[NC_IP4_OUT_OCTETS]);
	cons_printf(portid, "\t送信データグラム数\t%lu\n",		net_count_ip4[NC_IP4_OUT_PACKETS]);
	cons_printf(portid, "\t送信エラーデータグラム数\t%lu\n",	net_count_ip4[NC_IP4_OUT_ERR_PACKETS]);
	cons_printf(portid, "\t分割送信数\t%lu\n",			net_count_ip4[NC_IP4_FRAG_OUT]);
	cons_printf(portid, "\t分割送信フラグメント数\t%lu\n",		net_count_ip4[NC_IP4_FRAG_OUT_FRAGS]);

	net_count_struct(portid, "ICMP", &net_count_icmp4);

#endif	/* of #if defined(SUPPORT_INET4) */

#if defined(SUPPORT_INET6)

	cons_printf(portid, "\nIPv6\n");
	cons_printf(portid, "\t受信オクテット数\t%lu\n",		net_count_ip6[NC_IP6_IN_OCTETS]);
	cons_printf(portid, "\t受信データグラム数\t%lu\n",		net_count_ip6[NC_IP6_IN_PACKETS]);
	cons_printf(portid, "\t受信エラーデータグラム数\t%lu\n",	net_count_ip6[NC_IP6_IN_ERR_PACKETS]);
	cons_printf(portid, "\t長さエラー数\t%lu\n",			net_count_ip6[NC_IP6_IN_ERR_SHORT]);
	cons_printf(portid, "\tバージョンエラー数\t%lu\n",		net_count_ip6[NC_IP6_IN_ERR_VER]);
	cons_printf(portid, "\tアドレスエラー数\t%lu\n",		net_count_ip6[NC_IP6_IN_ERR_ADDR]);
	cons_printf(portid, "\tプロトコルエラー数\t%lu\n",		net_count_ip6[NC_IP6_IN_ERR_PROTO]);
	cons_printf(portid, "\t分割受信数\t%lu\n",			net_count_ip6[NC_IP6_FRAG_IN]);
	cons_printf(portid, "\t分割受信フラグメント数\t%lu\n",		net_count_ip6[NC_IP6_FRAG_IN_FRAGS]);
	cons_printf(portid, "\t分割受信再構成成功数\t%lu\n",		net_count_ip6[NC_IP6_FRAG_IN_OK]);
	cons_printf(portid, "\t分割受信破棄数\t%lu\n",			net_count_ip6[NC_IP6_FRAG_IN_DROP]);
	cons_printf(portid, "\t分割受信バッファり当て失敗数\t%lu\n",	net_count_ip6[NC_IP6_FRAG_IN_NO_BUF]);
	cons_printf(portid, "\t分割受信タイムアウト数\t%lu\n",		net_count_ip6[NC_IP6_FRAG_IN_TMOUT]);
	cons_printf(portid, "\t送信オクテット数\t%lu\n",		net_count_ip6[NC_IP6_OUT_OCTETS]);
	cons_printf(portid, "\t送信データグラム数\t%lu\n",		net_count_ip6[NC_IP6_OUT_PACKETS]);
	cons_printf(portid, "\t送信エラーデータグラム数\t%lu\n",	net_count_ip6[NC_IP6_OUT_ERR_PACKETS]);
	cons_printf(portid, "\t分割送信数\t%lu\n",			net_count_ip6[NC_IP6_FRAG_OUT]);
	cons_printf(portid, "\t分割送信フラグメント数\t%lu\n",		net_count_ip6[NC_IP6_FRAG_OUT_FRAGS]);

	cons_printf(portid, "\nICMPv6\n");
	cons_printf(portid, "\t受信オクテット数\t%lu\n",		net_count_icmp6[NC_ICMP6_IN_OCTETS]);
	cons_printf(portid, "\t受信パケット数\t%lu\n",			net_count_icmp6[NC_ICMP6_IN_PACKETS]);
	cons_printf(portid, "\t受信エラーパケット数\t%lu\n",		net_count_icmp6[NC_ICMP6_IN_ERR_PACKETS]);
	cons_printf(portid, "\t受信チックサムエラー数\t%lu\n",		net_count_icmp6[NC_ICMP6_IN_ERR_CKSUM]);
	cons_printf(portid, "\t送信オクテット数\t%lu\n",		net_count_icmp6[NC_ICMP6_OUT_OCTETS]);
	cons_printf(portid, "\t送信パケット数\t%lu\n",			net_count_icmp6[NC_ICMP6_OUT_PACKETS]);
	cons_printf(portid, "\t送信エラーパケット数\t%lu\n",		net_count_icmp6[NC_ICMP6_OUT_ERR_PACKETS]);

	cons_printf(portid, "\n近隣探索\n");
	cons_printf(portid, "\tルータ通知受信数\t%lu\n",		net_count_nd6[NC_ND6_RA_IN_PACKETS]);
	cons_printf(portid, "\tルータ要請送信数\t%lu\n",		net_count_nd6[NC_ND6_RS_OUT_PACKETS]);
	cons_printf(portid, "\t近隣通知受信数\t%lu\n",			net_count_nd6[NC_ND6_NA_IN_PACKETS]);
	cons_printf(portid, "\t近隣通知送信数\t%lu\n",			net_count_nd6[NC_ND6_NA_OUT_PACKETS]);
	cons_printf(portid, "\t近隣要請受信数\t%lu\n",			net_count_nd6[NC_ND6_NS_IN_PACKETS]);
	cons_printf(portid, "\t近隣要請送信数\t%lu\n",			net_count_nd6[NC_ND6_NS_OUT_PACKETS]);
	cons_printf(portid, "\t重複アドレス検出送信数\t%lu\n",		net_count_nd6[NC_ND6_DAD_OUT_PACKETS]);

#endif	/* of #if defined(SUPPORT_INET6) */

#ifdef SUPPORT_TCP

	cons_printf(portid, "\nTCP\n");
	cons_printf(portid, "\t受信オクテット数\t%lu\n",		net_count_tcp[NC_TCP_RECV_OCTETS]);
	cons_printf(portid, "\t受信データオクテット数\t%lu\n",		net_count_tcp[NC_TCP_RECV_DATA_OCTETS]);
	cons_printf(portid, "\t受信セグメント数\t%lu\n",		net_count_tcp[NC_TCP_RECV_SEGS]);
	cons_printf(portid, "\t受信データセグメント数\t%lu\n",		net_count_tcp[NC_TCP_RECV_DATA_SEGS]);
	cons_printf(portid, "\t受信、緊急セグメント数\t%lu\n",		net_count_tcp[NC_TCP_RECV_URG_SEGS]);
	cons_printf(portid, "\t受信、ヘッダ不正数\t%lu\n",		net_count_tcp[NC_TCP_RECV_BAD_HEADERS]);
	cons_printf(portid, "\t受信、チェックサム不正数\t%lu\n",	net_count_tcp[NC_TCP_RECV_BAD_CKSUMS]);
	cons_printf(portid, "\t受信、ACK 数\t%lu\n",			net_count_tcp[NC_TCP_RECV_ACKS]);
	cons_printf(portid, "\t受信、多重 ACK 数\t%lu\n",		net_count_tcp[NC_TCP_RECV_DUP_ACKS]);
	cons_printf(portid, "\t受信、RST 数\t%lu\n",			net_count_tcp[NC_TCP_RECV_RSTS]);
	cons_printf(portid, "\t受信、多重数\t%lu\n",			net_count_tcp[NC_TCP_RECV_DUP_SEGS]);
	cons_printf(portid, "\t受信、破棄数\t%lu\n",			net_count_tcp[NC_TCP_RECV_DROP_SEGS]);
	cons_printf(portid, "\t受信キュー解放数\t%lu\n",		net_count_tcp[NC_TCP_FREE_RCV_QUEUE]);
	cons_printf(portid, "\t送信データオクテット数\t%lu\n",		net_count_tcp[NC_TCP_SEND_DATA_OCTETS]);
	cons_printf(portid, "\t送信制御セグメント数\t%lu\n",		net_count_tcp[NC_TCP_SEND_CNTL_SEGS]);
	cons_printf(portid, "\t送信セグメント数\t%lu\n",		net_count_tcp[NC_TCP_SEND_SEGS]);
	cons_printf(portid, "\t再送信セグメント数\t%lu\n",		net_count_tcp[NC_TCP_SEND_REXMIT_SEGS]);
	cons_printf(portid, "\t送信データセグメント数\t%lu\n",		net_count_tcp[NC_TCP_SEND_DATA_SEGS]);
	cons_printf(portid, "\t送信、緊急セグメント数\t%lu\n",		net_count_tcp[NC_TCP_SEND_URG_SEGS]);
	cons_printf(portid, "\t送信、ACK 数\t%lu\n",			net_count_tcp[NC_TCP_SEND_ACKS]);
	cons_printf(portid, "\t送信、RST 数\t%lu\n",			net_count_tcp[NC_TCP_SEND_RSTS]);
	cons_printf(portid, "\tRTT 更新数\t%lu\n",			net_count_tcp[NC_TCP_RTT_UPDATES]);
	cons_printf(portid, "\t受動オープン数\t%lu\n",			net_count_tcp[NC_TCP_ACCEPTS]);
	cons_printf(portid, "\t能動オープン数\t%lu\n",			net_count_tcp[NC_TCP_CONNECTS]);

#endif	/* of #ifdef SUPPORT_TCP */

#ifdef SUPPORT_UDP

	net_count_struct(portid, "UDP",  &net_count_udp);

#endif	/* of #ifdef SUPPORT_UDP */

	tbl = nbuf_get_tbl();
	cons_printf(portid, "\nネットワークバッファ\n\tサイズ\t用意数\t割当要求数\t割当数\t割当てエラー数\n");
	for (ix = nbuf_get_tbl_size(); ix -- > 0; ) {
		cons_printf(portid, "\t%lu\t%lu\t%lu\t%lu\t%lu\n",
		                    tbl[ix].size,
		                    tbl[ix].prepares,
		                    tbl[ix].requests,
		                    tbl[ix].allocs,
		                    tbl[ix].errors);
		}

	cons_printf(portid, "\nネットワーク統計情報\t経過時間[ms]\t%lu\t", now);
	if (now > (1000 * 3600 * 24))
		cons_printf(portid, "%3lu日 %2lu時間 %2lu分 %2lu秒\n",
		             now / (1000 * 3600 * 24),
		            (now / (1000 * 3600)) % 24,
		            (now / (1000 * 60  )) % 60,
		            (now /  1000        ) % 60);
	else
		cons_printf(portid, "%2lu時間 %2lu分 %2lu秒\n",
		            (now / (1000 * 3600)) % 24,
		            (now / (1000 * 60  )) % 60,
		            (now /  1000        ) % 60);

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();
	}

#if defined(SUPPORT_INET4)

#ifdef IP4_CFG_FRAGMENT

/*
 *  nbuf_stat_ip4_frag_queue -- IPv4 再構成キュー内ネットワークバッファ情報
 */

static void
nbuf_stat_ip4_frag_queue (ID portid)
{
	const T_NET_BUF_ENTRY	*tbl;
	const T_NET_BUF		*nbuf, **queue;
	int_t			ix, fix, count;

	cons_printf(portid, "\nIPv4再構成キュー内\nIndex\tTime");
	tbl = nbuf_get_tbl();
	for (ix = nbuf_get_tbl_size(); ix -- > 0; )
		cons_printf(portid, "\t%d", tbl[ix].size);
	cons_printf(portid, "\n");

	queue = ip_get_frag_queue();
	for (fix = 0; fix < NUM_IP4_FRAG_QUEUE; fix ++) {
		if (queue[fix] != NULL) {
			cons_printf(portid, "%d\t%d", fix, GET_QIP4_HDR(queue[fix])->ttl);
			for (ix = nbuf_get_tbl_size(); ix -- > 0; ) {
				count = 0;
				for (nbuf = queue[fix]; nbuf != NULL; nbuf = GET_QIP4_HDR(nbuf)->next)
					if (nbuf->idix == ix)
						count ++;
				cons_printf(portid, "\t%d", count);
				}
			cons_printf(portid, "\n");
			}
		}
	}

#endif	/* of #ifdef IP4_CFG_FRAGMENT */

#endif	/* of #if defined(SUPPORT_INET4) */

#if defined(SUPPORT_INET6)

#ifdef IP6_CFG_FRAGMENT

/*
 *  nbuf_stat_ip6_frag_queue -- IPv6 再構成キュー内ネットワークバッファ情報
 */

static void
nbuf_stat_ip6_frag_queue (ID portid)
{
	const T_NET_BUF_ENTRY	*tbl;
	const T_NET_BUF		*nbuf, **queue;
	int_t			ix, fix, count;

	cons_printf(portid, "\nIPv6再構成キュー内\nIndex\tTime");
	tbl = nbuf_get_tbl();
	for (ix = nbuf_get_tbl_size(); ix -- > 0; )
		cons_printf(portid, "\t%d", tbl[ix].size);
	cons_printf(portid, "\n");

	queue = ip6_get_frag_queue();
	for (fix = 0; fix < NUM_IP6_FRAG_QUEUE; fix ++) {
		if (queue[fix] != NULL) {
			cons_printf(portid, "%d\t%d", fix, GET_QIP6_HDR(queue[fix])->ftim);
			for (ix = nbuf_get_tbl_size(); ix -- > 0; ) {
				count = 0;
				for (nbuf = queue[fix]; nbuf != NULL; nbuf = GET_QIP6_HDR(nbuf)->next_frag)
					if (nbuf->idix == ix)
						count ++;
				cons_printf(portid, "\t%d", count);
				}
			cons_printf(portid, "\n");
			}
		}
	}

#endif	/* of #ifdef IP6_CFG_FRAGMENT */

#endif	/* of #if defined(SUPPORT_INET6) */

#endif	/* of #if NET_COUNT_ENABLE */

/*
 *  nbuf_status -- ネットワークバッファ情報
 */

static void
nbuf_status (ID portid, char *line)
{
	SYSTIM		now;

#if NET_COUNT_ENABLE
	const T_NET_BUF_ENTRY	*tbl;
	int_t			ix;
#endif	/* of #if NET_COUNT_ENABLE */

#ifdef SUPPORT_TCP
	T_NET_BUF		*nbuf;
	T_TCP_CEP*		cep;
	T_TCP_Q_HDR*		tqhdr;

#ifdef TCP_CFG_SWBUF_CSAVE
	T_IP_HDR*		iph;
	T_TCP_HDR*		tcph;
#endif	/* of #ifdef TCP_CFG_SWBUF_CSAVE */
#endif	/* of #ifdef SUPPORT_TCP */

	WAI_NET_CONS_PRINTF();
	get_tim(&now);
	cons_printf(portid, "ネットワークバッファ情報\t経過時間[ms]\t%lu\n", now);

#if NET_COUNT_ENABLE

	cons_printf(portid, "\nネットワークバッファ\n\tサイズ\t用意数\t割当要求数\t割当数\t割当てエラー数\n");
	tbl = nbuf_get_tbl();
	for (ix = nbuf_get_tbl_size(); ix -- > 0; ) {
		cons_printf(portid, "\t%lu\t%lu\t%lu\t%lu\t%lu\n",
		                    tbl[ix].size,
		                    tbl[ix].prepares,
		                    tbl[ix].requests,
		                    tbl[ix].allocs,
		                    tbl[ix].errors);
		}

#endif	/* of #if NET_COUNT_ENABLE */

#ifdef SUPPORT_TCP

	cons_printf(portid, "\nCEP内\nCEPID\n");

	for (cep = tcp_cep; cep < &tcp_cep[tmax_tcp_cepid]; cep ++) {
		cons_printf(portid, "%2d", GET_TCP_CEPID(cep));
		for (nbuf = cep->reassq; nbuf != NULL; nbuf = tqhdr->next) {
			tqhdr = GET_TCP_Q_HDR(nbuf, GET_TCP_IP_Q_HDR(nbuf)->thoff);
			cons_printf(portid, "[a:%6d-%6d] ",
			            ntohl(tqhdr->seq) - cep->irs + tqhdr->soff,
			            ntohl(tqhdr->seq) - cep->irs + tqhdr->soff + tqhdr->slen);
			}

#ifdef TCP_CFG_RWBUF_CSAVE
		for (nbuf = cep->rwbufq; nbuf != NULL; nbuf = tqhdr->next) {
			tqhdr = GET_TCP_Q_HDR(nbuf, GET_TCP_IP_Q_HDR(nbuf)->thoff);
			cons_printf(portid, "[r:%6d-%6d] ",
			            ntohl(tqhdr->seq) - cep->irs + tqhdr->soff,
			            ntohl(tqhdr->seq) - cep->irs + tqhdr->soff + tqhdr->slen);
			}
#endif	/* of #ifdef TCP_CFG_RWBUF_CSAVE */

#ifdef TCP_CFG_SWBUF_CSAVE
		if (cep->swbufq != NULL) {
			iph  = GET_IP_HDR(cep->swbufq);
			tcph = GET_TCP_HDR(cep->swbufq, GET_TCP_HDR_OFFSET(cep->swbufq));
			cons_printf(portid, "[s:%6d-%6d] ",
			            ntohl(tcph->seq) - cep->iss,
			            ntohl(tcph->seq) - cep->iss + (GET_IP_SDU_SIZE(iph) - 
			                                           GET_TCP_HDR_SIZE2(cep->swbufq, GET_TCP_HDR_OFFSET(cep->swbufq))));
			}
#endif	/* of #ifdef TCP_CFG_SWBUF_CSAVE */

		cons_printf(portid, "\n");
		}

#endif	/* of #ifdef SUPPORT_TCP */

#if NET_COUNT_ENABLE

#if defined(SUPPORT_INET4)

#ifdef IP4_CFG_FRAGMENT

	nbuf_stat_ip4_frag_queue(portid);

#endif	/* of #ifdef IP4_CFG_FRAGMENT */

#endif	/* of #if defined(SUPPORT_INET4) */

#if defined(SUPPORT_INET6)

#ifdef IP6_CFG_FRAGMENT

	nbuf_stat_ip6_frag_queue(portid);

#endif	/* of #ifdef IP6_CFG_FRAGMENT */

#endif	/* of #if defined(SUPPORT_INET6) */

#endif	/* of #if NET_COUNT_ENABLE */

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();
	}

#if defined(SUPPORT_INET4)

#if NUM_ROUTE_ENTRY > 0

/*
 *  routing_table_status -- 経路表の出力
 */

static void
routing_table_status (ID portid, char *line)
{
	int_t	ix;

	if ('0' <= *(line = skip_blanks(line)) && *line <= '9') {
		T_IN4_ADDR	target, mask, gateway;

		ix = atoi(line);
		while ('0' <= *line && *line <= '9')
			line ++;
		line = GET_IPADDR(&target,  skip_blanks(line));
		line = GET_IPADDR(&mask,    skip_blanks(line));
		       GET_IPADDR(&gateway, skip_blanks(line));
		in4_add_route(ix, target, mask, gateway);
		}

	WAI_NET_CONS_PRINTF();
	cons_printf(portid,
	            "Routing Table Status\n"
	            "IX Target          Mask            Gateway         Expire\n");

	for (ix = 0; ix < NUM_ROUTE_ENTRY; ix ++) {
		if ((routing_tbl[ix].flags & IN_RTF_DEFINED) == 0)
			;
		else if ((routing_tbl[ix].flags & IN_RTF_REDIRECT) != 0)
			cons_printf(portid, "%2d %15I %15I %15I %4d\n",
			            ix,
			            &routing_tbl[ix].target,
			            &routing_tbl[ix].mask,
			            &routing_tbl[ix].gateway,
			            &routing_tbl[ix].expire);
		else
			cons_printf(portid, "%2d %15I %15I %I\n",
			            ix,
			            &routing_tbl[ix].target,
			            &routing_tbl[ix].mask,
			            &routing_tbl[ix].gateway);
		            
		}

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();
	}

#endif	/* of #if NUM_ROUTE_ENTRY > 0 */

#endif	/* of #if defined(SUPPORT_INET4) */

#if defined(SUPPORT_INET6)

#if NUM_ND6_DEF_RTR_ENTRY > 0

/*
 *  defrtrlist_status -- ディフォルトルータ・リスト状態の出力
 */

static void
defrtrlist_status (ID portid, char *line)
{
	const T_DEF_ROUTER	*dr;
	SYSTIM			now;
	int_t			rix, pix, pcnt, ptitle;
	uint_t			count, mask;

	WAI_NET_CONS_PRINTF();
	cons_printf(portid,
	            "Default Router List Status\n"
	            "IX Expire Lifetime flags prefix");
	for (ptitle = NUM_ND6_PREFIX_ENTRY * 2 - sizeof("prefix"); ptitle -- > 0; )
		cons_printf(portid, " ");
	cons_printf(portid, " IP Address\n");

	/* expire の単位は [ms]。*/
	get_tim(&now);
	dr = nd6_get_drl(&count);
	for (rix = 0; rix < count; rix ++) {
		if (dr[rix].state == ND6_DEF_RTR_BUSY) {
			cons_printf(portid, "%2d", rix);
			if (dr[rix].lifetime == ND6_INFINITE_LIFETIME)
				cons_printf(portid, "  INFIN    INFIN");
			else
				cons_printf(portid, " %6d %8d",
				            (uint32_t)(dr[rix].expire - now) <= 0
				                ? 0 : (dr[rix].expire - now) / SYSTIM_HZ,
				            dr[rix].lifetime / SYSTIM_HZ);
			cons_printf(portid, "    %02x", dr[rix].flags);
			pcnt = NUM_ND6_PREFIX_ENTRY;
			ptitle = NUM_ND6_PREFIX_ENTRY * 2 - sizeof(" prefix");
			mask = 1;
			for (pix = 0; pix < NUM_ND6_PREFIX_ENTRY; pix ++) {
				if ((dr[rix].plistmap & mask) != 0) {
					if (pcnt == NUM_ND6_PREFIX_ENTRY)
						cons_printf(portid, " ");
					else
						cons_printf(portid, ",");
					cons_printf(portid, "%d", pix);
					pcnt --;
					ptitle -= 2;
					}
				mask <<= 1;
				}
			while (pcnt --) {
				cons_printf(portid, "  ");
				ptitle -= 2;
				}
			while (ptitle -- > 0)
				cons_printf(portid, " ");
			cons_printf(portid, " %I\n",  &dr[rix].addr);
			}
		}

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();
	}

/*
 *  prefixlist_status -- プレフィックス・リスト状態の出力
 */

static void
prefixlist_status (ID portid, char *line)
{
	const T_ND6_PREFIX	*pr;
	SYSTIM			now;
	int_t			rix, pix, rcnt, rtitle;
	uint_t			mask;

	WAI_NET_CONS_PRINTF();
	cons_printf(portid,
	            "Prefix List Status\n"
	            "IX  Expire Lifetime flags router");
	for (rtitle = NUM_ND6_DEF_RTR_ENTRY * 2 - sizeof("router"); rtitle -- > 0; )
		cons_printf(portid, " ");
	cons_printf(portid, " Len Prefix\n");

	/* expire の単位は [s]。*/
	get_tim(&now);
	now /= SYSTIM_HZ;
	pr = nd6_get_prl();
	for (pix = 0; pix < NUM_ND6_PREFIX_ENTRY; pix ++) {
		if ((pr[pix].flags & ND6_PREFIX_STATE_BUSY) != 0) {
			cons_printf(portid, "%2d", pix);
			if (pr[pix].vltime == ND6_INFINITE_LIFETIME)
				cons_printf(portid, "  INFIN    INFIN");
			else
				cons_printf(portid, " %7d %8d",
				            (uint32_t)(pr[pix].expire - now) <= 0
				                ? 0 : (pr[pix].expire - now),
				             pr[pix].vltime);
			cons_printf(portid, "    %02x", pr[pix].flags);
			rcnt = NUM_ND6_DEF_RTR_ENTRY;
			rtitle = NUM_ND6_DEF_RTR_ENTRY * 2 - sizeof(" router");
			mask = 1;
			for (rix = 0; rix < NUM_ND6_DEF_RTR_ENTRY; rix ++) {
				if ((pr[pix].routermap & mask) != 0) {
					if (rcnt == NUM_ND6_DEF_RTR_ENTRY)
						cons_printf(portid, " ");
					else
						cons_printf(portid, ",");
					cons_printf(portid, "%d", rix);
					rcnt --;
					rtitle -= 2;
					}
				mask <<= 1;
				}
			while (rcnt --) {
				cons_printf(portid, "  ");
				rtitle -= 2;
				}
			while (rtitle -- > 0)
				cons_printf(portid, " ");
			cons_printf(portid, "%4d %I\n",  pr[pix].prefix_len, &pr[pix].prefix);
			}
		}

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();
	}

#endif	/* of #if NUM_ND6_DEF_RTR_ENTRY > 0 */

#if NUM_ROUTE_ENTRY > 0

/*
 *  routing_table_status -- 経路表の出力
 */

static void
routing_table_status (ID portid, char *line)
{
	SYSTIM	now;
	int_t	ix;

	WAI_NET_CONS_PRINTF();
	cons_printf(portid,
	            "Routing Table Status\n"
	            "IX Expire flags Prefix Target                         Gateway\n");

	for (ix = 0; ix < NUM_STATIC_ROUTE_ENTRY; ix ++) {
		cons_printf(portid, "%2d STATIC     - %6d %30I %I\n",
		            ix,
		             routing_tbl[ix].prefix_len,
		            &routing_tbl[ix].target,
		            &routing_tbl[ix].gateway);
		            
		}

	/* expire の単位は [s]。*/
	get_tim(&now);
	now /= SYSTIM_HZ;

	for ( ; ix < NUM_ROUTE_ENTRY; ix ++)
		if (routing_tbl[ix].flags & IN_RTF_DEFINED)
			cons_printf(portid, "%2d %6d    %02x %6d %30I %I\n",
			            ix,
			            (uint32_t)(routing_tbl[ix].expire - now) <= 0
			                ? 0 : routing_tbl[ix].expire - now,
			             routing_tbl[ix].flags,
			             routing_tbl[ix].prefix_len,
			            &routing_tbl[ix].target,
			            &routing_tbl[ix].gateway);

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();
	}

#endif	/* of #if NUM_ROUTE_ENTRY > 0 */

#endif	/* of #if defined(SUPPORT_INET6) */

#if NUM_ROUTE_ENTRY > 0

/*
 *  routing_status -- ルーティング情報
 */

static void
routing_status (ID portid, char *line)
{
	switch (*line) {

#if defined(SUPPORT_INET6)

#if NUM_ND6_DEF_RTR_ENTRY > 0

	case 'l':		/* ディフォルトルータ・リスト情報 */
		defrtrlist_status(portid, line + 1);
		break;

	case 'p':		/* プレフィックス・リスト情報 */
		prefixlist_status(portid, line + 1);
		break;

#endif	/* of #if NUM_ND6_DEF_RTR_ENTRY > 0 */

#endif	/* of #if defined(SUPPORT_INET6) */

	default:		/* ルーティング表情報 */
		routing_table_status(portid, line);
		break;

		break;
		}
	}

#endif	/* of #if NUM_ROUTE_ENTRY > 0 */

/*
 *  network_status -- ネットワークの状態の出力
 */

static void
network_status (ID portid, char *line)
{
	switch (*line) {

#ifdef SUPPORT_ETHER

	case 'a':		/* IF アドレス情報 */
		ifa_status(portid, line + 1);
		break;

#endif	/* of #ifdef SUPPORT_ETHER */

	case 'b':		/* ネットワークバッファ情報 */
		nbuf_status(portid, line + 1);
		break;

#if NET_COUNT_ENABLE

	case 'c':		/* ネットワーク統計情報 */
		net_count(portid, line + 1);
		break;

#endif	/* of #if NET_COUNT_ENABLE */

#if NUM_ROUTE_ENTRY > 0

	case 'r':		/* ルーティング情報 */
		routing_status(portid, line + 1);
		break;

#endif	/* of #if NUM_ROUTE_ENTRY > 0 */

#ifdef SUPPORT_TCP

	case 't':		/* TCP status */
		tcp_status(portid, line + 1);
		break;

#endif	 /* of #ifdef SUPPORT_TCP */

#ifdef SUPPORT_UDP

	case 'u':		/* UDP status */
		udp_status(portid, line + 1);
		break;

#endif	 /* of #ifdef SUPPORT_UDP */

	default:
		cons_printf(portid, "no such command: '%c%c'\n", *(line - 1), *line);
		SIG_NET_CONS_PRINTF();
		FLUSH_SND_BUFF();
		break;
		}
	}

#if defined(SUPPORT_INET4)

/*
 *  ifconfig -- ネットワークインタフェースの構成情報
 */

static void
ifconfig (ID portid, char *line)
{
	T_IFNET		*ifp = IF_GET_IFNET();
	T_IN4_ADDR	bc;

#ifdef SUPPORT_ETHER

	if (*(line = skip_blanks(line))) {
		T_IN4_ADDR	addr, mask;

		GET_IPADDR(&mask, skip_blanks(GET_IPADDR(&addr, line)));
		in4_add_ifaddr(addr, mask);
		}

#endif	/* of #ifdef SUPPORT_ETHER */

	WAI_NET_CONS_PRINTF();
	cons_printf(portid,
	            "Network Interface Configurations\n");

#ifdef SUPPORT_ETHER

	cons_printf(portid,
	            "ether: %M\n",
	            IF_ETHER_NIC_GET_SOFTC()->ifaddr.lladdr);

#endif	/* of #ifdef SUPPORT_ETHER */

	bc = (ifp->in_ifaddr.addr & ifp->in_ifaddr.mask) | ~ifp->in_ifaddr.mask;
	cons_printf(portid,
	            "inet:  %I, mask: %I, boadcast: %I\n",
	            &ifp->in_ifaddr.addr,
	            &ifp->in_ifaddr.mask,
	            &bc);

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();
	}

#endif	/* of #if defined(SUPPORT_INET4) */

#if defined(SUPPORT_INET6)

/*
 *  ifconfig -- ネットワークインタフェースの構成情報
 */

static void
ifconfig (ID portid, char *line)
{
#ifdef SUPPORT_ETHER

	T_IF_SOFTC	*ic;
	T_IFNET		*ifp = IF_GET_IFNET();
	int_t		ix;
	SYSTIM		now;

	WAI_NET_CONS_PRINTF();
	ic = IF_ETHER_NIC_GET_SOFTC();
	cons_printf(portid, "ether: %M\ninet6:\nIX   Expire Preffered RTR PFX Flags IP Address\n",
	                    ic->ifaddr.lladdr);

	/* expire と vltime の単位は [s]。*/
	get_tim(&now);
	now /= SYSTIM_HZ;

	for (ix = 0; ix < NUM_IN6_IFADDR_ENTRY; ix ++) {
		if (ifp->in_ifaddrs[ix].flags & IN6_IFF_DEFINED) {
			cons_printf(portid, " %d", ix);
			if (ifp->in_ifaddrs[ix].lifetime.vltime == ND6_INFINITE_LIFETIME)
				cons_printf(portid, "    INFIN     INFIN");
			else
				cons_printf(portid, " %8d %9d",
				                    (uint32_t)(ifp->in_ifaddrs[ix].lifetime.expire - now) <= 0 
				                        ? 0 : ifp->in_ifaddrs[ix].lifetime.expire - now,
				                    (uint32_t)(ifp->in_ifaddrs[ix].lifetime.preferred - now) <= 0
				                        ? 0 : ifp->in_ifaddrs[ix].lifetime.preferred - now);
			if (ifp->in_ifaddrs[ix].router_index == IN6_RTR_IX_UNREACH)
				cons_printf(portid, "   -");
			else
				cons_printf(portid, " %3d", ifp->in_ifaddrs[ix].router_index);
			if (ifp->in_ifaddrs[ix].prefix_index == ND6_PREFIX_IX_INVALID)
				cons_printf(portid, "   -");
			else
				cons_printf(portid, " %3d", ifp->in_ifaddrs[ix].prefix_index);
			cons_printf(portid, "    %02x %I\n",
			                    ifp->in_ifaddrs[ix].flags,
			                    &ifp->in_ifaddrs[ix].addr);
			}
		}

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();

#endif	/* of #ifdef SUPPORT_ETHER */
	}

#endif	/* of #if defined(SUPPORT_INET6) */

/*
 *  show_config -- コンフィギュレーション設定の表示
 */

static void
show_config (ID portid, char *line)
{
#if defined(SUPPORT_ETHER)
#if defined(SUPPORT_INET4)
	T_IN4_ADDR	addr;
#endif
#endif	/* of #if defined(SUPPORT_ETHER) */

#if defined(DHCP_CFG)

	cons_printf(portid, "DHCP:\n");

	cons_printf(portid, "  DHCP_CFG: On\n");

#endif	/* of #if define(DHCP_CFG) */

#if defined(SUPPORT_TCP)

	cons_printf(portid, "TCP:\n");

	cons_printf(portid, "  TCP_CFG_PASSIVE_OPEN: ");
#if defined(TCP_CFG_PASSIVE_OPEN)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  TCP_CFG_OPT_MSS:      ");
#if defined(TCP_CFG_OPT_MSS)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  TCP_CFG_DELAY_ACK:    ");
#if defined(TCP_CFG_DELAY_ACK)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  TCP_CFG_ALWAYS_KEEP:  ");
#if defined(TCP_CFG_ALWAYS_KEEP)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  TCP_CFG_RWBUF_CSAVE:  ");
#if defined(TCP_CFG_RWBUF_CSAVE_ONLY)
	cons_printf(portid, "Only, ");
#if defined(TCP_CFG_RWBUF_CSAVE_MAX_QUEUES)
	cons_printf(portid, "TCP_CFG_RWBUF_CSAVE_MAX_QUEUES: %2d\n",
	                     TCP_CFG_RWBUF_CSAVE_MAX_QUEUES);
#else
	cons_printf(portid, "TCP_CFG_RWBUF_CSAVE_MAX_QUEUES: Undefined\n");
#endif
#elif defined(TCP_CFG_RWBUF_CSAVE)
	cons_printf(portid, "On,   ");
#if defined(TCP_CFG_RWBUF_CSAVE_MAX_QUEUES)
	cons_printf(portid, "TCP_CFG_RWBUF_CSAVE_MAX_QUEUES: %2d\n",
	                     TCP_CFG_RWBUF_CSAVE_MAX_QUEUES);
#else
	cons_printf(portid, "TCP_CFG_RWBUF_CSAVE_MAX_QUEUES: Undefined\n");
#endif
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  TCP_CFG_SWBUF_CSAVE:  ");
#if defined(TCP_CFG_SWBUF_CSAVE_ONLY)
	cons_printf(portid, "Only, TCP_CFG_SWBUF_CSAVE_MAX_SIZE: %4d\n"
	                    "                        "
	                    "      TCP_CFG_SWBUF_CSAVE_MIN_SIZE: %4d\n",
	                    TCP_CFG_SWBUF_CSAVE_MAX_SIZE, 
                            TCP_CFG_SWBUF_CSAVE_MIN_SIZE);
#elif defined(TCP_CFG_SWBUF_CSAVE)
	cons_printf(portid, "On,   TCP_CFG_SWBUF_CSAVE_MAX_SIZE: %4d\n"
	                    "                        "
	                    "      TCP_CFG_SWBUF_CSAVE_MIN_SIZE: %4d\n",
	                    TCP_CFG_SWBUF_CSAVE_MAX_SIZE, 
                            TCP_CFG_SWBUF_CSAVE_MIN_SIZE);
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  TCP_CFG_NON_BLOCKING: ");
#if defined(TCP_CFG_NON_BLOCKING)
#if defined(USE_TCP_NON_BLOCKING)
	cons_printf(portid, "Use\n");
#else
	cons_printf(portid, "On\n");
#endif
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  TCP_CFG_EXTENTIONS:   ");
#if defined(TCP_CFG_EXTENTIONS)
#if defined(USE_TCP_EXTENTIONS)
	cons_printf(portid, "Use\n");
#if defined(SUPPORT_INET4)
	cons_printf(portid, "  NUM_VRID_TCP_REPS:    %3d\n", NUM_VRID_TCP_REPS);
	cons_printf(portid, "  NUM_VRID_TCP_CEPS:    %3d\n", NUM_VRID_TCP_CEPS);
#endif	
#if defined(SUPPORT_INET6)
	cons_printf(portid, "  NUM_VRID_TCP6_REPS:   %3d\n", NUM_VRID_TCP6_REPS);
	cons_printf(portid, "  NUM_VRID_TCP6_CEPS:   %3d\n", NUM_VRID_TCP6_CEPS);
#endif	
#else
	cons_printf(portid, "On\n");
#endif
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  USE_COPYSAVE_API:     ");
#if defined(USE_COPYSAVE_API)
	cons_printf(portid, "Use\n");
#else
	cons_printf(portid, "Off\n");
#endif

#if NUM_TCP_TW_CEP_ENTRY > 0
	cons_printf(portid, "  TCP_TW_CEP:           On, NUM_TCP_TW_CEP_ENTRY:     %d\n", NUM_TCP_TW_CEP_ENTRY);
#else
	cons_printf(portid, "  TCP_TW_CEP:           Off\n");
#endif

#if defined(TCP_CFG_TRACE)
	cons_printf(portid, "  TCP_CFG_TRACE:        On, TCP_CFG_TRACE_LPORTNO:    %d\n", TCP_CFG_TRACE_LPORTNO);
#if defined(SUPPORT_INET4)
	addr = TCP_CFG_TRACE_IPV4_RADDR;
	cons_printf(portid, "                            TCP_CFG_TRACE_IPV4_RADDR: %I\n", &addr);
#endif
	cons_printf(portid, "                            TCP_CFG_TRACE_RPORTNO:    %d\n", TCP_CFG_TRACE_RPORTNO);
#else
	cons_printf(portid, "  TCP_CFG_TRACE:        Off\n");
#endif

	cons_printf(portid, "  MAX_TCP_SND_SEG:      %4d\n", MAX_TCP_SND_SEG);
	cons_printf(portid, "  DEF_TCP_SND_SEG:      %4d\n", DEF_TCP_SND_SEG);
	cons_printf(portid, "  MAX_TCP_RCV_SEG:      %4d\n", MAX_TCP_RCV_SEG);
	cons_printf(portid, "  DEF_TCP_RCV_SEG:      %4d\n", DEF_TCP_RCV_SEG);
	cons_printf(portid, "  MAX_TCP_REALLOC_SIZE: %4d\n", MAX_TCP_REALLOC_SIZE);

#endif	/* of #if defined(SUPPORT_TCP) */

#if defined(SUPPORT_UDP)

	cons_printf(portid, "UDP:\n");

	cons_printf(portid, "  UDP_CFG_IN_CHECKSUM:  ");
#if defined(UDP_CFG_IN_CHECKSUM)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  UDP_CFG_OUT_CHECKSUM: ");
#if defined(UDP_CFG_OUT_CHECKSUM)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  UDP_CFG_NON_BLOCKING: ");
#if defined(UDP_CFG_NON_BLOCKING)
#if defined(USE_UDP_NON_BLOCKING)
	cons_printf(portid, "Use\n");
#else
	cons_printf(portid, "On\n");
#endif
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  UDP_CFG_EXTENTIONS:   ");
#if defined(UDP_CFG_EXTENTIONS)
#if defined(USE_UDP_EXTENTIONS)
	cons_printf(portid, "Use\n");
#else
	cons_printf(portid, "On\n");
#endif
	cons_printf(portid, "  NUM_VRID_UDP_CEPS:    %3d\n", NUM_VRID_UDP_CEPS);
	cons_printf(portid, "  NUM_VRID_UDP6_CEPS:   %3d\n", NUM_VRID_UDP6_CEPS);
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  NUM_DTQ_UDP_RCVQ:     %3d\n", NUM_DTQ_UDP_RCVQ);
	cons_printf(portid, "  NUM_DTQ_UDP_OUTPUT:   %3d\n", NUM_DTQ_UDP_OUTPUT);

#endif	/* of #if defined(SUPPORT_UDP) */

	cons_printf(portid, "ICMP:\n");

	cons_printf(portid, "  ICMP_CFG_CALLBACK_ECHO_REPLY: ");
#if defined(ICMP_CFG_CALLBACK_ECHO_REPLY)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  ICMP_REPLY_ERROR:             ");
#if defined(ICMP_REPLY_ERROR)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

#if NUM_REDIRECT_ROUTE_ENTRY > 0
	cons_printf(portid, "  TMO_IN_REDIRECT: %4d[s]\n", TMO_IN_REDIRECT / NET_TIMER_HZ);
#endif

#if defined(SUPPORT_INET4)

	cons_printf(portid, "IPv4:\n");

	cons_printf(portid, "  IP4_CFG_FRAGMENT: ");

#if defined(IP4_CFG_FRAGMENT)
	cons_printf(portid, "On\n");
	cons_printf(portid, "    NUM_IP4_FRAG_QUEUE:       %4d\n", NUM_IP4_FRAG_QUEUE);
	cons_printf(portid, "    IP4_CFG_FRAG_REASSM_SIZE: %4d\n", IP4_CFG_FRAG_REASSM_SIZE);
#else
	cons_printf(portid, "Off\n");
#endif	/* of #if defined(IP4_CFG_FRAGMENT) */

#if defined(SUPPORT_ETHER)
	addr = IPV4_ADDR_LOCAL;
	cons_printf(portid, "  IPV4_ADDR_LOCAL:      %I\n", &addr);
	addr = IPV4_ADDR_LOCAL_MASK;
	cons_printf(portid, "  IPV4_ADDR_LOCAL_MASK: %I\n", &addr);
	addr = IPV4_ADDR_DEFAULT_GW;
	cons_printf(portid, "  IPV4_ADDR_DEFAULT_GW: %I\n", &addr);
#endif	/* of #if defined(SUPPORT_ETHER) */

	cons_printf(portid, "  Routing Table:\n");
	cons_printf(portid, "    NUM_STATIC_ROUTE_ENTRY:   %d\n", NUM_STATIC_ROUTE_ENTRY);
	cons_printf(portid, "    NUM_REDIRECT_ROUTE_ENTRY: %d\n", NUM_REDIRECT_ROUTE_ENTRY);

#endif	/* of #if defined(SUPPORT_INET4) */

#if defined(SUPPORT_INET6)

	cons_printf(portid, "IPv6:\n");

	cons_printf(portid, "  Routing Table:\n");
	cons_printf(portid, "    NUM_STATIC_ROUTE_ENTRY:   %d\n", NUM_STATIC_ROUTE_ENTRY);
	cons_printf(portid, "    NUM_REDIRECT_ROUTE_ENTRY: %d\n", NUM_REDIRECT_ROUTE_ENTRY);

	cons_printf(portid, "ND:\n");
	cons_printf(portid, "  TMO_ND6_RTR_SOL_DELAY:    %5d[ms]\n", TMO_ND6_RTR_SOL_DELAY);
	cons_printf(portid, "  TMO_ND6_RTR_SOL_INTERVAL: %5d[ms]\n", TMO_ND6_RTR_SOL_INTERVAL);
	cons_printf(portid, "  NUM_IP6_DAD_COUNT:        %5d\n",     NUM_IP6_DAD_COUNT);
	cons_printf(portid, "  NUM_ND6_CACHE_ENTRY:      %5d\n",     NUM_ND6_CACHE_ENTRY);

#if defined(SUPPORT_ETHER)

	cons_printf(portid, "  NUM_ND6_DEF_RTR_ENTRY     %5d\n",     NUM_ND6_DEF_RTR_ENTRY);
	cons_printf(portid, "  NUM_ND6_RTR_SOL_RETRY     %5d\n",     NUM_ND6_RTR_SOL_RETRY);

	cons_printf(portid, "  IP6_CFG_AUTO_LINKLOCAL: ");
#if defined(IP6_CFG_AUTO_LINKLOCAL)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

#endif	/* of #if defined(SUPPORT_ETHER) */

#endif	/* of #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET4) && defined(SUPPORT_ETHER)

	cons_printf(portid, "ARP:\n");
	cons_printf(portid, "  ARP_CACHE_KEEP: %4d[s]\n", ARP_CACHE_KEEP / NET_TIMER_HZ);

#endif	/* of #if defined(SUPPORT_INET4) && defined(SUPPORT_ETHER) */

#if defined(SUPPORT_ETHER)

	cons_printf(portid, "Ethernet:\n");

	cons_printf(portid, "  NUM_DTQ_ETHER_OUTPUT:    %3d\n", NUM_DTQ_ETHER_OUTPUT);

	cons_printf(portid, "  ETHER_CFG_ACCEPT_ALL:    ");
#if defined(ETHER_CFG_ACCEPT_ALL)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  ETHER_CFG_UNEXP_WARNING: ");
#if defined(ETHER_CFG_UNEXP_WARNING)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  ETHER_CFG_802_WARNING:   ");
#if defined(ETHER_CFG_802_WARNING)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  ETHER_CFG_MCAST_WARNING: ");
#if defined(ETHER_CFG_MCAST_WARNING)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

#endif	/* of #if defined(SUPPORT_ETHER) */

#if defined(SUPPORT_PPP)

	cons_printf(portid, "PPP:\n");

	cons_printf(portid, "  HDLC_PORTNO: %d", HDLC_PORTID);

	cons_printf(portid, "  PPP_IDLE_TIMEOUT: ");
#if defined(PPP_IDLE_TIMEOUT)
	cons_printf(portid, "  %4d[s]\n", PPP_IDLE_TIMEOUT / NET_TIMER_HZ);
#else
	cons_printf(portid, "  Off\n");
#endif

	cons_printf(portid, "  PPP_CFG_MODEM: ");
#if defined(PPP_CFG_MODEM)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

#endif	/* of #if defined(SUPPORT_PPP) */

	cons_printf(portid, "Network Buffer:\n");

#if defined(NUM_MPF_NET_BUF_64)
	cons_printf(portid, "  NUM_MPF_NET_BUF_64:        %2d\n", NUM_MPF_NET_BUF_64);
#endif

#if defined(NUM_MPF_NET_BUF_CSEG)
	cons_printf(portid, "  NUM_MPF_NET_BUF_CSEG:      %2d\n", NUM_MPF_NET_BUF_CSEG);
#endif

#if defined(NUM_MPF_NET_BUF_128)
	cons_printf(portid, "  NUM_MPF_NET_BUF_128:       %2d\n", NUM_MPF_NET_BUF_128);
#endif

#if defined(NUM_MPF_NET_BUF_256)
	cons_printf(portid, "  NUM_MPF_NET_BUF_256:       %2d\n", NUM_MPF_NET_BUF_256);
#endif

#if defined(NUM_MPF_NET_BUF_512)
	cons_printf(portid, "  NUM_MPF_NET_BUF_512:       %2d\n", NUM_MPF_NET_BUF_512);
#endif

#if defined(NUM_MPF_NET_BUF_IP_MSS)
	cons_printf(portid, "  NUM_MPF_NET_BUF_IP_MSS:    %2d\n", NUM_MPF_NET_BUF_IP_MSS);
#endif

#if defined(NUM_MPF_NET_BUF_1024)
	cons_printf(portid, "  NUM_MPF_NET_BUF_1024:      %2d\n", NUM_MPF_NET_BUF_1024);
#endif

#if defined(NUM_MPF_NET_BUF_IPV6_MMTU)
	cons_printf(portid, "  NUM_MPF_NET_BUF_IPV6_MMTU: %2d\n", NUM_MPF_NET_BUF_IPV6_MMTU);
#endif

#if defined(NUM_MPF_NET_BUF_IF_PDU)
	cons_printf(portid, "  NUM_MPF_NET_BUF_IF_PDU:    %2d\n", NUM_MPF_NET_BUF_IF_PDU);
#endif

#if defined(NUM_MPF_NET_BUF_REASSM)
	cons_printf(portid, "  NUM_MPF_NET_BUF_REASSM:    %2d\n", NUM_MPF_NET_BUF_REASSM);
#endif

#if defined(NUM_MPF_NET_BUF6_65536)
	cons_printf(portid, "  NUM_MPF_NET_BUF6_65536:    %2d\n", NUM_MPF_NET_BUF6_65536);
#endif

	cons_printf(portid, "Network Counter Mask: %08x\n", NET_COUNT_ENABLE);

	cons_printf(portid, "SUPPORT_MIB: ");
#if defined(SUPPORT_MIB)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();
	}

/*
 *  デバッグコマンド解析
 */

static void
dbg_parse (ID portid, char *line)
{
	int_t	cepid;
	ER	error;

	switch (*line) {

	case 'c':		/* cancel CEP */
		switch (*(line + 1)) {

		case 'f':	/* show configurations */
			show_config(portid, line + 1);
			break;

#ifdef SUPPORT_TCP

		case 't':	/* cancel TCP */
			cepid = atoi(line = skip_blanks(line + 2));
			while ('0' <= *line && *line <= '9')
				line ++;
			if ((error = tcp_can_cep((ID)cepid, atoi(skip_blanks(line)))) != E_OK && error != E_RLWAI) {
				cons_printf(portid, "[TCP CAN CEP] error: %s\n", itron_strerror(error));
				SIG_NET_CONS_PRINTF();
				FLUSH_SND_BUFF();
				}
			break;

#endif	/* of #ifdef SUPPORT_TCP */

#ifdef SUPPORT_UDP

		case 'u':	/* cancel UDP */
			cepid = atoi(line = skip_blanks(line + 2));
			while ('0' <= *line && *line <= '9')
				line ++;
			if ((error = udp_can_cep((ID)cepid, atoi(skip_blanks(line)))) != E_OK && error != E_RLWAI) {
				cons_printf(portid, "[UDP CAN CEP] error: %s\n", itron_strerror(error));
				SIG_NET_CONS_PRINTF();
				FLUSH_SND_BUFF();
				}
			break;

#endif	/* of #ifdef SUPPORT_UDP */

			}

		break;

#if defined(USE_UDP_DISCARD_CLI) || defined(USE_TCP_DISCARD_CLI) || defined(SUPPORT_PPP) || defined(SUPPORT_ETHER) && defined(USE_NET_CONS)

	case 'd':	/* discard client */
		switch (*(line + 1)) {

#if defined(SUPPORT_PPP)

		case 'c':		/* disconnect */
			if (lcp_fsm.state != PPP_FSM_STOPPED)
				lcp_close();
			break;

#endif	/* of #if defined(SUPPORT_PPP) */

#if defined(SUPPORT_ETHER) && defined(USE_NET_CONS)

		case 'c':		/* disconnect */
			discon_net_cons();
			break;

#endif	/* of #if defined(SUPPORT_ETHER) && defined(USE_NET_CONS) */

#ifdef USE_TCP_DISCARD_CLI

		case 't':	/* TCP discard client */
			line += 2;
			switch (*line) {

			case 's':	/* cancel TCP discard client */
				tcp_discard_cli_valid = false;
				break;

			default:
				if ((error = psnd_dtq(DTQ_TCP_DISCARD_CLI, (intptr_t)line)) != E_OK) {
					cons_printf(portid, "[TCP DISCARD CLI] error: %s\n",
					       itron_strerror(error));
					SIG_NET_CONS_PRINTF();
					FLUSH_SND_BUFF();
					}
				break;

				}
			break;

#endif	/* of #ifdef USE_TCP_DISCARD_CLI */

#ifdef USE_UDP_DISCARD_CLI

		case 'u':	/* UDP discard client */
			line += 2;
			switch (*line) {

			case 's':	/* cancel UDP discard client */
				udp_discard_cli_valid = false;
				break;

			default:
				if ((error = psnd_dtq(DTQ_UDP_DISCARD_CLI, (intptr_t)line)) != E_OK) {
					cons_printf(portid, "[UDP DISCARD CLI] error: %s\n",
					       itron_strerror(error));
					SIG_NET_CONS_PRINTF();
					FLUSH_SND_BUFF();
					}
				break;

				}
			break;

#endif	/* of #ifdef USE_UDP_DISCARD_CLI */

			}

		break;

#endif	/* of #if defined(USE_UDP_DISCARD_CLI) || defined(USE_TCP_DISCARD_CLI) || defined(SUPPORT_PPP) || defined(SUPPORT_ETHER) && defined(USE_NET_CONS) */

#if defined(USE_UDP_ECHO_CLI) || defined(USE_TCP_ECHO_CLI)

	case 'e':	/* echo client */
		switch (*(line + 1)) {

#ifdef USE_TCP_ECHO_CLI

		case 't':	/* TCP echo client */
			line += 2;
			switch (*line) {

			case 's':	/* cancel TCP echo */
				tcp_echo_cli_valid = false;
				break;

			default:
				if ((error = psnd_dtq(DTQ_TCP_ECHO_CLI_SND, (intptr_t)line)) != E_OK) {
					cons_printf(portid, "[TCP ECHO CLI] error: %s\n", itron_strerror(error));
					SIG_NET_CONS_PRINTF();
					FLUSH_SND_BUFF();
					}
				break;

				}
			break;

#endif	/* of #ifdef USE_TCP_ECHO_CLI */

#ifdef USE_UDP_ECHO_CLI

		case 'u':	/* UDP echo client */
			line += 2;
			switch (*line) {

			case 's':	/* cancel UDP echo */
				udp_echo_cli_valid = false;
				break;

			default:
				if ((error = psnd_dtq(DTQ_UDP_ECHO_CLI, (intptr_t)line)) != E_OK) {
					cons_printf(portid, "[UDP ECHO CLI] error: %s\n", itron_strerror(error));
					SIG_NET_CONS_PRINTF();
					FLUSH_SND_BUFF();
					}
				break;

				}
			break;

#endif	/* of #ifdef USE_UDP_ECHO_CLI */

			}

		break;

#endif	/* of #if defined(USE_UDP_ECHO_CLI) || defined(USE_TCP_ECHO_CLI) */

	case 'i':

		if (*(line + 1) == 'f')
			ifconfig(portid, line + 2);

#ifdef SUPPORT_PPP

#ifdef PPP_CFG_MODEM

		else if (lcp_fsm.state == PPP_FSM_STOPPED)
			dial();		/* initiate */

#else	/* of #ifdef PPP_CFG_MODEM */

		else if (lcp_fsm.state == PPP_FSM_STOPPED)
			lcp_open(PPP_OPEN_ACTIVE);

#endif	/* of #ifdef PPP_CFG_MODEM */

#endif	/* of #ifdef SUPPORT_PPP */

		break;

	case 'n':		/* network status */
		network_status(portid, line + 1);
		break;

	case 'p':		/* ping or task status */

		if (*(line + 1) == 's')
			task_status(portid, skip_blanks(line + 2));

#ifdef USE_PING

		else {
			int_t		tmo, size;
			T_IN_ADDR	addr;

#ifdef PING_CFG_I6RLP
			static const char i6rlp_pmtu_str1[] = " FF1E::1:2 1 1452";
			static const char i6rlp_pmtu_str2[] = " FF1E::1:2 1 1352";
			static const char i6rlp_pmtu_str3[] = " fe80::0200:00ff:fe00:0100 1 2";

			if      (*(line + 1) == '1')
				strcpy(line + 1, i6rlp_pmtu_str1);
			else if (*(line + 1) == '2')
				strcpy(line + 1, i6rlp_pmtu_str2);
			else if (*(line + 1) == '3')
				strcpy(line + 1, i6rlp_pmtu_str3);

#endif	/* of #ifdef PING_CFG_I6RLP */

			line = skip_blanks(GET_IPADDR(&addr, skip_blanks(line + 1)));
			if ('0' <= *line && *line <= '9')
				line = get_int(&tmo, line);
			else
				tmo = 3;

			line = skip_blanks(line + 1);
			if ('0' <= *line && *line <= '9')
				line = get_int(&size, line);
			else
				size = 64;

			PING(&addr, (uint_t)tmo, (uint_t)size);
			}

#endif	/* of #ifdef USE_PING */

		break;

	case 'r':		/* release wait task */
		if ((error = rel_wai(atoi(skip_blanks(line + 1)))) != E_OK) {
			cons_printf(portid, "[REL WAI TSK] error: %s\n", itron_strerror(error));
			SIG_NET_CONS_PRINTF();
			FLUSH_SND_BUFF();
			}
		break;

	case 't':		/* terminate (delete) REP/CEP */
		switch (*(line + 1)) {

#ifdef USE_TCP_EXTENTIONS

		case 't':		/* TCP REP */
			if ((error = tcp_del_rep(atoi(skip_blanks(line + 2)))) != E_OK) {
				cons_printf(portid, "[TCP DEL REP] error: %s\n", itron_strerror(error));
				SIG_NET_CONS_PRINTF();
				FLUSH_SND_BUFF();
				}
			break;

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

#ifdef USE_UDP_EXTENTIONS

		case 'u':		/* UDP CEP */
			if ((error = udp_del_cep(atoi(skip_blanks(line + 2)))) != E_OK) {
				cons_printf(portid, "[UDP DEL CEP] error: %s\n", itron_strerror(error));
				SIG_NET_CONS_PRINTF();
				FLUSH_SND_BUFF();
				}
			break;

#endif	/* of #ifdef USE_UDP_EXTENTIONS */

			}
		break;

#if defined(USE_TCP_EXTENTIONS) || defined(USE_UDP_EXTENTIONS)

	case 'w':		/* wakeup server */
				/* wake up task */
		switch (*(line + 1)) {

#ifdef USE_TCP_EXTENTIONS

		case 't':		/* tcp server */
			switch (*(line + 2)) {

#ifdef USE_TCP_DISCARD_SRV
			case 'd':		/* tcp discard server */
				if ((error = wup_tsk(TCP_DISCARD_SRV_TASK)) != E_OK) {
					cons_printf(portid, "[WUP TCP DISCARD SRV] error: %s\n", itron_strerror(error));
					SIG_NET_CONS_PRINTF();
					FLUSH_SND_BUFF();
					}
				break;
#endif	/* of #ifdef USE_TCP_DISCARD_SRV */

#if defined(USE_TCP_ECHO_SRV1) || defined(USE_TCP_ECHO_SRV2)
			case 'e':		/* tcp echo server */
				if ((error = wup_tsk(TCP_ECHO_SRV_TASK)) != E_OK) {
					cons_printf(portid, "[WUP UDP DISCARD SRV] error: %s\n", itron_strerror(error));
					SIG_NET_CONS_PRINTF();
					FLUSH_SND_BUFF();
					}
				break;
#endif	/* of #if defined(USE_TCP_ECHO_SRV1) || defined(USE_TCP_ECHO_SRV2) */

#ifdef USE_WWW_SRV
			case 'w':		/* WWW server */
				if ((error = wup_tsk(WWW_SRV_TASK1)) != E_OK) {
					cons_printf(portid, "[WUP WWW SRV] error: %s\n", itron_strerror(error));
					SIG_NET_CONS_PRINTF();
					FLUSH_SND_BUFF();
					}
				break;
#endif	/* of #ifdef USE_TCP_DISCARD_SRV */

				}
			break;

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

#ifdef USE_UDP_EXTENTIONS

		case 'u':		/* udp server */
			switch (*(line + 2)) {

#if defined(USE_UDP_ECHO_SRV) && !defined(USE_UDP_CALL_BACK)
			case 'e':		/* udp echo server */
				if ((error = wup_tsk(UDP_ECHO_SRV_TASK)) != E_OK) {
					cons_printf(portid, "[WUP UDP ECHO SRV] error: %s\n", itron_strerror(error));
					SIG_NET_CONS_PRINTF();
					FLUSH_SND_BUFF();
					}
				break;
#endif	/* of #if defined(USE_UDP_ECHO_SRV) && !defined(USE_UDP_CALL_BACK) */

				}
			break;

#endif	/* of #ifdef USE_UDP_EXTENTIONS */

		default:
			if ((error = wup_tsk(atoi(skip_blanks(line + 1)))) != E_OK) {
				cons_printf(portid, "[WUP TSK] error: %s\n", itron_strerror(error));
				SIG_NET_CONS_PRINTF();
				FLUSH_SND_BUFF();
				}
			break;

			}
		break;

#else	/* of #if defined(USE_TCP_EXTENTIONS) || defined(USE_UDP_EXTENTIONS) */

	case 'w':		/* wake up task */
		if ((error = wup_tsk(atoi(skip_blanks(line + 1)))) != E_OK) {
			cons_printf(portid, "[WUP TSK] error: %s\n", itron_strerror(error));
			SIG_NET_CONS_PRINTF();
			FLUSH_SND_BUFF();
			}
		break;

#endif	/* of #if defined(USE_TCP_EXTENTIONS) || defined(USE_UDP_EXTENTIONS) */

	default:

		if (*line) {

#ifdef USE_EXTRA_PARSE

			extra_parse(line);

#else	/* of #ifdef USE_EXTRA_PARSE */

			cons_printf(portid, "no such command: '%c'\n", *line);
			SIG_NET_CONS_PRINTF();
			FLUSH_SND_BUFF();

#endif	/* of #ifdef USE_EXTRA_PARSE */

			}
		break;
		}
	}

/*
 *  デバッグコンソールタスク
 */

void
dbg_con_task (intptr_t exinf)
{
	static char line[DBG_LINE_SIZE + 1];

	uint_t	len;
	ID	tskid;

#ifdef USE_LCD
#ifdef SUPPORT_ETHER

#if defined(SUPPORT_INET6)

	int_t		lllen;
	const uint8_t	*lladdr;

#endif	/* of #if defined(SUPPORT_INET6) */

	lcd_init();

	if ((TINET_PRVER & UINT_C(0x0f)) > 0)
		lcd_printf("TINET-%d.%d.%d\n",
		       (TINET_PRVER   >> 12) & UINT_C(0x0f),
		       (TINET_PRVER   >>  4) & UINT_C(0x0f),
		        TINET_PRVER          & UINT_C(0x0f));
	else
		lcd_printf("TINET-%d.%d\n",
		       (TINET_PRVER   >> 12) & UINT_C(0x0f),
		       (TINET_PRVER   >>  4) & UINT_C(0x0f));

#if defined(SUPPORT_INET6)

	lladdr = IF_ETHER_NIC_GET_SOFTC()->ifaddr.lladdr;
	for (lllen = ETHER_ADDR_LEN; lllen --; )
		lcd_printf("%02x", *lladdr ++);
	lcd_printf("\n");

#else	/* of #if defined(SUPPORT_INET6) */

	lcd_puts(ip2str(NULL, in4_get_ifaddr(0)));

#endif	/* of #if defined(SUPPORT_INET6) */

#endif	/* of #ifdef SUPPORT_ETHER */
#else	/* of #ifdef USE_LCD */

	get_tid(&tskid);
	cons_printf(CONSOLE_PORTID, "[CONSOLE:%d] started.\n", tskid);

#endif	/* of #ifdef USE_LCD */

	serial_ctl_por(CONSOLE_PORTID,
	               IOCTL_ECHO  | IOCTL_CRLF  |
	               IOCTL_FCSND | IOCTL_FCANY | IOCTL_FCRCV);
	while (true) {
		len = cons_getline(CONSOLE_PORTID, line, DBG_LINE_SIZE);
		dbg_parse(CONSOLE_PORTID, line);
		}
	}

#endif	/* of #ifdef USE_DBG_CONS */
