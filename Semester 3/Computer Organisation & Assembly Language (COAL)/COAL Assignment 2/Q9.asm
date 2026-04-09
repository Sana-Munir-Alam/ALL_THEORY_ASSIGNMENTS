INCLUDE Irvine32.inc

.data
array      WORD 10,60,20,33,72,89,45,65,72,18
sample     WORD 50
sum        WORD 0
msg        BYTE "Sum of elements less than equal to 50: ",0

.code
main PROC
    mov ESI, OFFSET array     ; Point ESI to start of array
    mov ECX, LENGTHOF array   ; Loop counter
    mov AX, 0                 ; Clear sum (16-bit)
    
    WhileLoop:
        mov BX, [ESI]            ; BX = array[index]   Get current array element
        CMP BX, sample           ; if (array element <= sample)
        JA SkipAdd               ; JA for unsigned > (skip if greater)
    
        ADD AX, BX               ; If condition true Then add to sum
    
        SkipAdd:
            ADD ESI, TYPE array  ; move to next element
            loop WhileLoop
    
    mov sum, AX               ; Store result
    
    mov EDX, OFFSET msg       ; Display the result
    Call WriteString
    movzx EAX, AX             ; Zero-extend to 32-bit for WriteDec
    Call WriteDec
    Call Crlf
    
exit
main ENDP
END main
