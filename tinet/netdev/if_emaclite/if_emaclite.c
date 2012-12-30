/*
 *  TINET (TCP/IP Protocol Stack)
 * 
 *  Copyright (C) 2001-2009 by Dep. of Computer Science and Engineering
 *                   Tomakomai National College of Technology, JAPAN
 *
 *  Copyright (C) 2012 by Tetsuya Morizumi
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
 *  @(#) $Id: if_emaclite.c 15 2012-04-16 14:29:01Z tetsuya $
 */

/*
 * Copyright (c) 1995, David Greenman
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: src/sys/i386/isa/if_ed.c,v 1.148.2.4 1999/09/25 13:08:18 nyan Exp $
 */

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
#include <t_syslog.h>
#include "kernel_cfg.h"
#include "target_config.h"

#endif  /* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <s_services.h>
#include <t_services.h>
#include "kernel_id.h"

#endif  /* of #ifdef TARGET_KERNEL_JSP */

#include <tinet_defs.h>
#include <tinet_config.h>

#include <net/if.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <net/net.h>
#include <net/net_timer.h>
#include <net/net_count.h>
#include <net/net_buf.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/if_ether.h>

#if defined(SUPPORT_INET4)
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#endif  /* of #if defined(SUPPORT_INET4) */

#if defined(SUPPORT_INET6)
#include <netinet/ip6.h>
#include <netinet6/ip6_var.h>
#endif  /* of #if defined(SUPPORT_INET6) */

#include "if_emaclitereg.h"


/**
 * emaclite default ethernet address
 */
/* EMACLITE default */
// const uint8_t ethernet_addr[] = {0x00, 0x00, 0x5E, 0x00, 0xFA, 0xCE};
/* Xilinx default */
const uint8_t ethernet_addr[] = {0x00, 0x0A, 0x35, 0x00, 0x01, 0x02};

/*
 *  ネットワークインタフェースに依存するソフトウェア情報 
 */
typedef struct t_emaclite_block
{
    void *tx_buffer;
    void *mdio_addr;
    void *mdio_wr;
    void *mdio_rd;
    void *mdio_ctrl;
    void *tx_length;
    void *gie;
    void *tx_control;
    void *rx_buffer;
    void *rx_control;
} T_EMACLITE_BLOCK;

typedef struct t_emaclite_softc
{
    T_EMACLITE_BLOCK *emac_block;
    uint8_t     txb_inuse;      /* tx buffer inuse flag */
    uint8_t     rxb_release;    /* rx buffer release flag */
    uint8_t     rxb_inuse;      /* rx buffer inuse flag */
} T_EMACLITE_SOFTC;

#define SWITCH_PONG(a)  ((void*)(((uint32_t)(a)) + EMACLITE_BUFFER_OFFSET))

/*
 *  ネットワークインタフェースのソフトウェア情報
 */

/* ネットワークインタフェースに依存するソフトウェア情報 */
static T_EMACLITE_BLOCK emaclite_block =
{
    (void*)(XEMACLITE_BASE + EMACLITE_TX_BUFFER),
    (void*)(XEMACLITE_BASE + EMACLITE_MDIOADDR),
    (void*)(XEMACLITE_BASE + EMACLITE_MDIOWR),
    (void*)(XEMACLITE_BASE + EMACLITE_MDIORD),
    (void*)(XEMACLITE_BASE + EMACLITE_MDIOCTRL),
    (void*)(XEMACLITE_BASE + EMACLITE_TX_LENGTH),
    (void*)(XEMACLITE_BASE + EMACLITE_GIE),
    (void*)(XEMACLITE_BASE + EMACLITE_TX_CONTROL),
    (void*)(XEMACLITE_BASE + EMACLITE_RX_BUFFER),
    (void*)(XEMACLITE_BASE + EMACLITE_RX_CONTROL)
};

static T_EMACLITE_SOFTC emaclite_softc =
{
    &emaclite_block,
    0,
    0,
    0
};

/* ネットワークインタフェースに依存しないソフトウェア情報 */
T_IF_SOFTC if_softc = {
    {},                         /* ネットワークインタフェースのアドレス */
    0,                          /* 送信タイムアウト                     */
    &emaclite_softc,            /* ディバイス依存のソフトウェア情報     */
    SEM_IF_EMACLITE_SBUF_READY, /* 送信セマフォ                         */
    SEM_IF_EMACLITE_RBUF_READY, /* 受信セマフォ                         */

#ifdef SUPPORT_INET6

    IF_MADDR_INIT,              /* マルチキャストアドレスリスト */

#endif  /* of #ifdef SUPPORT_INET6 */
};

