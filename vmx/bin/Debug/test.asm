\\ terminator equ 0 

scpy:   push bp
        mov bp, sp
        push eax
        push ebx
        mov eax, [bp+8]
        mov ebx, [bp+12]
        cmp [ebx], terminator
        jz scpy_fin
        mov [eax], [ebx]
        add eax, 1
        add ebx, 1
        jmp scpy_loop
scpy_fin mov [eax], terminator
        pop ebx
        pop eax
        mov sp, bp
        pop bp
        ret

_start: mov b[0], 'h
        mov b[1], 'o
        mov b[2], 'l
        mov b[3], 'a
        mov b[4], 0
        mov ebx, ds
        add ebx, 5
        push ds
        push ecx
        call scpy
        add sp, 8
        mov al, %02
        mov cl, 4
        mov ch, 1
        sys %1