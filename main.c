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
    STOPWATCH_IDLE,
    STOPWATCH_RUNNING,
    STOPWATCH_PAUSED
} StopwatchState;

typedef enum {
    CLOCK_IDLE,
    CLOCK_RUNNING,
	/*Note : Stops counting time while you're changing the seconds and minutes */
	CHANGE_SEC,
	CHANGE_MIN
} Min2Sec_ClockState;

typedef struct{
	uint8_t display_count;
	uint16_t ms_count;
	uint16_t sec_count;
	StopwatchState state;
} Stopwatch;

typedef struct{
	uint8_t display_count;
	uint16_t ms_count;
	uint16_t sec_count;
	Min2Sec_ClockState state;
} Min2Sec_Clock;

volatile Stopwatch *main_stopwatch_ptr;
volatile Min2Sec_Clock *main_min2secClock_ptr;

void stop_watch_state(Stopwatch* pStopwatch, Min2Sec_Clock* pMin2sec_clock);
void min2sec_clock_state(Stopwatch* pStopwatch, Min2Sec_Clock* pMin2sec_clock);
void fnd_display(Stopwatch* pStopwatch, Min2Sec_Clock* pMin2sec_clock);
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
	main_min2secClock_ptr->display_count++;
	if(main_stopwatch_ptr->state == STOPWATCH_RUNNING){
		main_stopwatch_ptr->ms_count++;
	}
	if(main_min2secClock_ptr->state == CLOCK_RUNNING){
		main_min2secClock_ptr->ms_count++;
	}
}


int main(void)
{
	/* initialize stopwatch */
	// display_count = 0;
	// ms_count = 0;
	// sec_count = 0;
	// state = STOPWATCH_IDLE
    Stopwatch stopwatch = {0,0,0,STOPWATCH_IDLE};
	main_stopwatch_ptr = &stopwatch;
	
	/* initialize min2sec clock */
	// display_count = 0;
	// ms_count = 0;
	// sec_count = 0;
	// state = CLOCK_IDLE
	Min2Sec_Clock min2sec_clock = {0,0,0,CLOCK_IDLE};
	main_min2secClock_ptr = &min2sec_clock;
	
	init_fnd();
	init_button();
	init_timer0();
	
    while (1) 
    {
		stop_watch_state(&stopwatch,&min2sec_clock);
		min2sec_clock_state(&stopwatch,&min2sec_clock);
		fnd_display(&stopwatch,&min2sec_clock);
    }
}

void min2sec_clock_state(Stopwatch* pStopwatch, Min2Sec_Clock* pMin2sec_clock){
	if(pStopwatch->state == STOPWATCH_IDLE && pMin2sec_clock->state!= CLOCK_IDLE){
		switch (pMin2sec_clock->state){
			case CLOCK_RUNNING:
				// increase clock time
				if(pMin2sec_clock->ms_count >= 1000){
					pMin2sec_clock->ms_count = 0;
					pMin2sec_clock->sec_count++;
					if(main_min2secClock_ptr->sec_count == 600){
						// 10분이되면 초기화
						main_min2secClock_ptr->sec_count = 0;
					}
				}
				// Back to stopwatch mode
				if(get_button(BUTTON1_PIN,BUTTON1)) {
					pStopwatch->state = STOPWATCH_RUNNING;
					pMin2sec_clock->state = CLOCK_IDLE;
				}
				// change second
				else if(get_button(BUTTON2_PIN,BUTTON2)) pMin2sec_clock->state = CHANGE_SEC;
				break;
			case CHANGE_SEC:
				break;
			case CHANGE_MIN:
				break;
		}
	}
	return;
}

