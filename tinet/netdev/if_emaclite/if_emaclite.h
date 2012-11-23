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
 *  @(#) $Id: if_emaclite.h 11 2012-03-28 12:56:09Z tetsuya $
 */

#ifndef _IF_EMACLITE_H_
#define _IF_EMACLITE_H_

/*
 *  NIC の選択マクロ
 */

#define IF_ETHER_NIC_GET_SOFTC()    emaclite_get_softc()
#define IF_ETHER_NIC_WATCHDOG(i)    emaclite_watchdog(i)
#define IF_ETHER_NIC_PROBE(i)       emaclite_probe(i)
#define IF_ETHER_NIC_INIT(i)        emaclite_init(i)
#define IF_ETHER_NIC_READ(i)        emaclite_read(i)
#define IF_ETHER_NIC_RESET(i)       emaclite_reset(i)
#define IF_ETHER_NIC_START(i,o)     emaclite_start(i,o)

#define T_IF_ETHER_NIC_SOFTC        struct t_emaclite_softc

/* IPv6 関係 */

#define IF_ETHER_NIC_IN6_IFID(i,a)  get_mac6_ifid(i,a)  /* インタフェース識別子の設定       */
#define IF_ETHER_NIC_ADDMULTI(s)    emaclite_addmulti(s)      /* マルチキャストアドレスの登録     */

#if !defined(TOPPERS_MACRO_ONLY) && !defined(_MACRO_ONLY)

/*
 *  前方参照
 */

#ifndef T_IF_SOFTC_DEFINED

typedef struct t_if_softc T_IF_SOFTC;

#define T_IF_SOFTC_DEFINED

#endif  /* of #ifndef T_IF_SOFTC_DEFINED */

#ifndef T_NET_BUF_DEFINED

typedef struct t_net_buf T_NET_BUF;
//typedef struct t_net_buf_if_pdu T_NET_BUF;

#define T_NET_BUF_DEFINED

#endif  /* of #ifndef T_NET_BUF_DEFINED */

/*
 *  関数
 */

extern ER emaclite_addmulti(T_IF_SOFTC *ic);
extern T_IF_SOFTC *emaclite_get_softc(void);
extern void emaclite_watchdog(T_IF_SOFTC *ic);
extern void emaclite_probe(T_IF_SOFTC *ic);
extern void emaclite_init(T_IF_SOFTC *ic);
extern void emaclite_reset(T_IF_SOFTC *ic);
extern T_NET_BUF *emaclite_read(T_IF_SOFTC *ic);
extern void emaclite_start(T_IF_SOFTC *ic, T_NET_BUF *output);
extern void if_emaclite_isr(intptr_t);

#endif /* #if !defined(TOPPERS_MACRO_ONLY) && !defined(_MACRO_ONLY) */

#endif  /* of #ifndef _IF_EMACLITE_H_ */
