#include "x86.h"
#include "device.h"

extern TSS tss;
extern ProcessTable pcb[MAX_PCB_NUM];
extern int current;

extern int displayRow;
extern int displayCol;

void GProtectFaultHandle(struct StackFrame *sf);

void timerHandle(struct StackFrame *sf);

void syscallHandle(struct StackFrame *sf);

void syscallWrite(struct StackFrame *sf);
void syscallPrint(struct StackFrame *sf);
void syscallFork(struct StackFrame *sf);
void syscallExec(struct StackFrame *sf);
void syscallSleep(struct StackFrame *sf);
void syscallExit(struct StackFrame *sf);

void irqHandle(struct StackFrame *sf) { // pointer sf = esp
	/* Reassign segment register */
	asm volatile("movw %%ax, %%ds"::"a"(KSEL(SEG_KDATA)));
	/*XXX Save esp to stackTop */
	uint32_t tmpStackTop = pcb[current].stackTop;
	pcb[current].prevStackTop = pcb[current].stackTop;
	pcb[current].stackTop = (uint32_t)sf;

	switch(sf->irq) {
		case -1:
			break;
		case 0xd:
			GProtectFaultHandle(sf);
			break;
		case 0x20:
			timerHandle(sf);
			break;
		case 0x80:
			syscallHandle(sf);
			break;
		default:assert(0);
	}
	/*XXX Recover stackTop */
	pcb[current].stackTop = tmpStackTop;
}

void GProtectFaultHandle(struct StackFrame *sf) {
	assert(0);
	return;
}

void timerHandle(struct StackFrame *sf) {
	int i;
	int flag = 0;
	//uint32_t tmpStackTop;
	//i = (current+1) % MAX_PCB_NUM;
	if(pcb[current].state == STATE_RUNNING) {
		for(i = 0; i < MAX_PCB_NUM; i++) {
    // make blocked processes sleep time -1, sleep time to 0, re-run
			if(pcb[i].state == STATE_BLOCKED) {
				pcb[i].sleepTime--;
				if(pcb[i].sleepTime <= 0) {
					pcb[i].state = STATE_RUNNABLE;
					pcb[i].timeCount = 0;
					pcb[i].sleepTime = 0;
				}
			}
		}
    // time count not max, process continue
  	pcb[current].timeCount++;
		if(pcb[current].timeCount < MAX_TIME_COUNT) return;
	  // else switch to another process
		pcb[current].state = STATE_RUNNABLE;
		pcb[current].timeCount = 0;
	}
	for(i = (current+1)%MAX_PCB_NUM; i != current; i = (i+1)%MAX_PCB_NUM) {
		if(pcb[i].state == STATE_RUNNABLE && i != 0) {
			current = i;	
			pcb[current].state = STATE_RUNNING;
			pcb[current].timeCount = 0;
			flag = 1;
			break;
		}
	}
	
		/* echo pid of selected process */
	if(flag == 0 && pcb[current].state != STATE_RUNNING) {
		current = 0;
		pcb[current].state = STATE_RUNNING;
		putStr("Idle\n");
	}
	else {
		putStr("Now exec process:");
		putNum(pcb[current].pid);
		putChar('\n');
	}
		/*XXX recover stackTop of selected process */
	asm volatile("movl %0, %%esp"::"m"(pcb[current].stackTop));
	pcb[current].stackTop = pcb[current].prevStackTop;
        // setting tss for user process
	//tss.ss0 = pcb[current].regs.ss;
	tss.esp0 = pcb[current].stackTop;//process's kernel
		// switch kernel stack
		asm volatile("popl %gs");
		asm volatile("popl %fs");
		asm volatile("popl %es");
		asm volatile("popl %ds");
		asm volatile("popal");
		asm volatile("addl $8, %esp");
		asm volatile("iret");
}

