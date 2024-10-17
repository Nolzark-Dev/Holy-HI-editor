#ifndef SYNTAX_HIGHLIGHT_H
#define SYNTAX_HIGHLIGHT_H

#include <ncurses.h>
#include <string.h>
#include <ctype.h>

#define MAX_KEYWORD_LENGTH 20
#define MAX_KEYWORDS 100

// Color pairs
#define COLOR_DEFAULT   1
#define COLOR_KEYWORD   2
#define COLOR_TYPE      3
#define COLOR_STRING    4
#define COLOR_COMMENT   5
#define COLOR_NUMBER    6
#define COLOR_PREPROC   7
#define COLOR_OPERATOR  8

typedef struct {
    char word[MAX_KEYWORD_LENGTH];
} Keyword;

Keyword keywords[] = {
    // Control flow
    {"if"}, {"else"}, {"for"}, {"while"}, {"do"}, {"switch"}, {"case"}, {"default"},
    {"break"}, {"continue"}, {"return"}, {"goto"},
    
    // Type qualifiers
    {"const"}, {"volatile"}, {"register"}, {"static"}, {"extern"}, {"inline"},
    
    // Storage classes
    {"auto"}, {"typedef"},
    
    // Other keywords
    {"sizeof"}, {"null"}, {"NULL"}
};

Keyword types[] = {
    {"void"}, {"char"}, {"short"}, {"int"}, {"long"}, {"float"}, {"double"},
    {"signed"}, {"unsigned"}, {"struct"}, {"union"}, {"enum"}
};

Keyword preprocessor[] = {
    {"#include"}, {"#define"}, {"#undef"}, {"#ifdef"}, {"#ifndef"}, {"#if"},
    {"#else"}, {"#elif"}, {"#endif"}, {"#error"}, {"#pragma"}
};

int num_keywords = sizeof(keywords) / sizeof(Keyword);
int num_types = sizeof(types) / sizeof(Keyword);
int num_preprocessor = sizeof(preprocessor) / sizeof(Keyword);

void init_syntax_highlight() {
    start_color();
    init_pair(COLOR_DEFAULT, COLOR_WHITE, COLOR_BLACK);
    init_pair(COLOR_KEYWORD, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLOR_TYPE, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_STRING, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(COLOR_COMMENT, COLOR_CYAN, COLOR_BLACK);
    init_pair(COLOR_NUMBER, COLOR_RED, COLOR_BLACK);
    init_pair(COLOR_PREPROC, COLOR_BLUE, COLOR_BLACK);
    init_pair(COLOR_OPERATOR, COLOR_WHITE, COLOR_BLACK);
}

int is_keyword(const char* word) {
    for (int i = 0; i < num_keywords; i++) {
        if (strcmp(word, keywords[i].word) == 0) {
            return 1;
        }
    }
    return 0;
}

int is_type(const char* word) {
    for (int i = 0; i < num_types; i++) {
        if (strcmp(word, types[i].word) == 0) {
            return 1;
        }
    }
    return 0;
}

int is_preprocessor(const char* word) {
    for (int i = 0; i < num_preprocessor; i++) {
        if (strcmp(word, preprocessor[i].word) == 0) {
            return 1;
        }
    }
    return 0;
}

int is_operator(char c) {
    return (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
            c == '=' || c == '<' || c == '>' || c == '&' || c == '|' ||
            c == '^' || c == '~' || c == '!' || c == '?' || c == ':');
}

void highlight_syntax(WINDOW* win, const char* line) {
    int in_string = 0;
    int in_char = 0;
    int in_comment = 0;
    int in_preproc = 0;
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
        } else if (in_string || in_char) {
            wattron(win, COLOR_PAIR(COLOR_STRING));
            waddch(win, line[i]);
            wattroff(win, COLOR_PAIR(COLOR_STRING));
            if ((in_string && line[i] == '"' && line[i-1] != '\\') ||
                (in_char && line[i] == '\'' && line[i-1] != '\\')) {
                in_string = in_char = 0;
            }
        } else if (line[i] == '/' && line[i+1] == '*') {
            in_comment = 1;
            wattron(win, COLOR_PAIR(COLOR_COMMENT));
            waddch(win, line[i]);
            wattroff(win, COLOR_PAIR(COLOR_COMMENT));
        } else if (line[i] == '/' && line[i+1] == '/') {
            wattron(win, COLOR_PAIR(COLOR_COMMENT));
            waddstr(win, line + i);
            wattroff(win, COLOR_PAIR(COLOR_COMMENT));
            break;
        } else if (line[i] == '"') {
            in_string = 1;
            wattron(win, COLOR_PAIR(COLOR_STRING));
            waddch(win, line[i]);
            wattroff(win, COLOR_PAIR(COLOR_STRING));
        } else if (line[i] == '\'') {
            in_char = 1;
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
                } else if (is_type(word)) {
                    wattron(win, COLOR_PAIR(COLOR_TYPE));
                    waddstr(win, word);
                    wattroff(win, COLOR_PAIR(COLOR_TYPE));
                } else if (is_preprocessor(word)) {
                    wattron(win, COLOR_PAIR(COLOR_PREPROC));
                    waddstr(win, word);
                    wattroff(win, COLOR_PAIR(COLOR_PREPROC));
                    in_preproc = 1;
                } else {
                    waddstr(win, word);
                }
                word_len = 0;
            }
            if (isdigit(line[i])) {
                wattron(win, COLOR_PAIR(COLOR_NUMBER));
                waddch(win, line[i]);
                wattroff(win, COLOR_PAIR(COLOR_NUMBER));
            } else if (is_operator(line[i])) {
                wattron(win, COLOR_PAIR(COLOR_OPERATOR));
                waddch(win, line[i]);
                wattroff(win, COLOR_PAIR(COLOR_OPERATOR));
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
        } else if (is_type(word)) {
            wattron(win, COLOR_PAIR(COLOR_TYPE));
            waddstr(win, word);
            wattroff(win, COLOR_PAIR(COLOR_TYPE));
        } else if (is_preprocessor(word)) {
            wattron(win, COLOR_PAIR(COLOR_PREPROC));
            waddstr(win, word);
            wattroff(win, COLOR_PAIR(COLOR_PREPROC));
        } else {
            waddstr(win, word);
        }
    }
}

#endif // SYNTAX_HIGHLIGHT_H
