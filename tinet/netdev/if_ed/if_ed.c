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
 *  @(#) $Id: if_ed.c,v 1.5 2009/12/24 05:46:31 abe Exp abe $
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

/*
 * Device driver for National Semiconductor DS8390/WD83C690 based ethernet
 *   adapters. By David Greenman, 29-April-1993
 *
 * Currently supports the Western Digital/SMC 8003 and 8013 series,
 *   the SMC Elite Ultra (8216), the 3Com 3c503, the NE1000 and NE2000,
 *   and a variety of similar clones.
 *
 */

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
#include <t_syslog.h>
#include "kernel_cfg.h"
#include "target_config.h"

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <s_services.h>
#include <t_services.h>
#include "kernel_id.h"

#endif	/* of #ifdef TARGET_KERNEL_JSP */

#include <tinet_defs.h>
#include <tinet_config.h>

#include <net/if.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <net/net.h>
#include <net/net_timer.h>
#include <net/net_count.h>
#include <net/net_buf.h>

#include "if_edreg.h"

/*
 *  内部 RAM のページ指定
 */

#define ED_INT_TXBUF_START	(ED_INT_RAM_BASE / ED_PAGE_SIZE)
#define ED_INT_TXBUF_STOP	(ED_INT_RAM_BASE / ED_PAGE_SIZE + IF_ED_TXBUF_PAGE_SIZE)
#define ED_INT_RXBUF_START	 ED_INT_TXBUF_STOP
#define ED_INT_RXBUF_STOP	((ED_INT_RAM_BASE + ED_INT_RAM_SIZE) / ED_PAGE_SIZE)

/*
 *  ネットワークインタフェースに依存するソフトウェア情報 
 */

typedef struct t_ed_softc {
	uint32_t	nic_addr;			/* NIC のベースアドレス		*/
	uint32_t	asic_addr;			/* ASIC のベースアドレ		*/
	uint16_t	txb_len[NUM_IF_ED_TXBUF];	/* 送信バッファのオクテット数	*/
	uint8_t		txb_inuse;			/* 使用中の送信バッファ		*/
	uint8_t		txb_insend;			/* 送信中の送信バッファ		*/
	uint8_t		txb_write;			/* 書き込む送信バッファ		*/
	uint8_t		txb_send;			/* 送信する送信バッファ		*/
	uint8_t		rxb_read;			/* 読み込む受信ページ		*/
	} T_ED_SOFTC;

/*
 *  ネットワークインタフェースのソフトウェア情報
 */

/* ネットワークインタフェースに依存するソフトウェア情報 */

static T_ED_SOFTC ed_softc = {
	ED_BASE_ADDRESS + ED_NIC_OFFSET,	/* NIC のベースアドレス		*/
	ED_BASE_ADDRESS + ED_ASIC_OFFSET,	/* ASIC のベースアドレス		*/
	};

/* ネットワークインタフェースに依存しないソフトウェア情報 */


T_IF_SOFTC if_softc = {
	{},					/* ネットワークインタフェースのアドレス	*/
	0,					/* 送信タイムアウト			*/
	&ed_softc,				/* ディバイス依存のソフトウェア情報	*/
	SEM_IF_ED_SBUF_READY,			/* 送信セマフォ			*/
	SEM_IF_ED_RBUF_READY,			/* 受信セマフォ			*/

#ifdef SUPPORT_INET6

	IF_MADDR_INIT,				/* マルチキャストアドレスリスト	*/

#endif	/* of #ifdef SUPPORT_INET6 */
	};

/*
 *  局所変数
 */

static void ed_pio_readmem (T_ED_SOFTC *sc, uint32_t src, uint8_t *dst, uint16_t amount);
static T_NET_BUF *ed_get_frame (T_ED_SOFTC *sc, uint32_t ring, uint16_t len);
static void ed_xmit (T_IF_SOFTC *ic);
static void ed_stop (T_ED_SOFTC *sc);
static void ed_init_sub (T_IF_SOFTC *ic);
static void ed_setrcr (T_IF_SOFTC *ic);

#ifdef SUPPORT_INET6

static uint32_t ds_crc (uint8_t *addr);
static void ds_getmcaf (T_IF_SOFTC *ic, uint32_t *mcaf);

/*
 *  ds_crc -- イーサネットアドレスの CRC を計算する。
 */

#define POLYNOMIAL	0x04c11db6

static uint32_t
ds_crc (uint8_t *addr)
{
	uint32_t	crc = ULONG_C(0xffffffff);
	int_t		carry, len, bit;
	uint8_t		byte;

	for (len = ETHER_ADDR_LEN; len -- > 0; ) {
		byte = *addr ++;
		for (bit = 8; bit -- > 0; ) {
			carry   = ((crc & ULONG_C(0x80000000)) ? 1 : 0) ^ (byte & UINT_C(0x01));
			crc   <<= 1;
			byte   >>= 1;
			if (carry)
				crc = (crc ^ POLYNOMIAL) | carry;
			}
		}
	return crc;
	}

#undef POLYNOMIAL

/*
 *  ds_getmcaf -- マルチキャストアドレスのリストからマルチキャストアドレス
 *                フィルタを計算する。
 */

static void
ds_getmcaf (T_IF_SOFTC *ic, uint32_t *mcaf)
{
	uint32_t	count, index;
	uint8_t		*af = (uint8_t*)mcaf;

	mcaf[0] = mcaf[1] = 0;

	for (count = MAX_IF_MADDR_CNT; count -- > 0; ) {
		index = ds_crc(ic->maddrs[count].lladdr) >> 26;
		af[index >> 3] |= 1 << (index & 7);
		}
	}

/*
 * ed_setrcr -- 受信構成レジスタ (RCR) を設定する。
 */

static void
ed_setrcr (T_IF_SOFTC *ic)
{
	T_ED_SOFTC	*sc = ic->sc;
	int_t		ix;
	uint32_t	mcaf[2];

	/* レジスタページ 1 を選択する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_CR),
	                 ED_CR_RD2 | ED_CR_PAGE1 | ED_CR_STP);

	/* マルチキャストフィルタを計算する。*/
	ds_getmcaf(ic, mcaf);

	/* マルチキャストの受信設定 */
	for (ix = 0; ix < 8; ix ++)
		sil_wrb_mem((void*)(sc->nic_addr + ED_P1_MAR(ix)), ((uint8_t *)mcaf)[ix]);

	/* レジスタページ 0 を選択する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_CR),
	            ED_CR_RD2 | ED_CR_PAGE0 | ED_CR_STP);

	/* マルチキャストとユニキャストアドレスのみ受信するように設定する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_RCR), ED_RCR_AM);

	/* NIC を起動する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_CR),
	            ED_CR_RD2 | ED_CR_PAGE0 | ED_CR_STA);
	}

/*
 * ed_addmulti -- マルチキャストアドレスを追加する。
 */

ER
ed_addmulti (T_IF_SOFTC *ic)
{
	ed_setrcr(ic);
	return E_OK;
	}

#endif	/* of #ifdef SUPPORT_INET6 */

/*
 * ed_pio_readmem -- プログラム I/O を使って NIC のデータを読み込む
 */

static void
ed_pio_readmem (T_ED_SOFTC *sc, uint32_t src, uint8_t *dst, uint16_t amount)
{
	/* レジスタページ 0 を選択し、DMA を停止する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_CR),
	            ED_CR_RD2 | ED_CR_PAGE0 | ED_CR_STA);

	/* DMA 転送数を設定する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_RBCR0), amount);
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_RBCR1), amount >> 8);

	/* NIC メモリの転送元アドレスを設定する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_RSAR0), src);
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_RSAR1), src >> 8);

	/* DMA 読み込みを選択する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_CR),
	            ED_CR_RD0 | ED_CR_PAGE0 | ED_CR_STA);

	/* NIC メモリから読み込む */
	while (amount -- > 0)
		*dst ++ = sil_reb_mem((void*)(sc->asic_addr + ED_DATA_OFFSET));
	}

/*
 * ed_pio_writemem -- プログラム I/O を使って NIC にデータを書き込む
 */

static void
ed_pio_writemem (T_ED_SOFTC *sc, uint8_t *src, uint32_t dst, uint16_t amount)
{
	/* レジスタページ 0 を選択し、DMA を停止する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_CR),
	            ED_CR_RD2 | ED_CR_PAGE0 | ED_CR_STA);

	/* DMA 転送数を設定する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_RBCR0), amount);
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_RBCR1), amount >> 8);

	/* NIC メモリの転送先アドレスを設定する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_RSAR0), dst);
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_RSAR1), dst >> 8);

	/* DMA 書込みを選択する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_CR),
	                 ED_CR_RD1 | ED_CR_PAGE0 | ED_CR_STA);

	/* NIC メモリに書き込む */
	while (amount -- > 0)
		sil_wrb_mem((void*)(sc->asic_addr + ED_DATA_OFFSET), *src ++);
	}

/*
 * ed_get_frame -- Ethernet フレームを入力する。
 */

static T_NET_BUF *
ed_get_frame (T_ED_SOFTC *sc, uint32_t ring, uint16_t len)
{
	T_NET_BUF	*input = NULL;
	uint16_t	sublen, align;
	uint8_t		*dst;
	ER		error;

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

	if ((error = tget_net_buf(&input, align, TMO_IF_ED_GET_NET_BUF)) == E_OK && input != NULL) {
		NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_IN_PACKETS], 1);
		NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_IN_OCTETS],  len);
		dst = input->buf + IF_ETHER_NIC_HDR_ALIGN;
		if (ring + len > ED_INT_RAM_BASE + ED_INT_RAM_SIZE) {
			sublen = (ED_INT_RAM_BASE + ED_INT_RAM_SIZE) - ring;
			ed_pio_readmem(sc, ring, dst, sublen);
			len -= sublen;
			dst += sublen;
			ring = ED_INT_RXBUF_START * ED_PAGE_SIZE;
			}
		ed_pio_readmem(sc, ring, dst, len);
		}
	else {
		NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_IN_ERR_PACKETS], 1);
		NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_NO_BUFS], 1);
		}
	return input;
	}

