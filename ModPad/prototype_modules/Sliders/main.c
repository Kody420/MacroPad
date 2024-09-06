/*
 * Sliders.c
 *
 * Created: 19.08.2024 15:25:32
 * Author : Kody
 */ 

#define F_CPU 8000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include "SPI_AVR8.h"


	//Macros:
		//For ADMUX register to select pin
		#define SLIDER1	0x00
		#define SLIDER2 0x01
		#define SLIDER3 0x02
		#define NUM_OF_SLIDERS 3
		
		#define MODULE_ID 0x01	//max is 0x7f, first bit is reserved for status
		
		#define SPI_STATUS 0x01
		#define SPI_DATA 0x02
		#define SPI_ERROR 0xFF
	//Variables:
	

void SlidersInit(){
	ADMUX |= (1 << REFS0) | (1 << ADLAR);	//AVCC reference with external capacitor at AREF pin, left adjusted result 
	ADCSRA |= (1 << ADEN) | (1 << ADPS2);	//Turn ADC on, division factor at 16
}
			
	
uint8_t* ReadSliders(){
	uint8_t sliders [] = {SLIDER1, SLIDER2, SLIDER3};
	static uint8_t send[NUM_OF_SLIDERS];
	for (uint8_t activeSlider = 0; activeSlider < NUM_OF_SLIDERS; activeSlider++)
	{
		ADMUX = (ADMUX & 0xF0) | (sliders[activeSlider] & 0x0F);
		ADCSRA |= (1 << ADSC);
		while(ADCSRA & (1 << ADSC));
		//Remapping to 0-100
		uint16_t values = ((ADCH >> 1) * 100 + 63) / 127;
		send[activeSlider] = values;
	}
	return send;
}
int main(void)
{
	SlidersInit();
	SPI_Init(SPI_SPEED_FCPU_DIV_8 | SPI_ORDER_MSB_FIRST  | SPI_MODE_SLAVE);
	static uint8_t prevSliderValues[NUM_OF_SLIDERS];
	bool newValues = true;
    while (1) 
    {
		uint8_t* sliderValues = ReadSliders();
		for (int i = 0; i < NUM_OF_SLIDERS; i++)
		{
			if (prevSliderValues[i] != sliderValues[i])
			{
				newValues = true;
				prevSliderValues[i] = sliderValues[i];
			}
		}
		if (!(PINB & (1 << SS)))
		{
			switch(SPDR)
			{
				case SPI_STATUS:
					SPI_SendByte(MODULE_ID | (newValues << 7));
				break;
				case SPI_DATA:
					SPI_SendByte(sliderValues[0]);
					SPI_SendByte(sliderValues[1]);
					SPI_SendByte(sliderValues[2]);
					newValues = false;
				break;
			}
		}
    }
}



