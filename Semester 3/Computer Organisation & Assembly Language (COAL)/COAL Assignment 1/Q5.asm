INCLUDE Irvine32.inc

.data
; ======== SYMBOLIC CONSTANTS Q5(i) ========
Seconds EQU 60          ; No.of Seconds in a minute
Minutes EQU 60          ; No.of Miuntes in an Hour
Hours   EQU 24          ; No.of Hours in a day
Days    EQU 7           ; No.of Days in a week

SecondsInWeek DWORD ?   ; Variable to Store the Result in      
msg BYTE "Number of Seconds In A Week: ", 0

.code
main PROC
    ; ======== Calculating Seconds In a Week (Q5ii) ========
    mov eax, Seconds        ; EAX = Seconds         => EAX = 60
    imul eax, Minutes       ; EAX = EAX * Munites   => EAX = (60) * 60 = 3600
    imul eax, Hours         ; EAX = EAX * Hours     => EAX = (3600) * 24 = 86400
    imul eax, Days          ; EAX = EAX * Days      => EAX = (86400) * 7 = 604800

    ; ======== Saving Result in the Memory (Q5iii) ========
    mov SecondsInWeek, eax  ; Value in EAX stored in Variable SecondsInWeek

    ; ======== Displaying Result  ========
    mov   edx, OFFSET msg   ; EDX = address of the message string
    call  WriteString       ; Print the message
    call  WriteDec          ; Print the unsigned decimal value in EAX (604800)
    call  Crlf              ; Print a newline

    exit
main ENDP
END main
