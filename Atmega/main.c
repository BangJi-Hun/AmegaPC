#ifndef TIMER_H_
#define TIMER_H_

#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <util/delay.h>
#include "lcd.h"
#include "led.h"
#include "usart.h"
#include "eeprom.h"

#define TRUE 1
#define FALSE 0

#define Ticks_Per_Sec 500
#define Prescaler 128

volatile unsigned char strLn1[17] = {0};

void EEPROM_write(unsigned int uiAddress, unsigned char ucData);
unsigned char EEPROM_read(unsigned int uiAddress);
void readKeyCount();

char debounce(char last);
volatile unsigned char curSwitch;
volatile unsigned char lastSwitch;
volatile unsigned char isStopped = TRUE;

volatile unsigned int chattering;

volatile unsigned int cnt_1000ms = 0;
volatile unsigned int check_1000ms = FALSE;
volatile unsigned int cnt_3000ms = 0;
volatile unsigned int cnt_4000ms = 0;
volatile unsigned int check_3000ms = FALSE;
volatile unsigned int cnt_10000ms = 0;
volatile unsigned int check_10000ms = TRUE;


volatile int Adc_Value;	
volatile unsigned char voltage[6];
volatile unsigned char button[9] = "00000000";
volatile unsigned char button_check = 0;

volatile unsigned char rx_complete = 0;					// 인터럽트를 이용한 USART0 수신에서 수신 완료 플레그
volatile unsigned char rx0_data;

volatile int hour;
volatile int minute;
volatile int second;
volatile unsigned char time[20];

volatile char buffer[50] = {0, };
volatile char response[10] = {0, };
volatile char packet[50] = {0, };
volatile char data[40] = {0, };
volatile char text[40] = {0, };
volatile char data_size = 0;
volatile char size = 0;
volatile char c;
volatile char conn = 'X';
volatile char conn_check = FALSE;
volatile char ack_check = FALSE;
volatile char *front;
volatile char *start_string;
volatile char *rear;


//ADC 인터럽트

ISR(ADC_vect)  {
	Adc_Value = (int)ADCL;			// Read시 하위 먼저
	Adc_Value = Adc_Value + ((int)ADCH << 8);	// A/D 변환값 읽기
	//ADCSRA = ADCSRA | (1 << ADIF);
	//adc_flag = 1;
}



// 타이머 인터럽트
ISR(TIMER0_OVF_vect) {
	cnt_1000ms++;
	cnt_3000ms++;
	cnt_4000ms++;
	cnt_10000ms++;
	chattering++;

	if(chattering == 250)
		EIMSK = 0x01;
	
	if(cnt_1000ms >= 1000){
		cnt_1000ms = 0;
		second++;
		check_1000ms = TRUE;
	}
	if(cnt_3000ms >= 3000){
		cnt_3000ms = 0;
		check_3000ms = TRUE;
	}
	if(cnt_10000ms >= 10000){
		check_10000ms = TRUE;
	}

	TCNT0 = 256 - 125;
}



// 버튼 인터럽트
ISR(TIMER3_COMPC_vect) {
	if(isStopped!=TRUE){
		Led_Value = ~Led_Value;
		PORTB = Led_Value;
	}
	else
	PORTB = 0xFF;
}

ISR(INT0_vect) {
	//SREG = 0x00;
	curSwitch = PIND;
	lastSwitch = debounce(PIND);
	if(curSwitch != lastSwitch)return;
	isStopped = ~isStopped;
	//SREG = 0x80;         // Global Interrupt Enable
}

ISR( INT2_vect )
{
	curSwitch = PIND;
	lastSwitch = debounce(PIND);
	if(curSwitch != lastSwitch)return;
	if (Key_Count < 99) { // 1~15 Level(15 단계), 최대 75% 유지
		Key_Count++;
	}
	Duty_Ratio = Key_Count;
	Comp_Operation = (unsigned int) (Duty_Ratio / 130.0 * Pwm_Period);
	// Float type을 위해서 100.0을 사용
	OCR3CH = (Comp_Operation >> 8); // PWM Duty Ratio 를 위한 Compare 값 변경
	OCR3CL = Comp_Operation & 0xFF; // 16 비트 Write시에는 High를 먼저하고 Low를 한다.
}

