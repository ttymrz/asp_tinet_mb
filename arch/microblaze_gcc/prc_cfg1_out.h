/*
 *  @(#) $Id: prc_cfg1_out.h 5 2012-01-14 09:35:55Z tetsuya $
 */

/*
 *		cfg1_out.cをリンクするために必要なスタブの定義
 */

void sta_ker(void){}
void vectors_reset(void){}
STK_T *const	_kernel_istkpt = 0x00;

/* 
 *  オフセットファイルを生成するための定義 
 */ 
const uint8_t   MAGIC_1 = 0x12; 
const uint16_t  MAGIC_2 = 0x1234; 
const uint32_t  MAGIC_4 = 0x12345678;

const TCB	TCB_enatex = {
	{ NULL, NULL },			/* task_queue */
	NULL,					/* p_tinib */
	0U,						/* tstat */
#ifdef TOPPERS_SUPPORT_MUTEX
	0U,						/* bpriority */
#endif /* TOPPERS_SUPPORT_MUTEX */
	0U,						/* priority */
	false,					/* acqeue */
	false,					/* wupque */
	true,					/* enatex */
	0U,						/* texprn */
	NULL,					/* p_winifo */
#ifdef TOPPERS_SUPPORT_MUTEX
	{ NULL, NULL },			/* mutex_queue */
#endif /* TOPPERS_SUPPORT_MUTEX */
#ifdef TOPPERS_SUPPORT_OVRTIM
	0U,						/* leftotm */
#endif /* TOPPERS_SUPPORT_OVRTIM */
	{ NULL, NULL }			/* tskctxb */
};
