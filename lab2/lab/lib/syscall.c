#include "lib.h"
#include "types.h"
/*
 * io lib here
 * 库函数写在这
 */
//static inline int32_t syscall(int num, uint32_t a1,uint32_t a2,
int32_t syscall(int num, uint32_t a1,uint32_t a2,
		uint32_t a3, uint32_t a4, uint32_t a5)
{
	int32_t ret = 0;
	//Generic system call: pass system call number in AX
	//up to five parameters in DX,CX,BX,DI,SI
	//Interrupt kernel with T_SYSCALL
	//
	//The "volatile" tells the assembler not to optimize
	//this instruction away just because we don't use the
	//return value
	//
	//The last clause tells the assembler that this can potentially
	//change the condition and arbitrary memory locations.

	/*
	XXX Note: ebp shouldn't be flushed
	    May not be necessary to store the value of eax, ebx, ecx, edx, esi, edi
	*/
	uint32_t eax, ecx, edx, ebx, esi, edi;
	uint16_t selector;
	
	asm volatile("movl %%eax, %0":"=m"(eax)); //temp_eax = %eax
	asm volatile("movl %%ecx, %0":"=m"(ecx)); //temp_ecx = %ecx
 	asm volatile("movl %%edx, %0":"=m"(edx)); //temp_edx = %edx
	asm volatile("movl %%ebx, %0":"=m"(ebx)); //temp_ebx = %ebx
	asm volatile("movl %%esi, %0":"=m"(esi)); //temp_esi = %esi
	asm volatile("movl %%edi, %0":"=m"(edi)); //temp_edi = %edi
	asm volatile("movl %0, %%eax"::"m"(num)); //%eax = num, system call number
	asm volatile("movl %0, %%ecx"::"m"(a1)); //%ecx = a1, 
	asm volatile("movl %0, %%edx"::"m"(a2)); //%edx = a2
	asm volatile("movl %0, %%ebx"::"m"(a3)); //%ebx = a3
	asm volatile("movl %0, %%esi"::"m"(a4)); //%esi = a4
	asm volatile("movl %0, %%edi"::"m"(a5)); //%edi = a5
	asm volatile("int $0x80");  //int $0x80
	asm volatile("movl %%eax, %0":"=m"(ret)); //ret = %eax
	asm volatile("movl %0, %%eax"::"m"(eax)); //%eax = temp_eax
	asm volatile("movl %0, %%ecx"::"m"(ecx)); //%ecx = temp_ecx
	asm volatile("movl %0, %%edx"::"m"(edx)); //%edx = temp_edx
	asm volatile("movl %0, %%ebx"::"m"(ebx)); //%ebx = temp_ebx
	asm volatile("movl %0, %%esi"::"m"(esi)); //%esi = temp_esi
	asm volatile("movl %0, %%edi"::"m"(edi)); //%edi = temp_edi
	
	asm volatile("movw %%ss, %0":"=m"(selector)); //XXX %ds is reset after iret
	//selector = 16;
	asm volatile("movw %%ax, %%ds"::"a"(selector));
	//%ds = %ss
	return ret; //%eax = ret
}

// API to support format in printf, if you can't understand, use your own implementation!
int dec2Str(int decimal, char *buffer, int size, int count);
int hex2Str(uint32_t hexadecimal, char *buffer, int size, int count);
int str2Str(char *string, char *buffer, int size, int count);

//stdarg.h
//cite from https://blog.csdn.net/qq_38365495/article/details/80524029
typedef char * va_list;//字符串指针
#define _INTSIZEOF(n) ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )
#define va_start(ap,v) ( ap = (va_list)&v + _INTSIZEOF(v) )
#define va_arg(ap,t) ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap) ( ap = (va_list)0 )

