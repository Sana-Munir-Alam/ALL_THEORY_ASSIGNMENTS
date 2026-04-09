INCLUDE Irvine32.inc

.data
Sum     SDWORD 0
i       DWORD 0
Count   DWORD 5        ; Example: process 5 elements
Array   SDWORD 7, -3, 15, 0, -8  ; Example Array

.code
main PROC
    WhileLoop:              ; Starting While Loop
        mov ecx, i          ; Setting Counter Loop
        cmp ecx, Count      ; Compare i with Count
        jge EndWhile        ; Exit loop if i >= Count
    
        ; Calculate Array[i] address and load value
        mov esi, OFFSET Array  ; ESI points to beginning of Array
        mov eax, i
        imul eax, 4            ; Multiply index by 4 (size of SDWORD)
        add esi, eax           ; ESI now points to Array[i]
        mov ebx, [esi]         ; EBX = Array[i]
    
        cmp ebx, 0             ; Check if Array[i] > 0
        jle ElsePart           ; Jump if Array[i] <= 0
    
        ; If part is true then: Sum = Sum + Array[i]
        mov eax, Sum
        add eax, ebx
        mov Sum, eax
        jmp Increment_i
                                
        ElsePart:              ; Else part: Sum = Sum - 1
            mov eax, Sum
            dec eax            ; Subtract 1 from Sum
            mov Sum, eax
    
        Increment_i:
            ; i = i + 1
            mov eax, i
            inc eax
            mov i, eax
            jmp WhileLoop
    
    EndWhile:
        mov eax, Sum          ; Display the result
        call WriteInt         ; Display final Sum
exit
main ENDP
END main
