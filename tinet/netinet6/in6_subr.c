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
 *  @(#) $Id: in6_subr.c,v 1.5 2009/12/24 05:48:16 abe Exp abe $
 */

/*
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Copyright (c) 1985, 1986, 1993
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
#include "kernel_cfg.h"

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <s_services.h>
#include <t_services.h>
#include "kernel_id.h"
#include "tinet_id.h"

#endif	/* of #ifdef TARGET_KERNEL_JSP */

#include <tinet_defs.h>
#include <tinet_config.h>

#include <net/if.h>
#include <net/if_loop.h>
#include <net/if_ppp.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <net/net.h>
#include <net/net_var.h>
#include <net/net_buf.h>
#include <net/net_timer.h>

#include <netinet/in.h>
#include <netinet/in_var.h>

#include <netinet6/in6.h>
#include <netinet6/in6_var.h>
#include <netinet/ip6.h>
#include <netinet6/ip6_var.h>
#include <netinet6/ah.h>
#include <netinet6/nd6.h>

#include <net/if6_var.h>

#ifdef SUPPORT_INET6

#if NUM_IN6_HOSTCACHE_ENTRY > 0

/*
 *  ホストキャッシュ
 */

static T_IN6_HOSTCACHE_ENTRY in6_hostcache[NUM_IN6_HOSTCACHE_ENTRY];

#endif	/* of #if NUM_IN6_HOSTCACHE_ENTRY > 0 */

/*
 *  in6_lookup_ifaddr -- ネットワークインタフェースに割り当てられているアドレスを探索する。
 */

T_IN6_IFADDR *
in6_lookup_ifaddr (T_IFNET *ifp, T_IN6_ADDR *addr)
{
	int_t ix;

	for (ix = NUM_IN6_IFADDR_ENTRY; ix -- > 0; ) {
		if ((ifp->in_ifaddrs[ix].flags & IN6_IFF_DEFINED) &&
		    IN6_ARE_ADDR_EQUAL(addr, &ifp->in_ifaddrs[ix].addr))
			return &ifp->in_ifaddrs[ix];
		}
	return NULL;
	}

/*
 *  in6_lookup_multi -- ネットワークインタフェースのマルチキャストアドレスを検索する。
 */

bool_t
in6_lookup_multi (T_IFNET *ifp, T_IN6_ADDR *maddr)
{
	int_t ix;

	for (ix = MAX_IN6_MADDR_CNT; ix -- > 0; )
		if (IN6_ARE_ADDR_EQUAL(maddr, &ifp->in_maddrs[ix]))
			return true;
	return false;
	}

/*
 *  in6_set_header -- IPv6 ヘッダを設定する。
 */

ER
in6_set_header (T_NET_BUF *nbuf, uint_t len,
                T_IN6_ADDR *dstaddr, T_IN6_ADDR *srcaddr,
                uint8_t next, uint8_t hlim)
{
	T_IFNET		*ifp = IF_GET_IFNET();
	T_IP6_HDR	*ip6h;
	T_IN6_IFADDR	*ia;

	/*
	 *  宛先アドレスにふさわしい送信元アドレスを、
	 *  ネットワークインタフェースから探索して利用する。
	 */
	if (srcaddr == NULL || !IN6_IS_ADDR_UNSPECIFIED(srcaddr))
		;
	else if ((ia = in6_ifawithifp(ifp, dstaddr)) == NULL)
		return E_SYS;
	else
		srcaddr = &ia->addr;

	/* IPv6 ヘッダを設定する。*/
	ip6h		= GET_IP6_HDR(nbuf);
	ip6h->vcf	= htonl(IP6_MAKE_VCF(IPV6_VERSION, 0));
	ip6h->plen	= htons(len);
	ip6h->next	= next;
	ip6h->hlim	= hlim;

	if (dstaddr == NULL)
		memset(&ip6h->dst, 0, sizeof(T_IN6_ADDR));
	else
		ip6h->dst = *dstaddr;

	if (srcaddr == NULL)
		memset(&ip6h->src, 0, sizeof(T_IN6_ADDR));
	else
		ip6h->src = *srcaddr;

	return E_OK;
	}

/*
 *  in6_get_datagram -- IPv6 データグラムを獲得し、ヘッダを設定する。
 */

ER
in6_get_datagram (T_NET_BUF **nbuf, uint_t len, uint_t maxlen,
                  T_IN6_ADDR *dstaddr, T_IN6_ADDR *srcaddr,
                  uint8_t next, uint8_t hlim, ATR nbatr, TMO tmout)
{
	ER		error;
	uint_t		align;

	/* データ長は 4 オクテット境界に調整する。	*/
	align = (len + 3) >> 2 << 2;

	/* ネットワークバッファを獲得する。	*/
	if ((error = tget_net_buf_ex(nbuf, IF_IP6_HDR_SIZE + align,
	                                   IF_IP6_HDR_SIZE + maxlen, nbatr, tmout)) != E_OK)
		return error;

	/*
	 *  より大きなサイズのネットワークバッファを獲得する場合のみ長さを調整する。
	 *  より小さなサイズのネットワークバッファの獲得は、送信ウィンドバッファの
	 *  省コピー機能で使用され、実際に送信するまで、データサイズは決定できない。
	 */
	if ((nbatr & NBA_SEARCH_ASCENT) != 0)
		(*nbuf)->len = IF_IP6_HDR_SIZE + len;

	/* IPv6 ヘッダを設定する。*/
	if ((error = in6_set_header(*nbuf, len, dstaddr, srcaddr, next, hlim)) != E_OK)
		return error;

	/* 4 オクテット境界までパディングで埋める。*/
	if (align > len)
		memset(GET_IP6_SDU(*nbuf) + len, 0, align - len);

	return E_OK;
	}

/*
 *  in6_get_maxnum_ifaddr -- インタフェースに設定可能な最大アドレス数を返す。
 */

ER_UINT
in6_get_maxnum_ifaddr (void)
{
	return NUM_IN6_IFADDR_ENTRY;
	}

/*
 *  in6_get_ifaddr -- インタフェースに設定されているアドレスを返す。
 */

const T_IN6_ADDR *
in6_get_ifaddr (int_t index)
{
	T_IFNET		*ifp = IF_GET_IFNET();

	if (index < NUM_IN6_IFADDR_ENTRY &&
	    (ifp->in_ifaddrs[index].flags & IN6_IFF_DEFINED))
		return &ifp->in_ifaddrs[index].addr;
	else
		return NULL;
	}

/*
 *  ipv62str -- IPv6 アドレスを文字列に変換する。
 */

char *
ipv62str (char *buf, const T_IN6_ADDR *p_ip6addr)
{
	static char	addr_sbuf[NUM_IPV6ADDR_STR_BUFF][sizeof("0123:4567:89ab:cdef:0123:4567:89ab:cdef")];
	static int_t	bix = NUM_IPV6ADDR_STR_BUFF;

	bool_t	omit = false, zero = false;
	char	*start;
	int_t	ix;

	if (buf == NULL) {
		syscall(wai_sem(SEM_IP2STR_BUFF_LOCK));
		buf = addr_sbuf[-- bix];
		if (bix <= 0)
			bix = NUM_IPV6ADDR_STR_BUFF;
		syscall(sig_sem(SEM_IP2STR_BUFF_LOCK));
		}

	start = buf;
	if (p_ip6addr == NULL) {
		*buf ++ = ':';
		*buf ++ = ':';
		}
	else {
		for (ix = 0; ix < sizeof(T_IN6_ADDR) / 2; ix ++) {
			if (omit) {
				buf += convert_hexdigit(buf, ntohs(p_ip6addr->s6_addr16[ix]), 16, 0, ' '); 
				if (ix < 7)
					*buf ++ = ':';
				}
			else if (ix > 0 && ix < 7 && p_ip6addr->s6_addr16[ix] == 0)
				zero = true;
			else {
				if (zero) {
					omit = true;
					*buf ++ = ':';
					}
				buf += convert_hexdigit(buf, ntohs(p_ip6addr->s6_addr16[ix]), 16, 0, ' '); 
				if (ix < 7)
					*buf ++ = ':';
				}
			}
		}
	*buf = '\0';
	return start;
	}

/*
 *  in6_cksum -- IPv6 のトランスポート層ヘッダのチェックサムを計算する。
 *
 *  注意: 戻り値はネットワークバイトオーダ
 */

uint16_t
in6_cksum (T_NET_BUF *nbuf, uint8_t proto, uint_t off, uint_t len)
{
	uint32_t	sum;
	uint_t		align;

	/* 4 オクテット境界のデータ長 */
	align = (len + 3) >> 2 << 2;

	/* 4 オクテット境界までパディングで埋める。*/
	if (align > len)
		memset((uint8_t*)nbuf->buf + off + len, 0, align - len);

	sum = in_cksum_sum(nbuf->buf + off, align)
	    + in_cksum_sum(&GET_IP6_HDR(nbuf)->src, sizeof(T_IN6_ADDR) * 2)
	    + len + proto;
	sum = in_cksum_carry(sum);

	return ~htons((uint16_t)sum);
	}

/*
 *  in6_is_dstaddr_accept -- 宛先アドレスとして正しいかチェックする。
 */

bool_t
in6_is_dstaddr_accept (T_IN6_ADDR *myaddr, T_IN6_ADDR *dstaddr)
{
	if (IN6_IS_ADDR_UNSPECIFIED(myaddr))
		return in6_lookup_ifaddr(IF_GET_IFNET(), dstaddr) != NULL;
	else
		return IN6_ARE_ADDR_EQUAL(dstaddr, myaddr);
	}

/*
 *  get_ip6_hdr_size -- 拡張ヘッダも含めた IPv6 ヘッダ長を返す。
 */

uint_t
get_ip6_hdr_size (T_IP6_HDR *iph)
{
	uint_t	size = IP6_HDR_SIZE, hsize;
	uint8_t	curr = iph->next, next;
	uint8_t	*hdr = ((uint8_t *)iph) + IP6_HDR_SIZE;

	while (1) {
		next = *hdr;
		if (curr ==IPPROTO_NONE)		/* 次ヘッダ無し			*/
			break;
		else if (curr == IPPROTO_FRAGMENT)	/* 断片化			*/
			hsize = sizeof(T_IP6_FRAG_HDR);
		else if (curr == IPPROTO_AH)		/* IPv6 認証			*/
			hsize = (((T_IP6_AH_HDR *)hdr)->len + 2) * 4;
		else if (curr == IPPROTO_HOPOPTS ||	/* 中継点 (Hop-by-Hop) オプション	*/
		         curr == IPPROTO_DSTOPTS ||	/* IPv6 終点オプション		*/
		         curr == IPPROTO_ROUTING)	/* 経路制御			*/
			hsize = (((T_IP6_EXT_HDR *)hdr)->len + 1) * 8;
		else
			break;
		size += hsize;
		hdr  += hsize;
		curr = next;
		}
	return size;
	}

/*
 *  in6_plen2pmask -- プレフィックス長からマスクを生成する。
 */

void
in6_plen2pmask (T_IN6_ADDR *mask, uint_t prefix_len)
{
	uint8_t	*ptr, bit;

	memset(mask->s6_addr + prefix_len / 8, 0, (128 - prefix_len + 7) / 8);
	for (ptr = mask->s6_addr; prefix_len > 0; ptr ++) {
		for (bit = 0x80; bit && (prefix_len > 0); prefix_len --) {
			*ptr |= bit;
			bit >>= 1;
			}
		}
	}

/*
 *  in6_rtalloc -- ルーティング表を探索する。
 */

T_IN6_ADDR *
in6_rtalloc (T_IFNET *ifp, T_IN6_ADDR *dst)
{
	if (IN6_IS_ADDR_LINKLOCAL(dst) || IN6_IS_ADDR_MULTICAST(dst))
		return dst;
	else {
		T_IN6_IFADDR	*ia;
		SYSTIM		now;
		int_t		ix;

		/*
		 *  サイトローカルアドレスか集約可能（グローバル）アドレスの場合は、
		 *  同一リンク内のノードであるかをチェックする。
		 */

		syscall(get_tim(&now));
		now /= SYSTIM_HZ;

		/* 推奨有効時間内のアドレスを探索する。*/
		for (ix = NUM_IN6_IFADDR_ENTRY; ix -- > 0; ) {
			ia = &ifp->in_ifaddrs[ix];
			if (IFA6_IS_READY(ia) &&
			    in6_are_prefix_equal(dst, &ia->addr, ia->prefix_len) &&
			    (int32_t)(ia->lifetime.preferred - now) > 0) {
		                   /* ia->lifetime.preferred > now */
				return dst;
				}
			}

		/* 有効時間内のアドレスを探索する。*/
		for (ix = NUM_IN6_IFADDR_ENTRY; ix -- > 0; ) {
			ia = &ifp->in_ifaddrs[ix];
			if (IFA6_IS_READY(ia) &&
			    in6_are_prefix_equal(dst, &ia->addr, ia->prefix_len) &&
			    (int32_t)(ia->lifetime.expire - now) > 0)
		                   /* ia->lifetime.expire > now */
				return dst;
			}

		/*
		 *  プレフィックスを探索する。
		 */
		if (nd6_onlink_prefix_lookup (dst) != NULL)
			return dst;

		/*
		 *  静的経路表を探索する。
		 */

#if NUM_ROUTE_ENTRY > 0

		syscall(wai_sem(SEM_IN_ROUTING_TBL));
		for (ix = NUM_ROUTE_ENTRY; ix --; ) {
			if ((routing_tbl[ix].flags & IN_RTF_DEFINED) &&
			    in6_are_prefix_equal(dst, &routing_tbl[ix].target,
			                               routing_tbl[ix].prefix_len)) {

				/*
				 *  向け直しによるエントリは、有効時間が切れる時刻を延長する。
				 *  expire の単位は [s]。
				 *  TMO_IN_REDIRECT の単位は [ms]。
				 */
				if (ix > NUM_STATIC_ROUTE_ENTRY) {
					SYSTIM	now;

					syscall(get_tim(&now));
					routing_tbl[ix].expire = now / SYSTIM_HZ + TMO_IN_REDIRECT / 1000;
					}

				syscall(sig_sem(SEM_IN_ROUTING_TBL));
				return &routing_tbl[ix].gateway;
				}
			}
		syscall(sig_sem(SEM_IN_ROUTING_TBL));

#endif	/* of #if NUM_ROUTE_ENTRY > 0 */

		/*
		 *  ディフォルトルータ・リストを探索する。
		 */
		return nd6_router_lookup();
		}
	}

#if NUM_REDIRECT_ROUTE_ENTRY > 0

/*
 *  in6_gateway_lookup -- ルーティング表のルータを探索する。
 */

T_IN_RTENTRY *
in6_gateway_lookup (T_IN6_ADDR *gw)
{
	int_t	ix;

	for (ix = NUM_ROUTE_ENTRY; ix --; )
		if ((routing_tbl[ix].flags & IN_RTF_DEFINED) &&
		    IN6_ARE_ADDR_EQUAL(&routing_tbl[ix].gateway, gw))
			return &routing_tbl[ix];
	return NULL;
	}

/*
 *  in6_rtredirect -- ルーティング表にエントリを登録する。
 *
 *    注意: 引数 tmo の単位は [ms]。
 */

void
in6_rtredirect (T_IN6_ADDR *gateway, T_IN6_ADDR *target, uint_t prefix_len, uint8_t flags, uint32_t tmo)
{
	T_IN_RTENTRY	*frt;

	frt = in_rtnewentry(flags, tmo);
	frt->gateway    = *gateway;
	frt->target     = *target;
	frt->prefix_len = prefix_len;
	}

#endif	/* of #if NUM_REDIRECT_ROUTE_ENTRY > 0 */

#if NUM_IN6_HOSTCACHE_ENTRY > 0

/*
 *  in6_hostcache_lookup -- IPv6 ホストキャッシュを探索する。
 */

static T_IN6_HOSTCACHE_ENTRY*
in6_hostcache_lookup (T_IN6_ADDR *dst)
{
	int_t ix;

	for (ix = NUM_IN6_HOSTCACHE_ENTRY; ix -- > 0; ) {
		if (IN6_ARE_ADDR_EQUAL(dst, &in6_hostcache[ix].dst))
			return &in6_hostcache[ix];
		}
	return NULL;
	}

/*
 *  in6_hostcache_update -- IPv6 ホストキャッシュを更新する。
 */

void
in6_hostcache_update (T_IN6_ADDR *dst, uint32_t mtu)
{
	T_IN6_HOSTCACHE_ENTRY	*hc;
	SYSTIM	now, old;
	int_t	ix, oix;

	syscall(get_tim(&now));

	/* 既に登録されているか探索する。*/
	if ((hc = in6_hostcache_lookup (dst)) == NULL) {

		/* 空きのホストキャッシュを探す。*/
		for (ix = NUM_IN6_HOSTCACHE_ENTRY; ix -- > 0; ) {
			if (IN6_IS_HOSTCACHE_FREE(&in6_hostcache[ix]))
				break;
			}

		if (ix < 0) {
			/*
			 *  空きが無い時は、有効時間の切れる時間が、
			 *  最も短いホストキャッシュを置換する。
			 */
			old = now - 1;
			oix = 0;
			for (ix = NUM_IN6_HOSTCACHE_ENTRY; ix -- > 0; ) {
				hc = &in6_hostcache[ix];
				if (IN6_IS_HOSTCACHE_BUSY(hc) && (int32_t)(hc->expire - old) < 0) {
				                                        /* hc->expire < old */
					oix = ix;
					old = hc->expire;
					}
				}
			ix = oix;
			}
		hc = &in6_hostcache[ix];
		}
	else if (mtu > hc->mtu &&
	         (int32_t)(hc->expire - now) >= (IN6_HOSTCACHE_EXPIRE - IN6_HOSTCACHE_INCREASE)) {

		/*
		 *  既に登録されていて、新しい MTU が、登録されている MTU より大きいとき、
		 *  IN6_HOSTCACHE_INCREASE（推奨 10 分）間は更新しない。
		 */
		return;
		}
	hc->dst    = *dst;
	hc->expire = now + IN6_HOSTCACHE_EXPIRE;
	hc->mtu    = mtu;
	}

/*
 *  in6_hostcache_getmtu -- IPv6 ホストキャッシュをから MTU を取得する。
 *
 *    戻り値が 0 であれば、ホストキャッシュに登録されていない。
 */

uint32_t
in6_hostcache_getmtu (T_IN6_ADDR *dst)
{
	T_IN6_HOSTCACHE_ENTRY	*hc;

	/* 既に登録されているか探索する。*/
	if ((hc = in6_hostcache_lookup(dst)) == NULL)
		return 0;
	else
		return hc->mtu;
	}

/*
 *  in6_hostcache_timer -- IPv6 ホストキャッシュ更新タイマー
 *
 *    1秒周期で起動される。
 */

static void
in6_hostcache_timer (void)
{
	static int_t interval = IN6_HOSTCACHE_PRUNE / SYSTIM_HZ;

	SYSTIM	now;

	interval --;
	if (interval <= 0) {
		syscall(get_tim(&now));
		interval = IN6_HOSTCACHE_PRUNE / SYSTIM_HZ;
		}
	}

#endif	/* of #if NUM_IN6_HOSTCACHE_ENTRY > 0 */

/*
 *  in6_timer -- IPv6 共通タイマー
 *
 *    1秒周期で起動される。
 */

static void
in6_timer (void)
{
#if NUM_REDIRECT_ROUTE_ENTRY > 0

	in_rttimer();

#endif	/* of #if NUM_REDIRECT_ROUTE_ENTRY > 0 */

#ifdef IP6_CFG_FRAGMENT

	frag6_timer();

#endif	/* of #ifdef IP6_CFG_FRAGMENT */

#if NUM_IN6_HOSTCACHE_ENTRY > 0

	in6_hostcache_timer();

#endif	/* of #if NUM_IN6_HOSTCACHE_ENTRY > 0 */

	timeout((callout_func)in6_timer, NULL, IN_TIMER_TMO);
	}

/*
 *  in6_init -- IPv6 共通機能を初期化する。
 */

void
in6_init (void)
{
#if NUM_REDIRECT_ROUTE_ENTRY > 0

	in_rtinit();

#endif	/* of #if NUM_REDIRECT_ROUTE_ENTRY > 0 */

	timeout((callout_func)nd6_timer, NULL, ND6_TIMER_TMO);
	timeout((callout_func)in6_timer, NULL, IN_TIMER_TMO);
	}

#endif /* of #ifdef SUPPORT_INET6 */
