/*
 *  TINET (TCP/IP Protocol Stack)
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
 *  @(#) $Id: if_emaclitereg.h 9 2012-03-15 12:57:27Z tetsuya $
 */

/*
 * AXI Ethernet Lite MAC (v1.00a)
 */

#ifndef _IF_EMACLITEREG_H_
#define _IF_EMACLITEREG_H_

#define EMACLITE_TX_PING        0x1U
#define EMACLITE_TX_PONG        0x2U
#define EMACLITE_RX_PING        0x4U
#define EMACLITE_RX_PONG        0x8U

#define EMACLITE_BUFFER_OFFSET  0x0800U

#define EMACLITE_TX_BUFFER      0x0000U
#define EMACLITE_MDIOADDR       0x07E4U
#define EMACLITE_MDIOWR         0x07E8U
#define EMACLITE_MDIORD         0x07ECU
#define EMACLITE_MDIOCTRL       0x07F0U
#define EMACLITE_TX_LENGTH      0x07F4U
#define EMACLITE_GIE            0x07F8U  /* Global interrupt register */
#define EMACLITE_TX_CONTROL     0x07FCU
#define EMACLITE_RX_BUFFER      0x1000U
#define EMACLITE_RX_CONTROL     0x17FCU


/* Global Interrupt Enable Register (GIE) */
#define EMACLITE_GIE_GIE        (1U<<31)

/* Transmit Control Register */
#define EMACLITE_TX_LOOP_BACK   (1U<<4)
#define EMACLITE_TX_INT_EN      (1U<<3)
#define EMACLITE_TX_PROGRAM     (1U<<1)
#define EMACLITE_TX_STATUS      (1U<<0)

/* Recieve Control Register */
#define EMACLITE_RX_INT_EN      (1U<<3)
#define EMACLITE_RX_STATUS      (1U<<0)

#endif	/* of #ifndef _IF_EMACLITEREG_H_ */
