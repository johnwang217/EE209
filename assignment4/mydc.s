# --------------------------------------------------------------------
# dc.s
#
# Desk Calculator (dc) (x86-64)
#
# Name: Wang Jonghyuk
# Student ID: 20220425
# --------------------------------------------------------------------

    .equ   BUFFERSIZE, 32
    .equ   INTSIZE, 4
    .equ   EOF, -1
    .equ   INT_MIN, -2147483648
    .equ   LENGTHOFFSET, -36
    .equ   STACKBASEOFFSET, -48
    .equ   ELEMENTOFFSET, 8
    .equ   EXIT_SUCCESS, 0
    .equ   EXIT_FAILURE, 1

# --------------------------------------------------------------------

.section ".rodata"

scanfFormat:
     .asciz "%s"
printfFormat:
     .asciz "%d\n"
emptystackerror:
     .asciz "dc: stack empty\n"
overflowerror:
     .asciz "dc: overflow happens\n"
divideerror:
     .asciz "dc: divide by zero\n"
remaindererror:
     .asciz "dc: remainder by zero\n"


# --------------------------------------------------------------------

    .section ".data"

# --------------------------------------------------------------------

    .section ".bss"

# --------------------------------------------------------------------
    .section ".text"

    # -------------------------------------------------------------
    # int powerfunc(int base, int exponent)
    # Runs the power function.  Returns result.
    # -------------------------------------------------------------

    .globl	powerfunc
    .type   powerfunc, @function

    # base is stored in %rdi
    # exponent is stored in %rsi

powerfunc:

    pushq   %rbp
    movq    %rsp, %rbp
    movl    $1, %eax                # set result to 1

    # repeatedly multiply base to result 'exponent' times
    power_loop:
    cmp     $0, %esi                # check if exponent is 0
    je      power_return            # if 0, jump to power_return 
    imul    %edi, %eax
    jo      power_return            # break if overflow occurs
    decl    %esi
    jmp     power_loop              # goto start of loop
    power_return:
    movq    %rbp, %rsp
    popq    %rbp
    ret                             # return result

# --------------------------------------------------------------------

    # -------------------------------------------------------------
    # int main(void)
    # Runs desk calculator program.  Returns 0.
    # -------------------------------------------------------------

    .text
    .globl  main
    .type   main, @function

main:

    pushq   %rbp
    movq    %rsp, %rbp

    # char buffer[BUFFERSIZE]
    subq    $BUFFERSIZE, %rsp

    # int length
    subq    $INTSIZE, %rsp
    movl    $0, LENGTHOFFSET(%rbp)  # store length of stack, set to 0
    subq    $12, %rsp               # 16-B allign %rsp

    # Note %rsp must be 16-B aligned before call

.input:

    # while (1) {
    # scanf("%s", buffer)
    # %al must be set to 0 before scanf call
    leaq    scanfFormat, %rdi
    leaq    -BUFFERSIZE(%rbp), %rsi
    movb    $0, %al
    call    scanf

    # check if user input == EOF
    cmp	    $EOF, %eax
    je	    .quit

    # check if first character of input is a number
    movq    $0, %rdi                    
    movb    -BUFFERSIZE(%rbp), %dil     # pass first char as argument
    call    isdigit
    cmp     $0, %rax                    
    jne     .isdigit            # if not number, jump to .isdigit

    # input isn't digit
    cmp     $'_', %dil          # negetive number input
    je      .minus
    cmp     $'p', %dil          # peek input
    je      .peek
    cmp     $'q', %dil          # quit input
    je      .quit
    cmp     $'f', %dil          # print all input
    je      .print
    cmp     $'c', %dil          # clear input
    je      .clear
    cmp     $'+', %dil          # add operator
    je      .add
    cmp     $'-', %dil          # subtract operator
    je      .subtract
    cmp     $'*', %dil          # multiply operator
    je      .mult
    cmp     $'/', %dil          # divide operator
    je      .divide
    cmp     $'%', %dil          # remainder operator
    je      .remainder
    cmp     $'^', %dil          # power operator
    je      .power
    jmp     .input              # if none of above, recieve new input

.isdigit:

    # input is number
    leaq    -BUFFERSIZE(%rbp), %rdi     
    call    atoi                    
    pushq   %rax                    # push input to stack
    subq    $ELEMENTOFFSET, %rsp    # 16-B allign %rsp
    incl    LENGTHOFFSET(%rbp)      # increment length
    jmp     .input

.minus:

    # input is negative number
    leaq    -BUFFERSIZE(%rbp), %rdi 
    incq    %rdi                    # read input excluding the sign
    call    atoi
    xorl    $0xffffffff, %eax
    incl    %eax                    # negate the input
    pushq   %rax
    subq    $ELEMENTOFFSET, %rsp
    incl    LENGTHOFFSET(%rbp) 
    jmp     .input

.peek:

    # print the element at the top of stack
    cmpl    $0, LENGTHOFFSET(%rbp)  # check if stack is empty
    je      .emptystack             # if empty, error
    leaq    printfFormat, %rdi
    movq    ELEMENTOFFSET(%rsp), %rsi   # get the element
    movb    $0, %al
    call    printf
    jmp     .input

.quit:

    # end process
    movq    $EXIT_SUCCESS, %rax
    addq    $BUFFERSIZE, %rsp
    movq    %rbp, %rsp
    popq    %rbp
    ret            # return EXIT_SUCCESS

