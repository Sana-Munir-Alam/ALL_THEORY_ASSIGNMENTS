INCLUDE Irvine32.inc

.data
msg1 BYTE "After SHL AL,3: ",0
msg2 BYTE "After SAL AL,3: ",0
msg3 BYTE "After ROL AL,1: ",0
msg4 BYTE "After RCR AL,3: ",0

.code
main PROC
    ; SHL AL,3
    MOV AL, 0D4H
    SHL AL, 3
    MOV EDX, OFFSET msg1
    call WriteString
    MOVZX EAX, AL
    call WriteHex
    call Crlf

    ; SAL AL,3
    MOV AL, 0D4H
    SAL AL, 3
    MOV EDX, OFFSET msg2
    call WriteString
    MOVZX EAX, AL
    call WriteHex
    call Crlf
    
    ; ROL AL,1
    STC
    MOV AL, 0D4H
    ROL AL, 1
    MOV EDX, OFFSET msg3
    call WriteString
    MOVZX EAX, AL
    call WriteHex
    call Crlf
    
    ; RCR AL,3
    STC
    MOV AL, 0D4H
    RCR AL, 3
    MOV EDX, OFFSET msg4
    call WriteString
    MOVZX EAX, AL
    call WriteHex
    call Crlf
    
    exit
main ENDP
END main
