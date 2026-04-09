INCLUDE Irvine32.inc

.data
    promptString  BYTE "Enter a string: ",0
    promptChar    BYTE "Enter the leading character to remove: ",0
    originalMsg   BYTE "Original string: ",0
    trimmedMsg    BYTE "Trimmed string: ",0
    
    inputBuffer   BYTE 256 DUP(0)      ; Buffer for user input string
    trimChar      BYTE ?               ; Character to trim

.code
main PROC
    ; Read the string from user
    mov edx, OFFSET promptString
    call WriteString
    mov edx, OFFSET inputBuffer
    mov ecx, SIZEOF inputBuffer
    call ReadString

    ; Read the character to trim from user
    mov edx, OFFSET promptChar
    call WriteString
    call ReadChar        ; Character goes into AL
    call WriteChar       ; Echo the character
    mov trimChar, al     ; Store the character
    call Crlf
    call Crlf

    ; Display original string
    mov edx, OFFSET originalMsg
    call WriteString
    mov edx, OFFSET inputBuffer
    call WriteString
    call Crlf
    
    ; Call the custom trim procedure
    mov edx, OFFSET inputBuffer    ; pointer to string
    mov al, trimChar               ; character to remove
    call Str_trimLeading

    ; Display trimmed string
    mov edx, OFFSET trimmedMsg
    call WriteString
    mov edx, OFFSET inputBuffer
    call WriteString
    call Crlf
    
    exit
main ENDP

Str_trimLeading PROC
    pushad              ; Save all registers
    
    mov edi, edx        ; EDI = pointer to string (destination)
    mov esi, edx        ; ESI = pointer to string (source)
    mov bl, al          ; BL = character to remove
    
    ; Find first character that is NOT the trim character
SkipLoop:
    mov al, [esi]       ; Get current character
    cmp al, 0           ; Check for null terminator
    je Done             ; If null, we're done
    
    cmp al, bl          ; Compare with trim character
    jne CopyRemaining   ; If not equal, start copying
    
    inc esi             ; Skip this character (it matches trim char)
    jmp SkipLoop

CopyRemaining:
    ; Now copy the remaining string back to the beginning
    cmp esi, edi        ; Check if we need to copy at all
    je Done             ; If source = destination, no copy needed
    
CopyLoop:
    mov al, [esi]       ; Copy character from source
    mov [edi], al       ; to destination
    cmp al, 0           ; Check for null terminator
    je Done             ; If null, we're done
    inc esi             ; Move to next source character
    inc edi             ; Move to next destination position
    jmp CopyLoop

Done:
    popad               ; Restore all registers
    ret
Str_trimLeading ENDP
END main