/*
 *  局所変数
 */
static void emaclite_alignedwrite(void *src, uint32_t *dst, uint16_t len);
static void emaclite_alignedread(uint32_t *src, void *dst, uint16_t len);
static T_NET_BUF *emaclite_getbuf (uint16_t len);
static uint16_t emaclite_getlen(T_NET_BUF *input);
static void emaclite_init_sub (T_IF_SOFTC *ic);


#ifdef SUPPORT_INET6

/*
 * emaclite_addmulti -- マルチキャストアドレスを追加する。
 */

ER emaclite_addmulti (T_IF_SOFTC *ic)
{
    return E_OK;
}

#endif  /* of #ifdef SUPPORT_INET6 */

/**
 * aligned write
 */
static void emaclite_alignedwrite(void *src, uint32_t *dst, uint16_t len)
{
    uint32_t    aligned_buf = 0;
    uint8_t     *src8ptr;
    uint8_t     *dst8ptr;
    uint16_t    *src16ptr;
    uint16_t    *dst16ptr;
    uint32_t    *src32ptr;
    uint32_t    remain;

    if((((uint32_t)src) & 0x3) == 0)
    {
        /* uint32_t aligned buffer */
        src32ptr = (uint32_t*)src;
        while(3 < len)
        {
            sil_wrw_mem((void*)dst, *src32ptr);
            src32ptr++;
            dst++;
            len -= 4;
        }
        src8ptr = (uint8_t*)src32ptr;
        dst8ptr = (uint8_t*)((void*)&aligned_buf);

    } else if((((uint32_t)src) & 0x1) == 0)
    {
        /* uint16_t aligned buffer */
        src16ptr = (uint16_t*)src;
        while(3 < len)
        {
            dst16ptr = (uint16_t*)((void*)&aligned_buf);

            *dst16ptr++ = *src16ptr++;
            *dst16ptr++ = *src16ptr++;
            sil_wrw_mem((void*)dst, aligned_buf);
            dst++;
            len -= 4;
        }
        src8ptr = (uint8_t*)src16ptr;
        dst8ptr = (uint8_t*)((void*)&aligned_buf);

    } else
    {
        /* uint8_t algined buffer */
        src8ptr = (uint8_t*)src;
        while(3 < len)
        {
            dst8ptr = (uint8_t*)((void*)&aligned_buf);

            *dst8ptr++ = *src8ptr++;
            *dst8ptr++ = *src8ptr++;
            *dst8ptr++ = *src8ptr++;
            *dst8ptr++ = *src8ptr++;
            sil_wrw_mem((void*)dst, aligned_buf);
            dst++;
            len -= 4;
        }
        dst8ptr = (uint8_t*)((void*)&aligned_buf);
    }

    aligned_buf = 0;
    for(remain = 0; remain < len; remain++)
    {
        *dst8ptr++ = *src8ptr++;
    }
    sil_wrw_mem((void*)dst, aligned_buf);
}

/**
 * aligned read
 */
static void emaclite_alignedread(uint32_t *src, void *dst, uint16_t len)
{
    uint32_t    aligned_buf = 0;
    uint8_t     *src8ptr;
    uint8_t     *dst8ptr;
    uint16_t    *src16ptr;
    uint16_t    *dst16ptr;
    uint32_t    *dst32ptr;
    uint32_t    remain;

    if((((uint32_t)dst) & 0x3) == 0)
    {
        /* uint32_t aligned buffer */
        dst32ptr = (uint32_t*)dst;
        while(3 < len)
        {
            *dst32ptr = sil_rew_mem((void*)src);
            src++;
            dst32ptr++;
            len -= 4;
        }
        dst8ptr = (uint8_t*)dst32ptr;

    } else if((((uint32_t)dst) & 0x1) == 0)
    {
        /* uint16_t aligned buffer */
        dst16ptr = (uint16_t*)dst;
        while(3 < len)
        {
            aligned_buf = sil_rew_mem((void*)src);
            src++;

            src16ptr = (uint16_t*)((void*)&aligned_buf);
            *dst16ptr++ = *src16ptr++;
            *dst16ptr++ = *src16ptr++;
            len -= 4;
        }
        dst8ptr = (uint8_t*)dst16ptr;

    } else
    {
        /* uint8_t algined buffer */
        dst8ptr = (uint8_t*)dst;
        while(3 < len)
        {
            aligned_buf = sil_rew_mem((void*)src);
            src++;

            src8ptr = (uint8_t*)((void*)&aligned_buf);
            *dst8ptr++ = *src8ptr++;
            *dst8ptr++ = *src8ptr++;
            *dst8ptr++ = *src8ptr++;
            *dst8ptr++ = *src8ptr++;
            len -= 4;
        }
    }

    aligned_buf = sil_rew_mem((void*)src);
    src8ptr = (uint8_t*)((void*)&aligned_buf);
    for(remain = 0; remain < len; remain++)
    {
        *dst8ptr++ = *src8ptr++;
    }
}

