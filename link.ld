/*
 * Copyright (C) 2025 Gabriel Sîrbu
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

ENTRY(_start)

SECTIONS {
  . = 1M;  /* Load the kernel at 1MB */

  .multiboot : {
     *(.multiboot)
  }

  .text : {
     *(.text*)
  }

  .rodata : {
     *(.rodata*)
  }

  .data : {
     *(.data*)
  }

  .bss : {
     __bss_start__ = .;
     *(.bss*)
     __bss_end__ = .;
  }
}
