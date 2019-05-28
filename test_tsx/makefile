wrapper: shellcode.asm
	nasm -felf64 shellcode.asm -o shellcode.o
	( echo -n 'const char code[] = "'; for i in $$(objdump -d shellcode.o |grep "^ " |cut -f2); do echo -n '\x'$$i; done; echo '";' )  > wrapper.1
	cat wrapper.1 wrapper.2 > wrapper.c
	gcc -z execstack -o wrapper wrapper.c 