/**
 * get rx buffer
 */
static T_NET_BUF *emaclite_getbuf (uint16_t len)
{
    T_NET_BUF   *input = NULL;
    uint16_t    align;

    /*
     *  +-----------+--------+---------+---------+
     *  | Ehter HDR | IP HDR | TCP HDR | TCP SDU |
     *  +-----------+--------+---------+---------+
     *        14        20        20        n
     *   <----------------- len ---------------->
     *              ^
     *              t_net_buf で 4 オクテット境界にアラインされている。
     *
     *  tcp_input と udp_input では、擬似ヘッダと SDU でチェックサムを
     *  計算するが、n が 4 オクテット境界になるように SDU の後ろに 0 を
     *  パッディングする。その分を考慮して net_buf を獲得しなければならない。
     */
    align = ((((len - sizeof(T_IF_HDR)) + 3) >> 2) << 2) + sizeof(T_IF_HDR);
    if (tget_net_buf(&input, align, TMO_IF_EMACLITE_GET_NET_BUF) == E_OK && input != NULL)
    {
        NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_IN_PACKETS], 1);
        NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_IN_OCTETS],  len);
    } else
    {
        syslog(LOG_ERROR, "cannot allocate input buf");
        NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_IN_ERR_PACKETS], 1);
        NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_NO_BUFS], 1);
    }
    return input;
}

/**
 * get remaining ethernet frame length
 */
static uint16_t emaclite_getlen(T_NET_BUF *input)
{
    T_ETHER_HDR *eth;
#if defined(SUPPORT_INET4)
    T_IP4_HDR   *ip4h;
#endif  /* of #if defined(SUPPORT_INET4) */
#if defined(SUPPORT_INET6)
    T_IP6_HDR   *ip6h;
#endif  /* of #if defined(SUPPORT_INET6) */
    uint16_t    proto;
    uint16_t    len = 0;

    eth = GET_ETHER_HDR(input);
    proto = ntohs(eth->type);

    switch(proto)
    {
#if defined(SUPPORT_INET4)
        case ETHER_TYPE_IP:     /* IP   */
            ip4h = GET_IP4_HDR(input);
            len = ntohs(ip4h->len);
            len += sizeof(T_ETHER_HDR) + ETHER_CRC_LEN;
            break;

        case ETHER_TYPE_ARP:    /* ARP  */
            len = IF_ARP_ETHER_HDR_SIZE;
            break;
#endif  /* of #if defined(SUPPORT_INET4) */

#if defined(SUPPORT_INET6)
        case ETHER_TYPE_IPV6:   /* IPv6 */
            ip6h = GET_IP6_HDR(input);
	        len = ntohs(ip6h->plen);
            len += IF_IP6_HDR_SIZE + sizeof(T_ETHER_HDR) + ETHER_CRC_LEN; 
            break;
#endif  /* of #if defined(SUPPORT_INET6) */
        default:
            len = IF_PDU_SIZE;
            break;
    }

    if(len == 0)
    {
        len = IF_PDU_SIZE - ETHER_MIN_LEN;
    } else if(ETHER_MIN_LEN < len)
    {
        len -= ETHER_MIN_LEN;
    } else
    {
        len = 0;
    }

    return len;
}

/*
 *  emaclite_init_sub -- ed ネットワークインタフェースの初期化
 *
 *    注意: NIC 割り込み禁止状態で呼び出すこと。
 */

