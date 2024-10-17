#ifndef SYNTAX_HIGHLIGHT_H
#define SYNTAX_HIGHLIGHT_H

#include <ncurses.h>
#include <string.h>
#include <ctype.h>

#define MAX_KEYWORD_LENGTH 20
#define MAX_KEYWORDS 50

// Color pairs
#define COLOR_DEFAULT   1
#define COLOR_KEYWORD   2
#define COLOR_STRING    3
#define COLOR_COMMENT   4
#define COLOR_NUMBER    5

typedef struct {
    char word[MAX_KEYWORD_LENGTH];
} Keyword;

Keyword keywords[MAX_KEYWORDS] = {
    {"int"}, {"char"}, {"float"}, {"double"}, {"void"},
    {"if"}, {"else"}, {"while"}, {"for"}, {"do"},
    {"switch"}, {"case"}, {"break"}, {"continue"}, {"return"},
    {"struct"}, {"union"}, {"typedef"}, {"enum"}, {"sizeof"}, {"#include"}
};

int num_keywords = 20;  // Number of keywords defined above

void init_syntax_highlight() {
    start_color();
    init_pair(COLOR_DEFAULT, COLOR_WHITE, COLOR_BLACK);
    init_pair(COLOR_KEYWORD, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLOR_STRING, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_COMMENT, COLOR_CYAN, COLOR_BLACK);
    init_pair(COLOR_NUMBER, COLOR_RED, COLOR_BLACK);
}

int is_keyword(const char* word) {
    for (int i = 0; i < num_keywords; i++) {
        if (strcmp(word, keywords[i].word) == 0) {
            return 1;
        }
    }
    return 0;
}

void highlight_syntax(WINDOW* win, const char* line) {
    int in_string = 0;
    int in_comment = 0;
    char word[MAX_KEYWORD_LENGTH] = {0};
    int word_len = 0;

    for (int i = 0; line[i] != '\0'; i++) {
        if (in_comment) {
            wattron(win, COLOR_PAIR(COLOR_COMMENT));
            waddch(win, line[i]);
            wattroff(win, COLOR_PAIR(COLOR_COMMENT));
            if (line[i] == '*' && line[i+1] == '/') {
                in_comment = 0;
                i++;
            }
        } else if (in_string) {
            wattron(win, COLOR_PAIR(COLOR_STRING));
            waddch(win, line[i]);
            wattroff(win, COLOR_PAIR(COLOR_STRING));
            if (line[i] == '"' && (i == 0 || line[i-1] != '\\')) {
                in_string = 0;
            }
        } else if (line[i] == '/' && line[i+1] == '*') {
            in_comment = 1;
            wattron(win, COLOR_PAIR(COLOR_COMMENT));
            waddch(win, line[i]);
            wattroff(win, COLOR_PAIR(COLOR_COMMENT));
        } else if (line[i] == '"') {
            in_string = 1;
            wattron(win, COLOR_PAIR(COLOR_STRING));
            waddch(win, line[i]);
            wattroff(win, COLOR_PAIR(COLOR_STRING));
        } else if (isalnum(line[i]) || line[i] == '_') {
            word[word_len++] = line[i];
            if (word_len >= MAX_KEYWORD_LENGTH - 1) {
                word[MAX_KEYWORD_LENGTH - 1] = '\0';
                word_len = 0;
            }
        } else {
            if (word_len > 0) {
                word[word_len] = '\0';
                if (is_keyword(word)) {
                    wattron(win, COLOR_PAIR(COLOR_KEYWORD));
                    waddstr(win, word);
                    wattroff(win, COLOR_PAIR(COLOR_KEYWORD));
                } else {
                    waddstr(win, word);
                }
                word_len = 0;
            }
            if (isdigit(line[i])) {
                wattron(win, COLOR_PAIR(COLOR_NUMBER));
                waddch(win, line[i]);
                wattroff(win, COLOR_PAIR(COLOR_NUMBER));
            } else {
                waddch(win, line[i]);
            }
        }
    }

    if (word_len > 0) {
        word[word_len] = '\0';
        if (is_keyword(word)) {
            wattron(win, COLOR_PAIR(COLOR_KEYWORD));
            waddstr(win, word);
            wattroff(win, COLOR_PAIR(COLOR_KEYWORD));
        } else {
            waddstr(win, word);
        }
    }
}

#endif // SYNTAX_HIGHLIGHT_H
