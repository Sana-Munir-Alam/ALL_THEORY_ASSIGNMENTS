INCLUDE Irvine32.inc

.data
op1    SDWORD 22
op2    SDWORD 36
x      SDWORD 15
y      SDWORD 29
z      SDWORD 10

.code
main PROC
    ; While (op1 <= op2)
    while_loop:
        mov eax, op1
        cmp eax, op2        ; Compare op1 with op2
        jg end_while        ; Exit if op1 > op2
    
        ; if (op1 >= x && op1 <= y)
        mov eax, op1
        cmp eax, x          ; Compare op1 with x
        jl else_part        ; Jump if op1 < x (first condition fails)
    
        cmp eax, y          ; Compare op1 with y
        jg else_part        ; Jump if op1 > y (second condition fails)
    
        ; Then part: z += 10
        mov eax, z
        add eax, 10
        mov z, eax
        jmp decrement_op1
    
    else_part:
        ; Else part: z -= 10
        mov eax, z
        sub eax, 10
        mov z, eax
    
    decrement_op1:
        ; op1--
        mov eax, op1
        dec eax
        mov op1, eax
    
        jmp while_loop      ; Continue while loop
    
end_while:
    exit
main ENDP
END main
