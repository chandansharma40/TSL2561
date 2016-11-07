/*
 * TSL2561_Code.c
 *
 * Created: 1/25/2016 3:39:21 PM
 * Author : Chandan
 */ 

#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "UART_1.h"
#include <math.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>

#define SCL 400000L
#define TRUE 1
#define FALSE 0

void i2c_init()
{
	TWBR = (((F_CPU/SCL)-16)/8);
	TWSR = 0;
	TWCR |= (1<<TWEN);
}

void i2c_start()
{
	TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTA);
	while(!(TWCR & (1<<TWINT)));
}

void i2c_stop()
{
	TWCR=(1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
	while(TWCR & (1<<TWSTO));
}

uint8_t i2c_write(uint8_t data)
{
	TWDR=data;
	TWCR=(1<<TWEN)|(1<<TWINT);
	while(!(TWCR & (1<<TWINT)));
	if((TWSR & 0xF8) == 0x18 || (TWSR & 0xF8) == 0x28 || (TWSR & 0xF8) == 0x40)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
	
}

uint8_t i2c_read(uint8_t *data,uint8_t ack)
{
	if(ack)
	{
		TWCR|=(1<<TWEA);
	}
	else
	{
		TWCR&=(~(1<<TWEA));
	}

	TWCR|=(1<<TWINT);
	while(!(TWCR & (1<<TWINT)));
	if((TWSR & 0xF8) == 0x58 || (TWSR & 0xF8) == 0x50)
	{
		*data=TWDR;
		return TRUE;
	}
	else
	return FALSE;
}

uint8_t writedata(uint8_t address,uint8_t *data)
{
	uint8_t res;
	
	i2c_start();
	
	res=i2c_write(0b01110010);
	if(!res)	return FALSE;
	
	res=i2c_write(address);
	
	if(!res)	return FALSE;
	
	res=i2c_write(data);
	
	if(!res)	return FALSE;
	
	i2c_stop();
	
	return TRUE;
}

uint8_t readdata(uint8_t *data, uint8_t address)
{
	uint8_t res;
	i2c_start();

	res=i2c_write(0b01110010);
	
	if(!res)	return FALSE;
	
	res=i2c_write(address);
	if(!res)	return FALSE;
	i2c_start();
	
	res=i2c_write(0b01110011);
	
	if(!res)	return FALSE;
	
	res=i2c_read(data,0);

	if(!res)	return FALSE;
	
	i2c_stop();
	
	return TRUE;
}

int main(void)
{
	DDRD=0b00000010;
	uint8_t h0,l0,h1,l1;
	double data0, data1;
	unsigned char lux_value[7];
	double lux=0.0;
	
	_delay_ms(1000);
	
	UART_1_init();
	
	i2c_init();
	
	uint8_t res;
	
	i2c_start();
	i2c_write(0b01110010);
	i2c_write(0x03);
	i2c_stop();
	
	while (1)
	{
		
		_delay_ms(100); 
		readdata(&l0,0x8C);
		readdata(&h0,0x8D);
		
		_delay_ms(100);
		readdata(&l1,0x8E);
		readdata(&h1,0x8F);
		
		data0 = 256*h0 + l0;
		data1 = 256*h1 + l1;
		
		if((data1/data0) > 0 && (data1/data0) <= 0.50)
		{
			lux = (0.0304*data0) - (0.062*data0*(pow((data1/data0),1.4)));
		}
		
		else if((data1/data0) > 0.50 && (data1/data0) <= 0.61)
		{
			lux = (0.0224*data0) - (0.031*data1);
		}
		
		else if((data1/data0) > 0.61 && (data1/data0) <= 0.80)
		{
			lux = (0.0128*data0) - (0.0153*data1);
		} 
		
		else if((data1/data0) > 0.80 && (data1/data0) <= 1.30)
		{
			lux = (0.00146*data0) - (0.00112*data1);
		}
		
		else
		{
			lux = 0;
		}
		
		dtostrf(lux,4,3,lux_value);
		
		UART_1_puts("Irradiance : ");
		UART_1_puts(lux_value);
		UART_1_putc('\r');
		UART_1_putc('\n');
		
		_delay_ms(1000);
	}
	return 0;
}
