#ifndef __USART_HEADER_FILE__
#define __USART_HEADER_FILE__

void usart0_recv( void );
void usart0_send( unsigned char s_data );
void usart0_string( char *str );
void usart0_string_range( char *str, int size );

#endif
