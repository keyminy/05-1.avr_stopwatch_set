#include "fnd.h"

void init_fnd(void){
	// PortC를 모두 출력(FND 8pin)으로 하는거니까 모두 write모드로 하기
	FND_DATA_DDR = 0xFF;
	FND_DIGIT_DDR |= 0xF0;// 4,5,6,7만 출력이니까 거기만 write모드
	
	FND_DATA_PORT = 0x00; // PORTC 설정 : FND를 all off
}