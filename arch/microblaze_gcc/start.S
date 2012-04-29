/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2002-2008 by Y.D.K Co.,Ltd
 *  Copyright (C) 2012 by Tetsuya Morizumi
 * 
 *  �嵭����Ԥϡ��ʲ���(1)��(4)�ξ������������˸¤ꡤ�ܥ��եȥ���
 *  �����ܥ��եȥ���������Ѥ�����Τ�ޤࡥ�ʲ�Ʊ���ˤ���ѡ�ʣ������
 *  �ѡ������ۡʰʲ������ѤȸƤ֡ˤ��뤳�Ȥ�̵���ǵ������롥
 *  (1) �ܥ��եȥ������򥽡��������ɤη������Ѥ�����ˤϡ��嵭������
 *      ��ɽ�����������Ѿ�浪��Ӳ�����̵�ݾڵ��꤬�����Τޤޤη��ǥ���
 *      ����������˴ޤޤ�Ƥ��뤳�ȡ�
 *  (2) �ܥ��եȥ������򡤥饤�֥������ʤɡ�¾�Υ��եȥ�������ȯ�˻�
 *      �ѤǤ�����Ǻ����ۤ�����ˤϡ������ۤ�ȼ���ɥ�����ȡ�����
 *      �ԥޥ˥奢��ʤɡˤˡ��嵭�����ɽ�����������Ѿ�浪��Ӳ���
 *      ��̵�ݾڵ����Ǻܤ��뤳�ȡ�
 *  (3) �ܥ��եȥ������򡤵�����Ȥ߹���ʤɡ�¾�Υ��եȥ�������ȯ�˻�
 *      �ѤǤ��ʤ����Ǻ����ۤ�����ˤϡ����Τ����줫�ξ�����������
 *      �ȡ�
 *    (a) �����ۤ�ȼ���ɥ�����ȡ����Ѽԥޥ˥奢��ʤɡˤˡ��嵭����
 *        �ɽ�����������Ѿ�浪��Ӳ�����̵�ݾڵ����Ǻܤ��뤳�ȡ�
 *    (b) �����ۤη��֤��̤�������ˡ�ˤ�äơ�TOPPERS�ץ��������Ȥ�
 *        ��𤹤뤳�ȡ�
 *  (4) �ܥ��եȥ����������Ѥˤ��ľ��Ū�ޤ��ϴ���Ū�������뤤���ʤ�»
 *      ������⡤�嵭����Ԥ����TOPPERS�ץ��������Ȥ����դ��뤳�ȡ�
 *      �ޤ����ܥ��եȥ������Υ桼���ޤ��ϥ���ɥ桼������Τ����ʤ���
 *      ͳ�˴�Ť����ᤫ��⡤�嵭����Ԥ����TOPPERS�ץ��������Ȥ�
 *      ���դ��뤳�ȡ�
 * 
 *  �ܥ��եȥ������ϡ�̵�ݾڤ��󶡤���Ƥ����ΤǤ��롥�嵭����Ԥ�
 *  ���TOPPERS�ץ��������Ȥϡ��ܥ��եȥ������˴ؤ��ơ�����λ�����Ū
 *  ���Ф���Ŭ������ޤ�ơ������ʤ��ݾڤ�Ԥ�ʤ����ޤ����ܥ��եȥ���
 *  �������Ѥˤ��ľ��Ū�ޤ��ϴ���Ū�������������ʤ�»���˴ؤ��Ƥ⡤��
 *  ����Ǥ�����ʤ���
 *
 *  @(#) $Id: start.S 6 2012-03-15 12:51:23Z tetsuya $
 */

/*
 *  �����ͥ��б��Υ������ȥ��åץ⥸�塼���Microblaze�ѡ�
 */

#define TOPPERS_MACRO_ONLY
#define UINT_C(val)		(val)		/* uint_t�����������ޥ��� */
#define ULONG_C(val)	(val)		/* ulong_t�����������ޥ��� */
#define CAST(type, val)	(val)		/* �����㥹�Ȥ�Ԥ��ޥ��� */
#include "kernel_impl.h"
        
	.text
	.align  2
	.global start
start:
	/*
	 *  msr������
	 *  �㳰�ϵ���
	 */
	la   r1, r0, (MSR_INIT_VAL|MSR_EE)
	mts  rmsr,r1

	/*
	 * R2��R13������
	 *��R2  : Read-only small data area anchor
	 *   _SDA2_BASE_ .sdata2���濴
	 *��R13 : Read-write small data area anchor
	 *   .sdata��sbss��Ϣ³�����֤���������֤�_SDA_BASE_ �����ꤹ��
	 */
	la   r13, r0, _SDA_BASE_
	la   r2,  r0, _SDA2_BASE_

	/*
	 * �����å��ݥ��󥿤ν����
	 */
	lwi  r1,  r2, _kernel_istkpt

	/*
	 *  _hardware_init_hook ��ƤӽФ���(0�Ǥʤ����)
	 *
	 *  �ϡ��ɥ�������¸��ɬ�פʽ����������������ϡ�
	 *  hardware_init_hook �Ȥ����ؿ����Ѱդ���Ф褤��
	 *  ����Ū�ˤ�ROM���κݡ�RAM����Ѳ�ǽ�ˤ��뤿���
	 *  �Х�����ȥ�����ν��������Ԥ���
	 *  sys_support.S ��� _hardware_init_hook ��������Ƥ�
	 *  �롥
	 */
start_0:
	la    r3,  r0,  hardware_init_hook
	beqi  r3,  start_01
	brald r15, r3
	 nop

start_01:
	/*
	 * �٥����Υ��ԡ�
     * �����ͥ�Υ٥����� 0���Ϥ˥��ԡ�����
	 */
#ifdef VECTOR_COPY
	addk	r5,r0,r0
start_02:
	addik	r3,r5,vectors_reset
	lwi	    r4,r3,0
	swi	    r4,r5,0
	addik	r5,r5,4
	xori	r18,r5,80
	bnei	r18,start_02
#endif /* VECTOR_COPY */

	/*
	 *  bss���������򥯥ꥢ
	 */
start_s1:
    la      r3, r0, __sbss_start
    la      r4, r0, __sbss_end
    sub     r5, r4, r3
    beqi    r5, start_1
start_s2:
    sb      r0, r0, r3
    addi    r3, r3, 1
    sub     r5, r4, r3
    bgti    r5, start_s2

start_1:
	la    r3,  r0,  __bss_start
	la    r4,  r0,  __bss_end
	sub   r5,  r4,  r3
	beqi  r5,  start_3
start_2:
	sb    r0,  r0,  r3
	addi  r3,  r3,  1
	sub   r5,  r4,  r3
	bgti  r5,  start_2

	/*
	 *  data���������ν������ROM���б���
	 *
	 *  __idata_start ���� __idata_end �ޤǤ�__data_start �ʹߤ�
	 *  ���ԡ����롥
	 */
start_3:
#ifndef OMIT_DATA_IN_ROM
	la    r3,  r0,  __idata_start
	la    r4,  r0,  __idata_end
	sub   r5,  r4,  r3     /* ������������å� */
	beqi  r5,  start_5
	la    r6,  r0,  __data_start
start_4:
	lbu   r7,  r0,  r3
	sb    r7,  r0,  r6
	addi  r3,  r3,  1
	addi  r6,  r6,  1
	sub   r5,  r4,  r3
	bgti  r5,  start_4

start_5:
	la    r3,  r0,  __isdata_start
	la    r4,  r0,  __isdata_end
	sub   r5,  r4,  r3     /* ������������å� */
	beqi  r5,  start_9
	la    r6,  r0,  __sdata_start
start_6:
	lbu   r7,  r0,  r3
	sb    r7,  r0,  r6
	addi  r3,  r3,  1
	addi  r6,  r6,  1
	sub   r5,  r4,  r3
	bgti  r5,  start_6
#endif /* OMIT_DATA_IN_ROM */
	/*
	 *  _software_init_hook ��ƤӽФ���0 �Ǥʤ����ˡ�
	 *
	 *  ���եȥ������Ķ����ä˥饤�֥��ˤ˰�¸����ɬ�פʽ������
	 *  ����������ϡ�software_init_hook �Ȥ����ؿ����Ѱդ����
	 *  �褤��
	 */
start_9:
	la    r3,  r0,  software_init_hook
	beqi  r3,  start_10
	brald r15, r3
	nop

start_10:
	brlid	r15, sta_ker
	nop
