/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xmbox.h"
#include "xstatus.h"
#include <string.h>
#include "xmutex.h"
#include "xgpio.h"
#include "xscugic.h"
#include "xil_exception.h"

#define MUTEX_DEVICE_ID XPAR_MUTEX_0_IF_1_DEVICE_ID
#define MBOX_DEVICE_ID XPAR_MBOX_1_DEVICE_ID
#define printf xil_printf
#define MUTEX_NUM 0

int toggle;
#define LED_DELAY 100000
#define MSGSIZ 1024
#define HELLO_SIZE 40
#define TEST_BUFFER_SIZE 16
#define TIMEOUT_MAX_COUNT 0xF0000000

char *sw4 = "Switch 4 pressed from R5!\r\n";
char *sw3 = "Switch 3 pressed from R5!\r\n";
char *sw2 = "Switch 2 pressed from R5!\r\n";
#define LED_CHANNEL 2
#define SW_INT XGPIO_IR_CH1_MASK
#define GPIO_0_INTERRUPT_ID XPAR_FABRIC_AXI_GPIO_0_IP2INTC_IRPT_INTR

static XMbox Mbox;
XMutex Mutex;
XGpio sw;
XGpio led;
XScuGic gic;
static XScuGic_Config *IntcConfig;

char RecvMsg[MSGSIZ] __attribute__ ((aligned(4)));

static int Mailbox_Receive(XMbox *MboxInstancePtr)
{
	int Status;
	u32 Nbytes;
	u32 BytesRcvd;
	int Timeout;

	Nbytes = 0;
	Timeout = 0;

	while (Nbytes < HELLO_SIZE) {
		Status = XMbox_Read(MboxInstancePtr, (u32 *)(RecvMsg + Nbytes), HELLO_SIZE - Nbytes, &BytesRcvd);

		if (Status == XST_SUCCESS)
			Nbytes += BytesRcvd;

		if (Timeout++ > TIMEOUT_MAX_COUNT)
			return XST_FAILURE;
	}

	int message = *RecvMsg;
	switch (message) {
	case 1:
		XGpio_DiscreteWrite(&led, LED_CHANNEL, 0x1); // D14: ON
		break;
	case 2:
		XGpio_DiscreteWrite(&led, LED_CHANNEL, 0x2); // D13: ON
		break;
	case 3:
		XGpio_DiscreteWrite(&led, LED_CHANNEL, 0x4); // D12: ON
		break;
	}

	return XST_SUCCESS;
}

static int Mailbox_Send(XMbox *MboxInstancePtr, char *message)
{
	int Status;
	u32 Nbytes;
	u32 BytesSent;

	Nbytes = 0;

	while (Nbytes != HELLO_SIZE) {
		Status = XMbox_Write(MboxInstancePtr, (u32 *)((u8 *)message + Nbytes), HELLO_SIZE - Nbytes, &BytesSent);
		if (Status == XST_SUCCESS)
			Nbytes += BytesSent;
	}

	return XST_SUCCESS;
}

void SW_Irq_Handler(void *gpio)
{
	int Status;

	//disable GPIO Interrupts
	XGpio_InterruptDisable(gpio, SW_INT);

	//ignore addition button press
	if ((XGpio_InterruptGetStatus(gpio) & SW_INT) != SW_INT)
		return;

	int val = XGpio_DiscreteRead(gpio, 1);
	switch(val) {
	case 0x4:
		XGpio_DiscreteWrite(&led, LED_CHANNEL, 0x4); // D14: ON
		Status = Mailbox_Send(&Mbox, sw4);
		if (Status != XST_SUCCESS)
			return;
		break;
	case 0x2:
		XGpio_DiscreteWrite(&led, LED_CHANNEL, 0x2); // D13: ON
		Status = Mailbox_Send(&Mbox, sw3);
		if (Status != XST_SUCCESS)
			return;
		break;
	case 0x1:
		XGpio_DiscreteWrite(&led, LED_CHANNEL, 0x1); // D12: ON
		Status = Mailbox_Send(&Mbox, sw2);
		if (Status != XST_SUCCESS)
			return;
		break;
	}

	//clear the interrupt bit
	XGpio_InterruptClear(gpio, SW_INT);
	//enable gpio interrupts
	XGpio_InterruptEnable(gpio, SW_INT);
}

int SetUpInterruptSystem(XScuGic *XScuGicInstancePtr)
{
	//connect the interrupt controller interrupt handler to the hardware interrupt handling logic in the arm processor
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler) XScuGic_InterruptHandler, XScuGicInstancePtr);

	//enable interrupt in arm
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

int MailboxExample(u16 MboxDeviceId)
{
	int Status;

	//initialize switch 2 to 4
	Status = XGpio_Initialize(&sw, XPAR_GPIO_0_DEVICE_ID);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	//setup gpio direction to IN
	XGpio_SetDataDirection(&sw, 1, 0xff);

	//initialize LEDs
	Status = XGpio_Initialize(&led, XPAR_GPIO_0_DEVICE_ID);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	//setup gpio direction to OUT
	XGpio_SetDataDirection(&led, 2, 0xff);

	XMbox_Config *MboxConfig = XMbox_LookupConfig(MboxDeviceId);
	Status = XMbox_CfgInitialize(&Mbox, MboxConfig, MboxConfig -> BaseAddress);
	if (Status != XST_SUCCESS) {
		printf("mbox not initialized :( \n\r");
	   	return XST_FAILURE;
	}

	XMutex_Config *MutConfig = XMutex_LookupConfig(MUTEX_DEVICE_ID);
	Status = XMutex_CfgInitialize(&Mutex, MutConfig, MutConfig -> BaseAddress);
	if (Status != XST_SUCCESS) {
	   	printf("mutex not initialized :( \n\r");
	   	return XST_FAILURE;
	}

	//interrupt controller initialization
	IntcConfig = XScuGic_LookupConfig(XPAR_SCUGIC_0_DEVICE_ID);
	Status = XScuGic_CfgInitialize(&gic, IntcConfig, IntcConfig -> CpuBaseAddress);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	//register interrupt handler
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler) XScuGic_InterruptHandler, &gic);
	Xil_ExceptionEnable();
	//perform self-test
	Status = XScuGic_SelfTest(&gic);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	//setup interrupt system
	Status = SetUpInterruptSystem(&gic);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XScuGic_Connect(&gic, GPIO_0_INTERRUPT_ID, (Xil_ExceptionHandler) SW_Irq_Handler, (void *) &sw);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	//enable gpio interrupts
	XGpio_InterruptEnable(&sw, SW_INT);
	XGpio_InterruptGlobalEnable(&sw);
	//enable gpio interrupts in the controller
	XScuGic_Enable(&gic, GPIO_0_INTERRUPT_ID);

	while (1) {
		Status = Mailbox_Receive(&Mbox);
//		XMutex_Lock(&Mutex, MUTEX_NUM);
//		usleep(LED_DELAY);
//		XMutex_Unlock(&Mutex, MUTEX_NUM);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;
	}

	return XST_SUCCESS;

}

int main()
{
	if (MailboxExample(MBOX_DEVICE_ID) != XST_SUCCESS) {
		printf("MailboxExample :\t R5 mbox example failed. \r\n");
		printf("MailboxExample :\t Ends.\r\n");
		return XST_FAILURE;
	}

	printf("MailboxExample :\tSuccessfully ran mbox example \r\n");
	printf("MailboxExample :\tEnds for R5.\r\n");

}
