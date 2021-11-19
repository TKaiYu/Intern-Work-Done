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

#define MUTEX_DEVICE_ID XPAR_MUTEX_0_IF_1_DEVICE_ID
#define MBOX_DEVICE_ID XPAR_MBOX_0_DEVICE_ID
#define printf xil_printf
#define HELLO_SIZE 40
#define TIMEOUT_MAX_COUNT 0xF0000000

#define MSGSIZ 1024

static XMbox Mbox;
XMutex Mutex;

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

	print(RecvMsg);

	return XST_SUCCESS;

}

static int Mailbox_Send(XMbox *MboxInstancePtr, int *message)
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

void R5LedBlink()
{
	int option;
	int Status;

	printf("Enter which LED to blink\r\n");
	scanf("%d", &option);

	Status = Mailbox_Send(&Mbox, &option);
	if (Status != XST_SUCCESS)
		return;
}

int MailboxExample(u16 MboxDeviceId)
{
	int Status;
	int i;

	XMbox_Config *MboxConfig = XMbox_LookupConfig(MboxDeviceId);
	Status = XMbox_CfgInitialize(&Mbox, MboxConfig, MboxConfig -> BaseAddress);
	if (Status != XST_SUCCESS) {
		printf("mailbox not initialized :( \n\r");
	   	return XST_FAILURE;
	}

	XMutex_Config *MutConfig = XMutex_LookupConfig(MUTEX_DEVICE_ID);
	Status = XMutex_CfgInitialize(&Mutex, MutConfig, MutConfig -> BaseAddress);
	if (Status != XST_SUCCESS) {
	   	printf("mutex not initialized :( \n\r");
	   	return XST_FAILURE;
	}

	while (1) {
		printf("Enter your option:\n1. Blink LED\n2. Receive from R5\r\n");
		scanf("%d", &i);
		switch(i) {
		case 1:
			printf("Option 1 chosen.\r\n");
			R5LedBlink();
			break;
		case 2:
			printf("Option 2 chosen.\r\n");
			printf("checking for messages\r\n");
			Status = Mailbox_Receive(&Mbox);
			if (Status != XST_SUCCESS)
				return XST_FAILURE;
		default:
			printf("please choose again\r\n");
			break;
		}
	}

	return XST_SUCCESS;

}

int main()
{
	init_platform();

	if (MailboxExample(MBOX_DEVICE_ID) != XST_SUCCESS) {
		printf("MailboxExample :\t A53 mbox example failed. \r\n");
		printf("MailboxExample :\t Ends.\r\n");
		return XST_FAILURE;
	}

	printf("MailboxExample :\tSuccessfully ran mbox example \r\n");
	printf("MailboxExample :\tEnds for A53.\r\n");

	cleanup_platform();
	return 0;
}