int printf(const char *format,...){
	int i = 0; // format index
	char buffer[MAX_BUFFER_SIZE];
	int count = 0; // buffer index
	int index=0; // parameter index
	// void *paraList=(void*)&format; // address of format in stack
	// int state=0; // 0: legal character; 1: '%'; 2: illegal format
	int decimal=0;
	uint32_t hexadecimal=0;
	char *string=0;
	char character=0;
	va_list ap;
	va_start(ap, format);
	while(format[i] != 0) {
        // TODO: support more format %s %d %x and so on
		if(format[i] == '%') { //format character        	
			i++;
			switch(format[i]) {
			case 's':
				string = va_arg(ap, char*);
				count = str2Str(string, buffer, MAX_BUFFER_SIZE, count);
				break;
			case 'd': 
				decimal = va_arg(ap, int);
				count = dec2Str(decimal, buffer, MAX_BUFFER_SIZE, count);
				break;
			case 'x': 
				hexadecimal = va_arg(ap, int);
				count = hex2Str(hexadecimal, buffer, MAX_BUFFER_SIZE, count);
				break;
			case 'c':
				character = (char) va_arg(ap, int); //warning when the seconde parameter is "char"
				buffer[count++] = character;
				break;
			case '%': //escape character
				index--;
				buffer[count++] = '%';
				break;			
			default:
				break;
			}
			index++;
		}
		else {
			buffer[count] = format[i];
			count++;
		}
		if(count == MAX_BUFFER_SIZE) {
			syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)MAX_BUFFER_SIZE, 0, 0);
			count = 0;
		}
		i++;
	}
	va_end(ap);
	if(count != 0)
		syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)count, 0, 0);
    return index;
}

int dec2Str(int decimal, char *buffer, int size, int count) {
	int i = 0;
	int temp;
	int number[16];

	if(decimal < 0){
		buffer[count] = '-';
		count++;
		if(count == size) {
			syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)size, 0, 0);
			count = 0;
		}
		temp = decimal/10;
		number[i] = temp*10 - decimal;
		decimal = temp;
		i++;
		while(decimal != 0){
			temp = decimal/10;
			number[i] = temp*10 - decimal;
			decimal = temp;
			i++;
		}
	}
	else{
		temp = decimal/10;
		number[i] = decimal-temp*10;
		decimal = temp;
		i++;
		while(decimal != 0){
			temp = decimal/10;
			number[i] = decimal-temp*10;
			decimal = temp;
			i++;
		}
	}

	while(i != 0){
		buffer[count] = number[i-1]+'0';
		count++;
		if(count == size) {
			syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)size, 0, 0);
			count = 0;
		}
		i--;
	}
	return count;
}

int hex2Str(uint32_t hexadecimal, char *buffer, int size, int count) {
	int i = 0;
	uint32_t temp = 0;
	int number[16];

	temp = hexadecimal >> 4;
	number[i] = hexadecimal - (temp<<4);
	hexadecimal = temp;
	i++;
	while(hexadecimal != 0){
		temp = hexadecimal>>4;
		number[i] = hexadecimal - (temp<<4);
		hexadecimal = temp;
		i++;
	}

	while(i!=0){
		if(number[i-1] < 10)
			buffer[count] = number[i-1] + '0';
		else
			buffer[count] = number[i-1] - 10 + 'a';
		count++;
		if(count == size) {
			syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)size, 0, 0);
			count = 0;
		}
		i--;
	}
	return count;
}

int str2Str(char *string, char *buffer, int size, int count) {
	int i = 0;
	while(string[i] != 0){
		buffer[count] = string[i];
		count++;
		if(count == size) {
			syscall(SYS_WRITE, STD_OUT, (uint32_t)buffer, (uint32_t)size, 0, 0);
			count=0;
		}
		i++;
	}
	return count;
}

// API to support format in scanf, if you can't understand, use your own implementation!
int matchWhiteSpace(char *buffer, int size, int *count);
int str2Dec(int *dec, char *buffer, int size, int *count);
int str2Hex(int *hex, char *buffer, int size, int *count);
int str2Str2(char *string, int avail, char *buffer, int size, int *count);

int str2avail(const char *str) {
	int num = 0;
	int i = 0;
	while(str[i] >= '0'&&str[i] <= '9') {
		num = num*10 + str[i] - '0';
		i++;
	}
	return num;
}

