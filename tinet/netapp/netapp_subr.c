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
 *  @(#) $Id: netapp_subr.c,v 1.5 2009/12/24 05:44:56 abe Exp abe $
 */

/* 
 *  ネットワーク応用プログラムサポートルーチン
 */

#include <stdarg.h>
#include <string.h>

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
#include <syssvc/serial.h>
#include "kernel_cfg.h"
#include "tinet_cfg.h"

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <s_services.h>
#include <t_services.h>
#include "kernel_id.h"
#include "tinet_id.h"

#endif	/* of #ifdef TARGET_KERNEL_JSP */

#include <tinet_defs.h>
#include <tinet_config.h>

#include <net/net.h>

#include <netinet/in.h>
#include <netinet/in_itron.h>

#include <netapp/netapp.h>
#include <netapp/netapp_var.h>

#ifdef USE_NETAPP_SUBR

/*
 *  シリアルポートへの書式付文字列出力ライブラリ
 */

#define EOF		(-1)

/*
 *  数値変換のための変換表
 */

const char radhex[] = "0123456789abcdef";
const char radHEX[] = "0123456789ABCDEF";

#ifndef USE_NET_CONS

/*
 *  cons_putchar -- シリアルポートへの文字出力
 */

void
cons_putchar (ID portid, char ch)
{
	serial_wri_dat(portid, &ch, 1);
	}

/*
 *  cons_getchar -- シリアルポートからの文字入力
 */

int_t
cons_getchar (ID portid)
{
	char ch;

	if (serial_rea_dat(portid, &ch, sizeof(ch)) > 0)
		return ch;
	else
		return EOF;
	}

#endif	/* of #ifndef USE_NET_CONS */

/*
 *  cons_putnumber -- cons_printf の数値変換
 */

int_t
cons_putnumber(ID portid, ulong_t 	val, int_t radix,
         const char *radchar, int_t width, bool_t minus, char padchar)
{
	char	digits[24];
	int_t	ix, pad, pchars;
	bool_t	left;

	if (width < 0) {
		width = -width;
		left = true;
		}
	else
		left = false;

	ix = 0;
	do {
		digits[ix ++] = radchar[val % radix];
		val /= radix;
		} while (val != 0);

	if (minus)
		digits[ix ++] = '-';

	if (width > ix)
		pchars = width;
	else
		pchars = ix;

	pad = ix;
	if (!left)	/* 右詰め */
		for ( ; pad < width; pad ++)
			cons_putchar(portid, padchar);

	while (ix -- > 0)
		cons_putchar(portid, digits[ix]);

	if (left)	/* 左詰め */
		for ( ; pad < width; pad ++)
			cons_putchar(portid, padchar);

	return pchars;
	}

#if defined(SUPPORT_INET4)

/*
 *  put_ipv4addr -- IPv4 アドレス出力
 */

int_t
put_ipv4addr (ID portid, T_IN4_ADDR *addr, int_t width)
{
	int_t len = 3;	/* 3 は '.' の文字数 */

	len += cons_putnumber(portid, (*addr >> 24) & 0xff, 10, radhex, 0, false, ' '); 
	cons_putchar(portid, '.');
	len += cons_putnumber(portid, (*addr >> 16) & 0xff, 10, radhex, 0, false, ' '); 
	cons_putchar(portid, '.');
	len += cons_putnumber(portid, (*addr >>  8) & 0xff, 10, radhex, 0, false, ' '); 
	cons_putchar(portid, '.');
	len += cons_putnumber(portid,  *addr        & 0xff, 10, radhex, 0, false, ' '); 

	for ( ; len < width; len ++)
		cons_putchar(portid, ' ');

	return len;
	}

#define PUT_IPADDR(p,a,w)	put_ipv4addr(p,a,w)

#endif	/* of #if defined(SUPPORT_INET4) */

#if defined(SUPPORT_INET6)

/*
 *  ipv6addr -- IPv6 アドレス出力
 */

int_t
put_ipv6addr (ID portid, const T_IN6_ADDR *addr, int_t width)
{
	int_t	len = 0, ix;
	bool_t	omit = false, zero = false;

	if (addr == NULL) {
		cons_putchar(portid, '0');
		cons_putchar(portid, ':');
		cons_putchar(portid, ':');
		cons_putchar(portid, '0');
		len = 4;
		}
	else {
		for (ix = 0; ix < sizeof(T_IN6_ADDR) / 2; ix ++) {
			if (omit) {
				len += cons_putnumber(portid, ntohs(addr->s6_addr16[ix]), 16, radhex, 0, false, ' '); 
				if (ix < 7) {
					cons_putchar(portid, ':');
					len ++;
					}
				}
			else if (ix > 0 && ix < 7 && addr->s6_addr16[ix] == 0)
				zero = true;
			else {
				if (zero) {
					omit = true;
					cons_putchar(portid, ':');
					len ++;
					}
				len += cons_putnumber(portid, ntohs(addr->s6_addr16[ix]), 16, radhex, 0, false, ' '); 
				if (ix < 7) {
					cons_putchar(portid, ':');
					len ++;
					}
				}
			}

		for ( ; len < width; len ++)
			cons_putchar(portid, ' ');
		}
	return len;
	}

#define PUT_IPADDR(p,a,w)	put_ipv6addr(p,a,w)

#endif	/* of #if defined(SUPPORT_INET6) */

/*
 *  put_macaddr -- MAC アドレス出力
 */

int_t
put_macaddr (ID portid, uint8_t *mac, int_t width)
{
	int_t oct, len;

	for (oct = 5; oct -- > 0; ) {
		cons_putnumber(portid, *mac ++, 16, radhex, 2, false, '0'); 
		cons_putchar(portid, ':');
		}
	cons_putnumber(portid, *mac, 16, radhex, 2, false, '0'); 

	for (len = 17; len < width; len ++)
		cons_putchar(portid, ' ');

	return len;
	}

/*
 *  cons_printf -- シリアルポートへの書式付文字列出力
 */

void
cons_printf (ID portid, const char *fmt, ...)
{
	va_list	ap;
	long_t	val;
	char	padchar, *str;
	int_t	ch, width, longflag, left;

#if defined(SUPPORT_INET4)
	T_IN4_ADDR	*addr;
#endif	/* of #if defined(SUPPORT_INET4) */

	va_start(ap, fmt);
	while ((ch = *fmt ++) != '\0') {
		if (ch != '%') {		/* 書式指定以外 */
			cons_putchar(portid, (char)ch);
			continue;
			}

		width = longflag = 0;
		padchar = ' ';

		if (ch == '-') {		/* 左詰め */
			fmt ++;
			left = -1;
			}
		else
			left = 1;

		if ((ch = *fmt ++) == '0') {	/* 上位桁の 0 */
			padchar = '0';
			ch = *fmt ++;
			}

		while ('0' <= ch && ch <= '9') {	/* 出力幅 */
			width = width * 10 + ch - '0';
			ch = *fmt ++;
			}

		while (ch == 'l') {		/* long (long) の指定 */
			longflag ++;
			ch = *fmt ++;
			}

		switch (ch) {
		case 'd':
			val = longflag ? (ulong_t)va_arg(ap, long_t)
			               : (ulong_t)va_arg(ap, int_t);
			if (val >= 0)
				cons_putnumber(portid,  val, 10, radhex, width * left, false, padchar);
			else
				cons_putnumber(portid, -val, 10, radhex, width * left, true, padchar);
			break;

		case 'u':
			val = longflag ? (ulong_t)va_arg(ap, ulong_t)
			               : (ulong_t)va_arg(ap, uint_t);
			cons_putnumber(portid, val, 10, radhex, width * left, false, padchar);
			break;

		case 'x':
			val = longflag ? (ulong_t)va_arg(ap, ulong_t)
			               : (ulong_t)va_arg(ap, uint_t);
			cons_putnumber(portid, val, 16, radhex, width * left, false, padchar);
			break;

		case 'X':
			val = longflag ? (ulong_t)va_arg(ap, ulong_t)
			               : (ulong_t)va_arg(ap, uint_t);
			cons_putnumber(portid, val, 16, radHEX, width * left, false, padchar);
			break;

		case 'c':
			ch = va_arg(ap, int_t);
			cons_putchar(portid, (char)ch);
			break;

		case 's':
			str = va_arg(ap, char*);
			while ((ch = *str ++) != '\0') {
				cons_putchar(portid, (char)ch);
				width --;
				}
			while (width -- > 0)
				cons_putchar(portid, ' ');
			break;

		case 'I':

#if defined(SUPPORT_INET4)

			addr = va_arg(ap, T_IN4_ADDR *);
			put_ipv4addr(portid, addr, width);

#endif	/* of #if defined(SUPPORT_INET4) */

#if defined(SUPPORT_INET6)

			str = va_arg(ap, char*);
			put_ipv6addr(portid, (T_IN6_ADDR *)str, width);

#endif	/* of #if defined(SUPPORT_INET6) */

			break;

		case 'M':
			str = va_arg(ap, char*);
			put_macaddr(portid, str, width);
			break;

		case '%':
			cons_putchar(portid, '%');
			break;

		case '0':
			fmt --;
			break;

		default:
			break;
			}

		}
	va_end(ap);
	}

/*
 *  skip_blanks -- 空白と TAB をスキップする。
 */

char *
skip_blanks (char *line)
{
	while (*line == ' ' || *line == '\t')
		line ++;
	return line;
	}

/*
 *  get_ipv4addr -- 文字列の IPv4 アドレスを T_IN4_ADDR 値に変換する。
 */

char *
get_ipv4addr (T_IN4_ADDR *addr, char *line)
{
	int_t oct;

	*addr = 0;
	while ('0' <= *line && *line <= '9') {
		oct = 0;
		while ('0' <= *line && *line <= '9')
			oct = oct * 10 + (*line ++) - '0';
		*addr = (*addr << 8) | (oct & 0xff);
		if (*line == '.')
			line ++;
		}
	return line;
	}

#if defined(SUPPORT_INET6)

/*
 *  get_ipv6addr -- 文字列の IPv6 アドレスを T_IN6_ADDR 値に変換する。
 */

char *
get_ipv6addr (T_IN6_ADDR *addr, char *line)
{
	int_t word, ix = 0, omit = 0, six;

	memset(addr, 0, sizeof(T_IN6_ADDR));
	while (ix < 8 &&
	       (('0' <= *line && *line <= '9') ||
	        ('a' <= *line && *line <= 'f') ||
	        ('A' <= *line && *line <= 'F'))) {
		word = 0;
		while (('0' <= *line && *line <= '9') ||
 	               ('a' <= *line && *line <= 'f') ||
	               ('A' <= *line && *line <= 'F')) {
			if      ('0' <= *line && *line <= '9')
				word = (word << 4) + (*line ++) - '0';
			else if ('a' <= *line && *line <= 'f')
				word = (word << 4) + (*line ++) - 'a' + 10;
			else if ('A' <= *line && *line <= 'F')
				word = (word << 4) + (*line ++) - 'A' + 10;
			}
		addr->s6_addr16[ix ++] = htons(word);
		if (*line == ':') {
			line ++;
			if (*line == ':') {
				omit = ix;
				line ++;
				}
			}
		}
	if (omit > 0) {
		six = 7;
		while (ix > omit) {
			addr->s6_addr16[six --] = addr->s6_addr16[-- ix];
			addr->s6_addr16[ ix   ] = 0;
			}
		}
	return line;
	}

#endif	/* of #if defined(SUPPORT_INET6) */

/*
 *  デバッグ行入力
 */

int_t
cons_getline (ID portid, char *line, int_t size)
{
	int_t	ch, len;

	len = size;
	while (size > 0) {
		if ((ch = cons_getchar(portid)) != EOF) {
			if (ch == '\r') {
				cons_putchar(portid, '\n');
				break;
				}
			else if (ch == '\b') {
				if (size < len) {
					line --;
					size ++;
					}
				}
			else {
				*line ++ = ch;
				size --;
				}
			}
		}
	*line = '\0';
	return len - size;
	}

/*
 *  get_int -- 整数変換
 */

char *
get_int (int_t *val, char *line)
{
	int_t sign = 1;

	line = skip_blanks(line);
	if (*line == '-') {
		sign = -1;
		line ++;
		}
	else if (*line == '+')
		line ++;

	line = skip_blanks(line);
	*val = 0;
	while ('0' <= *line && *line <= '9') {
		*val = *val * 10 + *line - '0';
		line ++;
		}

	*val = *val * sign;
	return line;
	}

/*
 *  get_xuint -- 符号なし整数 (16進) 変換
 */

char *
get_xuint (uint_t *val, char *line)
{
	line = skip_blanks(line);
	*val = 0;
	while (1) {
		if      ('0' <= *line && *line <= '9')
			*val = (*val << 4) + *line - '0';
		else if ('a' <= *line && *line <= 'f')
			*val = (*val << 4) + *line - 'a' + 10;
		else if ('A' <= *line && *line <= 'F')
			*val = (*val << 4) + *line - 'A' + 10;
		else
			break;
		line ++;
		}
	return line;
	}

#ifdef USE_TCP_EXTENTIONS

const ID vrid_tcp_rep[] =
{
#if NUM_VRID_TCP_REPS >= 1
	TCP_RSV_REPID1,
#endif
#if NUM_VRID_TCP_REPS >= 2
	TCP_RSV_REPID2,
#endif
	};

const ID vrid_tcp_cep[] =
{
#if NUM_VRID_TCP_CEPS >= 1
	TCP_RSV_CEPID1,
#endif
#if NUM_VRID_TCP_CEPS >= 2
	TCP_RSV_CEPID2,
#endif
#if NUM_VRID_TCP_CEPS >= 3
	TCP_RSV_CEPID3,
#endif
#if NUM_VRID_TCP_CEPS >= 4
	TCP_RSV_CEPID4,
#endif
	};

ID tskid_tcp_rep[NUM_VRID_TCP_REPS];
ID tskid_tcp_cep[NUM_VRID_TCP_CEPS];

/*
 *  alloc_tcp_rep -- TCP 受付口を獲得する。
 */

ER
alloc_tcp_rep (ID *repid, ID tskid, T_TCP_CREP *crep)
{
	int_t	ix;

	*repid = TCP_REP_NONE;
	syscall(wai_sem(SEM_ALLOC_TCP_REP_LOCK));
	for (ix = NUM_VRID_TCP_REPS; ix -- > 0; ) {
		if (tskid_tcp_rep[ix] == TSK_NONE) {
			*repid = vrid_tcp_rep[ix];
			tskid_tcp_rep[ix] = tskid;
			break;
			}
		}
	syscall(sig_sem(SEM_ALLOC_TCP_REP_LOCK));

	if (*repid == TCP_REP_NONE)
		return E_NOEXS;
	else
		return TCP_CRE_REP(*repid, crep);
	}

/*
 *  free_tcp_rep -- TCP 受付口を解放する。
 */

ER
free_tcp_rep (ID repid, bool_t call_tcp_del_rep)
{
	int_t	ix;

	syscall(wai_sem(SEM_ALLOC_TCP_REP_LOCK));
	for (ix = NUM_VRID_TCP_REPS; ix -- > 0; ) {
		if (repid == vrid_tcp_rep[ix]) {
			tskid_tcp_rep[ix] = TSK_NONE;
			break;
			}
		}
	syscall(sig_sem(SEM_ALLOC_TCP_REP_LOCK));

	if (call_tcp_del_rep)
		return tcp_del_rep(repid);
	else
		return E_OK;
	}

/*
 *  alloc_tcp_cep -- TCP 通信端点を獲得する。
 */

ER
alloc_tcp_cep (ID *cepid, ID tskid, T_TCP_CCEP *ccep)
{
	int_t	ix;

	*cepid = TCP_CEP_NONE;
	syscall(wai_sem(SEM_ALLOC_TCP_CEP_LOCK));
	for (ix = NUM_VRID_TCP_CEPS; ix -- > 0; ) {
		if (tskid_tcp_cep[ix] == TSK_NONE) {
			*cepid = vrid_tcp_cep[ix];
			tskid_tcp_cep[ix] = tskid;
			break;
			}
		}
	syscall(sig_sem(SEM_ALLOC_TCP_CEP_LOCK));

	if (*cepid == TCP_CEP_NONE)
		return E_NOEXS;
	else
		return tcp_cre_cep(*cepid, ccep);
	}

/*
 *  free_tcp_cep -- TCP 通信端点を解放する。
 */

ER
free_tcp_cep (ID cepid)
{
	int_t	ix;

	syscall(wai_sem(SEM_ALLOC_TCP_CEP_LOCK));
	for (ix = NUM_VRID_TCP_CEPS; ix -- > 0; ) {
		if (cepid == vrid_tcp_cep[ix]) {
			tskid_tcp_cep[ix] = TSK_NONE;
			break;
			}
		}
	syscall(sig_sem(SEM_ALLOC_TCP_CEP_LOCK));

	return tcp_del_cep(cepid);
	}

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

#ifdef USE_UDP_EXTENTIONS

const ID vrid_udp_cep[] =
{
#if NUM_VRID_UDP_CEPS >= 1
	UDP_RSV_CEPID1,
#endif
#if NUM_VRID_UDP_CEPS >= 2
	UDP_RSV_CEPID2,
#endif
	};

ID tskid_udp_cep[NUM_VRID_UDP_CEPS];

/*
 *  alloc_udp_cep -- UDP 通信端点を獲得する。
 */

ER
alloc_udp_cep (ID *cepid, ID tskid, T_UDP_CCEP *ccep)
{
	int_t	ix;

	*cepid = UDP_CEP_NONE;
	syscall(wai_sem(SEM_ALLOC_UDP_CEP_LOCK));
	for (ix = NUM_VRID_UDP_CEPS; ix -- > 0; ) {
		if (tskid_udp_cep[ix] == TSK_NONE) {
			*cepid = vrid_udp_cep[ix];
			tskid_udp_cep[ix] = tskid;
			break;
			}
		}
	syscall(sig_sem(SEM_ALLOC_UDP_CEP_LOCK));

	if (*cepid == UDP_CEP_NONE)
		return E_NOEXS;
	else
		return UDP_CRE_CEP(*cepid, ccep);
	}

/*
 *  free_udp_cep -- UDP 通信端点を解放する。
 */

ER
free_udp_cep (ID cepid, bool_t call_udp_del_cep)
{
	int_t	ix;

	syscall(wai_sem(SEM_ALLOC_UDP_CEP_LOCK));
	for (ix = NUM_VRID_UDP_CEPS; ix -- > 0; ) {
		if (cepid == vrid_udp_cep[ix]) {
			tskid_udp_cep[ix] = TSK_NONE;
			break;
			}
		}
	syscall(sig_sem(SEM_ALLOC_UDP_CEP_LOCK));

	if (call_udp_del_cep)
		return udp_del_cep(cepid);
	else
		return E_OK;
	}

#endif	/* of #ifdef USE_UDP_EXTENTIONS */

#endif	/* of #ifdef USE_NETAPP_SUBR */