.print:

    # print the entire stack in LIFO order

    # length in %rbx
    # address of element in %r12
    movl    LENGTHOFFSET(%rbp), %ebx    
    leaq    ELEMENTOFFSET(%rsp), %r12   
    print_loop:
    cmp     $0, %ebx        # check if length is 0
    je      .input          # if 0, recieve new input
    leaq    printfFormat, %rdi
    movq    (%r12), %rsi    # get element at the address
    movb    $0, %al
    call    printf
    addq    $16, %r12       # store address of next element
    decl    %ebx            
    jmp     print_loop      # goto start of loop

.clear:
    
    # clear stack
    leaq    STACKBASEOFFSET(%rbp), %rsp     # clear stack
    movl    $0, LENGTHOFFSET(%rbp)          # set length to 0
    jmp     .input

.add:

    # does addition
    cmpl    $2, LENGTHOFFSET(%rbp)  # check if there are at least
                                    # two operands in stack      
    jl      .emptystack             # if not, error
    addq    $ELEMENTOFFSET, %rsp
    popq    %rbx                    # first operand in %rbx
    addq    $ELEMENTOFFSET, %rsp
    popq    %rax                    # second operand in %rax
    addl    %ebx, %eax              
    jo      .overflow               # if overflow, error
    pushq   %rax                    # push result to stack
    subq    $ELEMENTOFFSET, %rsp
    decl    LENGTHOFFSET(%rbp)      # decrement length
    jmp     .input

.subtract:

    # does subtraction
    cmpl    $2, LENGTHOFFSET(%rbp)
    jl      .emptystack
    addq    $ELEMENTOFFSET, %rsp
    popq    %rbx
    addq    $ELEMENTOFFSET, %rsp
    popq    %rax
    subl    %ebx, %eax
    jo      .overflow
    pushq   %rax
    subq    $ELEMENTOFFSET, %rsp
    decl    LENGTHOFFSET(%rbp)
    jmp     .input

.mult:

    # does multiplication
    cmpl    $2, LENGTHOFFSET(%rbp)
    jl      .emptystack
    addq    $ELEMENTOFFSET, %rsp
    popq    %rbx
    addq    $ELEMENTOFFSET, %rsp
    popq    %rax
    imul    %ebx, %eax
    jo      .overflow
    pushq   %rax
    subq    $ELEMENTOFFSET, %rsp
    decl    LENGTHOFFSET(%rbp)
    jmp     .input

.divide:

    # does division
    cmpl    $2, LENGTHOFFSET(%rbp)
    jl      .emptystack
    addq    $ELEMENTOFFSET, %rsp
    popq    %rbx                # divisor in %rbx
    addq    $ELEMENTOFFSET, %rsp
    popq    %rax                # dividend in %rax
    cmp     $0, %rbx            # check if divisor is 0
    je      .dividebyzero       # if 0, error

    # check the error possibility in division
    # when divisor is -1 and divident is INT_MIN
    # in this case, result is overflow
    cmp     $-1, %ebx
    jne     div_else
    cmp     $INT_MIN, %eax 
    jne     div_else
    jmp     .overflow   
    div_else:
    cdq
    idiv    %ebx
    pushq   %rax
    subq    $ELEMENTOFFSET, %rsp
    decl    LENGTHOFFSET(%rbp)
    jmp     .input

.remainder:

    # does remainder operation
    cmpl    $2, LENGTHOFFSET(%rbp)
    jl      .emptystack
    addq    $ELEMENTOFFSET, %rsp
    popq    %rbx
    addq    $ELEMENTOFFSET, %rsp
    popq    %rax
    cmp     $0, %rbx            # check if divisor is 0
    je      .remainderbyzero    # if 0, error

    # check the overflow possibility in division
    # when divisor is -1 and divident is INT_MIN
    # in this case, result is 0
    cmp     $-1, %ebx
    jne     rem_else
    cmp     $INT_MIN, %eax 
    jne     rem_else
    movq    $0, %rdx    # set result to 0
    jmp     rem_skip    # skip division
    rem_else:
    cdq
    idiv    %ebx
    rem_skip:
    pushq   %rdx
    subq    $ELEMENTOFFSET, %rsp
    decl    LENGTHOFFSET(%rbp)
    jmp     .input

.power: 

    # does power operation
    cmpl    $2, LENGTHOFFSET(%rbp)
    jl      .emptystack
    addq    $ELEMENTOFFSET, %rsp
    popq    %rsi
    addq    $ELEMENTOFFSET, %rsp
    popq    %rdi
    call    powerfunc
    jo      .overflow
    pushq   %rax
    subq    $ELEMENTOFFSET, %rsp
    decl    LENGTHOFFSET(%rbp)
    jmp     .input 

.emptystack:

    # error when stack is empty, or doen't have enough operands
    movq    stderr, %rdi
    leaq    emptystackerror, %rsi
    movb    $0, %al
    call    fprintf
    jmp     .input          # recieve new input

.overflow:

    # error when result is overflow
    movq    stderr, %rdi
    leaq    overflowerror, %rsi
    movb    $0, %al
    call    fprintf
    jmp     .exit_fail      # terminate with error

.dividebyzero:

    # error when trying to divide by 0
    movq    stderr, %rdi
    leaq    divideerror, %rsi
    movb    $0, %al
    call    fprintf
    jmp     .exit_fail      # terminate with error

.remainderbyzero:

    # error when trying to get remainder when dividing by 0
    movq    stderr, %rdi
    leaq    remaindererror, %rsi
    movb    $0, %al
    call    fprintf
    jmp     .exit_fail      # terminate with error

.exit_fail:

    # end process with error
    movq    $EXIT_FAILURE, %rax
    movq    %rbp, %rsp
    popq    %rbp
    ret            # return EXIT_FAILURE
