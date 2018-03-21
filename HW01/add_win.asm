                section         .text

                global          main
				extern 			GetStdHandle
				extern 			ReadConsoleA
				extern 			WriteConsoleA
				extern			ExitProcess
main:

                sub             esp, 2 * 128 * 4
                lea             edi, [esp + 128 * 4]
                mov             ecx, 128
                call            read_long
                mov             edi, esp
                call            read_long			
                lea             esi, [esp + 128 * 4]
                call            add_long_long
				
                call            write_long

                mov             al, 13
                call            write_char
                mov             al, 10
                call            write_char

                jmp             exit

; adds two long number
;    edi -- address of summand #1 (long number)
;    esi -- address of summand #2 (long number)
;    ecx -- length of long numbers in dwords
; result:
;    sum is written to edi
add_long_long:
                push            edi
                push            esi
                push            ecx

                clc
.loop:
                mov             eax, [esi]
                lea             esi, [esi + 4]
                adc             [edi], eax
                lea             edi, [edi + 4]
                dec             ecx
                jnz             .loop

                pop             ecx
                pop             esi
                pop             edi
                ret

; adds 32-bit number to long number
;    edi -- address of summand #1 (long number)
;    eax -- summand #2 (32-bit unsigned)
;    ecx -- length of long number in dwords
; result:
;    sum is written to edi
add_long_short:
                push            edi
                push            ecx
                push            edx

                xor             edx,edx
.loop:
                add             [edi], eax
                adc             edx, 0
                mov             eax, edx
                xor             edx, edx
                add             edi, 4
                dec             ecx
                jnz             .loop

                pop             edx
                pop             ecx
                pop             edi
                ret

; multiplies long number by a short
;    edi -- address of multiplier #1 (long number)
;    ebx -- multiplier #2 (32-bit unsigned)
;    ecx -- length of long number in dwords
; result:
;    product is written to edi
mul_long_short:
                push            eax
                push            edi
                push            ecx
				
                xor             esi, esi
.loop:
                mov             eax, [edi]
                mul             ebx
                add             eax, esi
                adc             edx, 0
                mov             [edi], eax
                add             edi, 4
                mov             esi, edx
                dec             ecx
                jnz             .loop
				
                pop             ecx
                pop             edi
                pop             eax
                ret

; divides long number by a short
;    edi -- address of dividend (long number)
;    ebx -- divisor (32-bit unsigned)
;    ecx -- length of long number in dwords
; result:
;    quotient is written to edi
;    edx -- remainder
div_long_short:
                push            edi
                push            eax
                push            ecx

                lea             edi, [edi + 4 * ecx - 4]
                xor             edx, edx

.loop:
                mov             eax, [edi]
                div             ebx
                mov             [edi], eax
                sub             edi, 4
                dec             ecx
                jnz             .loop

                pop             ecx
                pop             eax
                pop             edi
                ret

; assigns a zero to long number
;    edi -- argument (long number)
;    ecx -- length of long number in dwords
set_zero:
                push            eax
                push            edi
                push            ecx

                xor             eax, eax
.loop:
				mov 			[edi], eax
				add				edi, 4
				dec 			ecx
				jne 			.loop

                pop             ecx
                pop             edi
                pop             eax
                ret

; checks if a long number is a zero
;    edi -- argument (long number)
;    ecx -- length of long number in dwords
; result:
;    ebx is equal to number of non-zero words in number
is_zero:
                push            eax
                push            edi
				push			edx
                push            ecx

                xor             eax, eax
.loop:			
				mov				edx, [edi]
				
				xor				edx, 0
				jz 				.next
				inc 			eax
.next:			
				add 			edi, 4
				dec 			ecx
				cmp 			ecx, 1
				jne 			.loop

				mov				ebx, eax
				
                pop             ecx
				pop				edx
                pop             edi
                pop             eax
                ret

; read long number from stdin
;    edi -- location for output (long number)
;    ecx -- length of long number in dwords
read_long:
                push            ecx
                push            edi
				
                call            set_zero
				
.loop:
				
                call            read_char
                or              eax, eax
                js              exit
				cmp				eax, 13
				je				.loop
                cmp             eax, 10
                je              .done
                cmp             eax, '0'
                jb              .invalid_char
                cmp             eax, '9'
                ja              .invalid_char

                sub             eax, '0'
                mov             ebx, 10
                call            mul_long_short
                call            add_long_short
                jmp             .loop

.done:
                pop             edi
                pop             ecx
                ret

.invalid_char:
                mov             esi, invalid_char_msg
                mov             edx, invalid_char_msg_size
                call            print_string
                call            write_char
                mov             al, 0x0a
                call            write_char

.skip_loop:
                call            read_char
                or              eax, eax
                js              exit
                cmp             eax, 0x0a
                je              exit
                jmp             .skip_loop

; write long number to stdout
;    edi -- argument (long number)
;    ecx -- length of long number in dwords
write_long:
                push            eax
                push            ecx

                mov             eax, 20
                mul             ecx
                mov             ebp, esp
                sub             esp, eax

                mov             esi, ebp

.loop:
                mov             ebx, 10
                call            div_long_short
                add             edx, '0'
                dec             esi
                mov             [esi], dl
                call            is_zero
				xor				ebx, 0
                jnz             .loop
				
                mov             edx, ebp
                sub             edx, esi
                call            print_string

                mov             esp, ebp
                pop             ecx
                pop             eax
                ret

; read one char from stdin
; result:
;    eax == -1 if error occurs
;    eax \in [0; 255] if OK
read_char:
                push            ecx
                push            edi
		
				push 			-10
				call 			GetStdHandle
				
				push 			0
				push 			num_written   
				push 			1
				push 			read_place
				push 			eax
				call 			ReadConsoleA
				
				
				

                cmp             byte [num_written], 1
                jne             .error
                xor             eax, eax
                mov             al, [read_place]

                pop             edi
                pop             ecx
                ret
.error:
                mov             eax, -1
				pop edx
                pop             edi
                pop             ecx
                ret

; write one char to stdout, errors are ignored
;    al -- char
write_char:
				push 			edx
                sub             esp, 1
                mov             [esp], al
				mov 			edx, esp
				
				push 			-11
				call 			GetStdHandle
				
				push 			0
				push			num_written
				push 			1
				push 			edx
				push 			eax
				call 			WriteConsoleA
				
                add             esp, 1
				pop 			edx
                ret

exit:
                push 			0
				call 			ExitProcess

; print string to stdout
;    esi -- string
;    edx -- size
print_string:
                push            eax
				
				push 			-11
				call 			GetStdHandle
				
				push 			0
				push			num_written
				push 			edx
				push 			esi
				push 			eax
				call 			WriteConsoleA
				
                pop             eax
                ret
				
                section         .data
invalid_char_msg:		db              "Invalid character: "
invalid_char_msg_size: equ             $ - invalid_char_msg



				section 		.bss
write_place:			resb 			256
read_place:				resb 			256
num_written:			resb 			10 ; amount of processed bytes
				
				
				
				
