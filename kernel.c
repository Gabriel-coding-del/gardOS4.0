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

#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define WHITE_ON_BLACK 0x0F

volatile uint16_t* vga_buffer = (volatile uint16_t*)0xB8000;

static int cursor_row = 0;
static int cursor_col = 0;

// Scrolls the screen up by one row.
void scroll() {
    for (int row = 1; row < VGA_HEIGHT; row++) {
        for (int col = 0; col < VGA_WIDTH; col++) {
            vga_buffer[(row - 1) * VGA_WIDTH + col] = vga_buffer[row * VGA_WIDTH + col];
        }
    }
    uint16_t blank = (WHITE_ON_BLACK << 8) | ' ';
    for (int col = 0; col < VGA_WIDTH; col++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + col] = blank;
    }
    cursor_row = VGA_HEIGHT - 1;
    cursor_col = 0;
}

// Clears the entire screen and resets the cursor.
void clear_screen() {
    uint16_t blank = (WHITE_ON_BLACK << 8) | ' ';
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = blank;
    }
    cursor_row = 0;
    cursor_col = 0;
}

// Prints a single character at the current cursor position and advances the cursor.
void put_char(char c) {
    if (c == '\n') {
        cursor_row++;
        cursor_col = 0;
    } else {
        vga_buffer[cursor_row * VGA_WIDTH + cursor_col] = (WHITE_ON_BLACK << 8) | c;
        cursor_col++;
        if (cursor_col >= VGA_WIDTH) {
            cursor_col = 0;
            cursor_row++;
        }
    }
    // If the cursor reaches beyond the bottom, scroll the screen.
    if (cursor_row >= VGA_HEIGHT) {
        scroll();
    }
}

// Prints a null-terminated string.
void print_string(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        put_char(str[i]);
    }
}

// Reads a byte from an I/O port.
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Maps simple keyboard scancodes to ASCII characters.
char scancode_to_ascii(uint8_t sc) {
    switch (sc) {
        case 0x02: return '1';
        case 0x03: return '2';
        case 0x04: return '3';
        case 0x05: return '4';
        case 0x06: return '5';
        case 0x07: return '6';
        case 0x08: return '7';
        case 0x09: return '8';
        case 0x0A: return '9';
        case 0x0B: return '0';

        case 0x10: return 'q';
        case 0x11: return 'w';
        case 0x12: return 'e';
        case 0x13: return 'r';
        case 0x14: return 't';
        case 0x15: return 'y';
        case 0x16: return 'u';
        case 0x17: return 'i';
        case 0x18: return 'o';
        case 0x19: return 'p';

        // Backspace (scancode 0x0E)
        case 0x0E: return '\b';

        case 0x1E: return 'a';
        case 0x1F: return 's';
        case 0x20: return 'd';
        case 0x21: return 'f';
        case 0x22: return 'g';
        case 0x23: return 'h';
        case 0x24: return 'j';
        case 0x25: return 'k';
        case 0x26: return 'l';

        case 0x1C: return '\n'; // Enter key

        case 0x2C: return 'z';
        case 0x2D: return 'x';
        case 0x2E: return 'c';
        case 0x2F: return 'v';
        case 0x30: return 'b';
        case 0x31: return 'n';
        case 0x32: return 'm';

        case 0x39: return ' ';

        default: return 0;
    }
}

// Waits until there is data in the keyboard buffer.
void wait_for_key() {
    while ((inb(0x64) & 1) == 0) { }
}

// Reads characters into destination, up to a maximum length (including space for '\0').
// Stops when Enter is pressed. Handles backspace by removing the last character.
void input(int len, char *destination) {
    int count = 0;
    while (1) {
        wait_for_key();
        uint8_t scancode = inb(0x60);

        // Ignore key release events.
        if (scancode & 0x80)
            continue;

        char c = scancode_to_ascii(scancode);
        if (c == 0)
            continue;

        // Handle backspace: delete the last character.
        if (c == '\b') {
            if (count > 0) {
                count--;
                // Move cursor back one position.
                if (cursor_col > 0) {
                    cursor_col--;
                } else if (cursor_row > 0) {
                    cursor_row--;
                    cursor_col = VGA_WIDTH - 1;
                }
                // Erase the character on screen.
                vga_buffer[cursor_row * VGA_WIDTH + cursor_col] = (WHITE_ON_BLACK << 8) | ' ';
            }
            continue;
        }

        // On Enter key, terminate input.
        if (c == '\n') {
            destination[count] = '\0';
            put_char('\n');
            break;
        }

        // Append character to input buffer if there's space.
        if (count < len - 1) {
            destination[count++] = c;
            put_char(c);
        }
    }
}
// Function to compare strings
int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s2 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

int str_to_int(const char *str) {
    int num = 0;
    int sign = 1;
    int i = 0;

    // Check for negative numbers
    if (str[0] == '-') {
        sign = -1;
        i++;
    }

    for (; str[i] != '\0'; i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            num = num * 10 + (str[i] - '0');
        } else {
            break; // Stop conversion if non-numeric character is encountered
        }
    }

    return num * sign;
}

void print_int(int num) {
    char buffer[12]; // Enough for a 32-bit integer (-2^31 to 2^31-1)
    int i = 0, is_negative = 0;

    if (num < 0) {
        is_negative = 1;
        num = -num;
    }

    do {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    } while (num > 0);

    if (is_negative) buffer[i++] = '-';

    buffer[i] = '\0';

    // Reverse the buffer for correct ordering
    for (int j = 0; j < i / 2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }

    print_string(buffer);
}
// Function to execute commands
void execute_command(const char *cmd) {
    if (strcmp(cmd, "clear") == 0) {
        clear_screen();
    } else if (strcmp(cmd, "help") == 0) {
        print_string("\nAvailable commands:\n");
        print_string("  clear - Clears the screen\n");
        print_string("  reboot - Reboots the system\n");
        print_string("  percentages - run the percentages program\n");
    } else if (strcmp(cmd, "reboot") == 0) {
        print_string("\nRebooting...\n");
        __asm__ volatile ("outb %b0, %1" :: "a"((uint8_t) 0xFE), "Nd"(0x64));
    } else if (strcmp(cmd, "percentages") == 0) {
		char num1[10], num2[10];
        
        print_string("num 1: ");
        input(9, num1);
        print_string("\n");
        
        print_string("num 2: ");
        input(9, num2);
        print_string("\n");
        
        int int1 = str_to_int(num1);
        int int2 = str_to_int(num2);
        
        if (int2 == 0) {
            print_string("Error: Division by zero.\n");
            return;
        }
        
                // Cast one operand to float to get precision
        float percentage = ((float)int1 / (float)int2) * 100;
        
        print_string("Result: ");
        print_int((int)percentage);  // Convert back to int for display
        print_string("%\n");
    } else {
        print_string("\nUnknown command.\n");
    }
}

void kernel_main(void) {
    clear_screen();
    print_string("================================================================================");
    while (1) {
        print_string("[gardOS] $ ");
        char buffer[151];
        input(150, buffer);

        execute_command(buffer);
    }
}
