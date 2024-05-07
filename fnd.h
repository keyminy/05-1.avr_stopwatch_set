/*
 * fnd.h
 *
 * Created: 2024-05-07 오후 8:59:28
 *  Author: HARMAN-27
 */ 


#ifndef FND_H_
#define FND_H_

#define F_CPU 16000000UL
#include <avr/io.h> // PORTA DDRA 등의 symbom이 정의 되어 있다.
#include <util/delay.h> // _delay_ms 등의 함수가 들어있다.

#define FND_DATA_DDR DDRC //PortC
#define FND_DATA_PORT PORTC //data Port

#define FND_DIGIT_DDR DDRB //자리수 선택하는것
#define FND_DIGIT_PORT PORTB
#define FND_DIGIT_D1 4 //자리수1번 D1설정 4로
#define FND_DIGIT_D2 5 //자리수2번 D2설정 5로
#define FND_DIGIT_D3 6 //자리수3번 D3설정 6로
#define FND_DIGIT_D4 7 //자리수4번 D4설정 7로


void init_fnd(void);


#endif /* FND_H_ */