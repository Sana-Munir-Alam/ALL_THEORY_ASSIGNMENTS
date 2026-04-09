INCLUDE Irvine32.inc

.data
Result WORD ?

.code
main PROC
    mov AX, 0H          ; Initialize AX to 0
    mov ECX, 0AH        ; Initialize ECX to 10 (loop counter)
    
    DOLOOP:
        DEC AX          ; Decrement AX
        LOOP DOLOOP     ; Decrement ECX and loop if not zero
    
    mov Result, AX      ; Store final value
    
    ; Display AX in hexadecimal
    movzx EAX, AX       ; Zero-extend AX to EAX for display
    call WriteHex       ; Display hexadecimal value
exit
main ENDP
END main