ISR( INT3_vect )
{
	curSwitch = PIND;
	lastSwitch = debounce(PIND);
	if(curSwitch != lastSwitch)return;
	Key_Count--;

	if (Key_Count >= 2) { // 1~15 Level(15 단계), 최대 75% 유지
		Key_Count--;
	}
	
	Duty_Ratio = Key_Count;
	Comp_Operation = (unsigned int) (Duty_Ratio / 130.0 * Pwm_Period);
	// Float type을 위해서 100.0을 사용
	OCR3CH = (Comp_Operation >> 8); // PWM Duty Ratio 를 위한 Compare 값 변경
	OCR3CL = Comp_Operation & 0xFF; // 16 비트 Write시에는 High를 먼저하고 Low를 한다.

}

void timer_init();



// 통신 인터럽트
ISR(USART0_RX_vect) {
	// Old Signal Name = SIG_UART0_RECV
	rx0_data = UDR0;
	rx_complete = 1;
}



int main( void ) {

	SREG=0x00;
	MCUCR = 0x80;

	DDRB  = 0xFF;

	led_init();
	init_lcd();

	start_lcd();
	_delay_ms(3000);
	init_lcd();

	init_timer0();
	init_key_int();
	TWI_init();
	init_ADC();
	usart0_init();
	init_symbol();
	interrupt_init();
	

	SREG |= 0x80;
	
	int i;
	for(i = 0 ; i < 8 ; i ++){
		button[i] = 0x04;
	}
	
	readKeyCount();
	time_load();
	
	conn = 0x06;
	
	while(1){
		if(PIND != 0xFF)
		{
			key_check();
		}	
		PORTB = 0x00;
		_delay_ms(500);
		EIMSK |= (1 << INT2) | (1 << INT3);

		if(check_1000ms == TRUE){
			check_1000ms = FALSE;
			set_time();
			set_ADC();

			if(check_10000ms == TRUE){
				lcd_show();
				readKeyCount();
			}
			else{
				lcd_text_show();
			}
		}

		if(check_3000ms == TRUE)
		{
			check_3000ms = FALSE;
			connection();
		}

		if(cnt_4000ms > 4000){
			conn = 0x06;
		}

		if(rx_complete == 1){
			rx_complete = 0;
			add_buffer();
		}
		readKeyCount();
	}
	return 1;
}

void start_lcd()
{
	lcd_position(1,1);
	lcd_string("20110596");

	lcd_position(2,4);
	lcd_string("JIHUN BANG");
}

 char debounce(char last){
	 char current = PIND;
	 if(last != current)
	 {
		 _delay_ms(200);
		 current = PIND;
	 }
	 return current;
 }


void EEPROM_write(unsigned int uiAddress, unsigned char ucData) {

	  while (EECR & (1<<EEWE));     // 앞의 기록이 완료될 때가지 기다린다.
	  EEAR = uiAddress;                    // 기록할 주소를 EEAR에 지정한다.
	  EEDR = ucData;                        // 데이터를 EEDR에 기록한다.
	  
	  cli();
	  EECR |=(1<<EEMWE);             // EEMWE에 1을 기록한다.
	  EECR |=(1<<EEWE);                // EEWE를 세트 함으로써 EEPROM 기록을 시작.
	  sei();

}

 unsigned char EEPROM_read(unsigned int uiAddress) {

	 while (EECR & (1<<EEWE));    // 이전의 Write가 완료될 때가지 기다린다.
	 EEAR = uiAddress;                   //  읽어 들일 주소를 지정한다.
	 EECR |= (1<<EERE);              // EERE에 1을 기록함으로써 판독 시작한다.
	 return EEDR;                          // EEDR의 데이터를 리턴 한다.
 }

  void readKeyCount()
  {
	  Key_Count = EEPROM_read(4);
	  if(Key_Count >= 255)
	  Key_Count = 1;
  }

  void interrupt_init()
  {
	  EICRA = (1 << ISC01) | (0 << ISC00) | (1 << ISC21) | (0 << ISC20) | (1 << ISC31) | (0 << ISC30);      // INT0,2,3 Trigger 설정: Rising(Low:0, Change:1, Falling:2, Rising:3)
	  EIMSK = (1 << INT0) | (1 << INT2) | (1 << INT3);            // INT0,2,3 Interrupt Enable
  }

  void timer_init()
  {
	  TCCR0 = (1 << CS02) | (0 << CS01) | (1 << CS00);
	  TCNT0 = 256 - (F_CPU/Ticks_Per_Sec/Prescaler);
	  TIMSK = (1 << TOIE0);
	  ETIMSK = (1 << OCIE3C);
  }

  void init_key_int(){
	  EIMSK = 0x01;
	  EICRA = 0x02;

  }
