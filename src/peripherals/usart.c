/* High-level USART control functions.
 * This file implements a simple non-buffered serial channel over USART1,
 * including newlib stub functions.
 *
 * This file is a part of the firmware for the NoteOn Smartpen.
 * Copyright 2014 Nick Ames <nick@fetchmodus.org>. Licensed under the GNU GPLv3.
 * Contains code from the libopencm3 project.                                 */
#include "usart.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/gpio.h>
#include <stdint.h>
static uint8_t UsartEnabled = 0;

/* Initialize USART1. This function sets the baud rate based on the current clock
 * frequency. If the clock is changed this function must be called again. */
void init_usart(void){
	rcc_periph_clock_enable(RCC_USART1);
	rcc_periph_clock_enable(RCC_GPIOA);

	/* Setup GPIO pins for USART1 */
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9);
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO10);
	/* See page 42-43 of the STM32F302x6/x8 datasheet. */
	gpio_set_af(GPIOA, GPIO_AF7, GPIO9);
	gpio_set_af(GPIOA, GPIO_AF7, GPIO10);
	
	usart_set_baudrate(USART1, USART_BAUD);
	usart_set_databits(USART1, 8);
	usart_set_stopbits(USART1, USART_STOPBITS_1);
	usart_set_mode(USART1, USART_MODE_TX_RX);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
	
	usart_enable(USART1);
	UsartEnabled = 1;
}

/* Shutdown USART1 to save power.
 * If write() if called when the USART is shutdown it will behave as if the
 * data was instantly successfully transmitted. If read() is called when the
 * USART is shutdown the USART will be initialized and the read will proceed
 * normally. */
void shutdown_usart(void){
	rcc_periph_clock_disable(RCC_USART1);
	usart_disable(USART1);
	UsartEnabled = 0;
}

/* Newlib read() stub. */
int _read(int file, char *ptr, int len){
	int i;
	char data;

	if(!UsartEnabled){
		init_usart();
	}
	for(i=0;i<len;++i){
		data = usart_recv_blocking(USART1);
		ptr[i] = data;
	}
	return len;
	file=file; /* Suppresses unused parameter warnings. */
}

/* Newlib write() stub. */
int _write(int file, char *ptr, int len){
	int i;
	if(!UsartEnabled){
		return len;
	} else {
		for(i=0;i<len;++i){
			usart_send_blocking(USART1, ptr[i]);
		}
		return len;
	}
	file=file; /* Suppresses unused parameter warnings. */
}