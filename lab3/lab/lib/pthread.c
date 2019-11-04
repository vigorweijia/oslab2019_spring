#include "pthread.h"
#include "lib.h"
/*
 * pthread lib here
 * 用户态多线程写在这
 */
 
ThreadTable tcb[MAX_TCB_NUM];
int current;

void pthread_initial(void){
    int i;
    for (i = 0; i < MAX_TCB_NUM; i++) {
        tcb[i].state = STATE_DEAD;
        tcb[i].joinid = -1;
    }
    tcb[0].state = STATE_RUNNING;
    tcb[0].pthid = 0;
    current = 0; // main thread
    return;
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg){
    int i;
    //int tmp_esp;
    for(i = 0; i < MAX_TCB_NUM; i++) {
        if(tcb[i].state == STATE_DEAD) break;
    }
    if(i == MAX_TCB_NUM) return -1;
    
    *thread = i;
    tcb[i].retPoint = 0;
    tcb[i].pthArg = (uint32_t)arg;
    tcb[i].stackTop = (uint32_t)&tcb[i].cont;
    tcb[i].state = STATE_RUNNABLE;
    tcb[i].pthid = i;
    tcb[i].joinid = -1;
    tcb[i].cont.eip = (uint32_t)start_routine;
    tcb[i].cont.esp = tcb[i].stackTop;
    tcb[i].cont.ebp = tcb[i].stackTop;
    
    //asm volatile("movl %%esp, %0":"=m"(tmp_esp));
    //asm volatile("movl %0, %%esp"::"m"(tcb[i].stackTop));
    //asm volatile("push %0"::"m"((uint32_t)arg));
    //asm volatile("call %0"::"m"(start_routine));
    //asm volatile("movl %0, %%esp"::"m"(tmp_esp));


    /*asm volatile("movl %%eax, %0":"=m"(tcb[current].cont.eax));
    asm volatile("movl %%ecx, %0":"=m"(tcb[current].cont.ecx));
    asm volatile("movl %%edx, %0":"=m"(tcb[current].cont.edx));
    asm volatile("movl %%ebx, %0":"=m"(tcb[current].cont.ebx));
    asm volatile("movl %%esi, %0":"=m"(tcb[current].cont.esi));
    asm volatile("movl %%edi, %0":"=m"(tcb[current].cont.edi));

    //get eip, ebp
    asm volatile("leave"); 
    asm volatile("movl (%esp), %eax");
    asm volatile("movl %%eax, %0":"=m"(tcb[current].cont.eip));
    asm volatile("movl %%ebp, %0":"=m"(tcb[current].cont.ebp));
    //get esp
    asm volatile("subl $0x4, %esp");
    asm volatile("movl %%esp, %0":"=m"(tcb[current].cont.esp));

    current = i;
    
    
    //asm volatile("movl %0, %%eax"::"m"(tcb[current].cont.eax));
    //asm volatile("movl %0, %%ecx"::"m"(tcb[current].cont.ecx));
    //asm volatile("movl %0, %%edx"::"m"(tcb[current].cont.edx));
    //asm volatile("movl %0, %%ebx"::"m"(tcb[current].cont.ebx));
    //asm volatile("movl %0, %%esi"::"m"(tcb[current].cont.esi));
    //asm volatile("movl %0, %%edi"::"m"(tcb[current].cont.edi));

    //asm volatile("movl %0, %%ebp"::"m"(tcb[current].stackTop));
    asm volatile("movl %0, %%esp"::"m"(tcb[current].stackTop));
    asm volatile("push %0"::"m"(tcb[current].pthArg));
    while(1) {}
    asm volatile("call *%0"::"m"(start_routine));*/

    
    return 0;
}