void stop_watch_state(Stopwatch* pStopwatch, Min2Sec_Clock* pMin2sec_clock){
	if(pMin2sec_clock->state == CLOCK_IDLE){
		switch(pStopwatch->state){
			case STOPWATCH_IDLE:
			pStopwatch->ms_count = 0;
			pStopwatch->sec_count = 0;
			FND_DIGIT_PORT = 0xff;
			if(get_button(BUTTON1_PIN,BUTTON1)) {
				pStopwatch->state = STOPWATCH_RUNNING;
			}
			break;
			case STOPWATCH_RUNNING:
			if(pStopwatch->ms_count >= 1000){
				pStopwatch->ms_count = 0;
				pStopwatch->sec_count++;
				if(main_stopwatch_ptr->sec_count == 600){
					// 10분이되면 초기화
					main_stopwatch_ptr->sec_count = 0;
				}
			}
			// Switch to Min2Sec_Clock
			if(get_button(BUTTON1_PIN,BUTTON1)) {
				pStopwatch->state = STOPWATCH_IDLE;
				pMin2sec_clock->state = CLOCK_RUNNING;
			}
			else if(get_button(BUTTON2_PIN,BUTTON2)) pStopwatch->state = STOPWATCH_PAUSED;
			break;
			case STOPWATCH_PAUSED:
			// Switch to Min2Sec_Clock
			if(get_button(BUTTON1_PIN,BUTTON1)) {
				pStopwatch->state = STOPWATCH_IDLE;
				pMin2sec_clock->state = CLOCK_RUNNING;
			}
			else if(get_button(BUTTON2_PIN,BUTTON2)) pStopwatch->state = STOPWATCH_RUNNING;
			else if(get_button(BUTTON3_PIN,BUTTON3)) {
				pStopwatch->state = STOPWATCH_IDLE;
				pMin2sec_clock->state = STOPWATCH_IDLE;
			};
			break;
		}
	}
	return;
}

// display the FND
void fnd_display(Stopwatch* pStopwatch, Min2Sec_Clock* pMin2sec_clock){
	if(pStopwatch->state != STOPWATCH_IDLE){
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
		case 2: // 10단위 : 1s마다(0~9)
			FND_DIGIT_PORT = ~0b01000000; // cathode
			FND_DATA_PORT =fnd_font[pStopwatch->sec_count%10];
			break;
		case 4: // 100단위 : 10단위s마다(0~6)
			FND_DIGIT_PORT = ~0b00100000; // cathode
			FND_DATA_PORT = fnd_font[pStopwatch->sec_count/10%6];
			break;
		case 6: // 1000단위 : 분단위=60초마다(0~9)
			FND_DIGIT_PORT = ~0b00010000; // cathode
			FND_DATA_PORT = fnd_font[pStopwatch->sec_count/60%10];
			break;
		}
		digit_position = pStopwatch->display_count; // digit_position = stopwatch->display_count;
		digit_position %= 8; // 다음 표시할 자리수를 준비하고 함수 종료
	}	
	else if(pMin2sec_clock->state != CLOCK_IDLE){
		// shows fnd when in clock mode
		unsigned char fnd_font[] = {~0xc0, ~0xf9, ~0xa4, ~0xb0,~0x99,~0x92,~0x82,~0xd8,~0x80,~0x98,~0x7f};

		static int digit_position = 0; // 자리수 선택  변수 0~3 : 0,1,2,3

		switch(digit_position){
			case 0: // 1단위 : 1s마다(0~9)
			FND_DIGIT_PORT = ~0b10000000; // cathode
			FND_DATA_PORT = fnd_font[pMin2sec_clock->sec_count%10];
			break;
			case 2: // 10단위 : 10단위s마다(0~6)
			FND_DIGIT_PORT = ~0b01000000; // cathode
			FND_DATA_PORT =fnd_font[pMin2sec_clock->sec_count/10%6];
			break;
			case 4: // 100단위 : 분단위=60초마다(0~9)
			FND_DIGIT_PORT = ~0b00100000; // cathode
			FND_DATA_PORT = fnd_font[pMin2sec_clock->sec_count/60%10];
			break;
			case 6: // 1000단위 : 10단위분마다=600초마다(0~6)
			FND_DIGIT_PORT = ~0b00010000; // cathode
			FND_DATA_PORT = fnd_font[pMin2sec_clock->sec_count/600%6];
			break;
		}
		digit_position = pMin2sec_clock->display_count; // digit_position = stopwatch->display_count;
		digit_position %= 8; // 다음 표시할 자리수를 준비하고 함수 종료
	}
}