/*
 *  ed_xmit -- フレームを送信する。
 *
 *    注意: NIC 割り込み禁止状態で呼び出すこと。
 */

static void
ed_xmit (T_IF_SOFTC *ic)
{
	T_ED_SOFTC *sc = ic->sc;

	/* レジスタページ 0 を選択する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_CR),
	            ED_CR_RD2 | ED_CR_PAGE0 | ED_CR_STA);

	/* 送信するページを設定する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_TPSR),
	            ED_INT_TXBUF_START + sc->txb_send * NUM_IF_ED_TXBUF_PAGE);

	/* 送信するオクテット数を設定する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_TBCR0), sc->txb_len[sc->txb_send]);
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_TBCR1), sc->txb_len[sc->txb_send] >> 8);

	/* 送信する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_CR),
	            ED_CR_RD2 | ED_CR_PAGE0 | ED_CR_TXP | ED_CR_STA);

	/* 送信バッファを切り替える。*/
	sc->txb_send ++;
	if (sc->txb_send == NUM_IF_ED_TXBUF)
		sc->txb_send = 0;

	sc->txb_insend ++;

	/* 送信タイムアウトを設定する。*/
	ic->timer = TMO_IF_ED_XMIT;
	}

/*
 *  ed_stop -- ed ネットワークインタフェースを停止する。
 *
 *    注意: NIC 割り込み禁止状態で呼び出すこと。
 */

