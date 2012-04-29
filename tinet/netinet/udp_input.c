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
 *  @(#) $Id: udp_input.c,v 1.5 2009/12/24 05:47:21 abe Exp abe $
 */

/*
 * Copyright (c) 1982, 1986, 1988, 1990, 1993, 1995
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
 */

#include <string.h>

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
#include <t_syslog.h>

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <s_services.h>
#include <t_services.h>
#include "kernel_id.h"

#endif	/* of #ifdef TARGET_KERNEL_JSP */

#include <tinet_defs.h>
#include <tinet_config.h>

#include <net/if.h>
#include <net/if_loop.h>
#include <net/if_ppp.h>
#include <net/ethernet.h>
#include <net/ppp_ipcp.h>
#include <net/net.h>
#include <net/net_buf.h>
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
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>

#include <netinet/udp.h>
#include <netinet/udp_var.h>

#include <net/if_var.h>

#ifdef SUPPORT_UDP

#ifdef SUPPORT_MIB

/*
 *  SNMP の 管理情報ベース (MIB)
 */

T_UDP_STATS udp_stats;

#endif	/* of #ifdef SUPPORT_MIB */

#if defined(SUPPORT_INET4)

#ifdef DHCP_CFG

/*
 *  udp_dstaddr_accept -- UDP のあて先アドレスが正しいかチェックする。
 *
 *    DHCP_CFG が定義されているときは、以下のデータグラムを受信する。
 *
 *      ・あて先アドレスがブロードキャスト
 *      ・ローカルアドレスが未定義
 */

static bool_t
udp_is_dstaddr_accept (T_IN4_ADDR *myaddr, T_IN4_ADDR *dstaddr)
{
	T_IFNET	*ifp = IF_GET_IFNET();

	if (ifp->in_ifaddr.addr == IPV4_ADDRANY ||
	    ntohl(*dstaddr) == IPV4_ADDR_BROADCAST ||
	    ntohl(*dstaddr) == ((ifp->in_ifaddr.addr & ifp->in_ifaddr.mask) | ~ifp->in_ifaddr.mask))
		return true;
	else
		return IN_IS_DSTADDR_ACCEPT(myaddr, dstaddr);
	}

#else	/* of #ifdef DHCP_CFG */

/*
 *  udp_dstaddr_accept -- UDP のあて先アドレスが正しいかチェックする。
 *
 *    以下の場合もデータグラムを受信する。
 *
 *      ・あて先アドレスがブロードキャスト
 */

static bool_t
udp_is_dstaddr_accept (T_IN4_ADDR *myaddr, T_IN4_ADDR *dstaddr)
{
	T_IFNET	*ifp = IF_GET_IFNET();

	if (ntohl(*dstaddr) == IPV4_ADDR_BROADCAST ||
	    ntohl(*dstaddr) == ((ifp->in_ifaddr.addr & ifp->in_ifaddr.mask) | ~ifp->in_ifaddr.mask))
		return true;
	else
		return IN_IS_DSTADDR_ACCEPT(myaddr, dstaddr);
	}

#endif	/* of #ifdef DHCP_CFG */

#endif	/* of #if defined(SUPPORT_INET4) */

#if defined(SUPPORT_INET6)

/*
 *  udp_dstaddr_accept -- UDP のあて先アドレスが正しいかチェックする。
 *
 *    以下の場合もデータグラムを受信する。
 *
 *      ・あて先アドレスがマルチキャスト
 */

static bool_t
udp_is_dstaddr_accept (T_IN6_ADDR *myaddr, T_IN6_ADDR *dstaddr)
{
	if (IN6_IS_ADDR_MULTICAST(dstaddr))
		return true;
	else
		return IN_IS_DSTADDR_ACCEPT(myaddr, dstaddr);
	}

#endif	/* of #if defined(SUPPORT_INET6) */

/*
 *  udp_input -- UDP の入力関数
 */

uint_t
udp_input (T_NET_BUF **inputp, uint_t *offp, uint_t *nextp)
{
	T_NET_BUF	*input = *inputp;
	T_UDP_CEP	*cep;
	T_UDP_HDR	*udph;
	T_IP_HDR	*iph;
	int_t		ix;
	uint_t		len, hlen, ulen;

	hlen = (uint_t)GET_IF_IP_HDR_SIZE(input);

	NET_COUNT_UDP(net_count_udp.in_octets,  input->len - hlen);
	NET_COUNT_UDP(net_count_udp.in_packets, 1);
	NET_COUNT_MIB(udp_stats.udpInDatagrams, 1);

	/* ヘッダ長をチェックする。*/
	if (input->len < hlen + UDP_HDR_SIZE)
		goto buf_rel;

	udph = (T_UDP_HDR *)(input->buf + *offp);

	/* データグラム長をチェックする */
	ulen  = ntohs(udph->ulen);
	if (ulen != input->len - hlen)
		goto buf_rel;

	/* 宛先ポートが 0 のデータグラムは破棄する。RFC768 */
	if (udph->dport == 0)
		goto buf_rel;

#ifdef UDP_CFG_IN_CHECKSUM

	/* チェックサムをチェックする */
	if (udph->sum && IN_CKSUM(input, IPPROTO_UDP, *offp, ulen) != 0)
		goto buf_rel;

#endif	/* of #ifdef UDP_CFG_IN_CHECKSUM */

	iph = GET_IP_HDR(input);

	/* 宛先アドレスとポートをチェックする */
	for (ix = tmax_udp_ccepid; ix -- > 0; ) {
		cep = &udp_cep[ix];
		if (VALID_UDP_CEP(cep) &&
		    udp_is_dstaddr_accept(&cep->myaddr.ipaddr, &iph->dst) &&
		    ntohs(udph->dport) == cep->myaddr.portno) {
			len = (uint_t)(ntohs(udph->ulen) - UDP_HDR_SIZE);

			if (cep->rcv_tskid != TA_NULL) {	/* 非ノンブロッキングコールでペンディング中 */
				if (psnd_dtq(cep->rcvqid, (intptr_t)input) != E_OK)
					goto buf_rel;
				}

#ifdef UDP_CFG_NON_BLOCKING

			else if (cep->rcv_p_dstaddr != NULL) {	/* ノンブロッキングコールでペンディング中 */

				/* p_dstaddr を設定する。*/
				cep->rcv_p_dstaddr->portno = ntohs(udph->sport);
				IN_COPY_TO_HOST(&cep->rcv_p_dstaddr->ipaddr, &GET_IP_HDR(input)->src);

				/* データをバッファに移す。*/
				memcpy(cep->rcv_data, GET_UDP_SDU(input, *offp),
				       (size_t)(len < cep->rcv_len ? len : cep->rcv_len));
				syscall(rel_net_buf(input));

				if (IS_PTR_DEFINED(cep->callback))

#ifdef TCP_CFG_NON_BLOCKING_COMPAT14

					(*cep->callback)(GET_UDP_CEPID(cep), TFN_UDP_RCV_DAT, (void*)(uint32_t)len);

#else	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

					(*cep->callback)(GET_UDP_CEPID(cep), TFN_UDP_RCV_DAT, (void*)&len);

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

				else
					syslog(LOG_WARNING, "[UDP] no call back, CEP: %d.", GET_UDP_CEPID(cep));
				cep->rcv_p_dstaddr = NULL;
				}

#endif	/* of #ifdef UDP_CFG_NON_BLOCKING */

			else if (IS_PTR_DEFINED(cep->callback)) {

				/* コールバック関数を呼び出す。*/
				cep->cb_netbuf = input;

#ifdef TCP_CFG_NON_BLOCKING_COMPAT14

				(*cep->callback)(GET_UDP_CEPID(cep), TEV_UDP_RCV_DAT, (void*)(uint32_t)len);

#else	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

				(*cep->callback)(GET_UDP_CEPID(cep), TEV_UDP_RCV_DAT, (void*)&len);

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

				/*
				 *  ネットワークバッファがそのままであれば、コールバック関数内で
				 *  データを読み出さなかったことになるので、捨てる。
				 */
				if (cep->cb_netbuf != NULL)
					syscall(rel_net_buf(cep->cb_netbuf));
				}
			else
				goto buf_rel;

			return IPPROTO_DONE;
			}
		}

	NET_COUNT_MIB(udp_stats.udpNoPorts, 1);

#if defined(SUPPORT_INET4)

	/* ローカル IP アドレスに届いたデータグラムのみ ICMP エラーを通知する。*/
	if (ntohl(iph->dst) == IF_GET_IFNET()->in_ifaddr.addr) {
		syslog(LOG_INFO, "[UDP] unexp port: %d.", ntohs(udph->dport));

		icmp_error(ICMP4_UNREACH_PORT, input);
		/* icmp_error では、ネットワークバッファ input を返却しないので下へ抜ける。*/
		}

#endif	/* of #if defined(SUPPORT_INET4) */

#if defined(SUPPORT_INET6)

	/* マルチキャストアドレスに届いたデータグラムは ICMP エラーを通知しない。*/
	if (!IN6_IS_ADDR_MULTICAST(&iph->dst)) {
		syslog(LOG_INFO, "[UDP] unexp port: %d.", ntohs(udph->dport));

		/* icmp6_error で、ネットワークバッファ input を返却する。*/
		icmp6_error(input, ICMP6_DST_UNREACH, ICMP6_DST_UNREACH_NOPORT, 0);

		NET_COUNT_UDP(net_count_udp.in_err_packets, 1);
		NET_COUNT_MIB(udp_stats.udpInErrors, 1);
		return IPPROTO_DONE;
		}

#endif	/* of #if defined(SUPPORT_INET6) */

buf_rel:
	NET_COUNT_UDP(net_count_udp.in_err_packets, 1);
	NET_COUNT_MIB(udp_stats.udpInErrors, 1);
	syscall(rel_net_buf(input));
	return IPPROTO_DONE;
	}

#endif	/* of #ifdef SUPPORT_UDP */
