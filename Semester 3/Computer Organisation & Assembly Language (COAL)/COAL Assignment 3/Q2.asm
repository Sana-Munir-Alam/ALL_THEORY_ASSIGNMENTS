INCLUDE Irvine32.inc

.data
    buffer BYTE 200 DUP(0)
    len    DWORD ?

    countA DWORD 0
    countE DWORD 0
    countI DWORD 0
    countO DWORD 0
    countU DWORD 0

    msgPrompt BYTE "Enter a string: ",0
    msgVA BYTE "a or A = ",0
    msgVE BYTE "e or E = ",0
    msgVI BYTE "i or I = ",0
    msgVO BYTE "o or O = ",0
    msgVU BYTE "u or U = ",0

.code
main PROC
    mov  edx, OFFSET msgPrompt
    call WriteString
    mov  edx, OFFSET buffer
    mov  ecx, 199
    call ReadString
    mov  len, eax

    mov  ecx, len
    mov  esi, OFFSET buffer

nextChar:
    cmp  ecx, 0
    je   show

    mov  al, [esi]

    cmp  al, 'a'
    je   incA
    cmp  al, 'A'
    je   incA

    cmp  al, 'e'
    je   incE
    cmp  al, 'E'
    je   incE

    cmp  al, 'i'
    je   incI
    cmp  al, 'I'
    je   incI

    cmp  al, 'o'
    je   incO
    cmp  al, 'O'
    je   incO

    cmp  al, 'u'
    je   incU
    cmp  al, 'U'
    je   incU
    jmp  cont

incA:
    inc countA
    jmp cont

incE:
    inc countE
    jmp cont

incI:
    inc countI
    jmp cont

incO:
    inc countO
    jmp cont

incU:
    inc countU

cont:
    inc  esi
    dec  ecx
    jmp  nextChar

show:
    mov  edx, OFFSET msgVA
    call WriteString
    mov  eax, countA
    call WriteDec
    call Crlf

    mov  edx, OFFSET msgVI
    call WriteString
    mov  eax, countI
    call WriteDec
    call Crlf

    mov  edx, OFFSET msgVU
    call WriteString
    mov  eax, countU
    call WriteDec
