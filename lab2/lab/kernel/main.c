#include "common.h"
#include "x86.h"
#include "device.h"

void kEntry(void) {

	// Interruption is disabled in bootloader

	initSerial();// initialize serial port at kernel/serial.c
	initIdt(); // initialize idt at kernel/idt.c
	initIntr(); // iniialize 8259 interrupt controller at kernel/8s59.c
	initSeg(); // initialize gdt, tss at kernel/kvm.c
	initVga(); // initialize vga device at kernel/vga.c
    initKeyTable(); // initialize keyboard device at kernel/keyboard.c
	loadUMain(); // load user program, enter user space at kernel/kvm.c

	while(1);
	assert(0);
}
