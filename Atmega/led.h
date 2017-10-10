#ifndef LED_H_
#define LED_H_
#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>

volatile unsigned int Pwm_Period;
volatile unsigned int Duty_Ratio;
volatile unsigned int Comp_Operation;
volatile unsigned char Key_Count;
volatile unsigned char Led_Value;



void led_init() {

	Key_Count = 1;
	Led_Value = 0xFF; // LED를 켜고 시작
	// PWM 주파수=55Hz, 주기=1000ms/55=18.18ms
	// 8MHz를 64분주하면 타이머에 인가되는 신호는 주기는 0.125usx64=8us
	// PWM 주기는 Up과 Down Counting의 합이 됨(2배)으로
	// 18.18ms=18180us, 18180us/8us/2=1136.25
	// 1136로 하면: 1136x16uS=18176us -> 55.02Hz, 1137로 하면: 1126x16uS=18192us -> 54.97Hz.
	Pwm_Period = 1136;
	Duty_Ratio = Key_Count; // 최소로 시작, %, 최소-5%, Key_Count=1, 최대-75%, Key_Count=15)
	
	MCUCR = 0x00;
	DDRB = 0xFF;
	//PORTB = Led_Value; // LED를 켜고 시작

	// TCCR1A : Operation Mode 설정 , Output Compare 출력 설정 .
	// Phase & Frequency Correct PWM Mode = WGM13+WGM12+WGM11+WGM10 = 모드 9 이용 = 1001
	// Mode 9는 OCR1A의 값을 TOP으로 하여 , BOTTOM서 TOP ( OCR1A ) 으로 Up , 다시 BOTTOM으로 Down
	// COM1A1+COM1A0+COM1B1+COM1B0+COM1C1+COM1C0+WGM11+WGM10
	// PWM 채널 B ( OC1B , Pin 16 , PB6 ) 의 출력 단자를 LED에 사용하기 때문에 GPIO로 설정 .
	// TCCR1A = 0x01 , 00 00 ( COM1B1 , COM1B0 ) 00 01 ( WGM11 , 10 )
	// PWM 채널 B의 출력 단자를 PWM 출력으로 사용 - Up Match : Set , Down Match : Clear
	// TCCR1A = 0x21 , 00 11 ( COM1B1 , COM1B0 ) 00 01 ( WGM11 , 10 )
	TCCR3A = 0x01;
	// TCCR1B : Input Capture 설정 , Operation Mode 설정 , Prescaler 설정 ,
	// ICNC1+ICES1+ - +WGM13+WGM12+CS12+CS11+CS10
	// 00+ - +10 ( WGM13 , WGM12 ) +011 ( 64분주 )
	TCCR3B = 0x13;
	// TCCR1C : Force Output Compare for Channel A , B , C ,
	// FOC1A+FOC1B+FOC1C+ - + - + - + - + -
	TCCR3C = 0x00;

	OCR3AH = ( Pwm_Period >> 8 );
	OCR3AL = Pwm_Period & 0xFF;
	
	Comp_Operation = ( unsigned int ) ( Duty_Ratio / 100.0 * Pwm_Period ) ;
	OCR3CH = ( Comp_Operation>> 8 ) ; // PWM Duty Ratio를 위한 Compare 초기값 설정
	OCR3CL = Comp_Operation & 0xFF ; // Timer 1의 Phase and Frequency Correct PWM에서
	// COR1A를 TOP 지정으로 사용한다면 A Channel은 PWM 생성의 목적으로 사용할 수 없음 .
	// ( 2개의 Channel 만 가능 , 3개의 Channel을 사용하려면 TOP 값은 ICR1을 이용하여 설정해야함 )
	//ETIMSK = (1 << OCIE3C);

}




#endif /* LED_H_ */