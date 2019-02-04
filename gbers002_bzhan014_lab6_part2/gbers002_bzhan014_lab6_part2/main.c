#include <avr/io.h>
#include <avr/interrupt.h>


// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks
enum States{start, run, pause}state;

unsigned char count = 0x00;
unsigned char button = 0x00;
unsigned char buttonFlag = 0x00;
volatile unsigned char TimerFlag = 0x00;
unsigned char LED;

void flagChange() {
	if (buttonFlag == 0x00) {
		buttonFlag = 0x01;
	}
	else if (buttonFlag == 0x01) {
		buttonFlag = 0x00;
	}
}

void tick() {
	switch(state) { //transitions
		case start:
			state = run;
			break;
		case run:
		button = (~PINA & 0x01);
		if (button == 0x01) {
			flagChange();
		}
			//if (buttonFlag == 0x01) {
				//state = pause;
			//}
			//else if (buttonFlag == 0x00){
				state = run;
			//}
			break;
		//case pause:
			//if (buttonFlag == 0x00) {
				//state = run;
			//}
			//else if (buttonFlag == 0x01) {
				//state = pause;
			//}
			//break;
	}
	switch(state) {
		case run:
		button = (~PINA & 0x01);
		if (button == 0x01) {
			flagChange();
		}
			PORTB = LED;
			//button = (~PINA & 0x01);
			//if (button == 0x01) {
				//flagChange();
			//}
			break;
		//case pause:
			//button = (~PINA & 0x01);
			//if (button == 0x01) {
				//flagChange();
			//}
			//break;
	}
}

void TimerOn() {
	
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;

	//Initialize avr counter
	TCNT1=0;

	_avr_timer_cntcurr = _avr_timer_M;
	// TimerISR will be called every _avr_timer_cntcurr milliseconds

	//Enable global interrupts
	SREG |= 0x80; // 0x80: 1000000
}

void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}

void TimerISR() {
	//button = (~PINA & 0x01);
	//if (button == 0x01) {
		//flagChange();
	//}
	//
	if (buttonFlag == 0x00 && count == 0x00) {
		LED = LED << 1;
		if (LED == 0x08) {
			count = 0x01;
		}
	}
	if (buttonFlag == 0x00 && count == 0x01) {
		LED = LED >> 1;
		if (LED == 0x01) {
			count = 0x00;
		}
	}
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}


int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	state = start;
	LED = 0x01;
	TimerSet(300);
	TimerOn();
	while (1)
	{
		button = (~PINA & 0x01);
		if (button == 0x01) {
			flagChange();
		}
		if (buttonFlag == 0x00) {
			tick();
		}
		button = (~PINA & 0x01);
		if (button == 0x01) {
			flagChange();
		}
	}
}

