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
 *  @(#) $Id: if_edreg.h,v 1.5 2009/12/24 05:46:31 abe Exp abe $
 */

/*
 * Copyright (C) 1993, David Greenman. This software may be used, modified,
 *   copied, distributed, and sold, in both source and binary form provided
 *   that the above copyright and these terms are retained. Under no
 *   circumstances is the author responsible for the proper functioning
 *   of this software, nor does the author assume any responsibility
 *   for damages incurred with its use.
 *
 * $FreeBSD: src/sys/i386/isa/if_edreg.h,v 1.25.2.2 1999/09/25 13:08:18 nyan Exp $
 */

/*
 * National Semiconductor DS8390 NIC register definitions
 *
 *
 * Modification history
 *
 * Revision 2.2  1993/11/29  16:33:39  davidg
 * From Thomas Sandford <t.d.g.sandford@comp.brad.ac.uk>
 * Add support for the 8013W board type
 *
 * Revision 2.1  1993/11/22  10:52:33  davidg
 * patch to add support for SMC8216 (Elite-Ultra) boards
 * from Glen H. Lowe
 *
 * Revision 2.0  93/09/29  00:37:15  davidg
 * changed double buffering flag to multi buffering
 * made changes/additions for 3c503 multi-buffering
 * ...companion to Rev. 2.0 of 'ed' driver.
 *
 * Revision 1.1  93/06/23  03:01:07  davidg
 * Initial revision
 *
 */

/*
 * 2002/8/8	REALTEK RTL8019AS 用
 */

#ifndef _If_EDREG_H_
#define _If_EDREG_H_

/*
 *  NE2000 互換レジスタ
 */

/*
 *  内部レジスタオフセット
 */

/* ページ 0、読み出し */

#define ED_P0_CR		UINT_C(0x00)
#define ED_P0_CLDA0		UINT_C(0x01)
#define ED_P0_CLDA1		UINT_C(0x02)
#define ED_P0_BNRY		UINT_C(0x03)
#define ED_P0_TSR		UINT_C(0x04)
#define ED_P0_NCR		UINT_C(0x05)
#define ED_P0_FIFO		UINT_C(0x06)
#define ED_P0_ISR		UINT_C(0x07)
#define ED_P0_CRDA0		UINT_C(0x08)
#define ED_P0_CRDA1		UINT_C(0x09)
#define ED_P0_RSR		UINT_C(0x0c)
#define ED_P0_CNTR0		UINT_C(0x0d)
#define ED_P0_CNTR1		UINT_C(0x0e)
#define ED_P0_CNTR2		UINT_C(0x0f)

/* ページ 0、書き込み */

#define ED_P0_PSTART		UINT_C(0x01)
#define ED_P0_PSTOP		UINT_C(0x02)
#define ED_P0_TPSR		UINT_C(0x04)
#define ED_P0_TBCR0		UINT_C(0x05)
#define ED_P0_TBCR1		UINT_C(0x06)
#define ED_P0_RSAR0		UINT_C(0x08)
#define ED_P0_RSAR1		UINT_C(0x09)
#define ED_P0_RBCR0		UINT_C(0x0a)
#define ED_P0_RBCR1		UINT_C(0x0b)
#define ED_P0_RCR		UINT_C(0x0c)
#define ED_P0_TCR		UINT_C(0x0d)
#define ED_P0_DCR		UINT_C(0x0e)
#define ED_P0_IMR		UINT_C(0x0f)

/* ページ 1、読み書き */

#define ED_P1_CR		UINT_C(0x00)
#define ED_P1_PAR(x)		((x)+UINT_C(0x01))
#define ED_P1_CURR		UINT_C(0x07)
#define ED_P1_MAR(x)		((x)+UINT_C(0x08))

/*
 *  ページ 0、オフセット 0x00、R/W、Command Register (CR)
 */

#define ED_CR_STP_BIT		0
#define ED_CR_STA_BIT		1
#define ED_CR_TXP_BIT		2
#define ED_CR_RD0_BIT		3
#define ED_CR_RD1_BIT		4
#define ED_CR_RD2_BIT		5
#define ED_CR_PS0_BIT		6
#define ED_CR_PS1_BIT		7

#define ED_CR_STP		(1<<ED_CR_STP_BIT)
#define ED_CR_STA		(1<<ED_CR_STA_BIT)
#define ED_CR_TXP		(1<<ED_CR_TXP_BIT)
#define ED_CR_RD0		(1<<ED_CR_RD0_BIT)
#define ED_CR_RD1		(1<<ED_CR_RD1_BIT)
#define ED_CR_RD2		(1<<ED_CR_RD2_BIT)
#define ED_CR_PS0		(1<<ED_CR_PS0_BIT)
#define ED_CR_PS1		(1<<ED_CR_PS1_BIT)

#define ED_CR_PAGE0		(0)
#define ED_CR_PAGE1		(ED_CR_PS0)
#define ED_CR_PAGE2		(ED_CR_PS1)
#define ED_CR_PAGE3		(ED_CR_PS1|ED_CR_PS0)

/*
 *  ページ 0、オフセット 0x04、R、Transmit Status Register (TSR)
 */

#define ED_TSR_PTX_BIT		0
#define ED_TSR_COL_BIT		2
#define ED_TSR_ABT_BIT		3
#define ED_TSR_CRS_BIT		4
#define ED_TSR_CDH_BIT		6
#define ED_TSR_OWC_BIT		7

#define ED_TSR_PTX		(1<<ED_TSR_PTX_BIT)
#define ED_TSR_COL		(1<<ED_TSR_COL_BIT)
#define ED_TSR_ABT		(1<<ED_TSR_ABT_BIT)
#define ED_TSR_CRS		(1<<ED_TSR_CRS_BIT)
#define ED_TSR_CDH		(1<<ED_TSR_CDH_BIT)
#define ED_TSR_OWC		(1<<ED_TSR_OWC_BIT)

/*
 *  ページ 0、オフセット 0x07、R/W、Interrupt Status Register (ISR)
 */

#define ED_ISR_PRX_BIT		0
#define ED_ISR_PTX_BIT		1
#define ED_ISR_RXE_BIT		2
#define ED_ISR_TXE_BIT		3
#define ED_ISR_OVW_BIT		4
#define ED_ISR_CNT_BIT		5
#define ED_ISR_RDC_BIT		6
#define ED_ISR_RST_BIT		7

#define ED_ISR_PRX		(1<<ED_ISR_PRX_BIT)
#define ED_ISR_PTX		(1<<ED_ISR_PTX_BIT)
#define ED_ISR_RXE		(1<<ED_ISR_RXE_BIT)
#define ED_ISR_TXE		(1<<ED_ISR_TXE_BIT)
#define ED_ISR_OVW		(1<<ED_ISR_OVW_BIT)
#define ED_ISR_CNT		(1<<ED_ISR_CNT_BIT)
#define ED_ISR_RDC		(1<<ED_ISR_RDC_BIT)
#define ED_ISR_RST		(1<<ED_ISR_RST_BIT)

/*
 *  ページ 0、オフセット 0x0c、W
 *  ページ 2、オフセット 0x0c、R
 *  Recive Configuration Register (RCR)
 */

#define ED_RCR_SEP_BIT		0
#define ED_RCR_AR_BIT		1
#define ED_RCR_AB_BIT		2
#define ED_RCR_AM_BIT		3
#define ED_RCR_PRO_BIT		4
#define ED_RCR_MON_BIT		5

#define ED_RCR_SEP		(1<<ED_RCR_SEP_BIT)
#define ED_RCR_AR		(1<<ED_RCR_AR_BIT)
#define ED_RCR_AB		(1<<ED_RCR_AB_BIT)
#define ED_RCR_AM		(1<<ED_RCR_AM_BIT)
#define ED_RCR_PRO		(1<<ED_RCR_PRO_BIT)
#define ED_RCR_MON		(1<<ED_RCR_MON_BIT)

/*
 *  ページ 0、オフセット 0x0d、W
 *  ページ 2、オフセット 0x0d、R
 *  Transmit Configuration Register (TCR)
 */

#define ED_TCR_CRC_BIT		0
#define ED_TCR_LB0_BIT		1
#define ED_TCR_LB1_BIT		2
#define ED_TCR_ATD_BIT		3
#define ED_TCR_OFST_BIT		4

#define ED_TCR_CRC		(1<<ED_TCR_CRC_BIT)
#define ED_TCR_LB0		(1<<ED_TCR_LB0_BIT)
#define ED_TCR_LB1		(1<<ED_TCR_LB1_BIT)
#define ED_TCR_ATD		(1<<ED_TCR_ATD_BIT)
#define ED_TCR_OFST		(1<<ED_TCR_OFST_BIT)

/*
 *  ページ 0、オフセット 0x0e、W
 *  ページ 2、オフセット 0x0e、R
 *  Data Configuration Register (DCR)
 */

#define ED_DCR_WTS_BIT		0
#define ED_DCR_BOS_BIT		1
#define ED_DCR_LAS_BIT		2
#define ED_DCR_LS_BIT		3
#define ED_DCR_ARM_BIT		4
#define ED_DCR_FT0_BIT		5
#define ED_DCR_FT1_BIT		6

#define ED_DCR_WTS		(1<<ED_DCR_WTS_BIT)
#define ED_DCR_BOS		(1<<ED_DCR_BOS_BIT)
#define ED_DCR_LAS		(1<<ED_DCR_LAS_BIT)
#define ED_DCR_LS		(1<<ED_DCR_LS_BIT)
#define ED_DCR_ARM		(1<<ED_DCR_ARM_BIT)
#define ED_DCR_FT0		(1<<ED_DCR_FT0_BIT)
#define ED_DCR_FT1		(1<<ED_DCR_FT1_BIT)

/*
 *  ページ 0、オフセット 0x0f、W
 *  ページ 2、オフセット 0x0f、R
 *  Interrupt Mask Register (IMR)
 */

#define ED_IMR_PRX_BIT		0
#define ED_IMR_PTX_BIT		1
#define ED_IMR_RXE_BIT		2
#define ED_IMR_TXE_BIT		3
#define ED_IMR_OVW_BIT		4
#define ED_IMR_CNT_BIT		5
#define ED_IMR_RDC_BIT		6
#define ED_IMR_RST_BIT		7

#define ED_IMR_PRX		(1<<ED_IMR_PRX_BIT)
#define ED_IMR_PTX		(1<<ED_IMR_PTX_BIT)
#define ED_IMR_RXE		(1<<ED_IMR_RXE_BIT)
#define ED_IMR_TXE		(1<<ED_IMR_TXE_BIT)
#define ED_IMR_OVW		(1<<ED_IMR_OVW_BIT)
#define ED_IMR_CNT		(1<<ED_IMR_CNT_BIT)
#define ED_IMR_RDC		(1<<ED_IMR_RDC_BIT)
#define ED_IMR_RST		(1<<ED_IMR_RST_BIT)

/*
 *  受信フレームヘッダ構造体
 */

typedef struct t_ed_frame_hdr {
	uint8_t		rsr;		/* 受信ステータス	*/
	uint8_t		next;		/* 次のフレームのページ	*/
	uint16_t	count;		/* フレーム長 (長さ+4)	*/
	} T_ED_FRAME_HDR;

/*
 *  内蔵 RAM
 */

/* 注意: 16 ビットモードは未実装 */
#ifdef IF_ED_CFG_16BIT

#define ED_INT_RAM_SIZE		UINT_C(0x4000)

#else	/* of #ifdef IF_ED_CFG_16BIT */

#define ED_INT_RAM_SIZE		UINT_C(0x2000)

#endif	/* of #ifdef IF_ED_CFG_16BIT */

#define ED_INT_RAM_BASE		UINT_C(0x4000)

#define ED_PAGE_SIZE		256
#define NUM_IF_ED_TXBUF_PAGE	((ETHER_MAX_LEN + ED_PAGE_SIZE - 1) / ED_PAGE_SIZE)
#define IF_ED_TXBUF_PAGE_SIZE	(NUM_IF_ED_TXBUF_PAGE * NUM_IF_ED_TXBUF)
#define IF_ED_RXBUF_PAGE_SIZE	(ED_INT_RAM_SIZE / ED_PAGE_SIZE - IF_ED_TXBUF_PAGE_SIZE)

/*
 *  アドレスのオフセット
 */

#define ED_NIC_OFFSET		UINT_C(0x00)
#define ED_ASIC_OFFSET		UINT_C(0x10)
#define ED_DATA_OFFSET		UINT_C(0x00)
#define ED_RESET_OFFSET		UINT_C(0x0f)

/*
 *  RTL8019AS 専用レジスタ
 */

/*
 *  内部レジスタオフセット
 */

/* ページ 3、読み出し */

#define ED_RTL_P3_CR		UINT_C(0x00)
#define ED_RTL_P3_9346CR	UINT_C(0x01)
#define ED_RTL_P3_BPAGE		UINT_C(0x02)
#define ED_RTL_P3_CONFIG0	UINT_C(0x03)
#define ED_RTL_P3_CONFIG1	UINT_C(0x04)
#define ED_RTL_P3_CONFIG2	UINT_C(0x05)
#define ED_RTL_P3_CONFIG3	UINT_C(0x06)
#define ED_RTL_P3_TEST		UINT_C(0x07)
#define ED_RTL_P3_CSNSAV	UINT_C(0x08)
#define ED_RTL_P3_INTR		UINT_C(0x0b)
#define ED_RTL_P3_CONFIG4	UINT_C(0x0d)

/* ページ 3、書込み */

#define ED_RTL_P3_HTLCLK	UINT_C(0x09)
#define ED_RTL_P3_FMWP		UINT_C(0x0c)

/* ページ 3、オフセット 0x04、R/W、Configuration 0 */

#define ED_RTL_CFG0_IOS0_BIT	0
#define ED_RTL_CFG0_IOS1_BIT	1
#define ED_RTL_CFG0_IOS2_BIT	2
#define ED_RTL_CFG0_IOS3_BIT	3
#define ED_RTL_CFG0_IRQS0_BIT	4
#define ED_RTL_CFG0_IRQS1_BIT	5
#define ED_RTL_CFG0_IRQS2_BIT	6
#define ED_RTL_CFG0_IRQEN_BIT	7

#define ED_RTL_CFG0_IOS0	(1<<ED_RTL_CFG0_IOS0_BIT)
#define ED_RTL_CFG0_IOS1	(1<<ED_RTL_CFG0_IOS1_BIT)
#define ED_RTL_CFG0_IOS2	(1<<ED_RTL_CFG0_IOS2_BIT)
#define ED_RTL_CFG0_IOS3	(1<<ED_RTL_CFG0_IOS3_BIT)
#define ED_RTL_CFG0_IRQS0	(1<<ED_RTL_CFG0_IRQS0_BIT)
#define ED_RTL_CFG0_IRQS1	(1<<ED_RTL_CFG0_IRQS1_BIT)
#define ED_RTL_CFG0_IRQS2	(1<<ED_RTL_CFG0_IRQS2_BIT)
#define ED_RTL_CFG0_IRQEN	(1<<ED_RTL_CFG0_IRQEN_BIT)

#endif	/* of #ifndef _If_EDREG_H_ */