static void
ed_stop (T_ED_SOFTC *sc)
{
	int_t wait;

	/* DMA を停止する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_CR),
	            ED_CR_RD2 | ED_CR_PAGE0 | ED_CR_STP);

	/* NIC が停止状態になるまで待つ。ただし、5[ms]を上限にしている。*/
	for (wait = 5; ((sil_reb_mem((void*)(sc->nic_addr + ED_P0_ISR)) & ED_ISR_RST) == 0) && wait -- > 0; )
		syscall(dly_tsk(1));
	}

/*
 *  ed_init_sub -- ed ネットワークインタフェースの初期化
 *
 *    注意: NIC 割り込み禁止状態で呼び出すこと。
 */

static void
ed_init_sub (T_IF_SOFTC *ic)
{
	T_ED_SOFTC	*sc = ic->sc;
	int_t		ix;

	/* 受信ページの設定 */
	sc->rxb_read = ED_INT_RXBUF_START + 1;

	/* 送信バッファの設定 */
	NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_OUT_ERR_PACKETS], sc->txb_inuse);
	sc->txb_inuse  = 0;
	sc->txb_insend = 0;
	sc->txb_write  = 0;
	sc->txb_send   = 0;
	
	/* 送信タイムアウトをリセットする。*/
	ic->timer = 0;

	/* インタフェースを停止する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_CR), ED_CR_PAGE0 | ED_CR_STP);

	/*
	 *  DCR を設定する。
	 *    ・FIFO のしきい値を 8
	 *    ・ループバックモード
	 */
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_DCR), ED_DCR_FT1 | ED_DCR_LS);

	/* RBCR をクリアーする。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_RBCR0), 0);
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_RBCR1), 0);

	/* 入力フレームを保存しない。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_RCR), ED_RCR_MON);

	/* 内部ループバックモードに設定する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_TCR), ED_TCR_LB0);

	/* 送受信リングバッファの設定 */
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_TPSR),   ED_INT_TXBUF_START);
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_PSTART), ED_INT_RXBUF_START);
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_PSTOP),  ED_INT_RXBUF_STOP);
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_BNRY),   ED_INT_RXBUF_START);

	/* 全ての割り込みフラグをリセットする。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_ISR), 0xff);

	/*
	 *  割り込みを許可する。
	 *    ・送受信完了
	 *    ・送受信エラー
	 *    ・受信オーバーライト
	 */
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_IMR),
	            ED_IMR_PRX | ED_IMR_PTX | ED_IMR_RXE | ED_IMR_TXE | ED_IMR_OVW);

	/* レジスタページ 1 を選択する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_CR),
	            ED_CR_RD2 | ED_CR_PAGE1 | ED_CR_STP);

	/* MAC アドレスを設定する。*/
	for (ix = 0; ix < ETHER_ADDR_LEN; ix ++)
		sil_wrb_mem((void*)(sc->nic_addr + ED_P1_PAR(ix)), ic->ifaddr.lladdr[ix]);

	/* フレームを書き込むページを設定する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P1_CURR), sc->rxb_read);

 	/* 受信構成レジスタ (RCR) を設定する。*/
 	ed_setrcr(ic);

	/* 内部ループバックモードからぬける。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_TCR), 0);
	
	/* 送信可能セマフォを初期化する。*/
	for (ix = NUM_IF_ED_TXBUF; ix --; )
		sig_sem(ic->semid_txb_ready);

