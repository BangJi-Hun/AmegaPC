#ifndef __EEPROM__TWI_
#define __EEPROM__TWI_

void TWI_start(void) {
	// Start 조건(Int Flag가 Set되어 있으면 S/W로 Clear하고)
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
}

void TWI_stop(void) {
	// TWI STOP 조건
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
}

void TWI_enable(void) {
	// Enable TWI
	TWCR |= (1 << TWEN);
}

void TWI_disable(void) {
	// Disable TWI
	TWCR &= ~(1 << TWEN);
}

void TWI_wait(void) {
	// TWINT가 1로 설정되기를 기다림
	while (!(TWCR & (1 << TWINT)));

	// 혹은
	// TWI 상태 레지스터를 읽어 TWI 동작(상태) 확인 
	// Check value of TWI Status Register. Mask prescaler bits.
	//if ((TWSR & 0xF8) != START) ERROR();
}

unsigned char TWI_recv(unsigned char r_data) {
	TWCR = (1 << TWINT) | (1 << TWEN);
	TWI_wait();
	// Read data & Return
	return TWDR;

}

void TWI_send(unsigned char s_data) {
	// TWDR 레지스터에 TWI를 통하여 내 보낼 Address 혹은 Data를 Write
	// TWCR 레지스터는 TWINT를 클리어 시키기 위해서 새로 쓴다.
	TWDR = s_data;
	TWCR = (1 << TWINT) | (1 << TWEN);
	TWI_wait();
}

void TWI_init(void) {
	// The SCL period is controlled by settings in the TWI Bit Rate Register (TWBR) and
	// the Prescaler bits in the TWI Status Register (TWSR).
	// SCl Frequency = CPU Frequency / (16 + 2*(TWBR)*4^(TWPS))
	// TWSR Bits 1/0 ? TWPS: TWI Prescaler Bits. 00: 1, 01: 4, 10: 16, 11: 64.(4^TWPS의 값)
	// 12, 0는 CPU가 8MHz인 경우 TWI를 Fast Mode인 400KHZ로 설정
	// 10, 0는 CPU가 3.6MHz인 경우 TWI를 Standard Mode인 100KHZ로 설정
	TWBR = 10;
	TWSR = 0x00;

	//DDRD = 0xFF;					// Port D as All Output
}

void eeprom_24c_write(unsigned char dev_addr, unsigned short int addr, unsigned char ee_data)
{
	unsigned char hi_addr = addr >> 8;
	unsigned char lo_addr = (unsigned char)addr;
	unsigned char sla_w = dev_addr & 0xFE;		//dev_addr & 0xFE;

	//--- Initiate Start Bit
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));			//wait until TWI interrupt occured

	//--- Send SLA_W
	TWDR = sla_w;
	TWCR = 0xC4;							//TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
	while (!(TWCR & (1 << TWINT)));

	//--- Send High Address
	TWDR = hi_addr;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));

	//--- Send Lo Address
	TWDR = lo_addr;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));

	//--- Send Data
	TWDR = ee_data;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));

	//--- Initiate Communication Stop
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);

	delay_ms(6);
}

unsigned char eeprom_24c_read(unsigned char dev_addr, unsigned short int addr)
{
	unsigned char sla_w = dev_addr & 0xFE;
	unsigned char sla_r = dev_addr | 0x01;
	unsigned char uc_tmp;
	unsigned char hi_addr = addr >> 8;
	unsigned char lo_addr = (unsigned char)addr;

	//--- Initiate Start Bit
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));			//wait until TWI interrupt occured

	TWDR = sla_w;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));

	TWDR = hi_addr;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));

	TWDR = lo_addr;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));

	//--- Reading Function
	//--- Sending Start Condition
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));

	//--- SLA_R
	TWDR = sla_r;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));

	//--- Read Data
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
	uc_tmp = TWDR;

	//--- Initiate Communication Stop
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);

//	_delay_ms(1);
	return uc_tmp;
}

#endif
