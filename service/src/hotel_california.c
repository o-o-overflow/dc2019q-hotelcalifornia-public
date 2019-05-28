#include <cpuid.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include "header.h"

#define CPUID_HLE (1 << 4)

// Test if the CPU supports Hardware Lock Elision (HLE)
// Sadly, the fact that it is supported doesnt guarantee that
// it is working, and in fact it is often the case that all transactions
// fail.
static inline int cpu_has_hle(void)
{
    if (__get_cpuid_max(0, NULL) >= 7) {
        unsigned a, b, c, d;
        __cpuid_count(7, 0, a, b, c, d);
        return !!(b & CPUID_HLE);
    }
    return 0;
}

// This is run before main, to stop the execution and avoid an illegal 
// instruction on CPUs that do not support TSX
void __attribute__ ((constructor)) premain()
{
    if (cpu_has_hle()==0){
        printf("Listen, I don't think you can afford to stay with us.\nHave you tried with the Holiday Inn down the road?\n");
        exit(1); 
    }
}

#define PAGE_START(P) ((uintptr_t)(P) & ~(pagesize-1))
#define PAGE_END(P)   (((uintptr_t)(P) + pagesize - 1) & ~(pagesize-1))

// Initialize the buffer on the heap.
char* init_buffer(int shellcode_size, int pos, int base){
    char* buf = malloc(HEADER_LEN+shellcode_size+4);
    memset(buf,'\x90', HEADER_LEN+shellcode_size+4);
    uintptr_t pagesize = sysconf(_SC_PAGE_SIZE);
    mprotect((void *)PAGE_START(buf), 4000, PROT_READ|PROT_WRITE|PROT_EXEC);
    memcpy(&buf[pos], (char*)&base, 4);
    memcpy(&buf[4+pos], HEADER, HEADER_LEN);
    return buf;
}

// Stupid implementation of a memcopy. Cant use the real one because
// it doesn't overwrite the secret on the stack
int my_memcpy(char* where, char* from, int n){
    int result = 0;
    int i = 0;
    while (i<n){
        where[i] = from[i];
        i++;
    }
    return i;
}

int read_all(char* buf, int max){
    int count = 0;
    while (count < max){
        if (read(0, &buf[count], 1)==0)
	  return count;
	count++;
	if (buf[count-1] == '\x0')
	  return count;
    }
    return count;
}

int main(int argc, char **argv)
{
    char* buf;
    int base=0;
    int key_tmp=0;
    char shellcode[1048];
    int shellcode_size;
    int (*func)();
    register int key asm ("ebx");

    // We want to have buffered output so that printf allocates a buffer on
    // the heap
    setvbuf(stdout, NULL,_IOLBF,BUFSIZ);

    int urand = open("/dev/urandom", O_RDONLY);
    //int urand = open("_dev_urandom", O_RDONLY); // for testing

    write(1,"Welcome to the Hotel California.\n",33);   
    while(1){
        // we cant use printf yet    
        write(1,"\nShellcode > \0",14);
        fflush(stdout);
        //shellcode_size = read(0, &shellcode, 1024);
        shellcode_size = read_all(&shellcode, 1024);
        
        // read the two secrets from /dev/urandom
        read(urand, (char*)&base, 4);
        read(urand, (char*)&key_tmp, 4);

        // Initialize the buffer (this call malloc)
        // This call leaks the *base* value on the stack
        buf = init_buffer(shellcode_size, 0, base);

        // Now we can use printf - which will allocate a buffer too
        printf("\n(received %d bytes)\n",shellcode_size);

        // If we received at least one byte, we copy the shellcode
        // on the heap. 
        // THIS CALL OVERWRITE the base value.
        if (shellcode_size)
            my_memcpy(&buf[HEADER_LEN+4], shellcode, shellcode_size);

        // We clean one of the secrets
        base = 0;
        func = (int (*)()) &buf[20];
        //setvbuf(stdout, NULL, _IONBF, 0);
        
        // Let's run as many instrctions as possible before the sleep
        asm("vzeroall");
        asm("xor %r10,%r10");
        asm("xor %r13,%r13");
        asm("xor %r12,%r12");
        asm("xor %rsi,%rsi");
        asm("xor %rdi,%rdi");

        // This is fundamental to stop the execution!!
        // Without the sleep here the scheduler will very likely interrupt
        // the shellcode and therefore the transaction will fail.
        sleep(1);

        // Now we put the second secret in a register and we clean up
        // the variable on the stack.
        key = key_tmp;
        key_tmp = 0;

        // At this point, there is no track of the secrets on the stack
        // UNLESS my_memcpy was skipped.
        
        // Let's executed the shellcode
        (int)(*func)();

        // If we are here, the transaction failed.
        printf("We are all just prisoners here, of our own device\n");
        fflush(stdout);

        // We now free the first buffer, which at the first iteration is what
        // triggers allocator to put an address on the heap pointing to libc
        free(buf);

        // This sleep is useless, just here to avoid bruteforcing
        sleep(2);
    }

    // You should never reach here.. too bad, I liked the quote.
    printf("Last thing you remember, you were running for the door");
    return 0;
}

