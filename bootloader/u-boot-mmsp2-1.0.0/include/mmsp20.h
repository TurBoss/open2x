/*
 * (C) Copyright 2004
 *
 * this file is written for MMSP20 Linux projects
 * (C) Copyright 2004
 * DIGNSYS Inc. < www.dignsys.com >
 * Kane Ahn < hbahn@dignsys.com >
 *
 */

/************************************************
 * NAME	    : mmsp20.h
 * Version  : 08.1.2004
 *
 * Based on MMSP20 User's manual Rev 0.13
 ************************************************/

#ifndef __MMSP20_H__
#define __MMSP20_H__

#define MMSP20_UART_CHANNELS    4 	

/* MMSP20 only supports 512 Byte HW ECC */
#define MMSP20_ECCSIZE		512
#define MMSP20_ECCBYTES		3

typedef enum {
	MMSP20_UART0,
	MMSP20_UART1,
	MMSP20_UART2,
	MMSP20_UART3
} MMSP20_UARTS_NR;

#define IO_BASE				0xC0000000

/* MMSP20 device base addresses */ 
#define MMSP20_BANC_MEMCTL_BASE		(IO_BASE + 0x3A00)
#define MMSP20_BANA_MEMCTL_BASE		(IO_BASE + 0x3800)
#define MMSP20_DUAL_CPUCTL_BASE 	(IO_BASE + 0x3B00)
#define MMSP20_CLOCK_POWER_BASE		(IO_BASE + 0x0900)
#define MMSP20_INTERRUPT_BASE		(IO_BASE + 0x0800) 
#define MMSP20_DMA_BASE			(IO_BASE + 0x0200)
#define MMSP20_DMAREQ_BASE		(IO_BASE + 0x0100)			
#define MMSP20_UART_BASE		(IO_BASE + 0x1200)
#define MMSP20_TIMER_WATCHDOG_BASE	(IO_BASE + 0x0A00)
#define MMSP20_GPIO_BASE		(IO_BASE + 0x1020)
#define MMSP20_RTC_BASE			(IO_BASE + 0x0C00)

#ifdef CFG_SHADOW     /* Hardare Enviorment */
#define MMSP20_NAND_BASE		0x1C000000
#else
#define MMSP20_NAND_BASE		0x9C000000
#endif

/* include common stuff */
#include <mmsp20_type.h>


static inline MMSP20_BANA_MEMCTL * const MMSP20_GetBaseA_MEMCTL(void)
{
	return (MMSP20_BANA_MEMCTL * const)MMSP20_BANA_MEMCTL_BASE;
}
static inline MMSP20_BANC_MEMCTL * const MMSP20_GetBaseC_MEMCTL(void)
{
	return (MMSP20_BANC_MEMCTL * const)MMSP20_BANC_MEMCTL_BASE;
}
static inline MMSP20_INTERRUPT * const MMSP20_GetBase_INTERRUPT(void)
{
	return (MMSP20_INTERRUPT * const)MMSP20_INTERRUPT_BASE;
}
static inline MMSP20_DMAS * const MMSP20_GetBase_DMAS(void)
{
	return (MMSP20_DMAS * const)MMSP20_DMA_BASE;
}
static inline MMSP20_DMAREQ * const MMSP20_GetBase_DMAREQ(void)
{
	return (MMSP20_DMAREQ * const)MMSP20_DMAREQ_BASE;
}
static inline MMSP20_CLOCK_POWER * const MMSP20_GetBase_CLOCK_POWER(void)
{
	return (MMSP20_CLOCK_POWER * const)MMSP20_CLOCK_POWER_BASE;
}
static inline MMSP20_NAND * const MMSP20_GetBase_NAND(void)
{
	return (MMSP20_NAND * const)MMSP20_NAND_BASE;
}
static inline MMSP20_UART * const MMSP20_GetBase_UART(void)
{
	return (MMSP20_UART * const)MMSP20_UART_BASE;
}
static inline MMSP20_TIMER_WATCHDOG * const MMSP20_GetBase_TIMERS(void)
{
	return (MMSP20_TIMER_WATCHDOG * const)MMSP20_TIMER_WATCHDOG_BASE;
}
static inline MMSP20_GPIO * const MMSP20_GetBase_GPIO(void)
{
	return (MMSP20_GPIO * const)MMSP20_GPIO_BASE;
}
static inline MMSP20_RTC * const MMSP20_GetBase_RTC(void)
{
	return (MMSP20_RTC * const)MMSP20_RTC_BASE;
}

/* ISR */
#define pISR_RESET		(*(unsigned *)(_ISR_STARTADDRESS+0x0))
#define pISR_UNDEF		(*(unsigned *)(_ISR_STARTADDRESS+0x4))
#define pISR_SWI		(*(unsigned *)(_ISR_STARTADDRESS+0x8))
#define pISR_PABORT		(*(unsigned *)(_ISR_STARTADDRESS+0xC))
#define pISR_DABORT		(*(unsigned *)(_ISR_STARTADDRESS+0x10))
#define pISR_RESERVED		(*(unsigned *)(_ISR_STARTADDRESS+0x14))
#define pISR_IRQ		(*(unsigned *)(_ISR_STARTADDRESS+0x18))
#define pISR_FIQ		(*(unsigned *)(_ISR_STARTADDRESS+0x1C))

