#include "lcd.h"

void check_busy(void) 
{
	delay_us(80);
	while( ((lcd_inst) & 0x80) ) ;
}

void lcd_cmd(unsigned char cmd) 
{
	check_busy();
	lcd_inst = cmd;
}

void lcd_char(char ch) 
{
	check_busy();
	lcd_data = ch;
}

void lcd_string(char *str) 
{
	while (*str) {
		lcd_char(*str);
		str++;
	}
}

void lcd_string_delay(char *str) 
{
	while(*str) {
		lcd_char(*str);
		str++;
		delay_ms(50);
	}
}

void cur_move_yx(int y, int x) 
{
	lcd_cmd(0x80+(0x40*y+x));
}

void lcd_position( unsigned char y, unsigned char x )
{
	unsigned char position;
	if( x > 16 ) x = 16;
	if( y > 2 ) y = 2;
	position = 0x40 * (y - 1) + (x - 1);
	check_busy();
	lcd_cmd( position | 0x80 );
}

void delay_us(unsigned int time)
{
	register unsigned int i;

	for(i=0; i<time; i++) {
		_delay_us(1.0);
	}
}

void delay_ms(unsigned int time)
{
	register unsigned int i;

	for(i=0; i<time; i++) {
		_delay_ms(1.0);
	}
}



