/*
 * SecureBoot Project
 * Dev: x0rkr
 * GitHub: github.com/x0rkr
 *
 * Bare-metal x86 authenticated bootloader
 * Written in NASM Assembly + C (no stdlib)
 */
#include "../include/screen.h"
#include "../include/crypto.h"
#include "../include/integrity.h"

/* Boot password XOR encryption key */
static const unsigned char xor_key[KEY_LEN] = {
    0x4B, 0x52, 0x59, 0x50, 0x54, 0x30, 0x42, 0x4F,
    0x4F, 0x54, 0x4B, 0x45, 0x59, 0x31, 0x36, 0x21
};

/* Password input buffer */
static unsigned char pwd_buffer[64];
static unsigned int  pwd_len = 0;

/*----------------------------------------------------------
  Read a single byte from keyboard port
----------------------------------------------------------*/
static unsigned char kb_read(void) {
    unsigned char sc, ch;
    const unsigned char kb_map[] =
        "  1234567890-=  qwertyuiop[]  asdfghjkl;'` \\zxcvbnm,./";

    while (1) {
        /* Wait for key press */
        unsigned char status;
        do {
            __asm__ volatile ("inb $0x64, %0" : "=a"(status));
        } while (!(status & 0x01));

        __asm__ volatile ("inb $0x60, %0" : "=a"(sc));

        if (sc & 0x80) continue;
        if (sc == 0x1C) return '\n';
        if (sc == 0x0E) return '\b';
        if (sc < sizeof(kb_map)) {
            ch = kb_map[sc];
            if (ch != ' ') return ch;
        }
    }
}
/*----------------------------------------------------------
  Password prompt UI
----------------------------------------------------------*/
static void draw_banner(void) {
    screen_clear();
    screen_println("", COLOR_BRIGHT_CYAN);
    screen_println("  ================================", COLOR_BRIGHT_CYAN);
    screen_println("       SECUREBOOT v1.0            ", COLOR_BRIGHT_WHITE);
    screen_println("       Authenticated Boot System  ", COLOR_BRIGHT_WHITE);
    screen_println("  ================================", COLOR_BRIGHT_CYAN);
    screen_println("", COLOR_WHITE);
}

static int prompt_password(void) {
    int attempts = 3;

    while (attempts > 0) {
        draw_banner();
        screen_print("  Attempts remaining: ", COLOR_YELLOW);

        /* Print attempt count */
        screen_putchar('0' + attempts, COLOR_YELLOW);
        screen_println("", COLOR_WHITE);
        screen_println("", COLOR_WHITE);
        screen_print("  Password: ", COLOR_BRIGHT_WHITE);

        pwd_len = 0;
        unsigned char c;

        while (1) {
            c = kb_read();

            if (c == '\n') break;

            if (c == '\b') {
                if (pwd_len > 0) {
                    pwd_len--;
                    /* Erase the star on screen */
                    screen_putchar('\b', COLOR_WHITE);
                    screen_putchar(' ',  COLOR_WHITE);
                    screen_putchar('\b', COLOR_WHITE);
                }
                continue;
            }

            if (pwd_len < 63) {
                pwd_buffer[pwd_len++] = c;
                screen_putchar('*', COLOR_BRIGHT_GREEN);
            }
        }

        pwd_buffer[pwd_len] = 0;

        /* Verify password hash */
        if (verify_password(pwd_buffer, pwd_len, PASSWORD_HASH)) {
            secure_wipe(pwd_buffer, 64);
            return 1;
        }

        secure_wipe(pwd_buffer, 64);
        attempts--;
        screen_println("", COLOR_WHITE);
        screen_println("  [!] Incorrect password.", COLOR_BRIGHT_RED);
    }

    return 0;
}

/*----------------------------------------------------------
  Kernel entry point — called from stage2_entry.asm
----------------------------------------------------------*/
void kernel_main(void) {
    screen_clear();

    /* Step 1: Integrity checks */
    run_integrity_checks();

    /* Step 2: Password authentication */
    screen_println("[*] Starting authentication...", COLOR_BRIGHT_CYAN);

    if (!prompt_password()) {
        screen_println("", COLOR_WHITE);
        screen_println("  [!] Authentication failed.", COLOR_BRIGHT_RED);
        screen_println("  [!] Wiping sensitive memory...", COLOR_YELLOW);

        /* Wipe key material from RAM */
        secure_wipe((unsigned char*)xor_key, KEY_LEN);

        screen_println("  [!] System halted.", COLOR_BRIGHT_RED);
        __asm__ volatile ("cli; hlt");
        while(1);
    }

    /* Step 3: Authenticated — show success */
    screen_clear();
    screen_println("", COLOR_WHITE);
    screen_println("  [+] Authentication SUCCESS!", COLOR_BRIGHT_GREEN);
    screen_println("  [+] Decrypting kernel...",    COLOR_BRIGHT_CYAN);

    /* Step 4: XOR decrypt kernel in RAM
       In a real setup, kernel is loaded from disk first */
    unsigned char *kernel_addr = (unsigned char*)0x100000;
    unsigned int   kernel_size = 512;
    xor_crypt(kernel_addr, kernel_size, xor_key, KEY_LEN);

    screen_println("  [+] Kernel decrypted.", COLOR_BRIGHT_GREEN);
    screen_println("  [+] Booting...",        COLOR_BRIGHT_WHITE);

    /* Step 5: Jump to decrypted kernel */
    void (*kernel_entry)(void) = (void(*)(void))0x100000;
    kernel_entry();

    /* Should never reach here */
    while(1);
}