void pthread_exit(void *retval){
    tcb[current].state = STATE_DEAD; 
    int i;
    for(i = 0; i < MAX_TCB_NUM; i++) {
        if(tcb[i].joinid == current) {
            tcb[i].state = STATE_RUNNABLE;
            tcb[i].joinid = -1;
        }
    }
    //pthread_yield();
    asm volatile("movl (%%ebp),%%eax":"=a"(tcb[current].cont.ebp));
    asm volatile("movl 0x4(%%ebp),%%eax":"=a"(tcb[current].cont.eip));
    asm volatile("leal 0x8(%%ebp),%%eax;":"=a"(tcb[current].cont.esp));
                  
    
    if(tcb[current].state == STATE_RUNNING)    tcb[current].state = STATE_RUNNABLE;
    
    for(i = (current+1)%MAX_TCB_NUM; i != current; i = (i+1)%MAX_TCB_NUM) {
        if(tcb[i].state == STATE_RUNNABLE) {
            current = i;
            break;
        }
    }
    
    tcb[current].state = STATE_RUNNING;

    
    asm volatile("movl %0, %%ebp"::"m"(tcb[current].cont.ebp));
    asm volatile("movl %0, %%esp"::"m"(tcb[current].cont.esp));
    asm volatile("push %0"::"m"(tcb[current].pthArg));
    asm volatile("call *%0"::"m"(tcb[current].cont.eip));
    /*if(tcb[current].joinid != -1) {
        i = tcb[current].joinid;
        tcb[current].joinid = -1;
    }
    if(i == MAX_TCB_NUM) exit();*/

    /*current = i;
    tcb[current].state = STATE_RUNNING;
    asm volatile("movl %0, %%eax"::"m"(tcb[current].cont.eax));
    asm volatile("movl %0, %%ecx"::"m"(tcb[current].cont.ecx));
    asm volatile("movl %0, %%edx"::"m"(tcb[current].cont.edx));
    asm volatile("movl %0, %%ebx"::"m"(tcb[current].cont.ebx));
    asm volatile("movl %0, %%esi"::"m"(tcb[current].cont.esi));
    asm volatile("movl %0, %%edi"::"m"(tcb[current].cont.edi));

    asm volatile("movl %0, %%ebp"::"m"(tcb[current].cont.ebp));
    asm volatile("movl %0, %%esp"::"m"(tcb[current].cont.esp));
    asm volatile("jmp *%0"::"m"(tcb[current].cont.eip));*/

    return;
}

int pthread_join(pthread_t thread, void **retval){
    if(tcb[thread].state == STATE_DEAD) return 0;
    
    tcb[current].joinid = thread;
    tcb[current].state = STATE_BLOCKED;
    //pthread_yield();

    asm volatile("movl (%%ebp),%%eax":"=a"(tcb[current].cont.ebp));
    asm volatile("movl 0x4(%%ebp),%%eax":"=a"(tcb[current].cont.eip));
    asm volatile("leal 0x8(%%ebp),%%eax;":"=a"(tcb[current].cont.esp));
                  
    int i;
    
    if(tcb[current].state == STATE_RUNNING)    tcb[current].state = STATE_RUNNABLE;
    
    for(i = (current+1)%MAX_TCB_NUM; i != current; i = (i+1)%MAX_TCB_NUM) {
        if(tcb[i].state == STATE_RUNNABLE) {
            current = i;
            break;
        }
    }
    
    tcb[current].state = STATE_RUNNING;

    
    asm volatile("movl %0, %%ebp"::"m"(tcb[current].cont.ebp));
    asm volatile("movl %0, %%esp"::"m"(tcb[current].cont.esp));
    asm volatile("push %0"::"m"(tcb[current].pthArg));
    asm volatile("call *%0"::"m"(tcb[current].cont.eip));


    /*asm volatile("movl %%eax, %0":"=m"(tcb[current].cont.eax));
    asm volatile("movl %%ecx, %0":"=m"(tcb[current].cont.ecx));
    asm volatile("movl %%edx, %0":"=m"(tcb[current].cont.edx));
    asm volatile("movl %%ebx, %0":"=m"(tcb[current].cont.ebx));
    asm volatile("movl %%esi, %0":"=m"(tcb[current].cont.esi));
    asm volatile("movl %%edi, %0":"=m"(tcb[current].cont.edi));

    //get eip, ebp
    asm volatile("leave"); 
    asm volatile("movl (%esp), %eax");
    asm volatile("movl %%eax, %0":"=m"(tcb[current].cont.eip));
    asm volatile("movl %%ebp, %0":"=m"(tcb[current].cont.ebp));
    //get esp
    asm volatile("subl $0x4, %esp");
    asm volatile("movl %%esp, %0":"=m"(tcb[current].cont.esp));

    current = thread;

   
    asm volatile("movl %0, %%eax"::"m"(tcb[current].cont.eax));
    asm volatile("movl %0, %%ecx"::"m"(tcb[current].cont.ecx));
    asm volatile("movl %0, %%edx"::"m"(tcb[current].cont.edx));
    asm volatile("movl %0, %%ebx"::"m"(tcb[current].cont.ebx));
    asm volatile("movl %0, %%esi"::"m"(tcb[current].cont.esi));
    asm volatile("movl %0, %%edi"::"m"(tcb[current].cont.edi));

    asm volatile("movl %0, %%ebp"::"m"(tcb[current].cont.ebp));
    asm volatile("movl %0, %%esp"::"m"(tcb[current].cont.esp));
    if(tcb[current].pthArg != 0)
    asm volatile("push %0"::"m"(tcb[current].pthArg));
    asm volatile("call *%0"::"m"(tcb[current].cont.eip));*/
    return 0;
}

