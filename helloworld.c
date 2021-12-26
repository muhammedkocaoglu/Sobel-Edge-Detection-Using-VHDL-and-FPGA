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
#include "imageRaw.h"
#include "xil_printf.h"
#include "xuartlite.h"



#define UARTLITE_DEVICE_ID			XPAR_AXI_UARTLITE_0_DEVICE_ID

XUartLite Uartlite;


unsigned int *ipSobelAddr = XPAR_MK_IP_SOBELFILTER_0_S00_AXI_BASEADDR;

int main()
{
    init_platform();
    int Status;
    u32 result_val = 0;
    u8 SendBuffer[4];
    Status = XUartLite_Initialize(&Uartlite, UARTLITE_DEVICE_ID);
	if (Status != XST_SUCCESS) { return XST_FAILURE; }

	int result = 0;
	int cntr = 0;
	unsigned char data0 = 0;
	unsigned char data1 = 0;
	unsigned char data2 = 0;


	unsigned char start = 0;
	unsigned char busy = 0;

	for (int i = 0; i < 478; i++) {

		for (int j = 0; j < 640; j++) {
			XUartLite_ResetFifos(&Uartlite);
			data0 = raw_image[i + 0][j];
			data1 = raw_image[i + 1][j];
			data2 = raw_image[i + 2][j];


			start = 0xab;
			*(ipSobelAddr + 1) = (data2 << 16) + (data1 << 8) + data0;
			*(ipSobelAddr + 0) = start;
			busy = *(ipSobelAddr + 3);
			while(busy == 0x55) {
				start = 0xab;
			}
			result	= *(ipSobelAddr + 2);
			result_val = *(ipSobelAddr + 2);
			SendBuffer[3] = result_val >> 0;
			SendBuffer[2] = result_val >> 8;
			SendBuffer[1] = result_val >> 16;
			SendBuffer[0] = result_val >> 24;
			XUartLite_Send(&Uartlite, SendBuffer, 4);
			while(1) {
				int uart_busy = XUartLite_IsSending(&Uartlite);
				if (uart_busy == 0)
					break;
			}
			start = 0xcd;
			*(ipSobelAddr + 0) = start;
			XUartLite_ResetFifos(&Uartlite);
		}
	}

    cleanup_platform();
    return 0;
}
