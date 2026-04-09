INCLUDE Irvine32.inc

.data
num1    SWORD  -300
num2    SWORD  40
result  SDWORD ?

.code
main PROC
    ; Multiply using 16-bit operands
    mov EAX, 0          ; Clearing Register EBX
    mov EBX, 0          ; Clearing Register EBX
    mov EDX, 0
    mov AX, num1        ; Load first 16-bit number
    mov BX, num2        ; Load second 16-bit number
    IMUL BX             ; DX:AX = AX * BX (signed)
    
    ; Combine DX:AX into 32-bit result  [DX: Higher Byte AX: Lower Byte]
    movzx EAX, AX       ; Zero-extend AX to EAX
    movzx EDX, DX       ; Zero-extend DX to EDX
    SHL EDX, 16         ; Shift DX to upper 16 bits
    OR EAX, EDX         ; Combine into EAX (so Now EAx = Upper byte of EDX, and lower Byte of Ax stored)
    mov result, EAX     ; Store result
    
    ; Display the calculation
    movsx EAX, num1
    CALL WriteInt
    mov AL, ' '
    CALL WriteChar
    mov AL, '*'
    CALL WriteChar
    mov AL, ' '
    CALL WriteChar
    movsx EAX, num2
    CALL WriteInt
    mov AL, ' '
    CALL WriteChar
    mov AL, '='