static void emaclite_init_sub (T_IF_SOFTC *ic)
{
    T_EMACLITE_SOFTC *sc = ic->sc;
    T_EMACLITE_BLOCK *eb = sc->emac_block;
    int_t ix;

    /* clear flags */
    sc->txb_inuse = 0;
    sc->rxb_release = EMACLITE_RX_PING | EMACLITE_RX_PONG;
    sc->rxb_inuse = 0;
    
    /* reset tx timer */
    ic->timer = 0;

    /* clear rx ping buffer */
    sil_wrw_mem((void*)eb->rx_control, 0);
    /* clear rx pong buffer */
    sil_wrw_mem(SWITCH_PONG(eb->rx_control), 0);

    /* enable rx interrupt */
    sil_wrw_mem((void*)eb->rx_control, EMACLITE_RX_INT_EN);
    /* enable global interrupt */
    sil_wrw_mem((void*)eb->gie, EMACLITE_GIE_GIE);

    ix = sil_rew_mem(eb->tx_control);
    ix = sil_rew_mem(eb->rx_control);
    /* initialize tx semaphore */
    for (ix = NUM_IF_EMACLITE_TXBUF; ix --; )
    {
        sig_sem(ic->semid_txb_ready);
    }
}

#ifndef SUPPORT_INET6

#endif  /* of #ifndef SUPPORT_INET6 */

/*
 * emaclite_reset -- ed ネットワークインタフェースをリセットする。
 */

void emaclite_reset (T_IF_SOFTC *ic)
{
#ifdef TARGET_KERNEL_JSP
    IPM ipm;
#endif

    /* NIC からの割り込みを禁止する。*/
#ifdef TARGET_KERNEL_JSP
    ipm = ed_dis_inter();
#endif
#ifdef TARGET_KERNEL_ASP
    syscall(dis_int(INTNO_IF_EMACLITE));
#endif

    NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_RESETS], 1);
    emaclite_init_sub(ic);

    /* NIC からの割り込みを許可する。*/
#ifdef TARGET_KERNEL_JSP
    ed_ena_inter(ipm);
#endif
#ifdef TARGET_KERNEL_ASP
    syscall(ena_int(INTNO_IF_EMACLITE));
#endif
}

/*
 *  get_emaclite_softc -- ネットワークインタフェースのソフトウェア情報を返す。
 */

T_IF_SOFTC * emaclite_get_softc (void)
{
    return &if_softc;
}

/*
 * emaclite_watchdog -- ed ネットワークインタフェースのワッチドッグタイムアウト
 */
void emaclite_watchdog (T_IF_SOFTC *ic)
{
    NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_OUT_ERR_PACKETS], 1);
    NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_TIMEOUTS], 1);
    emaclite_reset(ic);
}

/*
 * emaclite_probe -- ed ネットワークインタフェースの検出
 */
void emaclite_probe (T_IF_SOFTC *ic)
{
    T_EMACLITE_SOFTC *sc = ic->sc;
    T_EMACLITE_BLOCK *eb = sc->emac_block;
    int32_t     ix;

    /*
    sil_wrw_mem(eb->mdio_addr, (0x1F << 5)|0x19);
    sil_wrw_mem(eb->mdio_wr, 0x801F);
    sil_wrw_mem(eb->mdio_ctrl, 0x9);
    */

    for(ix = 0; ix < ETHER_ADDR_LEN; ix++)
    {
        ic->ifaddr.lladdr[ix] = ethernet_addr[ix];
    }

    emaclite_alignedwrite((void*)ethernet_addr, eb->tx_buffer, ETHER_ADDR_LEN);

    sil_wrw_mem(eb->tx_length, ETHER_ADDR_LEN);
    sil_wrw_mem(eb->tx_control, EMACLITE_TX_PROGRAM | EMACLITE_TX_STATUS);
    while((sil_rew_mem(eb->tx_control) & (EMACLITE_TX_PROGRAM | EMACLITE_TX_STATUS)) != 0);
}

/*
 * emaclite_init -- ed ネットワークインタフェースの初期化
 */
void emaclite_init (T_IF_SOFTC *ic)
{
#ifdef TARGET_KERNEL_JSP
    IPM ipm;
#endif

    /* NIC からの割り込みを禁止する。*/
#ifdef TARGET_KERNEL_JSP
    ipm = ed_dis_inter();
#endif
#ifdef TARGET_KERNEL_ASP
    syscall(dis_int(INTNO_IF_EMACLITE));
#endif

    /* emaclite_init 本体を呼び出す。*/
    emaclite_init_sub(ic);

    /* NIC からの割り込みを許可する。*/
#ifdef TARGET_KERNEL_JSP
    ed_ena_inter(ipm);
#endif
#ifdef TARGET_KERNEL_ASP
    syscall(ena_int(INTNO_IF_EMACLITE));
#endif
}

