INCLUDE Irvine32.inc

.data
msgBefore BYTE "Before Rotation: ", 0
msgAfter BYTE "After Rotation: ", 0
Array WORD 1, 2, 3, 4, 5, 6, 7, 8, 9, 10    ; Initial array of 10 words

.code
main PROC
    ; ========== Print initial array ==========
    mov edx, OFFSET msgBefore
    call WriteString
    mov esi, OFFSET Array           ; ESI points to start of array
    mov ecx, LENGTHOF Array         ; Loop counter = number of elements
    
    PrintBeforeLoop:
        movzx eax, WORD PTR [esi]   ; Load current element to EAX pointed by ESI
        call WriteDec               ; Print value
        mov al, ' '                 ; Prepare space character
        call WriteChar              ; Print space
        add esi, TYPE Array         ; Move to next element
        loop PrintBeforeLoop        ; Loop until ECX is 0
    
    call Crlf
    
    ; ========== Rotate array left by one position ==========
    mov esi, OFFSET Array           ; ESI points to start of array
    mov ax, [esi]                   ; Save first element in AX
    mov ecx, LENGTHOF Array - 1     ; Loop counter = 9 (n-1 shifts)
    
    ShiftLoop:
        mov dx, [esi + TYPE Array]  ; Load next element
        mov [esi], dx               ; Move it to current position
        add esi, TYPE Array         ; Move to next element
        loop ShiftLoop              ; Loop until ECX is 0
    
    mov [esi], ax                   ; Store saved first element at the end of array
    
    ; ========== Print rotated array ==========
    mov edx, OFFSET msgAfter
    call WriteString
    mov esi, OFFSET Array        ; ESI points to start of array
    mov ecx, LENGTHOF Array      ; Loop counter = number of elements
    
    PrintAfterLoop:
        movzx eax, WORD PTR [esi]   ; Load current element
        call WriteDec               ; Print value
        mov al, ' '                 ; Prepare space character
        call WriteChar              ; Print space
        add esi, TYPE Array         ; Move to next element
        loop PrintAfterLoop
    
    exit
main ENDP
END main
