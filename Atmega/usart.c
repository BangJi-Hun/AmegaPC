#include <avr/io.h>
#include "usart.h"
/*
void usart0_recv( void ) 
{
	// Serial Port 0를 통한 1 문자 수신

	while (!(UCSR0A & 0x80))
		;
	return UDR0;	// 수신 버퍼가 차면 읽어서 리턴
}
*/
void usart0_send( unsigned char s_data )
{
	// Serial Port 0를 통한 1 문자 송신

	// 전송 가능 여부 검사 - 전송 버퍼가 빌때까지 기다림
	while (!(UCSR0A & 0x20)) // UDRE0 == 1 ?
	//while (!(UCSR0A & (1 << UDRE0)));
		;
	UDR0 = s_data;

}

void usart0_string( char *str )
{
	// Serial Port 0를 통한 문자열 전송

	while(*str) {
		usart0_send(*str);
		str++;
	}

}

void usart0_string_range( char *str, int size )
{
	// Serial Port 0를 통한 문자열 전송
	int i;

	for( i = 0; i < size ; i++ )
	{
		usart0_send(*str);
		str++;
	}

}
