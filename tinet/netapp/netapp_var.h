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
 *  @(#) $Id: netapp_var.h,v 1.5 2009/12/24 05:44:56 abe Exp abe $
 */

#ifndef _NETAPP_VAR_H_
#define _NETAPP_VAR_H_

/*
 *  マクロ定義
 */

#if defined(SUPPORT_INET4)
#define T_IPEP		T_IPV4EP
#endif

#if defined(SUPPORT_INET6)
#define T_IPEP		T_IPV6EP
#endif

/*
 *  ネットワーク経由コンソール入出力
 */

#ifdef USE_NET_CONS

/* リネーム */

#define syslog		net_syslog
#define serial_ctl_por	net_serial_ctl_por

/* 関数シミュレーションマクロ */

#define FLUSH_SND_BUFF()	flush_snd_buff()
#define WAI_NET_CONS_PRINTF()	syscall(wai_sem(SEM_NET_CONS_PRINTF))
#define SIG_NET_CONS_PRINTF()	sig_sem(SEM_NET_CONS_PRINTF)

#else	/* of #ifdef USE_NET_CONS */

/* 関数シミュレーションマクロ */

#define FLUSH_SND_BUFF()
#define WAI_NET_CONS_PRINTF()
#define SIG_NET_CONS_PRINTF()

#endif	/* of #ifdef USE_NET_CONS */

/*
 *  関数
 */

extern const char *in_strtfn (FN fncd);
extern const char *itron_strerror (ER ercd);
extern ER	net_syslog(uint_t prio, const char *format, ...) throw();
extern ER	net_serial_ctl_por(ID portid, uint_t ioctl) throw();
extern void	flush_snd_buff (void);
extern ER	discon_net_cons (void);
extern void	wup_net_cons (void);
extern ER	alloc_tcp_rep (ID *repid, ID tskid, T_TCP_CREP *crep);
extern ER	free_tcp_rep (ID repid, bool_t call_tcp_del_rep);
extern ER	alloc_tcp_cep (ID *cepid, ID tskid, T_TCP_CCEP *ccep);
extern ER	free_tcp_cep (ID cepid);
extern ER	alloc_udp_cep (ID *cepid, ID tskid, T_UDP_CCEP *ccep);
extern ER	free_udp_cep (ID cepid, bool_t call_udp_del_cep);

#if defined(SUPPORT_INET4)

#define TCP_CRE_REP(i,r)	tcp_cre_rep(i,r)
#define TCP_CRE_CEP(i,c)	tcp_cre_cep(i,c)
#define UDP_CRE_CEP(i,c)	udp_cre_cep(i,c)
#define TCP_ACP_CEP(c,r,d,t)	tcp_acp_cep(c,r,d,t)
#define TCP_CON_CEP(c,m,d,t)	tcp_con_cep(c,m,d,t)

#define UDP_SND_DAT(c,d,b,l,t)	udp_snd_dat(c,d,b,l,t)
#define UDP_RCV_DAT(c,d,b,l,t)	udp_rcv_dat(c,d,b,l,t)

#define IP2STR(s,a)		ip2str(s,a)

#define GET_IPADDR(a,l)		get_ipv4addr(a,l)
#define PING(a,t,l)		ping4(a,t,l)

extern void ping4 (T_IN4_ADDR *addr, uint_t tmo, uint_t len);
extern char *get_ipv4addr (T_IN4_ADDR *addr, char *line);

#endif	/* of #if defined(SUPPORT_INET4) */

#if defined(SUPPORT_INET6)

#define TCP_CRE_REP(i,r)	tcp6_cre_rep(i,r)
#define TCP_CRE_CEP(i,c)	tcp6_cre_cep(i,c)
#define UDP_CRE_CEP(i,c)	udp6_cre_cep(i,c)
#define TCP_ACP_CEP(c,r,d,t)	tcp6_acp_cep(c,r,d,t)
#define TCP_CON_CEP(c,m,d,t)	tcp6_con_cep(c,m,d,t)

#define UDP_SND_DAT(c,d,b,l,t)	udp6_snd_dat(c,d,b,l,t)
#define UDP_RCV_DAT(c,d,b,l,t)	udp6_rcv_dat(c,d,b,l,t)

#define IP2STR(s,a)		ipv62str(s,a)

#define GET_IPADDR(a,l)		get_ipv6addr(a,l)
#define PING(a,t,l)		ping6(a,t,l)

extern void ping6 (T_IN6_ADDR *addr, uint_t tmo, uint_t len);
extern char *get_ipv6addr (T_IN6_ADDR *addr, char *line);

#endif	/* of #if defined(SUPPORT_INET6) */

#endif	/* of #ifndef _NETAPP_VAR_H_ */
