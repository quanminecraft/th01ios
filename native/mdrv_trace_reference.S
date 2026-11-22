.intel_syntax noprefix
.code16
.section .text
.global _start
_start:
 cli
 mov sp,offset stack_top
 sti
 push cs
 pop es
 mov bx,offset end_program
 add bx,15
 shr bx,4
 mov ah,0x4a
 int 0x21
 mov word ptr [stage],1
 mov bx,4096
 mov ah,0x48
 int 0x21
 jc failed
 mov [buffer_seg],ax
 mov ah,9
 int 0xf2
 mov ax,0x35f2
 int 0x21
 mov [driver_seg],es
 mov word ptr [stage],2
 cmp word ptr es:[0x18e3],0x80fb
 jne failed
 mov byte ptr es:[0x18e3],0xea
 mov word ptr es:[0x18e4],offset logtick
 mov ax,cs
 mov es:[0x18e6],ax
 mov byte ptr es:[0x18e8],0x90
 mov ax,es
 mov [logtick_segment],ax
 mov si,offset patches
 mov cx,6
patch:
 mov di,[si]
 cmp byte ptr es:[di],0xc3
 je nextpatch
 cmp word ptr es:[di],0x5350
 jne failed
 cmp word ptr es:[di+2],0x8a52
 jne failed
 cmp byte ptr es:[di+4],0xd8
 jne failed
 mov byte ptr es:[di],0xea
 mov ax,[si+2]
 mov es:[di+1],ax
 mov ax,cs
 mov es:[di+3],ax
 mov bx,[si+4]
 mov ax,es
 mov [bx],ax
 nextpatch:
 add si,6
 loop patch
 mov si,offset song
 mov ah,6
 int 0xf2
 xor ax,ax
 int 0xf2
 mov ah,0x2c
 int 0x21
 mov [start_second],dh
wait:
 mov ah,0x2c
 int 0x21
 mov al,dh
 sub al,[start_second]
 jnc elapsed
 add al,60
elapsed:
 cmp al,8
 jb wait
 mov ah,3
 int 0xf2
 mov ax,[driver_seg]
 mov es,ax
 mov word ptr es:[0x18e3],0x80fb
 mov word ptr es:[0x18e5],0x383e
 mov word ptr es:[0x18e7],0xff01
 mov si,offset patches
 mov cx,6
restore:
 mov di,[si]
 cmp byte ptr es:[di],0xea
 jne nextrestore
 mov word ptr es:[di],0x5350
 mov word ptr es:[di+2],0x8a52
 mov byte ptr es:[di+4],0xd8
 nextrestore:
 add si,6
 loop restore
 mov dx,offset filename
 xor cx,cx
 mov ah,0x3c
 int 0x21
 jc failed
 mov bx,ax
 mov cx,[position]
 mov ax,[buffer_seg]
 mov ds,ax
 xor dx,dx
 mov ah,0x40
 int 0x21
 mov ah,0x3e
 int 0x21
 push cs
 pop ds
 mov dx,offset metafile
 xor cx,cx
 mov ah,0x3c
 int 0x21
 mov bx,ax
 mov dx,offset position
 mov cx,6
 mov ah,0x40
 int 0x21
 mov ah,0x3e
 int 0x21
 mov ax,0x4c00
 int 0x21
failed:
 mov [err_ax],ax
 mov [err_es],es
 mov [err_di],di
 mov ax,es:[di]
 mov [err_bytes],ax
 mov dx,offset errname
 xor cx,cx
 mov ah,0x3c
 int 0x21
 mov bx,ax
 mov cx,10
 mov dx,offset stage
 mov ah,0x40
 int 0x21
 mov ah,0x3e
 int 0x21
 mov ax,0x4c01
 int 0x21
.macro tracer name, back, bank
\name:
 pushf
 cli
 pusha
 push ds
 push es
 mov bl,al
 mov bh,dh
 mov cx,\bank
 .if \bank == 2
 xor cx,cx
 cmp bl,0x30
 jb bankdone\@
 cmp byte ptr [0x1ca],0
 je bankdone\@
 inc cx
bankdone\@:
 .endif
 push cs
 pop ds
 mov di,[position]
 cmp di,65526
 jae overflow\@
 mov ax,[buffer_seg]
 mov es,ax
 mov ax,[ticks]
 mov es:[di],ax
 mov al,bl
 mov ah,cl
 mov es:[di+2],ax
 mov es:[di+4],bh
 mov byte ptr es:[di+5],0
 add word ptr [position],6
 jmp done\@
overflow\@:
 mov word ptr [overflow],1
done\@:
 pop es
 pop ds
 popa
 popf
 push ax
 push bx
 push dx
 mov bl,al
 .byte 0xea
 .word \back+5
\name\()_segment:
 .word 0
.endm
tracer log0,0x1ed0,0
tracer log1,0x1efd,2
tracer log2,0x1f32,2
tracer log3,0x1f6f,0
tracer log4,0x1fc8,1
tracer log5,0x1fee,1
logtick:
 pushf
 inc word ptr cs:[ticks]
 popf
 sti
 cmp byte ptr [0x138],0xff
 .byte 0xea
 .word 0x18e9
logtick_segment:.word 0
patches:
 .word 0x1ed0,log0,log0_segment
 .word 0x1efd,log1,log1_segment
 .word 0x1f32,log2,log2_segment
 .word 0x1f6f,log3,log3_segment
 .word 0x1fc8,log4,log4_segment
 .word 0x1fee,log5,log5_segment
stage:.word 0
err_ax:.word 0
err_es:.word 0
err_di:.word 0
err_bytes:.word 0
errname:.asciz "ERR.BIN"
buffer_seg:.word 0
driver_seg:.word 0
position:.word 0
ticks:.word 0
overflow:.word 0
start_second:.byte 0
filename:.asciz "TRACE.BIN"
metafile:.asciz "META.BIN"
song:.incbin "build/assets/th01-ja/REIMU.MDT"
.balign 2
.space 2048
stack_top:
end_program:
