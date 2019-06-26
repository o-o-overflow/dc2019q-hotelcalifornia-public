# Hotel California

This is a shellcode challenge developed for the Defcon Quals 2019.

### In a Nutshell

In a classic shellcode-like scenario, the service simply asks the user to
provide a X64 shellcode, shorter than 1K. It then executes the code inside
a TSX Hardware Lock Elision (HLE) transaction. If the transaction fails,
the service prints an error message and asks the user to try again.

The catch is that there is really nothing useful you can do in a TSX
transaction, as invoking syscalls would make the transaction fail and all
read/write memory and register operations are cached by the CPU and
reverted in case of failure. 

### Intended Exploit

The idea is to first close the transaction, and then execute a traditional
shellcode to read and print the flag. However, a HLE transaction can only
be closed by restoring the lock value to its original content - which
unfortunately is not possible as the service acquires the lock by 
XORing two random 32bit numbers (properly deleted afterwards).

At a closer look, before jumping to the shellcode, one of the random value
was momentarily spilled on the stack, just to be immediately overwritten 
by the frame of a function responsible to copy the received shellcode on
the heap. Which brings us to the exploit procedure (a bit simplified here,
but it gives the idea):

1. Send a shellcode, padded to be at least 1K long. The shellcode 
   is copied on the heap and executed.. but it fails as it has no way
   to close the transaction properly.
2. The service prints the error message using `printf`, which internally
   also allocates a buffer on the heap.
3. The buffer is de-allocated, causing `libc` to leak on the heap the
   address of a symbol on the `libc` (related to the allocator arena).
4. When the service asks for a shellcode again, close the transmission side
   of the socket. This makes the `read` in the service fail. Since no bytes
   are received, the service does not invoke the function to copy the buffer
   to the heap. Therefore, the secret value is now available somewhere on the
   stack. Sadly, we have no reference to where the stack is.
5. The service now tries to execute the buffer again. But since it did not
   copy anything new there, it ends up executing bytes that were put there
   by the first shellcode (modulo some that got overwritten and that therefore
   the shellcode needs to be careful to jump over).
6. Use the shellcode to retrieve the pointer to the `libc`, from there move to
   the `environ` global variable which points to the stack. 
   From the base of the environment, move down till you find the secret
   value, use it to close the transaction, and you are done.

(Instead of using the allocator to leak the `libc` position, it is also
possible to use the `cs` register.)










