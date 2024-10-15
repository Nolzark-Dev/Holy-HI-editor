#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_LINES 1000
#define MAX_LINE_LENGTH 100
#define MAX_FILENAME_LENGTH 256

char lines[MAX_LINES][MAX_LINE_LENGTH];
int line_count = 0;
int current_line = 0;
int current_column = 0;
char clipboard[MAX_LINE_LENGTH] = "";
char current_filename[MAX_FILENAME_LENGTH] = "";

enum Mode { NORMAL, VIEW };
enum Mode current_mode = NORMAL;

struct termios orig_termios;

// ANSI color codes and screen clearing
#define RESET "\x1b[0m"
#define BLUE "\x1b[34m"
#define GREEN "\x1b[32m"
#define MAGENTA "\x1b[35m"
#define CLEAR_SCREEN "\x1b[2J\x1b[H"

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int read_key() {
    char c;
    read(STDIN_FILENO, &c, 1);
    if (c == '\x1b') {
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
        if (seq[0] == '[') {
            switch (seq[1]) {
                case 'A': return 'k';  // Up arrow
                case 'B': return 'j';  // Down arrow
                case 'C': return 'l';  // Right arrow
                case 'D': return 'h';  // Left arrow
            }
        }
        return '\x1b';
    }
    return c;
}

void print_syntax_highlighted(const char* line) {
    char* keywords[] = {"if", "else", "for", "while", "return", "int", "char", "void", "struct", "typedef", NULL};
    char word[MAX_LINE_LENGTH] = "";
    int word_len = 0;

    for (int i = 0; line[i] != '\0'; i++) {
        if (isalnum(line[i]) || line[i] == '_') {
            word[word_len++] = line[i];
            word[word_len] = '\0';
        } else {
            int is_keyword = 0;
            for (int j = 0; keywords[j] != NULL; j++) {
                if (strcmp(word, keywords[j]) == 0) {
                    printf("%s%s%s", BLUE, word, RESET);
                    is_keyword = 1;
                    break;
                }
            }
            if (!is_keyword) {
                printf("%s", word);
            }
            word_len = 0;
            word[0] = '\0';

            if (line[i] == '"') {
                printf("%s%c", GREEN, line[i]);
                i++;
                while (line[i] != '"' && line[i] != '\0') {
                    printf("%c", line[i]);
                    i++;
                }
                if (line[i] == '"') {
                    printf("%c%s", line[i], RESET);
                }
            } else if (line[i] == '/' && line[i+1] == '/') {
                printf("%s%s%s", MAGENTA, &line[i], RESET);
                break;
            } else {
                printf("%c", line[i]);
            }
        }
    }
    if (word_len > 0) {
        printf("%s", word);
    }
}

void print_line(int line_number) {
    if (line_number >= 0 && line_number < line_count) {
        printf("%3d ", line_number + 1);
        print_syntax_highlighted(lines[line_number]);
        printf("\n");
    }
}

void load_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Unable to open file for reading.\n");
        exit(1);
    }
    line_count = 0;
    while (fgets(lines[line_count], MAX_LINE_LENGTH, file) && line_count < MAX_LINES) {
        line_count++;
    }
    fclose(file);
    strncpy(current_filename, filename, MAX_FILENAME_LENGTH - 1);
    current_filename[MAX_FILENAME_LENGTH - 1] = '\0';
}

void handle_normal_mode(int c) {
    switch (c) {
        case 'j':
            if (current_line < line_count - 1) current_line++;
            break;
        case 'k':
            if (current_line > 0) current_line--;
            break;
        case 'h':
            if (current_column > 0) current_column--;
            break;
        case 'l':
            if (current_column < strlen(lines[current_line]) - 1) current_column++;
            break;
        case 'y':
            if (line_count > 0) {
                strncpy(clipboard, lines[current_line], MAX_LINE_LENGTH);
                printf("Line copied.\n");
            }
            break;
        case 'p':
            printf("Paste operation not allowed in read-only mode.\n");
            break;
    }
}

void refresh_screen() {
    printf(CLEAR_SCREEN);
    printf("Read-only 'ed' text editor (inspired by TempleOS and Vim)\n");
    printf("Viewing file: %s\n", current_filename);
    printf("Navigation: j (down), k (up), h (left), l (right)\n");
    printf("Commands: y (copy), p (paste - disabled)\n");
    printf("Press q to quit\n");
    printf("-- NORMAL --\n\n");

    int start_line = (current_line > 10) ? current_line - 10 : 0;
    int end_line = (start_line + 20 < line_count) ? start_line + 20 : line_count;

    for (int i = start_line; i < end_line; i++) {
        print_line(i);
    }

    printf("\nCurrent position: %d,%d\n", current_line + 1, current_column + 1);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    load_file(argv[1]);
    enable_raw_mode();

    while (1) {
        refresh_screen();
        
        int c = read_key();

        if (c == 'q') {
            break;
        } else {
            handle_normal_mode(c);
        }
    }

    disable_raw_mode();
    printf(CLEAR_SCREEN);
    return 0;
}
