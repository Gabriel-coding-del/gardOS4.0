; Copyright (C) 2025 Gabriel Sîrbu
;
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program. If not, see <https://www.gnu.org/licenses/>.
; boot.asm - Entry point for the kernel loaded by GRUB

; Multiboot header (must be in the first 8KB of the kernel)
section .multiboot
    align 4
    dd 0x1BADB002             ; Magic number
    dd 0                      ; Flags (set to 0 for minimal requirements)
    dd -(0x1BADB002)          ; Checksum

section .text
    global _start           ; Define entry point for the linker
    extern kernel_main      ; Reference the C kernel function

_start:
    call kernel_main        ; Jump to your C kernel code

hang:
    jmp hang                ; Infinite loop if kernel_main ever returns
