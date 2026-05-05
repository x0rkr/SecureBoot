/*
 * SecureBoot Project - Boot Menu + Mini Shell
 * Dev: radi0aktiv | github.com/radi0aktiv
 */

#include "../include/shell.h"
#include "../include/screen.h"

#define MAX_FILES  16
#define MAX_FNAME  32
#define MAX_DIRS   8

static char filenames[MAX_FILES][MAX_FNAME];
static int  file_count = 0;
static char dirnames[MAX_DIRS][MAX_FNAME];
static int  dir_count = 0;
static char cwd[64] = "/";

/*----------------------------------------------------------
  String helpers
----------------------------------------------------------*/
static int streq(const char *a, const char *b) {
    while (*a && *b) { if (*a != *b) return 0; a++; b++; }
    return *a == *b;
}
static int strstart(const char *s, const char *p) {
    while (*p) { if (*s != *p) return 0; s++; p++; }
    return 1;
}
static int slen(const char *s) {
    int n = 0; while (s[n]) n++; return n;
}
static void scopy(char *d, const char *s) {
    while (*s) { *d++ = *s++; }
    *d = 0;
}
static const char* skip_word(const char *s) {
    while (*s && *s != ' ') s++;
    while (*s == ' ') s++;
    return s;
}

/*----------------------------------------------------------
  Keyboard
----------------------------------------------------------*/
static unsigned char read_scancode(void) {
    unsigned char sc, st;
    while (1) {
        do {
            __asm__ volatile ("inb $0x64, %0" : "=a"(st));
        } while (!(st & 0x01));
        __asm__ volatile ("inb $0x60, %0" : "=a"(sc));
        if (!(sc & 0x80)) return sc;
    }
}

static void shell_readline(char *buf, int max) {
    int i = 0;
    unsigned char sc, ch;
    const unsigned char kb_map[] =
        "  1234567890-=  qwertyuiop[]  asdfghjkl;'` \\zxcvbnm,./";
    buf[0] = 0;
    while (1) {
        sc = read_scancode();
        if (sc == 0x1C) {
            buf[i] = 0;
            screen_putchar('\n', COLOR_WHITE);
            return;
        }
        if (sc == 0x0E) {
            if (i > 0) {
                i--;
                screen_putchar('\b', COLOR_WHITE);
                screen_putchar(' ',  COLOR_WHITE);
                screen_putchar('\b', COLOR_WHITE);
                buf[i] = 0;
            }
            continue;
        }
        if (sc == 0x39 && i < max-1) {
            buf[i++] = ' '; buf[i] = 0;
            screen_putchar(' ', COLOR_WHITE);
            continue;
        }
        if (sc < sizeof(kb_map)) {
            ch = kb_map[sc];
            if (ch != ' ' && i < max-1) {
                buf[i++] = ch; buf[i] = 0;
                screen_putchar(ch, COLOR_BRIGHT_GREEN);
            }
        }
    }
}

static void wait_key(void) { read_scancode(); }

/*----------------------------------------------------------
  Shell commands
----------------------------------------------------------*/
static void cmd_help(void) {
    screen_println("", COLOR_WHITE);
    screen_println("  Commands:", COLOR_BRIGHT_CYAN);
    screen_println("  ===========================", COLOR_BRIGHT_CYAN);
    screen_println("  help   - Show this list",    COLOR_WHITE);
    screen_println("  whoami - Show current user", COLOR_WHITE);
    screen_println("  pwd    - Print working dir", COLOR_WHITE);
    screen_println("  cd     - Change directory",  COLOR_WHITE);
    screen_println("  ls     - List files/dirs",   COLOR_WHITE);
    screen_println("  mkdir  - Make directory",    COLOR_WHITE);
    screen_println("  touch  - Create file",       COLOR_WHITE);
    screen_println("  echo   - Print text",        COLOR_WHITE);
    screen_println("  clear  - Clear screen",      COLOR_WHITE);
    screen_println("  exit   - Return to menu",    COLOR_WHITE);
    screen_println("  ===========================", COLOR_BRIGHT_CYAN);
    screen_println("", COLOR_WHITE);
}

static void cmd_whoami(void) {
    screen_println("  radi0aktiv", COLOR_BRIGHT_GREEN);
}

