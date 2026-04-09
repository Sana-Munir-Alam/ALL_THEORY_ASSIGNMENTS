INCLUDE Irvine32.inc

.data
    initDividend DWORD 0D4A4h
    initDivisor  DWORD 0Ah

    msgInitial     BYTE "Initial values:",0
    msgDividend    BYTE "  Dividend = ",0
    msgDivisor     BYTE "  Divisor  = ",0
    msgQuotient    BYTE "Quotient  = ",0
    msgRemainder   BYTE "Remainder = ",0
    szNewLine      BYTE 13,10,0

.code
main PROC
    ; Display initial values
    mov edx, OFFSET msgInitial
    call WriteString
    call Crlf

    mov edx, OFFSET msgDividend
    call WriteString
    mov eax, initDividend
    call WriteHex
    call Crlf

    mov edx, OFFSET msgDivisor
    call WriteString
    mov eax, initDivisor
    call WriteHex
    call Crlf
    call Crlf

    ; Call the recursive routine - push parameters right-to-left
    push initDivisor     ; [ebp+12] - divisor
    push initDividend    ; [ebp+8]  - dividend
    call DivideRecursive
    add esp, 8           ; caller cleans up params

    exit
main ENDP

DivideRecursive PROC
    push ebp
    mov ebp, esp
    push ebx            ; preserve EBX (callee-saved)
    push esi            ; preserve ESI (callee-saved)

    ; load parameters
    mov eax, [ebp+8]    ; eax = dividend
    mov ecx, [ebp+12]   ; ecx = divisor

    ; do unsigned division
    xor edx, edx
    div ecx             ; quotient -> EAX, remainder -> EDX

    ; save quotient and remainder
    mov ebx, eax        ; ebx = quotient
    mov esi, edx        ; esi = remainder

    ; Display quotient
    mov edx, OFFSET msgQuotient
    call WriteString
    mov eax, ebx        ; quotient in EAX
    call WriteHex
    call Crlf

    ; Display remainder
    mov edx, OFFSET msgRemainder
    call WriteString
    mov eax, esi        ; remainder in EAX
    call WriteHex
    call Crlf
    call Crlf

    ; BASE CASE: check if quotient (EBX) > 5h
    cmp ebx, 5h
    jbe BaseCase        ; if quotient <= 5h, stop recursion

    ; RECURSIVE CASE: call recursively with quotient as new dividend
    push [ebp+12]       ; push divisor
    push ebx            ; push quotient as new dividend
    call DivideRecursive
    add esp, 8          ; clean up pushed parameters

BaseCase:
    ; restore registers and return
    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret
DivideRecursive ENDP

END main
