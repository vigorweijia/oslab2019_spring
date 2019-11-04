#include "x86.h"

#define SERIAL_PORT  0x3F8

void initSerial(void) {
	outByte(SERIAL_PORT + 1, 0x00);
	outByte(SERIAL_PORT + 3, 0x80);
	outByte(SERIAL_PORT + 0, 0x01);
	outByte(SERIAL_PORT + 1, 0x00);
	outByte(SERIAL_PORT + 3, 0x03);
	outByte(SERIAL_PORT + 2, 0xC7);
	outByte(SERIAL_PORT + 4, 0x0B);
}

static inline int serialIdle(void) {
	return (inByte(SERIAL_PORT + 5) & 0x20) != 0;
}

void putChar(char ch) {
	while (serialIdle() != TRUE);
	outByte(SERIAL_PORT, ch);
}

void putStr(const char *str) {
	int i = 0;
	while(str[i] != 0) {
		putChar(str[i]);
		i++;
	}
}

void putNum(int num) {
	int sz = 0;
	int buf[20];
	if(num == 0) putChar('0');
	else {
		while(num > 0) {
			buf[sz] = num % 10;
			num /= 10;
			sz++;
		}
		while(sz > 0) {
			putChar(buf[sz-1]+'0');
			sz--;
		}
	}
}

