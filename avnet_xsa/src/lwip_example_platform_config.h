#ifndef __PLATFORM_CONFIG_H_
#define __PLATFORM_CONFIG_H_

#define SELECT_TFTPAPP

#ifdef SELECT_TFTPAPP
#define TFTP_APP
#endif

#if SELECT_STDOUT16550
#define STDOUT_IS_16550
#endif

#if SELECT_USESOFTETH
#if defined (__arm__) && !defined (ARMR5)
#define USE_SOFTETH_ON_ZYNQ 1
#endif
#endif

#ifdef XPAR_XEMACPS_3_BASEADDR
#define PLATFORM_EMAC_BASEADDR XPAR_XEMACPS_3_BASEADDR
#endif
#ifdef XPAR_XEMACPS_2_BASEADDR
#define PLATFORM_EMAC_BASEADDR XPAR_XEMACPS_2_BASEADDR
#endif
#ifdef XPAR_XEMACPS_1_BASEADDR
#define PLATFORM_EMAC_BASEADDR XPAR_XEMACPS_1_BASEADDR
#endif
#ifdef XPAR_XEMACPS_0_BASEADDR
#define PLATFORM_EMAC_BASEADDR XPAR_XEMACPS_0_BASEADDR
#endif

#ifdef XPAR_AXI_TIMER_0_BASEADDR
#define PLATFORM_TIMER_BASEADDR XPAR_AXI_TIMER_0_BASEADDR
#define PLATFORM_TIMER_INTERRUPT_INTR XPAR_AXI_TIMER_0_INTR
#define PLATFORM_TIMER_INTERRUPT_MASK (1 << XPAR_AXI_TIMER_0_INTR)
#endif

#if defined (__arm__) && !defined (ARMR5)
#define PLATFORM_ZYNQ
#endif
#if defined (ARMR5) || (__aarch64__) || (ARMA53_32)
#define PLATFORM_ZYNQMP
#endif

#endif /* __PLATFORM_CONFIG_H_ */
