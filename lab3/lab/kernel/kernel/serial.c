#include "x86.h"
#include "device.h"

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

void putStr(char *str) {
	while((*str) != 0) {
		putChar(*str);
		str++;
	}
}

void putNum(int x) {
	if(x == 0) putChar('0');
	else {
		char s[6];
		int i = 0;
		while(x) {
			s[i++] = x%10 + '0';
			x /= 10;
			if(i > 6) return;
		}
		i--;
		for(; i >= 0; i--) putChar(s[i]);
	}
}