/*
 * emaclite_read -- フレームの読み込み
 */

T_NET_BUF * emaclite_read (T_IF_SOFTC *ic)
{
    T_EMACLITE_SOFTC *sc = ic->sc;
    T_EMACLITE_BLOCK *eb = sc->emac_block;
    T_NET_BUF   *input = NULL;
    uint32_t control;
    uint16_t len;
#ifdef TARGET_KERNEL_JSP
    IPM ipm;
#endif

    /* NIC からの割り込みを禁止する。*/
#ifdef TARGET_KERNEL_JSP
    ipm = ed_dis_inter();
#endif
#ifdef TARGET_KERNEL_ASP
    syscall(dis_int(INTNO_IF_EMACLITE));
#endif

    if(sc->rxb_inuse & EMACLITE_RX_PING)
    {
        if((input = emaclite_getbuf(IF_PDU_SIZE)) != NULL)
        {
            /* get minimum ethernet frame */
            emaclite_alignedread(eb->rx_buffer,
                    input->buf + IF_ETHER_NIC_HDR_ALIGN, ETHER_MIN_LEN);
            /* get remaining frame */
            len = emaclite_getlen(input);
            if(len != 0)
            {
                emaclite_alignedread(eb->rx_buffer + ETHER_MIN_LEN,
                        input->buf + IF_ETHER_NIC_HDR_ALIGN + ETHER_MIN_LEN, len);
            }
            sc->rxb_inuse ^= EMACLITE_RX_PING;
        } else
        {
            control = sil_rew_mem((void*)eb->rx_control);
            if(control & EMACLITE_RX_STATUS)
            {
                isig_sem(ic->semid_rxb_ready);
            }
        }
    }
    else if(sc->rxb_inuse & EMACLITE_RX_PONG)
    {
        /* release ping buffer */
        sil_wrw_mem(eb->rx_control, EMACLITE_RX_INT_EN);
        sc->rxb_release ^= EMACLITE_RX_PING;

        /* read pong buffer */
        if((input = emaclite_getbuf(IF_PDU_SIZE)) != NULL)
        {
            /* get minimum ethernet frame */
            emaclite_alignedread(SWITCH_PONG(eb->rx_buffer),
                    input->buf + IF_ETHER_NIC_HDR_ALIGN, ETHER_MIN_LEN);
            /* get remaining frame */
            len = emaclite_getlen(input);
            if(len != 0)
            {
                emaclite_alignedread(SWITCH_PONG(eb->rx_buffer) + ETHER_MIN_LEN,
                        input->buf + IF_ETHER_NIC_HDR_ALIGN + ETHER_MIN_LEN, len);
            }

            /* release pong buffer */
            sil_wrw_mem(SWITCH_PONG(eb->rx_control), 0);
            sc->rxb_release ^= EMACLITE_RX_PONG;
            sc->rxb_inuse ^= EMACLITE_RX_PONG;
        } else
        {
            control = sil_rew_mem(SWITCH_PONG(eb->rx_control));
            if(control & EMACLITE_RX_STATUS)
            {
                isig_sem(ic->semid_rxb_ready);
            }
        }
    }

    /* NIC からの割り込みを許可する。*/
#ifdef TARGET_KERNEL_JSP
    ed_ena_inter(ipm);
#endif
#ifdef TARGET_KERNEL_ASP
    syscall(ena_int(INTNO_IF_EMACLITE));
#endif

    return input;
}

/*
 * emaclite_start -- 送信フレームをバッファリングする。
 */
void emaclite_start (T_IF_SOFTC *ic, T_NET_BUF *output)
{
    T_EMACLITE_SOFTC *sc = ic->sc;
    T_EMACLITE_BLOCK *eb = sc->emac_block;
    uint32_t    len;
#ifdef TARGET_KERNEL_JSP
    IPM ipm;
#endif

    NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_OUT_PACKETS], 1);
    NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_OUT_OCTETS],  output->len);

    /* NIC からの割り込みを禁止する。*/
#ifdef TARGET_KERNEL_JSP
    ipm = ed_dis_inter();
