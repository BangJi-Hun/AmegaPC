#include <avr/io.h>
#include "usart.h"
/*
void usart0_recv( void ) 
{
	// Serial Port 0�� ���� 1 ���� ����

	while (!(UCSR0A & 0x80))
		;
	return UDR0;	// ���� ���۰� ���� �о ����
}
*/
void usart0_send( unsigned char s_data )
{
	// Serial Port 0�� ���� 1 ���� �۽�

	// ���� ���� ���� �˻� - ���� ���۰� �������� ��ٸ�
	while (!(UCSR0A & 0x20)) // UDRE0 == 1 ?
	//while (!(UCSR0A & (1 << UDRE0)));
		;
	UDR0 = s_data;

}

void usart0_string( char *str )
{
	// Serial Port 0�� ���� ���ڿ� ����

	while(*str) {
		usart0_send(*str);
		str++;
	}

}

void usart0_string_range( char *str, int size )
{
	// Serial Port 0�� ���� ���ڿ� ����
	int i;

	for( i = 0; i < size ; i++ )
	{
		usart0_send(*str);
		str++;
	}

}