#define pISR_DISPINT		(*(unsigned *)(_ISR_STARTADDRESS+0x20)) /*00*/
#define pISR_IMGHINT		(*(unsigned *)(_ISR_STARTADDRESS+0x24)) /*01*/
#define pISR_IMGVINT		(*(unsigned *)(_ISR_STARTADDRESS+0x28)) /*02*/
#define pISR_reserved1		(*(unsigned *)(_ISR_STARTADDRESS+0x2C)) /*03*/
#define pISR_reserved2		(*(unsigned *)(_ISR_STARTADDRESS+0x30)) /*04*/
#define pISR_TIMERINT		(*(unsigned *)(_ISR_STARTADDRESS+0x34)) /*05*/
#define pISR_MSTICKINT		(*(unsigned *)(_ISR_STARTADDRESS+0x38)) /*06*/
#define pISR_SSPINT		(*(unsigned *)(_ISR_STARTADDRESS+0x3C)) /*07*/
#define pISR_PPMINT		(*(unsigned *)(_ISR_STARTADDRESS+0x40)) /*08*/
#define pISR_DMAINT		(*(unsigned *)(_ISR_STARTADDRESS+0x44)) /*09*/
#define pISR_UARTINT		(*(unsigned *)(_ISR_STARTADDRESS+0x48)) /*10*/
#define pISR_GRP2DINT		(*(unsigned *)(_ISR_STARTADDRESS+0x4C)) /*11*/
#define pISR_SCALERINT		(*(unsigned *)(_ISR_STARTADDRESS+0x50)) /*12*/
#define pISR_USBHINT		(*(unsigned *)(_ISR_STARTADDRESS+0x54)) /*13*/
#define pISR_SDINT		(*(unsigned *)(_ISR_STARTADDRESS+0x58)) /*14*/
#define pISR_USBDINT		(*(unsigned *)(_ISR_STARTADDRESS+0x5C)) /*15*/
#define pISR_RTCINT		(*(unsigned *)(_ISR_STARTADDRESS+0x60)) /*16*/
#define pISR_ADCINT		(*(unsigned *)(_ISR_STARTADDRESS+0x64)) /*17*/
#define pISR_I2CINT		(*(unsigned *)(_ISR_STARTADDRESS+0x68)) /*18*/
#define pISR_AC97INT		(*(unsigned *)(_ISR_STARTADDRESS+0x6C)) /*19*/
#define pISR_IRDAINT		(*(unsigned *)(_ISR_STARTADDRESS+0x70)) /*20*/
#define pISR_reserved3		(*(unsigned *)(_ISR_STARTADDRESS+0x74)) /*21*/
#define pISR_reserved4		(*(unsigned *)(_ISR_STARTADDRESS+0x78)) /*22*/
#define pISR_GPIOINT		(*(unsigned *)(_ISR_STARTADDRESS+0x7C)) /*23*/
#define pISR_CDROMINT		(*(unsigned *)(_ISR_STARTADDRESS+0x80)) /*24*/
#define pISR_OWMINT		(*(unsigned *)(_ISR_STARTADDRESS+0x84)) /*25*/
#define pISR_DUALCPUINT		(*(unsigned *)(_ISR_STARTADDRESS+0x88)) /*26*/
#define pISR_MCUCINT		(*(unsigned *)(_ISR_STARTADDRESS+0x8C)) /*27*/
#define pISR_VLDINT		(*(unsigned *)(_ISR_STARTADDRESS+0x90)) /*28*/
#define pISR_VIDEOINT		(*(unsigned *)(_ISR_STARTADDRESS+0x94)) /*29*/
#define pISR_MPEGIFINT		(*(unsigned *)(_ISR_STARTADDRESS+0x98)) /*30*/
#define pISR_I2SINT		(*(unsigned *)(_ISR_STARTADDRESS+0x9C)) /*31*/

/* PENDING BIT */
#define BIT_DISPINT		(0x1)
#define BIT_IMGHINT		(0x1<<1)
#define BIT_IMGVINT		(0x1<<2)
#define BIT_reserved1		(0x1<<3)
#define BIT_reserved2		(0x1<<4)
#define BIT_TIMERINT		(0x1<<5)
#define BIT_MSTICKINT		(0x1<<6)
#define BIT_SSPINT		(0x1<<7)
#define BIT_PPMINT		(0x1<<8)
#define BIT_DMAINT		(0x1<<9)
#define BIT_UARTINT		(0x1<<10)
#define BIT_GRP2DINT		(0x1<<11)
#define BIT_SCALERINT		(0x1<<12)
#define BIT_USBHINT		(0x1<<13)
#define BIT_SDINT		(0x1<<14)
#define BIT_USBDINT		(0x1<<15)
#define BIT_RTCINT		(0x1<<16)
#define BIT_ADCINT		(0x1<<17)
#define BIT_I2CINT		(0x1<<18)
#define BIT_AC97INT		(0x1<<19)
#define BIT_IRDAINT		(0x1<<20)
#define BIT_reserved3		(0x1<<21)
#define BIT_reserved4		(0x1<<22)
#define BIT_GPIOINT		(0x1<<23)
#define BIT_CDROMINT		(0x1<<24)
#define BIT_OWMINT		(0x1<<25)
#define BIT_DUALCPUINT		(0x1<<26)
#define BIT_MCUCINT		(0x1<<27)
#define BIT_VLDINT		(0x1<<28)
#define BIT_VIDEOINT		(0x1<<29)
#define BIT_MPEGIFINT		(0x1<<30)
#define BIT_I2SINT		(0x1<<31)
#define BIT_ALLMSK		(0xFFFFFFFF)

#define ClearPending(bit) {\
		 rSRCPND = bit;\
		 rINTPND = bit;\
		 rINTPND;\
		 }
/* Wait until rINTPND is changed for the case that the ISR is very short. */

/* MP2520F NF Control & Status */
#define NAND_READY_DETECT       (1 << 15)
#define NAND_IRQEN              (1 << 7)
#define NAND_RNB                (1 << 8)

#endif /*__MMSP20_H__*/