void syscallHandle(struct StackFrame *sf) {
	switch(sf->eax) { // syscall number
		case 0:
			syscallWrite(sf);
			break; // for SYS_WRITE
		case 1:
			syscallFork(sf);
			break; // for SYS_FORK
		case 2:
			syscallExec(sf);
			break; // for SYS_EXEC
		case 3:
			syscallSleep(sf);
			break; // for SYS_SLEEP
		case 4:
			syscallExit(sf);
			break; // for SYS_EXIT
		default:break;
	}
}

void syscallWrite(struct StackFrame *sf) {
	switch(sf->ecx) { // file descriptor
		case 0:
			syscallPrint(sf);
			break; // for STD_OUT
		default:break;
	}
}

void syscallPrint(struct StackFrame *sf) {
	int sel = sf->ds; //TODO segment selector for user data, need further modification
	char *str = (char*)sf->edx;
	int size = sf->ebx;
	int i = 0;
	int pos = 0;
	char character = 0;
	uint16_t data = 0;
	asm volatile("movw %0, %%es"::"m"(sel));
	for (i = 0; i < size; i++) {
		asm volatile("movb %%es:(%1), %0":"=r"(character):"r"(str+i));
		if(character == '\n') {
			displayRow++;
			displayCol=0;
			if(displayRow==25){
				displayRow=24;
				displayCol=0;
				scrollScreen();
			}
		}
		else {
			data = character | (0x0c << 8);
			pos = (80*displayRow+displayCol)*2;
			asm volatile("movw %0, (%1)"::"r"(data),"r"(pos+0xb8000));
			displayCol++;
			if(displayCol==80){
				displayRow++;
				displayCol=0;
				if(displayRow==25){
					displayRow=24;
					displayCol=0;
					scrollScreen();
				}
			}
		}
		//asm volatile("int $0x20"); //XXX Testing irqTimer during syscall
		//asm volatile("int $0x20":::"memory"); //XXX Testing irqTimer during syscall
	}
	
	updateCursor(displayRow, displayCol);
	//TODO take care of return value
	return;
}

void syscallFork(struct StackFrame *sf) {
    // find empty pcb
putStr("Now process ");
putNum(pcb[current].pid);
putStr(" execute fork().\n");
	int i, j;
	for (i = 0; i < MAX_PCB_NUM; i++) {
		if (pcb[i].state == STATE_DEAD)
			break;
	}
	if (i != MAX_PCB_NUM) {
		/*XXX copy userspace
		  XXX enable interrupt
		 */
		enableInterrupt();
		for (j = 0; j < 0x100000; j++) {
			*(uint8_t *)(j + (i+1)*0x100000) = *(uint8_t *)(j + (current+1)*0x100000);
			//asm volatile("int $0x20"); //XXX Testing irqTimer during syscall
		}
		/*XXX disable interrupt
		 */
		disableInterrupt();
		/*XXX set pcb
		  XXX pcb[i]=pcb[current] doesn't work
		*/
		for(j = 0; j < MAX_STACK_SIZE; j++) pcb[i].stack[j] = pcb[current].stack[j];
		//pcb[i].stackTop = pcb[current].stackTop - ((uint32_t)&pcb[current].regs) + ((uint32_t)&pcb[i].regs);
		pcb[i].stackTop = pcb[current].stackTop  + (uint32_t)&pcb[i].regs - (uint32_t)&pcb[current].regs;
		//pcb[i].prevStackTop = pcb[current].prevStackTop - pcb[current].stackTop + pcb[i].stackTop;
		pcb[i].prevStackTop = pcb[current].prevStackTop + (uint32_t)&pcb[i].stackTop - (uint32_t)&pcb[current].stackTop;
		pcb[i].state = STATE_RUNNABLE;
		pcb[i].timeCount = 0;
		pcb[i].sleepTime = 0;
		pcb[i].pid = (uint32_t)i;
		for(j = 0; j < 32; j++) pcb[i].name[j] = pcb[current].name[j];
		/*XXX set regs */
		pcb[i].regs.edi = pcb[current].regs.edi;
		pcb[i].regs.esi = pcb[current].regs.esi;
		pcb[i].regs.ebp = pcb[current].regs.ebp;
		pcb[i].regs.xxx = pcb[current].regs.xxx;
		pcb[i].regs.ebx = pcb[current].regs.ebx;
		pcb[i].regs.edx = pcb[current].regs.edx;
		pcb[i].regs.ecx = pcb[current].regs.ecx;
		pcb[i].regs.irq = pcb[current].regs.irq;
		pcb[i].regs.error = pcb[current].regs.error;
		pcb[i].regs.eip = pcb[current].regs.eip;
		pcb[i].regs.eflags = pcb[current].regs.eflags;
		pcb[i].regs.esp = pcb[current].regs.esp;
		pcb[i].regs.ss = USEL(i*2+2);
		//pcb[i].regs.esp = pcb[current].regs.esp;
		//pcb[i].regs.eflags = ;
		pcb[i].regs.cs = USEL(i*2+1);
		//pcb[i].regs.eip = pcb[current].regs.eip;
		pcb[i].regs.ds = USEL(i*2+2);
		pcb[i].regs.es = USEL(i*2+2);
		pcb[i].regs.fs = USEL(i*2+2);
		pcb[i].regs.gs = USEL(i*2+2);
		/*XXX set return value */
		pcb[i].regs.eax = 0;//Child Processes
		pcb[current].regs.eax = i;//Father Processes
	}
	else {
		pcb[current].regs.eax = -1;// Failed to create a new processes
	}
	return;
}