static void cmd_pwd(void) {
    screen_print("  ", COLOR_WHITE);
    screen_println(cwd, COLOR_BRIGHT_WHITE);
}

static void cmd_ls(void) {
    screen_println("", COLOR_WHITE);
    screen_print("  ", COLOR_WHITE);
    screen_print(cwd, COLOR_BRIGHT_CYAN);
    screen_println(":", COLOR_BRIGHT_CYAN);
    if (dir_count == 0 && file_count == 0) {
        screen_println("  (empty)", COLOR_WHITE);
        screen_println("", COLOR_WHITE);
        return;
    }
    for (int i = 0; i < dir_count; i++) {
        screen_print("  [DIR]  ", COLOR_BRIGHT_CYAN);
        screen_println(dirnames[i], COLOR_BRIGHT_CYAN);
    }
    for (int i = 0; i < file_count; i++) {
        screen_print("  [FILE] ", COLOR_BRIGHT_GREEN);
        screen_println(filenames[i], COLOR_BRIGHT_GREEN);
    }
    screen_println("", COLOR_WHITE);
}

static void cmd_mkdir(const char *line) {
    const char *name = skip_word(line);
    if (slen(name) == 0) {
        screen_println("  Usage: mkdir <name>", COLOR_YELLOW);
        return;
    }
    if (dir_count >= MAX_DIRS) {
        screen_println("  [!] Max dirs reached", COLOR_BRIGHT_RED);
        return;
    }
    scopy(dirnames[dir_count++], name);
    screen_print("  [+] Created dir: ", COLOR_BRIGHT_GREEN);
    screen_println(name, COLOR_BRIGHT_GREEN);
}

static void cmd_touch(const char *line) {
    const char *name = skip_word(line);
    if (slen(name) == 0) {
        screen_println("  Usage: touch <name>", COLOR_YELLOW);
        return;
    }
    if (file_count >= MAX_FILES) {
        screen_println("  [!] Max files reached", COLOR_BRIGHT_RED);
        return;
    }
    scopy(filenames[file_count++], name);
    screen_print("  [+] Created file: ", COLOR_BRIGHT_GREEN);
    screen_println(name, COLOR_BRIGHT_GREEN);
}

static void cmd_echo(const char *line) {
    const char *text = skip_word(line);
    screen_println(text, COLOR_BRIGHT_WHITE);
}

static void cmd_cd(const char *line) {
    const char *name = skip_word(line);
    if (slen(name) == 0 || streq(name, "/")) {
        cwd[0]='/'; cwd[1]=0;
        return;
    }
    if (streq(name, "..")) {
        cwd[0]='/'; cwd[1]=0;
        return;
    }
    for (int i = 0; i < dir_count; i++) {
        if (streq(dirnames[i], name)) {
            cwd[0]='/';
            int j = 1;
            const char *n = name;
            while (*n && j < 62) cwd[j++] = *n++;
            cwd[j] = 0;
            screen_print("  -> ", COLOR_BRIGHT_CYAN);
            screen_println(cwd, COLOR_BRIGHT_CYAN);
            return;
        }
    }
    screen_print("  [!] Not found: ", COLOR_BRIGHT_RED);
    screen_println(name, COLOR_BRIGHT_RED);
}

/*----------------------------------------------------------
  Shell main loop
----------------------------------------------------------*/
static void run_shell(void) {
    char cmd[128];
    screen_clear();
    screen_println("  ==============================", COLOR_BRIGHT_CYAN);
    screen_println("   SecureBoot Shell  v1.0        ", COLOR_BRIGHT_WHITE);
    screen_println("   Dev: radi0aktiv | github.com/radi0aktiv", COLOR_YELLOW);
    screen_println("  ==============================", COLOR_BRIGHT_CYAN);
    screen_println("  Type 'help' for commands", COLOR_WHITE);
    screen_println("", COLOR_WHITE);

    while (1) {
        screen_print("  radi0aktiv@secureboot:", COLOR_BRIGHT_GREEN);
        screen_print(cwd, COLOR_BRIGHT_CYAN);
        screen_print("# ", COLOR_BRIGHT_WHITE);
        shell_readline(cmd, 128);

        if (cmd[0] == 0)               continue;
        if (streq(cmd, "exit"))        return;
        if (streq(cmd, "help"))        { cmd_help();     continue; }
        if (streq(cmd, "whoami"))      { cmd_whoami();   continue; }
        if (streq(cmd, "pwd"))         { cmd_pwd();      continue; }
        if (streq(cmd, "ls"))          { cmd_ls();       continue; }
        if (streq(cmd, "clear"))       { screen_clear(); continue; }
        if (strstart(cmd, "mkdir "))   { cmd_mkdir(cmd); continue; }
        if (strstart(cmd, "touch "))   { cmd_touch(cmd); continue; }
        if (strstart(cmd, "echo "))    { cmd_echo(cmd);  continue; }
        if (strstart(cmd, "cd "))      { cmd_cd(cmd);    continue; }

        screen_print("  not found: ", COLOR_BRIGHT_RED);
        screen_println(cmd, COLOR_BRIGHT_RED);
        screen_println("  Type 'help' for commands", COLOR_YELLOW);
    }
}