void init_timer0(){
	TCNT0 = 256 - 125;
	TCCR0 = (1 << CS02) | (0 << CS01) | (0 << CS00); // Prescaler 설정(64 분주)
	TIMSK |= (0 << OCIE0) | (1 << TOIE0);	// Timer Overflow Interrupt Enable
}

void init_ADC(void){

	ADMUX = (0 << REFS1) | (1 << REFS0) | (0 << ADLAR) | (0 << MUX4) | (0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (0 << MUX0);
	ADCSRA = (1 << ADEN) | (0 << ADSC) | (1 << ADFR) | (0 << ADIF) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (0 << ADPS0);

	ADCSRA = ADCSRA | (1 << ADSC);

}

void usart0_init(void) {
	// USART0 Polling 방식 초기화

	SREG = 0x00;										// Serial Interrupt를 설정하기 전에 전체 인터럽트를 Disable

	// 정상모드, U2X0 = 0(배속을 Disable)
	UCSR0A = 0;

	// ===> 인터럽트 방식
	// ===> 수신완료 인터럽트 인에이블 : RXCIE0(DB7) = 1
	// 송수신 인에이블 : RXEN0 = TXEN0 = 1
	UCSR0B = 0x98;

	// No Parity, 1비트 Stop Bit, 비동기, 데이터 8비트
	// UMSEL0(동기/비동기) = 0, No Parity = UPM01 = UPM00 = 0, Stop = USBS0 = 0(1비트),
	// UCSZ02,01,00 = 0, 1, 1(8-bit)
	UCSR0C = 0x06;
	//UCSR0C = (0 << UMSEL0) | (0 << UPM01) | (0 << UPM00) | (0 << USBS0) | (1 << UCSZ01) | (1 << UCSZ00);	// With UCSZ02 = 0

	// 통신 속도 설정 : 보 레이트 19200bps
	UBRR0H = 0;
	UBRR0L = 51;
}

// lcd
void init_lcd(){
	MCUCR = ( 1 << SRE );

	lcd_cmd( L_ALL_CLEAR );
	lcd_cmd( L_LCD_FUNCTION_SET );
	lcd_cmd( L_ENTRY_MODE_SET );
	lcd_cmd( L_CUR_L_MOVE_NOT_SHIFT );
	lcd_cmd( L_CUR_OFF_BLINK_OFF );
	lcd_cmd( L_DISPLAY_ON );
}



void lcd_show(){
	lcd_cmd(L_RETURN_HOME);
	lcd_char(conn);
	lcd_char(' ');
	lcd_char(time[0]);
	lcd_char(time[1]);
	lcd_char(':');
	lcd_char(time[2]);
	lcd_char(time[3]);
	lcd_char(':');
	lcd_char(time[4]);
	lcd_char(time[5]);
	lcd_char(' ');

	lcd_position(1,14);
	lcd_char(Key_Count);
	
	lcd_position(2,1);
	lcd_string(voltage);

	EEPROM_write(4, Key_Count);

	lcd_position(2,14);
	lcd_string("RUN");
	
}

void make_ack(){
	response[0] = 0x02;
	response[1] = '1';
	response[2] = '0';
	response[3] = '1';
	response[4] = '1';
	response[5] = '0';
	response[6] = '0';
	response[7] = 0x03;
}

void make_nak(){
	response[0] = 0x02;
	response[1] = '1';
	response[2] = '0';
	response[3] = '1';
	response[4] = '2';
	response[5] = '0';
	response[6] = '0';
	response[7] = 0x03;
}

void reset_buffer(){
	int i;
	for(i = 0; i < 50 ; i++){
		buffer[i] = 0;
	}
	size = 0;
}


int check_data_sum(char error_code){
	int i;
	int data_len = (buffer[5] - '0') * 10 + (buffer[6]-'0');

	char check_sum = data[0];

	for(i = 1 ; i < data_len ; i++){
		check_sum ^= data[i];
	}

	check_sum ^= 0x01;

	if(check_sum == 0x02 || check_sum == 0x03){
		check_sum = 0x04;
	}

	if(check_sum == error_code)
		return TRUE;

	return FALSE;
}

int check_data(){
	int i;
	int data_len = (buffer[5] - '0') * 10 + (buffer[6]-'0');

	if(data_len == 0){
		return TRUE;
	}

	for( i = 0 ; i < data_len ; i++){
		data[i] = buffer[i+7];
	}

	if(i != data_len){
		return FALSE;
	}

	if(check_data_sum(buffer[i+7]) == FALSE){
		return FALSE;
	}

	return TRUE;
}

void response_messes(){
	int i;

	for(i = 0 ; i < 8 ; i++){
		usart0_send(response[i]);
	}
}

void button_handler(){
	int btn = data[0] - '0';
	char temp = 0x01;

	temp = temp << btn;

	if(button[btn] == 0x04){
		button[btn] = 0x03;
		PORTB &= ~temp;
		}else{
		button[btn] = 0x04;
		PORTB |= temp;
	}
	int i;


}

void init_text(){
	int i;
	for(i = 0 ; i < 50 ; i++){
		text[i] = 0;
	}
}

void text_hendler(){
	int i;
	int data_len = (buffer[5] - '0') * 10 + (buffer[6]-'0');

	init_text();
	text[0] = '*';
	text[1] = '*';
	for(i = 0 ; i < data_len; i++){
		text[i+2] = data[i];
	}
	text[i+2] = '*';
	text[i+3] = '*';
	front = text;
	rear = text[i+4];
	cnt_10000ms = 0;
	check_10000ms = FALSE;
}

int check_command(){
	int cmd = (buffer[3]-'0')*10 + (buffer[4]-'0');
	
	switch(cmd){
		case 11:
		//ack
			conn = 0x05;
			cnt_4000ms = 0;
			ack_check = TRUE;
			break;
		case 12:
			ack_check = TRUE;
			break;
		//nck
		case 16:
			//버튼
			button_handler();
			break;
		case 17:
			text_hendler();
			break;
		default:
			return FALSE;
	}
	return TRUE;
}

void check_buffer(){
	make_ack();

	if(check_data() == FALSE){
		make_nak();
		return;
	}
	if(buffer[1] != '0'){
		make_nak();
		return;
	}
	if(buffer[2] != '1'){
		make_nak();
		return;
	}
	if(check_command() == FALSE){
		make_nak();
		return;
	}
}

void add_buffer(){	
	c = rx0_data;

	if(c == 0x02){
		buffer[0] = 0x02;
	}

	if(buffer[0] == 0x02){
		buffer[size] = c;
		size++;
	}

	if(buffer[0] == 0x02 && buffer[size-1] == 0x03){
		check_buffer();
		if(ack_check == FALSE){
			response_messes();
		}
		reset_buffer();
		ack_check = FALSE;
	}
}

void send_massage(int size){
	int i;

	for(i = 0; i <= size ;i++){
		usart0_send(packet[i]);
	}
}


void make_packet(char* cmd, char* size, char* data){
	unsigned char check_sum = 0x01;
	int i;
	int j = 7;
	int data_size = (size[0]-'0')*10 +(size[1] - '0');

	packet[0] = 0x02;
	packet[1] = '1';
	packet[2] = '0';
	packet[3] = cmd[0];
	packet[4] = cmd[1];
	packet[5] = size[0];
	packet[6] = size[1];
	
	for(i = 0 ; i < data_size ; i++){
		packet[j] = data[i];
		check_sum ^= data[i];
		j++;
	}
	if(check_sum == 0x02 | check_sum == 0x03){
		check_sum = 0x04;
	}
	packet[j] = check_sum;
	packet[j+1] = 0x03;
	send_massage(j+1);
}

void connection(){
	make_packet("13", "00", '\0');
}


void set_etime(char* e_time){
	hour = (e_time[0]-'0')*10 + (e_time[1]-'0');
	minute = (e_time[2]-'0')*10 + (e_time[3]-'0');
	second = (e_time[4]-'0')*10 + (e_time[5]-'0');
}

void time_save(){
	int h;
	for(h=1 ; h < 7; h++){
		eeprom_24c_write(0xA0,(unsigned int)h, time[h-1]);
	}
}

void set_time(){

	if(second > 59){
		second = second - 60;
		minute++;
	}
	if(minute > 59){
		minute = 0;
		hour++;
	}

	sprintf(time, "%2.2d%2.2d%2.2d", hour, minute, second);
	time_save();
	make_packet("14", "06", time);
}

void time_load(){
	unsigned char e_data;
	unsigned char e_time[7];
	int h;

	//eeprom_24c_write(0xA0,(unsigned int)0,(unsigned char)('0'));
	e_data = eeprom_24c_read(0xA0,0);
	
	if(e_data == '1'){
		for(h=1 ; h < 7; h++){
			e_time[h-1] = eeprom_24c_read(0xA0,(unsigned int)h);
		}
		set_etime(e_time);
	}
	else{
		eeprom_24c_write(0xA0,0,'1');
		for(h=1 ; h < 7; h++){
			eeprom_24c_write(0xA0,(unsigned int)h,(unsigned char)('0'));
		}
		set_etime("000000");
	}
}

void set_ADC(){
	char vol[3];
	int i_tmp = (float)(Adc_Value * 5) / (float)(1024) * 100;
	int v_value = i_tmp * (0.2);

	if(v_value == 100){
		v_value = 99;
	}

	voltage[0] = 'V';
	voltage[1] = ':';
	voltage[2] = (v_value / 10) + 0x30;
	voltage[3] = (v_value % 10) + 0x30;
	voltage[4] = '%';

	vol[0] = voltage[2];
	vol[1] = voltage[3];

	make_packet("15", "02", vol);
}

void key_check(){

	unsigned char key = ~PIND;
	unsigned char but[8];
	char but_click[2];
	int i;

	for(i = 0; i<8 ; i++){
		but[i] = key & 0x01;
		key = key >> 1;
	}
	for(i = 0; i<8 ; i++){
		if (but[i] == 1){
			but_click[0] = i + 0x30;
			if(button[i] == 0x04){
				button[i] = 0x03;
				PORTB &= PIND;
			}else{
				button[i] = 0x04;
				PORTB |= ~PIND;
			}
		}
	}


	make_packet("16", "01", but_click);
	chattering = 0;
	while(chattering !=250);
}

void lcd_text_show(){
	lcd_cmd(L_RETURN_HOME);
	int i;
	start_string = front;
	for(i = 0 ; i < 17 ; i++){
		if(start_string[0] == 0)
			lcd_char(' ');
		else{
			lcd_char(start_string[0]);
			start_string++;
		}
	}
	if(front[0] != 0){
		front++;
	}
}

void init_symbol(void){


	lcd_cmd(0x58);	lcd_char(0x1f);	lcd_cmd(0x59);	lcd_char(0x1f);	lcd_cmd(0x5a);	lcd_char(0x1f);	lcd_cmd(0x5b);	lcd_char(0x1f);	lcd_cmd(0x5c);	lcd_char(0x1f);	lcd_cmd(0x5d);	lcd_char(0x1f);	lcd_cmd(0x5e);	lcd_char(0x1f);	lcd_cmd(0x5f);	lcd_char(0x1f);

	lcd_cmd(0x60);	lcd_char(0x1f);	lcd_cmd(0x61);	lcd_char(0x11);	lcd_cmd(0x62);	lcd_char(0x11);	lcd_cmd(0x63);	lcd_char(0x11);	lcd_cmd(0x64);	lcd_char(0x11);	lcd_cmd(0x65);	lcd_char(0x11);	lcd_cmd(0x66);	lcd_char(0x11);	lcd_cmd(0x67);	lcd_char(0x1f);

	//		symbol = 켬
	//		0x05
	lcd_cmd(0x68);	lcd_char(0x1d);	lcd_cmd(0x69);	lcd_char(0x07);	lcd_cmd(0x6a);	lcd_char(0x1d);	lcd_cmd(0x6b);	lcd_char(0x0b);	lcd_cmd(0x6c);	lcd_char(0x11);	lcd_cmd(0x6d);	lcd_char(0x1f);	lcd_cmd(0x6e);	lcd_char(0x11);	lcd_cmd(0x6f);	lcd_char(0x1f);
	//		symbol = 끔
	//		0x06
	lcd_cmd(0x70);	lcd_char(0x1b);	lcd_cmd(0x71);	lcd_char(0x09);	lcd_cmd(0x72);	lcd_char(0x09);	lcd_cmd(0x73);	lcd_char(0x1f);	lcd_cmd(0x74);	lcd_char(0x00);	lcd_cmd(0x75);	lcd_char(0x1f);	lcd_cmd(0x76);	lcd_char(0x11);	lcd_cmd(0x77);	lcd_char(0x1f);
	/////////////////
	
}

#endif /* TIMER_H_ */