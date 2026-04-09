INCLUDE Irvine32.inc

.data
positiveMsg BYTE " is POSITIVE",0
negativeMsg BYTE " is NEGATIVE",0
zeroMsg     BYTE " is ZERO",0
testMsg     BYTE "Testing value: ",0

.code
main PROC
    ; Test multiple values
    mov AL, 25          ; Will display positive message
    Call CheckSign
    
    mov AL, -15         ; Will display negative message
    Call CheckSign
    
    mov AL, 0           ; Will display zero message
    Call CheckSign

    mov AL, 127         ; Will display positive message
    Call CheckSign

    mov AL, -128        ; Will display negative message
    Call CheckSign
exit
main ENDP

CheckSign PROC
    mov EDX, OFFSET testMsg
    Call WriteString
    movSX EAX, AL       ; Sign-extend AL to EAX for display
    Call WriteInt       ; Display the signed value
    
    CMP AL, 0           ; Check if zero
    JE ZeroCase         ; If Compare Value 0 than jump to zero Case
    JG PositiveCase     ; If Compare Value is > 0 Jump to positive Case
    
    mov EDX, OFFSET negativeMsg     ; If both Jumps not used than the value is Negative so display
    JMP DisplayResult

    PositiveCase:
        mov EDX, OFFSET positiveMsg
        JMP DisplayResult

    ZeroCase:
        mov EDX, OFFSET zeroMsg

    DisplayResult:
        Call WriteString    ; Display the result message
        Call Crlf           ; New line
        ret
CheckSign ENDP
END main
