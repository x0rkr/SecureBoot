#ifndef SCREEN_H
#define SCREEN_H

/* VGA Colors */
#define COLOR_BLACK         0x00
#define COLOR_BLUE          0x01
#define COLOR_GREEN         0x02
#define COLOR_CYAN          0x03
#define COLOR_RED           0x04
#define COLOR_MAGENTA       0x05
#define COLOR_BROWN         0x06
#define COLOR_WHITE         0x07
#define COLOR_BRIGHT_GREEN  0x0A
#define COLOR_BRIGHT_CYAN   0x0B
#define COLOR_BRIGHT_RED    0x0C
#define COLOR_YELLOW        0x0E
#define COLOR_BRIGHT_WHITE  0x0F

/* Function Prototypes */
void screen_clear(void);
void screen_putchar(char c, unsigned char color);
void screen_print(const char *str, unsigned char color);
void screen_println(const char *str, unsigned char color);
void kprintf(const char *fmt, ...);

#endif