#if defined(TARGET_KERNEL_ASP)

	/* ターゲット依存部の割込み初期化 */
	ed_inter_init();

#endif	/* of #if defined(TARGET_KERNEL_ASP) */

#if defined(TARGET_KERNEL_JSP) && TKERNEL_PRVER >= 0x1042u	/* JSP-1.4.2 以降 */

	/* ターゲット依存部の割込み初期化 */
	ed_inter_init();

#endif	/* of #if defined(TARGET_KERNEL_JSP) && TKERNEL_PRVER >= 0x1042u */

	}

#ifndef SUPPORT_INET6

/*
 * ed_setrcr -- 受信構成レジスタ (RCR) を設定する。
 */

static void
ed_setrcr (T_IF_SOFTC *ic)
{
	T_ED_SOFTC	*sc = ic->sc;
	int_t		ix;

	/* レジスタページ 1 を選択する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_CR),
	            ED_CR_RD2 | ED_CR_PAGE1 | ED_CR_STP);

#ifdef IF_ED_CFG_ACCEPT_ALL

	/* マルチキャストの受信設定 */
	for (ix = 0; ix < 8; ix ++)

		/* マルチキャストを全て受信する。*/
		sil_wrb_mem((void*)(sc->nic_addr + ED_P1_MAR(ix)), 0xff);

	/* レジスタページ 0 を選択する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_CR),
	                 ED_CR_RD2 | ED_CR_PAGE0 | ED_CR_STP);

	/* マルチキャストとエラーフレームも受信するように設定する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_RCR),
	            ED_RCR_PRO | ED_RCR_AM | ED_RCR_AB |ED_RCR_SEP);

#else	/* of #ifdef IF_ED_CFG_ACCEPT_ALL */

	/* マルチキャストの受信設定 */
	for (ix = 0; ix < 8; ix ++)

		/* マルチキャストを全て受信しない。*/
		sil_wrb_mem((void*)(sc->nic_addr + ED_P1_MAR(ix)), 0x00);

	/* レジスタページ 0 を選択する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_CR),
	            ED_CR_RD2 | ED_CR_PAGE0 | ED_CR_STP);

	/* 自分とブロードキャストのみ受信するように設定する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_RCR), ED_RCR_AB);

#endif	/* of #ifdef IF_ED_CFG_ACCEPT_ALL */

	/* NIC を起動する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_CR),
	            ED_CR_RD2 | ED_CR_PAGE0 | ED_CR_STA);
	}

#endif	/* of #ifndef SUPPORT_INET6 */

/*
 * ed_reset -- ed ネットワークインタフェースをリセットする。
 */

void
ed_reset (T_IF_SOFTC *ic)
{
#ifdef TARGET_KERNEL_JSP
	IPM	ipm;
#endif

	/* NIC からの割り込みを禁止する。*/
#ifdef TARGET_KERNEL_JSP
	ipm = ed_dis_inter();
#endif
#ifdef TARGET_KERNEL_ASP
	syscall(dis_int(INTNO_IF_ED));
#endif

	NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_RESETS], 1);
	ed_stop(ic->sc);
	ed_init_sub(ic);

	/* NIC からの割り込みを許可する。*/
