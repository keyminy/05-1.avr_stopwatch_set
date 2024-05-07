/*
 * def.h
 *
 * Created: 2024-05-07 오후 8:58:19
 *  Author: HARMAN-27
 */ 


#ifndef DEF_H_
#define DEF_H_

#define F_CPU 16000000UL
#include <avr/io.h> // PORTA DDRA 등의 symbom이 정의 되어 있다.
#include <util/delay.h> // _delay_ms 등의 함수가 들어있다.
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>

#define LED_DDR DDRA
#define LED_PORT PORTA

#endif /* DEF_H_ */