int scanf(const char *format,...) {
    // TODO: implement scanf function, return the number of input parameters
	va_list ap;
	int i = 0;
	char buffer[MAX_BUFFER_SIZE];
	int index = 0;	
	int count = 0;
	//int ret = 0;
	int *decimal;
	int *hexadecimal;
	char *character;
	char *string;
	int avail;
	va_start(ap, format);
	int ret;
	if(buffer[count] == 0) {
		do {
			ret = syscall(SYS_READ, STD_IN, (uint32_t)buffer, (uint32_t)MAX_BUFFER_SIZE, 0, 0);
		} while(ret == 0 || ret == -1);
		count = 0;	
	}
	//if(ret == 0);

	while(format[i] != 0) {		
		if(format[i] == '%') {
			int j;			
			for(j = i-1; j >= 0; j--) if(format[j] == '%') break;
			j++;			
			if(j != 0) {
				if(format[j] == 'c' || format[j] == 'd' || format[j] == 'x') j++;
				else { //%6s
					while(format[j] >= '0' && format[j] <= '9') j++;
					j++; 
				}
			}
			while(j <= i-1) { // Match un-format string
				while(format[j] == ' ' || format[j] == '\t') j++;
				while(1) {
					if(buffer[count] == 0) {
						do {
							ret = syscall(SYS_READ, STD_IN, (uint32_t)buffer, (uint32_t)MAX_BUFFER_SIZE, 0, 0);
						} while(ret == 0 || ret == -1);
						count = 0;					
					}
					if(buffer[count] == ' ' || buffer[count] == '\t' || buffer[count] == '\n') count++;
					else break;
				}
				
				if(j > i-1) break;				
				//printf("%x %x\n",format[j],buffer[count]);
				if(format[j] == buffer[count]) {j++; count++;}
				else return index;
			}
				
			//matchWhiteSpace(buffer, MAX_BUFFER_SIZE, &count);
			i++;
			switch(format[i])
			{
			case 'd':
				decimal = va_arg(ap, int*);
				str2Dec(decimal, buffer, MAX_BUFFER_SIZE, &count);
				break;
			/*case 's':
				string = va_arg(ap, char*);
				str2Str2(string, 100, buffer, MAX_BUFFER_SIZE, &count);
				break;
			*/
			case 'x':
				hexadecimal = va_arg(ap, int*);
				str2Hex(hexadecimal, buffer, MAX_BUFFER_SIZE, &count);
				break;
			case 'c':
				character = (char *) va_arg(ap, int*);
				matchWhiteSpace(buffer, MAX_BUFFER_SIZE, &count);				
				(*character) = buffer[count];
				count++;
				break;
			default:
				string = va_arg(ap, char*);
				avail = str2avail(format+i);
				//printf("%d\n",avail);
				if(avail == 0) avail = 100;
				while(format[i] >= '0'&&format[i] <= '9') i++;
				if(format[i] != 's') return -1;
				str2Str2(string, avail, buffer, MAX_BUFFER_SIZE, &count);				
				break;			
			}
			index++;
		}
		/*if(buffer[count] == 0) {
			do {
				ret = syscall(SYS_READ, STD_IN, (uint32_t)buffer, (uint32_t)MAX_BUFFER_SIZE, 0, 0);
			} while(ret == 0 || ret == -1);
			count = 0;	
		}*/
		i++;
	}

	va_end(ap);	    
	return index;
}

int matchWhiteSpace(char *buffer, int size, int *count){
	int ret = 0;
	while(1){
		if(buffer[*count] == 0){
			do {
				ret = syscall(SYS_READ, STD_IN, (uint32_t)buffer, (uint32_t)size, 0, 0);
			} while(ret == 0 || ret == -1);
			(*count) = 0;
		}
		if(buffer[*count] == ' ' || buffer[*count] == '\t' || buffer[*count] == '\n') {
			(*count)++;
		}
		else
			return 0;
	}
}