#ifdef TARGET_KERNEL_JSP
	ed_ena_inter(ipm);
#endif
#ifdef TARGET_KERNEL_ASP
	syscall(ena_int(INTNO_IF_ED));
#endif
	}

/*
 *  get_ed_softc -- ネットワークインタフェースのソフトウェア情報を返す。
 */

T_IF_SOFTC *
ed_get_softc (void)
{
	return &if_softc;
	}

/*
 * ed_watchdog -- ed ネットワークインタフェースのワッチドッグタイムアウト
 */

void
ed_watchdog (T_IF_SOFTC *ic)
{
	NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_OUT_ERR_PACKETS], 1);
	NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_TIMEOUTS], 1);
	ed_reset(ic);
	}

/*
 * ed_probe -- ed ネットワークインタフェースの検出
 */

void
ed_probe (T_IF_SOFTC *ic)
{
#ifdef ED_CFG_HSB8S2638
	static const uint8_t mac_order[] = { 3, 1, 7, 5, 11, 9 };
#else	/* #ifdef ED_CFG_HSB8S2638 */
	static const uint8_t mac_order[] = { 1, 3, 5, 7, 9, 11 };
#endif	/* #ifdef ED_CFG_HSB8S2638 */

	uint8_t		romdata[ETHER_ADDR_LEN * 2], tmp;
	T_ED_SOFTC	*sc = ic->sc;
	int_t		ix;

#if defined(TARGET_KERNEL_ASP)

	/* ターゲット依存部のバスの初期化 */
	ed_bus_init();

#endif	/* of #if defined(TARGET_KERNEL_ASP) */

#if defined(TARGET_KERNEL_JSP) && TKERNEL_PRVER >= 0x1042u	/* JSP-1.4.2 以降 */

	/* ターゲット依存部のバスの初期化 */
	ed_bus_init();

#endif	/* of #if defined(TARGET_KERNEL_JSP) && TKERNEL_PRVER >= 0x1042u */

	/* リセットする。*/
	tmp = sil_reb_mem((void*)(sc->asic_addr + ED_RESET_OFFSET));
	sil_wrb_mem((void*)(sc->asic_addr + ED_RESET_OFFSET), tmp);
	dly_tsk(5);

	/* DMA を停止する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_CR),
	            ED_CR_RD2 | ED_CR_PAGE0 | ED_CR_STP);
	dly_tsk(5);

	/* 入力フレームを保存しない。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_RCR), ED_RCR_MON);

	/*
	 *  DCR を設定する。
	 *    ・FIFO のしきい値を 8
	 *    ・ループバックモード
	 */
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_DCR), ED_DCR_FT1 | ED_DCR_LS);

	/* MAC アドレスを読み込む。*/
	ed_pio_readmem(sc, 0, romdata, ETHER_ADDR_LEN * 2);
	for (ix = 0; ix < ETHER_ADDR_LEN; ix ++)
		ic->ifaddr.lladdr[ix] = romdata[mac_order[ix]];

	/* 全ての割り込みフラグをリセットする。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_ISR), 0xff);
	}

/*
 * ed_init -- ed ネットワークインタフェースの初期化
 */

