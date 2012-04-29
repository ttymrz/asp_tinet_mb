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
 *  @(#) $Id: echo.h,v 1.5 2009/12/24 05:44:56 abe Exp abe $
 */

#ifndef _ECHO_H_
#define _ECHO_H_

/* 
 *  ECHO クライアント・サーバのスタックサイズ
 */

#define TCP_ECHO_SRV_STACK_SIZE		1024		/* TCP ECHO サーバタスクのスタックサイズ		*/

#ifdef TOPPERS_S810_CLG3_85
#define TCP_ECHO_CLI_RCV_STACK_SIZE	512		/* TCP ECHO クライアント受信タスクのスタックサイズ	*/
#define TCP_ECHO_CLI_SND_STACK_SIZE	512		/* TCP ECHO クライアント送信タスクのスタックサイズ	*/
#else	/* of #ifdef TOPPERS_S810_CLG3_85 */
#define TCP_ECHO_CLI_RCV_STACK_SIZE	1024		/* TCP ECHO クライアント受信タスクのスタックサイズ	*/
#define TCP_ECHO_CLI_SND_STACK_SIZE	1024		/* TCP ECHO クライアント送信タスクのスタックサイズ	*/
#endif	/* of #ifdef TOPPERS_S810_CLG3_85 */

#define UDP_ECHO_SRV_STACK_SIZE		1024		/* UDP ECHO サーバタスクのスタックサイズ		*/
#define UDP_ECHO_CLI_STACK_SIZE		1024		/* UDP ECHO クライアントタスクのスタックサイズ	*/

/* 
 *  ECHO クライアント・サーバの優先度
 */

#define TCP_ECHO_SRV_MAIN_PRIORITY	5		/* TCP ECHO サーバタスクの優先度			*/
#define TCP_ECHO_CLI_RCV_MAIN_PRIORITY	5		/* TCP ECHO クライアント受信タスクの優先度		*/
#define TCP_ECHO_CLI_SND_MAIN_PRIORITY	5		/* TCP ECHO クライアント送信タスクの優先度		*/

#define UDP_ECHO_SRV_MAIN_PRIORITY	5		/* UDP ECHO サーバタスクの優先度			*/
#define UDP_ECHO_CLI_MAIN_PRIORITY	5		/* UDP ECHO クライアントタスクの優先度		*/

/*
 *  TCP 送受信ウインドバッファサイズ
 */

#if defined(NUM_MPF_NET_BUF_IF_PDU) && NUM_MPF_NET_BUF_IF_PDU > 0

#define TCP_ECHO_CLI_SWBUF_SIZE		((IF_MTU-(IP_HDR_SIZE+TCP_HDR_SIZE))*1)
#define TCP_ECHO_CLI_RWBUF_SIZE		((IF_MTU-(IP_HDR_SIZE+TCP_HDR_SIZE))*1)

#define TCP_ECHO_SRV_SWBUF_SIZE		((IF_MTU-(IP_HDR_SIZE+TCP_HDR_SIZE))*2)
#define TCP_ECHO_SRV_RWBUF_SIZE		((IF_MTU-(IP_HDR_SIZE+TCP_HDR_SIZE))*2)

#else	/* of #if defined(NUM_MPF_NET_BUF_IF_PDU) && NUM_MPF_NET_BUF_IF_PDU > 0 */

#if defined(SUPPORT_INET4)

#define TCP_ECHO_CLI_SWBUF_SIZE		(TCP_MSS)
#define TCP_ECHO_CLI_RWBUF_SIZE		(TCP_MSS)

#define TCP_ECHO_SRV_SWBUF_SIZE		(TCP_MSS)
#define TCP_ECHO_SRV_RWBUF_SIZE		(TCP_MSS)

#endif	/* of #if defined(SUPPORT_INET4) */

#if defined(SUPPORT_INET6)

#define TCP_ECHO_CLI_SWBUF_SIZE		(TCP6_MSS)
#define TCP_ECHO_CLI_RWBUF_SIZE		(TCP6_MSS)

#define TCP_ECHO_SRV_SWBUF_SIZE		(TCP6_MSS)
#define TCP_ECHO_SRV_RWBUF_SIZE		(TCP6_MSS)

#endif	/* of #if defined(SUPPORT_INET6) */

#endif	/* of #if defined(NUM_MPF_NET_BUF_IF_PDU) && NUM_MPF_NET_BUF_IF_PDU > 0 */

/*
 *  タスク数
 */

#if !(defined(USE_TCP_ECHO_SRV1) && !defined(USE_TCP_NON_BLOCKING) && defined(USE_COPYSAVE_API))

#undef  NUM_TCP_ECHO_SRV_TASKS
#define NUM_TCP_ECHO_SRV_TASKS		1

#endif	/* of #if !(defined(USE_TCP_ECHO_SRV1) && !defined(USE_TCP_NON_BLOCKING) && defined(USE_COPYSAVE_API)) */

/*
 *  変数
 */

extern bool_t	tcp_echo_cli_valid;
extern bool_t	udp_echo_cli_valid;

/* TCP 送受信バッファ */

extern uint8_t tcp_echo_cli_swbuf[];
extern uint8_t tcp_echo_cli_rwbuf[];

#ifdef _TCP_H_

extern uint8_t tcp_echo_srv_swbuf[NUM_TCP_ECHO_SRV_TASKS][TCP_ECHO_SRV_SWBUF_SIZE];
extern uint8_t tcp_echo_srv_rwbuf[NUM_TCP_ECHO_SRV_TASKS][TCP_ECHO_SRV_RWBUF_SIZE];

#endif	/* of #ifdef _TCP_H_ */

/*
 *  タスク
 */

extern void	tcp_echo_srv_task(intptr_t exinf);
extern void	tcp_echo_srv_rcv_task(intptr_t exinf);
extern void	tcp_echo_srv_snd_task(intptr_t exinf);
extern void	tcp_echo_cli_rcv_task(intptr_t exinf);
extern void	tcp_echo_cli_snd_task(intptr_t exinf);

extern void	udp_echo_srv_task(intptr_t exinf);
extern void	udp_echo_cli_task(intptr_t exinf);

/*
 *  関数
 */

extern ER callback_nblk_tcp_echo_srv (ID cepid, FN fncd, void *p_parblk);
extern ER callback_nblk_tcp_echo_cli (ID cepid, FN fncd, void *p_parblk);

extern ER callback_udp_echo_srv (ID cepid, FN fncd, void *p_parblk);
extern ER callback_udp_echo_cli (ID cepid, FN fncd, void *p_parblk);

extern ER callback_nblk_udp_echo_srv (ID cepid, FN fncd, void *p_parblk);
extern ER callback_nblk_udp_echo_cli (ID cepid, FN fncd, void *p_parblk);

#endif	/* of #ifndef _ECHO_H_ */
