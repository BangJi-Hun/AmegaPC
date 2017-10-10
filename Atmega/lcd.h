#ifndef __LCD_HEADER_FILE__
#define __LCD_HEADER_FILE__

#include <util/delay.h>

#define L_ALL_CLEAR		 	 0x01
#define L_RETURN_HOME	 	 0x02
#define L_LINE_FEED		 	 0xC0
#define L_DISPLAY_OFF		 0x08
#define L_DISPLAY_ON		 0x0F
#define L_LCD_FUNCTION_SET   0x38
#define L_ENTRY_MODE_SET	 0x06
#define L_LEFT_SHIFT		 0x18
#define L_STOP_SHIFT		 0x03
#define L_RIGHT_SHIFT		 0x1C
#define L_CUR_L_MOVE_NOT_SHIFT 0x10
#define L_CUR_R_MOVE_NOT_SHIFT 0x14
#define L_CUR_OFF_BLINK_OFF	 0x10
#define L_CUR_ON_BLINK_ON	 0x0F

#define lcd_data *((volatile unsigned char *) 0xC000)
#define lcd_inst *((volatile unsigned char *) 0x8000)

void check_busy(void);
void lcd_cmd(unsigned char cmd);
void lcd_char(char ch);
void lcd_string(char *str);
void lcd_string_delay(char *str);
void cur_move_yx(int y, int x);
void lcd_position( unsigned char x, unsigned char y );
void delay_us(unsigned int time);
void delay_ms(unsigned int time);

#endif