void
ed_init (T_IF_SOFTC *ic)
{
#ifdef TARGET_KERNEL_JSP
	IPM	ipm;
#endif

	/* NIC からの割り込みを禁止する。*/
#ifdef TARGET_KERNEL_JSP
	ipm = ed_dis_inter();
#endif
#ifdef TARGET_KERNEL_ASP
	syscall(dis_int(INTNO_IF_ED));
#endif

	/* ed_init 本体を呼び出す。*/
	ed_init_sub(ic);

	/* NIC からの割り込みを許可する。*/
#ifdef TARGET_KERNEL_JSP
	ed_ena_inter(ipm);
#endif
#ifdef TARGET_KERNEL_ASP
	syscall(ena_int(INTNO_IF_ED));
#endif
	}

/*
 * ed_read -- フレームの読み込み
 */

T_NET_BUF *
ed_read (T_IF_SOFTC *ic)
{
	T_ED_FRAME_HDR	frame_hdr;
	T_ED_SOFTC	*sc = ic->sc;
	T_NET_BUF	*input = NULL;
	uint32_t	frame_ptr;
	int_t		len;
	uint8_t		boundry;
	uint8_t		curr;
#ifdef TARGET_KERNEL_JSP
	IPM	ipm;
#endif

	/* NIC からの割り込みを禁止する。*/
#ifdef TARGET_KERNEL_JSP
	ipm = ed_dis_inter();
#endif
#ifdef TARGET_KERNEL_ASP
	syscall(dis_int(INTNO_IF_ED));
#endif

	/* レジスタページ 1 を選択する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_CR),
	            ED_CR_RD2 | ED_CR_PAGE1 | ED_CR_STA);

	curr = sil_reb_mem((void*)(sc->nic_addr + ED_P1_CURR));
	if (sc->rxb_read != curr) {

		/* 受信フレームの先頭を得る。*/
		frame_ptr = sc->rxb_read * ED_PAGE_SIZE;

		/* 受信フレームヘッダ構造体を取り出す。*/
		ed_pio_readmem(sc, frame_ptr, (char *)&frame_hdr, sizeof(frame_hdr));

#if defined(SIL_ENDIAN_BIG)

		frame_hdr.count = (frame_hdr.count << 8) | (frame_hdr.count >> 8);

#endif	/* of #if defined(SIL_ENDIAN_BIG) */

		len = frame_hdr.count;
		if (len >  sizeof(T_ED_FRAME_HDR) &&
		    len <= IF_MTU + sizeof(T_ETHER_HDR) + sizeof(T_ED_FRAME_HDR) &&
		    frame_hdr.next >= ED_INT_RXBUF_START &&
		    frame_hdr.next <  ED_INT_RXBUF_STOP) {
			input = ed_get_frame(sc, frame_ptr + sizeof(T_ED_FRAME_HDR),
			                               len - sizeof(T_ED_FRAME_HDR));
			}
		else {
			/* 受信エラーとリセットを記録する。*/
			NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_IN_ERR_PACKETS], 1);
			NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_RESETS], 1);
			ed_stop(sc);
			ed_init_sub(ic);

			/* NIC からの割り込みを許可する。*/
