#include <avr/io.h>
#include <avr/interrupt.h>


// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks
enum States{start, run}state;

volatile unsigned char TimerFlag = 0x00;
unsigned char LED = 0x00;

void tick() {
	switch(state) { //transitions
		case start:
			state = run;
			break;
		case run:
			state = run;
			break;
	}
	switch(state) {
		case start:
			LED = 0x00;
			PORTB = LED;
			break;
		case run:
			if(LED == 0x00 || LED == 0x04){
				LED = 0x01;
			}
			else {
				LED = LED << 1;	
			}	
			PORTB = LED;
			break;
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
	TimerFlag = 1;
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
	TimerSet(1000);
	TimerOn();
    while (1) 
    {
		tick();
		while(!TimerFlag);
		TimerFlag = 0;
		//PORTB = LED;
    }
}

