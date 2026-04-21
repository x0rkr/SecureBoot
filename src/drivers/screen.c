#include "../include/screen.h"

/* VGA text buffer starts at 0xB8000
   Each cell = 2 bytes: [ASCII][Color]
   80 columns x 25 rows = 2000 cells */

#define VGA_BASE    ((volatile unsigned short*)0xB8000)
#define VGA_COLS    80
#define VGA_ROWS    25
#define DEFAULT_COLOR 0x0A    /* Bright green on black */

static int cursor_row = 0;
static int cursor_col = 0;

/* Clear the entire screen */
void screen_clear(void) {
    volatile unsigned short *vga = VGA_BASE;
    for (int i = 0; i < VGA_COLS * VGA_ROWS; i++) {
        vga[i] = (DEFAULT_COLOR << 8) | ' ';
    }
    cursor_row = 0;
    cursor_col = 0;
}

/* Scroll screen up by one line */
static void scroll(void) {
    volatile unsigned short *vga = VGA_BASE;
    for (int i = 0; i < (VGA_ROWS - 1) * VGA_COLS; i++) {
        vga[i] = vga[i + VGA_COLS];
    }
    for (int i = (VGA_ROWS - 1) * VGA_COLS; i < VGA_ROWS * VGA_COLS; i++) {
        vga[i] = (DEFAULT_COLOR << 8) | ' ';
    }
    cursor_row = VGA_ROWS - 1;
}

/* Print a single character */
void screen_putchar(char c, unsigned char color) {
    volatile unsigned short *vga = VGA_BASE;

    if (c == '\n') {
        cursor_col = 0;
        cursor_row++;
    } else if (c == '\r') {
        cursor_col = 0;
    } else if (c == '\b') {
        if (cursor_col > 0) cursor_col--;
        vga[cursor_row * VGA_COLS + cursor_col] = (color << 8) | ' ';
        return;
    } else {
        vga[cursor_row * VGA_COLS + cursor_col] = (color << 8) | (unsigned char)c;
        cursor_col++;
        if (cursor_col >= VGA_COLS) {
            cursor_col = 0;
            cursor_row++;
        }
    }

    if (cursor_row >= VGA_ROWS)
        scroll();
}

/* Print a null-terminated string */
void screen_print(const char *str, unsigned char color) {
    while (*str)
        screen_putchar(*str++, color);
}

/* Print string with newline */
void screen_println(const char *str, unsigned char color) {
    screen_print(str, color);
    screen_putchar('\n', color);
}

/* Custom printf — supports %s, %c, %d, %x */
void kprintf(const char *fmt, ...) {
    /* va_list manually — no stdlib */
    unsigned char *args = (unsigned char*)&fmt + sizeof(fmt);

    for (int i = 0; fmt[i]; i++) {
        if (fmt[i] != '%') {
            screen_putchar(fmt[i], DEFAULT_COLOR);
            continue;
        }
        i++;
        switch (fmt[i]) {
            case 's': {
                char *s = *(char**)args;
                args += sizeof(char*);
                screen_print(s, DEFAULT_COLOR);
                break;
            }
            case 'c': {
                char c = *(char*)args;
                args += sizeof(int);
                screen_putchar(c, DEFAULT_COLOR);
                break;
            }
            case 'd': {
                int num = *(int*)args;
                args += sizeof(int);
                if (num < 0) { screen_putchar('-', DEFAULT_COLOR); num = -num; }
                char buf[12];
                int idx = 0;
                if (num == 0) { screen_putchar('0', DEFAULT_COLOR); break; }
                while (num > 0) { buf[idx++] = '0' + (num % 10); num /= 10; }
                for (int j = idx - 1; j >= 0; j--)
                    screen_putchar(buf[j], DEFAULT_COLOR);
                break;
            }
            case 'x': {
                unsigned int num = *(unsigned int*)args;
                args += sizeof(unsigned int);
                char hex[] = "0123456789ABCDEF";
                char buf[8];
                int idx = 0;
                if (num == 0) { screen_print("0x0", DEFAULT_COLOR); break; }
                while (num > 0) { buf[idx++] = hex[num % 16]; num /= 16; }
                screen_print("0x", DEFAULT_COLOR);
                for (int j = idx - 1; j >= 0; j--)
                    screen_putchar(buf[j], DEFAULT_COLOR);
                break;
            }
            case '%':
                screen_putchar('%', DEFAULT_COLOR);
                break;
        }
    }
}

