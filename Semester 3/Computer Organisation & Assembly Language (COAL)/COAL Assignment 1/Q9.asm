INCLUDE Irvine32.inc

.data
; Equation: A = (A + B) - (C + D) 
valuesMsg BYTE "Using values: A=27, B=32, C=12, D=45", 0
resultMsg BYTE "Result of Equation: ", 0

.code
main PROC
     ; ========== Display initial values ==========
    mov edx, OFFSET valuesMsg
    call WriteString
    call Crlf

    ; ========== Initialize registers with values (Decimal) ==========
    mov eax, 32d    ; A = 27
    mov ebx, 27d    ; B = 32
    mov ecx, 12d    ; C = 12
    mov edx, 45d    ; D = 45

    ; ========== Calculate A = (A + B) - (C + D) ==========
    ; Step 1: Calculate (A + B)
    add eax, ebx    ; EAX = EAX + EBX       => A = 27d + 32d    
                    ; EAX holds the sum of A and B which is EAX = 59d
    
    ; Step 2: Calculate (C + D)
    add ecx, edx    ; ECX = ECX + EDX       => C = 12d + 45d
                    ; ECX holds the sum of C and D which is ECX = 57d
    
    ; Step 3: Calculate (A+B) - (C+D)
    sub eax, ecx    ; EAX = EAX - ECX       => A = (A + B) - (C + D)
                    ; EAX holds the final answer which is EAX = 2d

    ; ========== Display the result ==========
    mov edx, OFFSET resultMsg
    call WriteString
    call WriteDec   ; Display result stored in EAX which is 2 in decimal

exit
main ENDP
END main
