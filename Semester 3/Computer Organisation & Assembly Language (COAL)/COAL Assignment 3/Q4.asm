INCLUDE Irvine32.inc

.data
    Arr1 DWORD 1,2,3,4,5,5,5,5,5,10        ; Has five consecutive 5's
    Arr2 DWORD 5,5,5,5,1,2,3,4,5           ; Only four consecutive 5's
    Arr3 DWORD 5,5,5,5,5,1,2,3,4,5         ; Has five consecutive 5's at start
    Arr4 DWORD 1,2,3,4,5,6,7,8,9,10        ; No consecutive 5's
    Arr5 DWORD 5,1,5,1,5,1,5,1,5           ; No consecutive 5's
    
    msgArray     BYTE "Array ",0
    msgContents  BYTE "Contents: ",0
    msgHas       BYTE "HAS five consecutive 5's",0
    msgNo        BYTE "NO five consecutive 5's",0
    msgSize      BYTE "Size: ",0
    space        BYTE " ",0
    comma        BYTE ", ",0

.code
main PROC
    
    mov edx, OFFSET msgArray    ; Test Array 1
    call WriteString
    mov eax, 1
    call WriteDec
    call Crlf
    
    push OFFSET Arr1
    push LENGTHOF Arr1
    call PrintArray
    push OFFSET Arr1
    push LENGTHOF Arr1
    call DisplayArrayResult
    call Crlf
    
    mov edx, OFFSET msgArray    ; Test Array 2
    call WriteString
    mov eax, 2
    call WriteDec
    call Crlf
    
    push OFFSET Arr2
    push LENGTHOF Arr2
    call PrintArray
    push OFFSET Arr2
    push LENGTHOF Arr2
    call DisplayArrayResult
    call Crlf
    
    mov edx, OFFSET msgArray     ; Test Array 3
    call WriteString
    mov eax, 3
    call WriteDec
    call Crlf
    
    push OFFSET Arr3
    push LENGTHOF Arr3
    call PrintArray
    push OFFSET Arr3
    push LENGTHOF Arr3
    call DisplayArrayResult
    call Crlf
        
    mov edx, OFFSET msgArray    ; Test Array 4
    call WriteString
    mov eax, 4
    call WriteDec
    call Crlf
    
    push OFFSET Arr4
    push LENGTHOF Arr4
    call PrintArray
    push OFFSET Arr4
    push LENGTHOF Arr4
    call DisplayArrayResult
    call Crlf
    
    mov edx, OFFSET msgArray    ; Test Array 5
    call WriteString
    mov eax, 5
    call WriteDec
    call Crlf
    
    push OFFSET Arr5
    push LENGTHOF Arr5
    call PrintArray
    push OFFSET Arr5
    push LENGTHOF Arr5
    call DisplayArrayResult
    exit
main ENDP

PrintArray PROC
    push ebp
    mov ebp, esp
    push eax            ; Preserve registers
    push ebx
    push ecx
    push edx
    push esi
        
    mov edx, OFFSET msgContents ; Display "Contents: " message
    call WriteString
    
    mov esi, [ebp+12]   ; ESI = array pointer
    mov ecx, [ebp+8]    ; ECX = array size
    mov ebx, 0          ; EBX = current index
    
    cmp ecx, 0          ; Check if array is empty
    je PrintDone
    
    PrintLoop:
        mov eax, [esi + ebx*4]  ; Get array element (DWORD = 4 bytes)
        call WriteDec
        inc ebx
        cmp ebx, ecx
        jge PrintDone
        mov edx, OFFSET comma
        call WriteString
        jmp PrintLoop

    PrintDone:
        call Crlf
        pop esi             ; Restore registers
        pop edx
        pop ecx
        pop ebx
        pop eax
        mov esp, ebp
        pop ebp
        ret 8               ; Clean up 8 bytes of parameters
PrintArray ENDP

DisplayArrayResult PROC
    push ebp
    mov ebp, esp
    pushad               ; Save all general-purpose registers
    
    mov edx, OFFSET msgSize ; Display array size
    call WriteString
    mov eax, [ebp+8]        ; Array size
    call WriteDec
    mov al, ' '
    call WriteChar
    
    push [ebp+12]       ; Array pointer
    push [ebp+8]        ; Array size
    call FindFive       ; Call FindFive procedure
    
    cmp eax, 1          ; Display result
    je HasFive
    mov edx, OFFSET msgNo
    jmp DisplayMsg

    HasFive:
        mov edx, OFFSET msgHas
    DisplayMsg:
        call WriteString
        call Crlf
        popad                ; Restore all general-purpose registers
        mov esp, ebp
        pop ebp
        ret 8               ; Clean up 8 bytes of parameters
DisplayArrayResult ENDP

FindFive PROC
    push ebp
    mov ebp, esp
    push ebx            ; Preserve registers
    push ecx
    push edx
    push esi
    push edi
    
    mov esi, [ebp+12]   ; ESI = array pointer
    mov ecx, [ebp+8]    ; ECX = array size
    
    ; If array size < 5, cannot have five consecutive 5's
    cmp ecx, 5
    jl NotFound
 
    xor edi, edi        ; EDI = consecutive count
    
    mov ebx, 0          ; EBX = current index
    CheckLoop:          ; Loop through array
        cmp ebx, ecx        ; Check if we've reached end of array
        jge NotFound
    
        ; Check if current element is 5
        mov eax, [esi + ebx*4]  ; Get array element (DWORD = 4 bytes)
        cmp eax, 5
        jne ResetCounter
    
        inc edi             ; Current element is 5 - increment consecutive counter
        cmp edi, 5          ; Check if we found five consecutive
        je Found
   
        inc ebx             ; Move to next element
        jmp CheckLoop

        ResetCounter:
            xor edi, edi        ; Current element is not 5 - reset counter
            inc ebx
            jmp CheckLoop
    Found:
        mov eax, 1          ; Return 1 (found)
        jmp Done
    NotFound:
        mov eax, 0          ; Return 0 (not found)
    Done:
        pop edi             ; Restore registers
        pop esi
        pop edx
        pop ecx
        pop ebx
        mov esp, ebp
        pop ebp
        ret 8               ; Clean up 8 bytes of parameters
FindFive ENDP
END main
