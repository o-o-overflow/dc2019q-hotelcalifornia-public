hotel_california: hotel_california.c header.h
	gcc -fno-builtin-printf -o hotel_california hotel_california.c 

header.h: header.asm
	nasm -felf64 header.asm -o header.o
	( echo -n 'const char HEADER[] = "'; for i in $$(objdump -d header.o |grep "^ " |cut -f2); do echo -n '\x'$$i; done; echo '";' )  > header.h
	( echo -n 'int HEADER_LEN = '; echo -n $$(cat header.h | grep -o '\\x' | wc -l); echo ';' ) >> header.h
