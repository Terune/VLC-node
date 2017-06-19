#include <TimerOne.h>
#include <util/atomic.h>

#define FRAMESIZE	38
#define WORD_LENGTH 10
#define HEADER		0xAA
#define SYNC_SYMBOL	0xF0
 #define ETX 0xAA
// #define STX 0x02

//Fast manipulation of LED IO. 
/* Pin configuration registers for atmega328p
 * port B (digital pin 8 to 13)
 * port C (analog input pins)
 * port D (digital pins 0 to 7)
 */
 /* 36: data 4: sync */
#define OUT_LED(x) DDRD |= ((1 << x))	// PIN conf
#define SET_LED(x) PORTD |= ((1 << x))	// LED ON
#define CLR_LED(x) PORTD &= ~((1 << x))	// LED OFF

#define LED1	3
#define SYMBOL_PERIOD 500

unsigned char frame_buffer [FRAMESIZE];
char frame_index = -1;
char frame_size = -1;
unsigned char counter = WORD_LENGTH;

void configFreq(int hz) {
	if (hz) {
		int period = 600000 / hz;
		Timer1.stop();
		Serial.println("Visual light timer stopped");
		delay(1000);
		Timer1.setPeriod(period);
		Timer1.restart();
		Serial.print("Visual light timer restarted at");
		Serial.print(hz);
		Serial.println("Hz");
	}
}

void init_frame(unsigned char * frame) {
	memset(frame, HEADER, 3);
	frame[3] = SYNC_SYMBOL;
	//frame[4] = STX;
	frame_index = -1;
	frame_size = -1;
}

int create_frame(char * data, int data_size, unsigned char * frame) {
	memcpy(&(frame[4]), data, data_size);	// header + sync + stx = 4
	//frame[4+data_size] = ETX;
	return 1;
}

// write data to frame_buffer
int write_data(char * data, int data_size) {
  //Serial.println(data);
	if (frame_index >=  0) {
		return -1;
	}
	if (data_size > 32) {
		Serial.print("write data_size: "); Serial.println(data_size, DEC);
		return -1;
	}
	create_frame(data, data_size,frame_buffer);
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		frame_index = 0;
		frame_size = data_size + 6;
	}
	return 0;
}

void lightemit() {
	if (*frame_buffer & 0x01) {
		SET_LED(LED1);
    //Serial.println("1");
	} else {
		CLR_LED(LED1);
    //Serial.println("0");
	}
	counter --;
	*frame_buffer = (*frame_buffer + 1);
	if (counter == 0) {
		if (frame_index >= 0) {
			if (frame_index < frame_size) {
				frame_index ++;
			} else {
				frame_index = -1;
				frame_size = -1;
			}
		}
		counter = WORD_LENGTH;
	}
}

void setup() {
	// initialize serial communication at 115200 bits per second:
	Serial.begin(115200);
	OUT_LED(LED1);
  
	init_frame(frame_buffer);
	Timer1.initialize(SYMBOL_PERIOD); //1200 bauds
	Timer1.attachInterrupt(lightemit);
	
	Serial.println("light emitter start");
}

char data[120];
char buffer[120];
//const char *msg="1234";

char* msg = "1234";	//Put your message here
int msgSize = strlen(msg);
void loop() {
 
	memcpy(buffer, msg, msgSize +1);
	if (msgSize)
		buffer[msgSize] = '0';
	if (write_data(buffer, msgSize + 1) < 0) {
		delay(1);
	}
}

