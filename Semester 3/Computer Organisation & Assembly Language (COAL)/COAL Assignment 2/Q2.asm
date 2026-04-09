INCLUDE Irvine32.inc

.data
Num1    DWORD ?
Num2    DWORD ?
Num3    DWORD ?
Num4    DWORD ?
result  DWORD ?
prompt  BYTE "Enter a Number: ", 0

.code
main PROC
    mov ecx, 4              ; Loop counter for 4 Numbers input
    mov esi, OFFSET Num1    ; Pointer to first variable
    
    InputLoop:
        mov edx, OFFSET prompt  ; Load Prompt message
        Call WriteString        ; Displays prompt
    
        Call ReadInt            ; This reads integer into EAX
        Call Crlf
    
        mov [esi], eax          ; Store in current variable
        add esi, 4              ; Move to next variable (Each DWORD is 4 bytes)
        loop InputLoop
    
    ; First condition: if (Num1 > Num2 && Num3 != Num4)
    mov eax, Num1
    cmp eax, Num2             ; Compare Num1 and Num2
    jle CheckElse_PartIf      ; Jump if Num1 <= Num2 (first condition fails)
    
    ; Second part of first condition: Num3 != Num4
    mov ebx, Num3
    cmp ebx, Num4             ; Compare Num3 and Num4
    je CheckElse_PartIf       ; Jump if Num3 == Num4 (first condition fails)
    
    ; First condition is true: result = Num1 + Num3
    mov eax, Num1
    add eax, Num3
    mov result, eax
    jmp End_If              ; Skip the rest
    
    CheckElse_PartIf:
        ; Second condition: Else_Part if (Num2 == Num4)
        mov eax, Num2
        cmp eax, Num4        ; Compare Num2 and Num4
        jne Else_Part        ; Jump if Num2 != Num4
    
        ; Second condition is true: result = Num2 - Num1
        mov eax, Num2
        sub eax, Num1
        mov result, eax
        jmp End_If           ; Skip Else_Part part
    
    Else_Part:
        mov result, 0        ; Else_Part part: result = 0
    
    End_If:
        mov eax, result
        call WriteInt
        call Crlf
exit
main ENDP
END main