#endif
#ifdef TARGET_KERNEL_ASP
    syscall(dis_int(INTNO_IF_EMACLITE));
#endif

    len = (uint32_t)output->len;
    if (len < (ETHER_MIN_LEN - ETHER_CRC_LEN))
    {
        len = ETHER_MIN_LEN - ETHER_CRC_LEN;
    }

    // check unused buffer
    if((sc->txb_inuse & EMACLITE_TX_PING) == 0)
    {
        emaclite_alignedwrite(output->buf + IF_ETHER_NIC_HDR_ALIGN, (void*)eb->tx_buffer, len);
        sil_wrw_mem(eb->tx_length, len);
        sil_wrw_mem(eb->tx_control, EMACLITE_TX_INT_EN | EMACLITE_TX_STATUS);
        sc->txb_inuse ^= EMACLITE_TX_PING;
        // set tx timeout
        ic->timer = TMO_IF_EMACLITE_XMIT;
    }
    else if((sc->txb_inuse & EMACLITE_TX_PONG) == 0)
    {
        emaclite_alignedwrite(output->buf + IF_ETHER_NIC_HDR_ALIGN, SWITCH_PONG(eb->tx_buffer), len);
        sil_wrw_mem(SWITCH_PONG(eb->tx_length), len);
        sil_wrw_mem(SWITCH_PONG(eb->tx_control), EMACLITE_TX_INT_EN | EMACLITE_TX_STATUS);
        sc->txb_inuse ^= EMACLITE_TX_PONG;
        // set tx timeout
        ic->timer = TMO_IF_EMACLITE_XMIT;
    }

    /* NIC からの割り込みを許可する。*/
#ifdef TARGET_KERNEL_JSP
    ed_ena_inter(ipm);
#endif
#ifdef TARGET_KERNEL_ASP
    syscall(ena_int(INTNO_IF_EMACLITE));
#endif
}

/*
 *  EMACLITE Interrupt Service Routine
 */
void if_emaclite_isr (intptr_t extinf)
{
    T_EMACLITE_SOFTC *sc;
    T_EMACLITE_BLOCK *eb;
    T_IF_SOFTC *ic;
    uint32_t control;

    ic = &if_softc;
    sc = ic->sc;
    eb = sc->emac_block;

    // Check Tx PING buffer
    if(sc->txb_inuse & EMACLITE_TX_PING)
    {
        control = sil_rew_mem((void*)eb->tx_control);
        if((control & EMACLITE_TX_STATUS) == 0)
        {
            sc->txb_inuse ^= EMACLITE_TX_PING;

            /* reset tx timer */
            ic->timer = 0;
            if (isig_sem(ic->semid_txb_ready) != E_OK)
            {
                NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_TXB_QOVRS], 1);
            }
        }
    }
    // Check Tx PONG buffer
    if(sc->txb_inuse & EMACLITE_TX_PONG)
    {
        control = sil_rew_mem(SWITCH_PONG(eb->tx_control));
        if((control & EMACLITE_TX_STATUS) == 0)
        {
            sc->txb_inuse ^= EMACLITE_TX_PONG;

            /* reset tx timer */
            ic->timer = 0;
            if (isig_sem(ic->semid_txb_ready) != E_OK)
            {
                NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_TXB_QOVRS], 1);
            }
        }
    }

    // Check Rx PING buffer
    if(sc->rxb_release & EMACLITE_RX_PING)
    {
        control = sil_rew_mem((void*)eb->rx_control);
        if(control & EMACLITE_RX_STATUS)
        {
            sc->rxb_release ^= EMACLITE_RX_PING;
            sc->rxb_inuse ^= EMACLITE_RX_PING;
            if (isig_sem(ic->semid_rxb_ready) != E_OK)
            {
                NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_RXB_QOVRS], 1);
            }
        }
    }

    // Check Rx PONG buffer
    if(sc->rxb_release & EMACLITE_RX_PONG)
    {
        control = sil_rew_mem(SWITCH_PONG(eb->rx_control));
        if(control & EMACLITE_RX_STATUS)
        {
            sc->rxb_release ^= EMACLITE_RX_PONG;
            sc->rxb_inuse ^= EMACLITE_RX_PONG;
            if (isig_sem(ic->semid_rxb_ready) != E_OK)
            {
                NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_RXB_QOVRS], 1);
            }
        }
    }
}