int pthread_yield(void){
    
    /*int i;
    
    asm volatile("movl %%eax, %0":"=m"(tcb[current].cont.eax));
    asm volatile("movl %%ecx, %0":"=m"(tcb[current].cont.ecx));
    asm volatile("movl %%edx, %0":"=m"(tcb[current].cont.edx));
    asm volatile("movl %%ebx, %0":"=m"(tcb[current].cont.ebx));
    asm volatile("movl %%esi, %0":"=m"(tcb[current].cont.esi));
    asm volatile("movl %%edi, %0":"=m"(tcb[current].cont.edi));

    //get eip, ebp
    asm volatile("leave");
    asm volatile("movl (%esp), %eax");
    asm volatile("movl %%eax, %0":"=m"(tcb[current].cont.eip));
    asm volatile("movl %%ebp, %0":"=m"(tcb[current].cont.ebp));
    //get esp
    asm volatile("subl $0x4, %esp");
    asm volatile("movl %%esp, %0":"=m"(tcb[current].cont.esp));

    tcb[current].state = STATE_RUNNABLE;

    for(i = (current+1)%MAX_TCB_NUM; i != current; i = (i+1)%MAX_TCB_NUM) {
        if(tcb[i].state == STATE_RUNNABLE) {
            current = i;
            break;
        }
    }

    tcb[current].state = STATE_RUNNING;

    asm volatile("movl %0, %%eax"::"m"(tcb[current].cont.eax));
    asm volatile("movl %0, %%ecx"::"m"(tcb[current].cont.ecx));
    asm volatile("movl %0, %%edx"::"m"(tcb[current].cont.edx));
    asm volatile("movl %0, %%ebx"::"m"(tcb[current].cont.ebx));
    asm volatile("movl %0, %%esi"::"m"(tcb[current].cont.esi));
    asm volatile("movl %0, %%edi"::"m"(tcb[current].cont.edi));

    asm volatile("movl %0, %%ebp"::"m"(tcb[current].cont.ebp));
    asm volatile("movl %0, %%esp"::"m"(tcb[current].cont.esp));
    asm volatile("push %0"::"m"(tcb[current].pthArg));
    asm volatile("call %0"::"m"(tcb[current].cont.eip));*/
    //int i;

    /*asm volatile("movl (%ebp),%eax");
    asm volatile("movl %%eax, %0":"=m"(tcb[current].cont.ebp));
    asm volatile("movl 0x4(%ebp),%eax");
    asm volatile("movl %%eax, %0":"=m"(tcb[current].cont.eip));
    asm volatile("leal 0x8(%ebp), %eax");
    asm volatile("movl %%eax, %0":"=m"(tcb[current].cont.esp));*/
   
    asm volatile("push %eax");
    asm volatile("push %ecx");
    asm volatile("push %edx");
    asm volatile("push %ebx");
    asm volatile("push %esi");
    asm volatile("push %edi");
    asm volatile("movl %esp, %eax");
    asm volatile("movl (%%eax), %%ecx":"=c"(tcb[current].cont.edi));
    asm volatile("movl 0x4(%%eax), %%ecx":"=c"(tcb[current].cont.esi));
    asm volatile("movl 0x8(%%eax), %%ecx":"=c"(tcb[current].cont.ebx));
    asm volatile("movl 0xc(%%eax), %%ecx":"=c"(tcb[current].cont.edx));
    asm volatile("movl 0x10(%%eax), %%ecx":"=c"(tcb[current].cont.ecx));
    asm volatile("movl 0x14(%%eax), %%ecx":"=c"(tcb[current].cont.eax));

    asm volatile("movl (%%ebp),%%eax":"=a"(tcb[current].cont.ebp));
    asm volatile("movl 0x4(%%ebp),%%eax":"=a"(tcb[current].cont.eip));
    asm volatile("leal 0x8(%%ebp),%%eax;":"=a"(tcb[current].cont.esp));
                  
    int i;
    
    if(tcb[current].state == STATE_RUNNING)    tcb[current].state = STATE_RUNNABLE;
    
    for(i = (current+1)%MAX_TCB_NUM; i != current; i = (i+1)%MAX_TCB_NUM) {
        if(tcb[i].state == STATE_RUNNABLE) {
            current = i;
            break;
        }
    }
    
    tcb[current].state = STATE_RUNNING;

    
    asm volatile("movl %0, %%ebp"::"m"(tcb[current].cont.ebp));
    asm volatile("movl %0, %%esp"::"m"(tcb[current].cont.esp));
    asm volatile("push %0"::"m"(tcb[current].cont.eax));
    asm volatile("push %0"::"m"(tcb[current].cont.ecx));
    asm volatile("push %0"::"m"(tcb[current].cont.edx));
    asm volatile("push %0"::"m"(tcb[current].cont.ebx));
    asm volatile("push %0"::"m"(tcb[current].cont.esi));
    asm volatile("push %0"::"m"(tcb[current].cont.edi));
    asm volatile("popl %edi");
    asm volatile("popl %esi");
    asm volatile("popl %ebx");
    asm volatile("popl %edx");
    asm volatile("popl %ecx");
    asm volatile("popl %eax");
    asm volatile("push %0"::"m"(tcb[current].pthArg));
    asm volatile("call *%0"::"m"(tcb[current].cont.eip));

    return 0;
}

