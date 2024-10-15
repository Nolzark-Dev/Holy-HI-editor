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
int is_modified = 0;
char current_filename[MAX_FILENAME_LENGTH] = "";

enum Mode { NORMAL, INSERT };
enum Mode current_mode = NORMAL;

struct termios orig_termios;

// ANSI color codes
#define RESET "\x1b[0m"
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"

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
                case 'A': return 's';  // Up arrow
                case 'B': return 'x';  // Down arrow
                case 'C': return 'c';  // Right arrow
                case 'D': return 'z';  // Left arrow
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
        printf("%d\t", line_number + 1);
        print_syntax_highlighted(lines[line_number]);
    }
}

void insert_line(int line_number, const char* text) {
    if (line_count < MAX_LINES) {
        for (int i = line_count; i > line_number; i--) {
            strcpy(lines[i], lines[i - 1]);
        }
        strncpy(lines[line_number], text, MAX_LINE_LENGTH - 1);
        lines[line_number][MAX_LINE_LENGTH - 1] = '\0';
        line_count++;
        is_modified = 1;
    } else {
        printf("Error: Maximum number of lines reached.\n");
    }
}

void delete_line(int line_number) {
    if (line_number >= 0 && line_number < line_count) {
        for (int i = line_number; i < line_count - 1; i++) {
            strcpy(lines[i], lines[i + 1]);
        }
        line_count--;
        is_modified = 1;
    }
}

void print_all_lines() {
    for (int i = 0; i < line_count; i++) {
        print_line(i);
        printf("\n");
    }
}

void save_file() {
    if (current_filename[0] == '\0') {
        printf("No file name specified. Please use ':w filename' to save.\n");
        return;
    }

    FILE* file = fopen(current_filename, "w");
    if (file == NULL) {
        printf("Error: Unable to open file for writing.\n");
        return;
    }
    for (int i = 0; i < line_count; i++) {
        fputs(lines[i], file);
    }
    fclose(file);
    is_modified = 0;
    printf("File saved successfully.\n");
}

void load_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Unable to open file for reading.\n");
        return;
    }
    line_count = 0;
    while (fgets(lines[line_count], MAX_LINE_LENGTH, file) && line_count < MAX_LINES) {
        line_count++;
    }
    fclose(file);
    is_modified = 0;
    strncpy(current_filename, filename, MAX_FILENAME_LENGTH - 1);
    current_filename[MAX_FILENAME_LENGTH - 1] = '\0';
    printf("File loaded successfully.\n");
}

void handle_normal_mode(int c) {
    switch (c) {
        case 'i':
            current_mode = INSERT;
            printf("-- INSERT --\n");
            break;
        case 's':
            if (current_line > 0) current_line--;
            break;
        case 'x':
            if (current_line < line_count - 1) current_line++;
            break;
        case 'z':
            if (current_column > 0) current_column--;
            break;
        case 'c':
            if (current_column < strlen(lines[current_line]) - 1) current_column++;
            break;
        case 'd':
            if (line_count > 0) {
                delete_line(current_line);
                if (current_line >= line_count) current_line = line_count - 1;
            }
            break;
        case 'C':
            if (line_count > 0) {
                strncpy(clipboard, lines[current_line], MAX_LINE_LENGTH);
                printf("Line copied.\n");
            }
            break;
        case 'v':
            if (clipboard[0] != '\0') {
                insert_line(current_line + 1, clipboard);
                current_line++;
                printf("Line pasted.\n");
            }
            break;
    }
}

void handle_insert_mode(int c) {
    if (c == 27) {  // ESC key
        current_mode = NORMAL;
        printf("-- NORMAL --\n");
    } else if (c == '\n') {
        insert_line(current_line + 1, "\n");
        current_line++;
        current_column = 0;
    } else {
        char input[2] = {c, '\0'};
        if (line_count == 0) {
            insert_line(0, input);
        } else {
            memmove(&lines[current_line][current_column + 1], &lines[current_line][current_column], 
                    MAX_LINE_LENGTH - current_column - 1);
            lines[current_line][current_column] = c;
            current_column++;
        }
        is_modified = 1;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    load_file(argv[1]);

    enable_raw_mode();
    printf("Modal 'ed' text editor (inspired by TempleOS and Vim)\n");
    printf("Editing file: %s\n", current_filename);
    printf("Navigation: s (up), x (down), z (left), c (right)\n");
    printf("Commands: C (copy), v (paste)\n");
    printf("Press ESC in normal mode to save and exit\n");
    printf("Press Shift+ESC in normal mode to exit without saving\n");
    printf("-- NORMAL --\n");

    while (1) {
        printf("\r\n%d,%d\t", current_line + 1, current_column + 1);
        print_syntax_highlighted(lines[current_line]);
        printf("\n");
        
        int c = read_key();

        if (current_mode == NORMAL && c == 27) {  // ESC key in normal mode
            int next = read_key();
            if (next == 27) {  // Shift+ESC
                printf("Exit without saving? (y/n): ");
                char confirm = getchar();
                if (confirm == 'y' || confirm == 'Y') {
                    break;
                }
            } else {
                if (is_modified) {
                    save_file();
                }
                break;
            }
        } else if (current_mode == NORMAL) {
            handle_normal_mode(c);
        } else if (current_mode == INSERT) {
            handle_insert_mode(c);
        }
    }

    disable_raw_mode();
    return 0;
}