/*----------------------------------------------------------
  Boot Menu
----------------------------------------------------------*/
static void draw_boot_menu(void) {
    screen_clear();
    screen_println("", COLOR_WHITE);
    screen_println("  ==============================", COLOR_BRIGHT_CYAN);
    screen_println("     SECUREBOOT - BOOT MENU     ", COLOR_BRIGHT_WHITE);
    screen_println("     Dev: radi0aktiv                 ", COLOR_YELLOW);
    screen_println("  ==============================", COLOR_BRIGHT_CYAN);
    screen_println("", COLOR_WHITE);
    screen_println("  [1]  Boot OS",     COLOR_BRIGHT_GREEN);
    screen_println("  [2]  System Info", COLOR_BRIGHT_GREEN);
    screen_println("  [3]  Memory Test", COLOR_BRIGHT_GREEN);
    screen_println("  [4]  Mini Shell",  COLOR_BRIGHT_GREEN);
    screen_println("", COLOR_WHITE);
    screen_print  ("  Select option: ",  COLOR_BRIGHT_WHITE);
}

void run_boot_menu(void) {
    unsigned char sc;
    while (1) {
        draw_boot_menu();
        sc = read_scancode();

        if (sc == 0x02) {
            screen_clear();
            screen_println("  [*] No OS loaded.", COLOR_YELLOW);
            screen_println("  Press any key...", COLOR_WHITE);
            wait_key();
        }
        else if (sc == 0x03) {
            screen_clear();
            screen_println("  System Info:", COLOR_BRIGHT_CYAN);
            screen_println("  -------------------------", COLOR_BRIGHT_CYAN);
            screen_println("  Project : SecureBoot v1.0", COLOR_WHITE);
            screen_println("  Dev     : radi0aktiv", COLOR_WHITE);
            screen_println("  GitHub  : github.com/radi0aktiv", COLOR_WHITE);
            screen_println("  Arch    : x86 32-bit Protected Mode", COLOR_WHITE);
            screen_println("  Stack   : 0x90000", COLOR_WHITE);
            screen_println("  VGA     : 0xB8000", COLOR_WHITE);
            screen_println("  -------------------------", COLOR_BRIGHT_CYAN);
            screen_println("  Press any key...", COLOR_YELLOW);
            wait_key();
        }
        else if (sc == 0x04) {
            screen_clear();
            screen_println("  Memory Test", COLOR_BRIGHT_CYAN);
            screen_println("  -------------------------", COLOR_BRIGHT_CYAN);
            screen_println("  Testing 1024 bytes at 0x200000...", COLOR_WHITE);
            volatile unsigned char *mem = (volatile unsigned char*)0x200000;
            int failed = 0;
            for (unsigned int i = 0; i < 1024; i++)
                mem[i] = (unsigned char)(i & 0xFF);
            for (unsigned int i = 0; i < 1024; i++)
                if (mem[i] != (unsigned char)(i & 0xFF)) failed++;
            if (failed == 0)
                screen_println("  Result: PASSED", COLOR_BRIGHT_GREEN);
            else
                screen_println("  Result: FAILED", COLOR_BRIGHT_RED);
            screen_println("  Press any key...", COLOR_YELLOW);
            wait_key();
        }
        else if (sc == 0x05) {
            run_shell();
        }
    }
}
