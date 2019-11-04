.code32

.global start
start:
	# TODO
	push %ebp        #Create new stack frame	
	mov %esp, %ebp
	pushl $16        #length of the string
	pushl $message   #address of the string
	calll displayStr
	leave
	ret              #Recover stack
message:
	.string "hello, NJUworld!\n\0"
displayStr:
	movl 4(%esp), %ebx
	movl 8(%esp), %ecx
	subl $0xa, %ecx #print 6 characters
	movl $0xa, %edx 
	movl $((80*10+30)*2), %edi #print at line 10, col 30
	movb $0x0a, %ah #color setting
nextChar1:
	movb (%ebx), %al
	movw %ax, %gs:(%edi)
	addl $2, %edi
	incl %ebx
	loopnz nextChar1
	movl %edx, %ecx #print 10 characters
	movl $((80*11+30)*2), %edi
	movb $0x0e, %ah #set new color
nextChar2:
	movb (%ebx), %al
	movw %ax, %gs:(%edi)
	addl $2, %edi
	incl %ebx
	loopnz nextChar2
	ret