#ifdef TARGET_KERNEL_JSP
			ed_ena_inter(ipm);
#endif
#ifdef TARGET_KERNEL_ASP
			syscall(ena_int(INTNO_IF_ED));
#endif
			return NULL;
			}

		/* フレームポインタを更新する。*/
		sc->rxb_read = frame_hdr.next;

		/* レジスタページ 0 を選択する。*/
		sil_wrb_mem((void*)(sc->nic_addr + ED_P0_CR),
		            ED_CR_RD2 | ED_CR_PAGE0 | ED_CR_STA);

		/* NIC の境界ポインタを更新する。*/
		boundry = sc->rxb_read - 1;
		if (boundry < ED_INT_RXBUF_START)
			boundry = ED_INT_RXBUF_STOP - 1;
		sil_wrb_mem((void*)(sc->nic_addr + ED_P0_BNRY), boundry);

		/* レジスタページ 1 を選択する。*/
		sil_wrb_mem((void*)(sc->nic_addr + ED_P0_CR),
		            ED_CR_RD2 | ED_CR_PAGE1 | ED_CR_STA);
		}

	/* 受信リングバッファにデータが残っていれば、受信処理を継続する。*/
	curr = sil_reb_mem((void*)(sc->nic_addr + ED_P1_CURR));
	if (sc->rxb_read != curr)
		sig_sem(ic->semid_rxb_ready);

	/* NIC からの割り込みを許可する。*/
#ifdef TARGET_KERNEL_JSP
	ed_ena_inter(ipm);
#endif
#ifdef TARGET_KERNEL_ASP
	syscall(ena_int(INTNO_IF_ED));
#endif

	return input;
	}

/*
 * ed_start -- 送信フレームをバッファリングする。
 */

void
ed_start (T_IF_SOFTC *ic, T_NET_BUF *output)
{
	T_ED_SOFTC	*sc = ic->sc;
#ifdef TARGET_KERNEL_JSP
	IPM	ipm;
#endif

	NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_OUT_PACKETS], 1);
	NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_OUT_OCTETS],  output->len);

	/* NIC からの割り込みを禁止する。*/
#ifdef TARGET_KERNEL_JSP
	ipm = ed_dis_inter();
#endif
#ifdef TARGET_KERNEL_ASP
	syscall(dis_int(INTNO_IF_ED));
#endif

	/* 送信バッファに書き込む。*/
	ed_pio_writemem(sc, output->buf + IF_ETHER_NIC_HDR_ALIGN,
	                ED_INT_RAM_BASE + sc->txb_write * NUM_IF_ED_TXBUF_PAGE * ED_PAGE_SIZE,
	                output->len);

	/* 送信バッファに書き込んだオクテット数を記録する。*/
	if (output->len > ETHER_MIN_LEN - ETHER_CRC_LEN)
		sc->txb_len[sc->txb_write] = output->len;
	else
		sc->txb_len[sc->txb_write] = ETHER_MIN_LEN - ETHER_CRC_LEN;

	/* 送信バッファを切り替える。*/
	sc->txb_write ++;
	if (sc->txb_write == NUM_IF_ED_TXBUF)
		sc->txb_write = 0;

	sc->txb_inuse ++;

	/* もし送信中でなければ、送信を開始する。*/
	if (sc->txb_insend == 0)
		ed_xmit(ic);

	/* NIC からの割り込みを許可する。*/