void syscallExec(struct StackFrame *sf) {
	return;
}

void syscallSleep(struct StackFrame *sf) {
	pcb[current].sleepTime = sf->ecx;
	pcb[current].state = STATE_BLOCKED;
	//timerHandle(sf);
	int i, flag = 0;
	for(i = (current+1) % MAX_PCB_NUM; i != current; i= (i+1)%MAX_PCB_NUM) {
		if(pcb[i].state==STATE_RUNNABLE && i != 0) {
			current = i;
			flag = 1;
			break;	
		}
	}
		/* echo pid of selected process */
	if(flag == 0) current = 0; //IDLE
	pcb[current].state = STATE_RUNNING;
		/*XXX recover stackTop of selected process */
	asm volatile("movl %0, %%esp"::"m"(pcb[current].stackTop));
	pcb[current].stackTop = pcb[current].prevStackTop;
    // setting tss for user process
	tss.esp0 = pcb[current].stackTop;
		// switch kernel stack
		asm volatile("popl %gs");
		asm volatile("popl %fs");
		asm volatile("popl %es");
		asm volatile("popl %ds");
		asm volatile("popal");
		asm volatile("addl $8, %esp");
		asm volatile("iret");
	return;
	
	return;
}

void syscallExit(struct StackFrame *sf) {
	pcb[current].state = STATE_DEAD;
	//timerHandle(sf);
	int i, flag = 0;
	for(i = (current+1) % MAX_PCB_NUM; i != current; i = (i+1)%MAX_PCB_NUM) {
		if(pcb[i].state == STATE_RUNNABLE && i != 0) {
			current = i;
			flag = 1;
			break;	
		}
	}
		/* echo pid of selected process */
	if(flag == 0) current = 0; //IDLE
	pcb[current].state = STATE_RUNNING;
		/*XXX recover stackTop of selected process */
	asm volatile("movl %0, %%esp"::"m"(pcb[current].stackTop));
	pcb[current].stackTop = pcb[current].prevStackTop;
        // setting tss for user process
	tss.esp0 = pcb[current].stackTop;
		// switch kernel stack
		asm volatile("popl %gs");
		asm volatile("popl %fs");
		asm volatile("popl %es");
		asm volatile("popl %ds");
		asm volatile("popal");
		asm volatile("addl $8, %esp");
		asm volatile("iret");
	return;
}

