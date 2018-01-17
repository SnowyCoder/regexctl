#include <iostream>
#include <ncurses.h>

#include "RegexCtl.h"

int main(int argc, char** argv) {
    initscr();// Init the default window
    cbreak();// Almost raw input, I don't really care about program kills
    noecho();// We're not in top of a mountain
    keypad(stdscr, true);// Lemme catch those special keystrokes
    set_escdelay(30);// Don't want to wait a life before this program closes!

    RegexCtl main;

    main.init();// Compute and print default values

    try {
        while (main.isRunning()) {
            int c = getch();
            main.type_char(c);
        }
        endwin();
    } catch (std::exception& e) {
        endwin();
        printf("Exception occurred: %s\n", e.what());
    }
}