#ifdef TARGET_KERNEL_JSP
	ed_ena_inter(ipm);
#endif
#ifdef TARGET_KERNEL_ASP
	syscall(ena_int(INTNO_IF_ED));
#endif
	}

/*
 *  NIC 割り込みハンドラ
 */

void
if_ed_handler (void)
{
	T_ED_SOFTC	*sc;
	T_IF_SOFTC	*ic;
	uint8_t		isr, tsr;
	uint16_t	collisions;

	ic = &if_softc;
	sc = ic->sc;

	/* レジスタページ 0 を選択する。*/
	sil_wrb_mem((void*)(sc->nic_addr + ED_P0_CR),
	            ED_CR_RD2 | ED_CR_PAGE0 | ED_CR_STA);

	isr = sil_reb_mem((void*)(sc->nic_addr + ED_P0_ISR));

	if (isr != 0) {

		/* 全ての割り込みフラグをリセットする。*/
		sil_wrb_mem((void*)(sc->nic_addr + ED_P0_ISR), isr);

		if (isr & (ED_ISR_PTX | ED_ISR_TXE)) {
			collisions = sil_reb_mem((void*)(sc->nic_addr + ED_P0_NCR)) & 0x0f;

			/* 送信処理 */
			tsr = sil_reb_mem((void*)(sc->nic_addr + ED_P0_TSR));
			if (isr & ED_ISR_TXE) {

				/* 送信エラーを記録する。*/
				if ((tsr & ED_TSR_ABT) && (collisions == 0)) {
					/*
					 * コリジョンが 16 のとき、P_NCR は 0、
					 * TSR_ABT は 1 になる。
					 */
					collisions = 16;
					}
				NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_OUT_ERR_PACKETS], 1);
				}

			if (sc->txb_insend)
				sc->txb_insend --;
			if (sc->txb_inuse)
				sc->txb_inuse  --;

			/* 送信タイムアウトをリセットする。*/
			ic->timer = 0;

			/* まだ送信バッファに残っていれば送信する。*/
			if (sc->txb_inuse)
				ed_xmit(ic);

			if (isig_sem(ic->semid_txb_ready) != E_OK)
				NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_TXB_QOVRS], 1);

			/* コリジョンを記録する。*/
			NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_COLS], collisions);
			}

		if (isr & (ED_ISR_PRX | ED_ISR_RXE | ED_ISR_OVW)) {
			if (isr & ED_ISR_OVW) {

				/* 上書きエラーとリセットを記録する。*/
				NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_IN_ERR_PACKETS], 1);
				NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_RESETS], 1);

				/* DMA を停止する。*/
				sil_wrb_mem((void*)(sc->nic_addr + ED_P0_CR),
				            ED_CR_RD2 | ED_CR_PAGE0 | ED_CR_STP);
				ed_init_sub(ic);
				}
			else {
				if (isr & ED_ISR_RXE) {

					/* 受信エラーを記録する。*/
					NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_IN_ERR_PACKETS], 1);
					}
				/* 受信割り込み処理 */
				if (isig_sem(ic->semid_rxb_ready) != E_OK)
					NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_RXB_QOVRS], 1);
				}
			}

		/* レジスタページ 0 を選択する。*/
		sil_wrb_mem((void*)(sc->nic_addr + ED_P0_CR),
		            ED_CR_RD2 | ED_CR_PAGE0 | ED_CR_STA);

		/* ネットワーク記録カウンタがオーバフローしたらリセットする。*/
		if (isr & ED_ISR_CNT) {
			(void)sil_reb_mem((void*)(sc->nic_addr + ED_P0_CNTR0));
			(void)sil_reb_mem((void*)(sc->nic_addr + ED_P0_CNTR1));
			(void)sil_reb_mem((void*)(sc->nic_addr + ED_P0_CNTR2));
			}
		}

	/* ターゲット依存部の割込みクリア */
#ifdef TARGET_KERNEL_JSP
	ed_inter_clear();
#endif
	}