int str2Dec(int *dec, char *buffer, int size, int *count) {
	int sign = 0; // positive integer
	int state = 0;
	int integer = 0;
	int ret = 0;
	while(1){
		if(buffer[*count] == 0){
			do{
				ret = syscall(SYS_READ, STD_IN, (uint32_t)buffer, (uint32_t)size, 0, 0);
			}while(ret == 0 || ret == -1);
			(*count) = 0;
		}
		if(state == 0){
			if(buffer[*count] == '-'){
				state = 1;
				sign = 1;
				(*count)++;
			}
			else if(buffer[*count] >= '0' && buffer[*count] <= '9') {
				state = 2;
				integer = buffer[*count]-'0';
				(*count)++;
			}
			else if(buffer[*count] == ' ' || buffer[*count] == '\t' || buffer[*count] == '\n') {
				state = 0;
				(*count)++;
			}
			else
				return -1;
		}
		else if(state == 1){
			if(buffer[*count] >= '0' && buffer[*count] <= '9'){
				state = 2;
				integer = buffer[*count]-'0';
				(*count)++;
			}
			else
				return -1;
		}
		else if(state == 2){
			if(buffer[*count] >= '0' && buffer[*count] <= '9'){
				state = 2;
				integer *= 10;
				integer += buffer[*count] - '0';
				(*count)++;
			}
			else{
				if(sign == 1)
					*dec = -integer;
				else
					*dec = integer;
				return 0;
			}
		}
		else
			return -1;
	}
	return 0;
}

int str2Hex(int *hex, char *buffer, int size, int *count) {
	int state = 0;
	int integer = 0;
	int ret = 0;
	while(1){
		if(buffer[*count] == 0){
			do{
				ret = syscall(SYS_READ, STD_IN, (uint32_t)buffer, (uint32_t)size, 0, 0);
			}while(ret == 0 || ret == -1);
			(*count) = 0;
		}
		if(state == 0){
			if(buffer[*count] == '0'){
				state = 1;
				(*count)++;
			}
			else if(buffer[*count]==' ' || buffer[*count]=='\t' || buffer[*count]=='\n') {
				state = 0;
				(*count)++;
			}
			else
				return -1;
		}
		else if(state == 1){
			if(buffer[*count] == 'x'){
				state = 2;
				(*count)++;
			}
			else
				return -1;
		}
		else if(state == 2){
			if(buffer[*count] >= '0' && buffer[*count] <= '9') {
				state = 3;
				integer *= 16;
				integer += buffer[*count] - '0';
				(*count)++;
			}
			else if(buffer[*count] >= 'a' && buffer[*count] <= 'f') {
				state = 3;
				integer *= 16;
				integer += buffer[*count] - 'a' + 10;
				(*count)++;
			}
			else if(buffer[*count] >= 'A' && buffer[*count] <= 'F'){
				state = 3;
				integer *= 16;
				integer += buffer[*count] - 'A' + 10;
				(*count)++;
			}
			else
				return -1;
		}
		else if(state == 3){
			if(buffer[*count] >= '0' && buffer[*count] <= '9') {
				state = 3;
				integer *= 16;
				integer += buffer[*count] - '0';
				(*count)++;
			}
			else if(buffer[*count] >= 'a' && buffer[*count] <= 'f') {
				state = 3;
				integer *= 16;
				integer += buffer[*count] - 'a' + 10;
				(*count)++;
			}
			else if(buffer[*count] >= 'A' && buffer[*count] <= 'F'){
				state = 3;
				integer *= 16;
				integer += buffer[*count]-'A'+10;
				(*count)++;
			}
			else{
				*hex=integer;
				return 0;
			}
		}
		else
			return -1;
	}
	return 0;
}

int str2Str2(char *string, int avail, char *buffer, int size, int *count) {
	int i = 0;
	int state = 0;
	int ret = 0;
	while(i < avail-1){
		if(buffer[*count] == 0){
			do{
				ret = syscall(SYS_READ, STD_IN, (uint32_t)buffer, (uint32_t)size, 0, 0);
			}while(ret == 0 || ret == -1);
			(*count) = 0;
		}
		if(state == 0){
			if(buffer[*count] == ' ' || buffer[*count] == '\t' || buffer[*count] == '\n') {
				state = 0;
				(*count)++;
			}
			else{
				state = 1;
				string[i] = buffer[*count];
				i++;
				(*count)++;
			}
		}
		else if(state == 1){
			if(buffer[*count]==' ' || buffer[*count]=='\t' || buffer[*count]=='\n') {
				string[i]=0;
				return 0;
			}
			else{
				state = 1;
				string[i] = buffer[*count];
				i++;
				(*count)++;
			}
		}
		else
			return -1;
	}
	string[i] = 0;
	return 0;
}
