INCLUDE Irvine32.inc

.data
Value DWORD 5    ; Test with value 5: 5 × 26 = 130
result DWORD 0
msg1 BYTE "Original value: ",0
msg2 BYTE "After multiplication by 26: ",0

.code
main PROC
    
    mov EDX, OFFSET msg1    ; Display original value
    Call WriteString
    mov EAX, Value
    Call WriteDec
    Call Crlf
    
    ; Multiply by 26 using shifts and additions
    mov EAX, Value      ; EAX = 5
    
    ; Multpiplication by 26 => (EAX * 2^4) + (EAX * 2^3) + (EAX * 2^1)
    mov EBX, EAX        ; Save original value
    mov EDX, EAX
    SHL EBX, 4          ; ×16   so EBX(5) = (0000 0101) => shift 4 => EBX(80) = 0101 0000
    SHL EDX, 3          ; ×8    so EDX(5) = (0000 0101) => shift 3 => EDX(40) = 0101 0000
    SHL EAX, 1          ; x2    so EAX(5) = (0000 0101) => shift 1 => EAX(10) = 0000 1010
    
    ADD EAX, EDX        ; x2 + x8       => EAX = EAX + EDX   =>  10 + 40 = 50
    ADD EAX, EBX        ; x10 + x16     => EAX = EAX + EBX   =>  50 + 80 = 130
    mov result, EAX     ; Store in result
    
    mov EDX, OFFSET msg2    ; Display result
    Call WriteString
    Call WriteDec
    Call Crlf
    
exit
main ENDP
END main
