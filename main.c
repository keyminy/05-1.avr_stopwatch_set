/*
 * watch_stopwatch_set.c
 *
 * Created: 2024-04-30 오후 4:15:13
 * Author : HARMAN-27
 */ 
#include "def.h"
#include "fnd.h"
#include "button.h"

typedef enum {
	IDLE,
	RUNNING,
	PAUSED
} StopwatchState;

typedef struct{
	uint8_t display_count;
	uint16_t ms_count;
	uint16_t sec_count;
	StopwatchState state;
} Stopwatch;

volatile Stopwatch *main_stopwatch_ptr;

void stop_watch_state(Stopwatch* pStopwatch);
void fnd_display(Stopwatch* pStopwatch);
void init_timer0();


//timer0를 초기화 한다.
void init_timer0(){
	TCNT0 = 6; // 6부터 count하는것임, TCNT : 0~256카운트하기때문에, 정확히 1ms마다 TIMER0_OVF_vect로 진입한다.

	TCCR0 |= (1 << CS02) | (0 << CS01) | (0 << CS00); // (1)분주비 64로 셋팅(p.296 표13-1 참고)
	TIMSK = 1 << TOIE0; // (2) TIMER0 overflow interrupt허용
	sei(); // 전역 인터럽트 허용
}

// 256개의 pulse(=1ms)를 count하면 이곳으로 자동적으로 진입한다.
// 즉, 256개의 pulse == 1ms
ISR(TIMER0_OVF_vect){
	/* 인터럽트 루틴을 가능한 짧게 짜라, ms_count만 증가시키고 빠져나오게함 */
	TCNT0=6; // 6 ~ 256개의 pulse카운트 --> 1ms를 맞춰주기 위해서 TCNT0을 6으로 설정
	main_stopwatch_ptr->display_count++;
	if(main_stopwatch_ptr->state == RUNNING){
		main_stopwatch_ptr->ms_count++;
	}
}


int main(void)
{
	// display_count = 0;
	// ms_count = 0;
	// sec_count = 0;
	// state = IDLE
    Stopwatch stopwatch = {0,0,0,IDLE};
	main_stopwatch_ptr = &stopwatch;
	
	init_fnd();
	init_button();
	init_timer0();
	
    while (1) 
    {
		stop_watch_state(&stopwatch);
		fnd_display(&stopwatch);
    }
}

void stop_watch_state(Stopwatch* pStopwatch){
	switch(pStopwatch->state){
		case IDLE:
			pStopwatch->ms_count = 0;
			pStopwatch->sec_count = 0;
			FND_DIGIT_PORT = 0xff;
			if(get_button(BUTTON1_PIN,BUTTON1)) pStopwatch->state = RUNNING;
			break;
		case RUNNING:
			if(pStopwatch->ms_count >= 1000){
				pStopwatch->ms_count = 0;
				pStopwatch->sec_count++;
				if(main_stopwatch_ptr->sec_count == 600){
					// 10분이되면 초기화
					main_stopwatch_ptr->sec_count = 0;
				}
			}
			if(get_button(BUTTON1_PIN,BUTTON1)) pStopwatch->state = PAUSED;
			else if(get_button(BUTTON2_PIN,BUTTON2)) pStopwatch->state = IDLE;
			break;
		case PAUSED:
			if(get_button(BUTTON1_PIN,BUTTON1)) pStopwatch->state = RUNNING;
			else if(get_button(BUTTON2_PIN,BUTTON2)) pStopwatch->state = IDLE;
			break;
	}
}


// display the FND
void fnd_display(Stopwatch* pStopwatch){
	if(pStopwatch->state != IDLE){
		// 0을 찍을려면 g빼고 1로 16진수값 3F입니다, b는 0011_1111(common anode)
		// common cathode는 1100_0000이고, 16진수로 C0
		//0    1     2     3    4    5    6    7    8    9		dp(10번방)
		unsigned char fnd_font[] = {~0xc0, ~0xf9, ~0xa4, ~0xb0,~0x99,~0x92,~0x82,~0xd8,~0x80,~0x98,~0x7f};

		static int digit_position = 0; // 자리수 선택  변수 0~3 : 0,1,2,3

		switch(digit_position){
		case 0: // 1단위 : 10분의1초 마다(0~9)
			FND_DIGIT_PORT = ~0b10000000; // cathode
			FND_DATA_PORT = fnd_font[pStopwatch->ms_count/100%10];
			break;
		case 1: // 10단위 : 1s마다(0~9)
			FND_DIGIT_PORT = ~0b01000000; // cathode
			FND_DATA_PORT =fnd_font[pStopwatch->sec_count%10];
			break;
		case 2: // 100단위 : 10단위s마다(0~6)
			FND_DIGIT_PORT = ~0b00100000; // cathode
			FND_DATA_PORT = fnd_font[pStopwatch->sec_count/10%6];
			break;
		case 3: // 1000단위 : 분단위=60초마다(0~9)
			FND_DIGIT_PORT = ~0b00010000; // cathode
			FND_DATA_PORT = fnd_font[pStopwatch->sec_count/60%10];
			break;
		}
		digit_position = pStopwatch->display_count; // digit_position = stopwatch->display_count;
		digit_position %= 8; // 다음 표시할 자리수를 준비하고 함수 종료
	}	
}

