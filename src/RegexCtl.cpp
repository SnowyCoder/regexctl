#include "RegexCtl.h"

#include <ncurses.h>

#define ASCII_BACKSPACE 127
#define ASCII_ENTER 10
#define ASCII_ESC 27
#define ASCII_START 262

using namespace std;

void RegexCtl::init() {
    running = true;
    lines.clear();
    lines.emplace_back("RegexCtl was created by SnowyCoder");
    lines.emplace_back("");
    lines.emplace_back("Click F2 to enter text mode and change the current text");
    lines.emplace_back("Then click F1 to enter regex mode and change the regex");
    lines.emplace_back("Use Ctrl+C to exit the program");
    print_all_lines();
    recalc_lines();
    reprint();
    refresh();
}

void RegexCtl::type_char(int c) {
// If event is consumed return
    if (check_common_key(c)) return;

    if (state == TypeState::REGEX) {
        switch (c) {
            case KEY_ENTER:
            case ASCII_ENTER:
            case KEY_F(2):// Both F2 and Enter may be used to enter in text mode
                state = TypeState::TEXT;
                break;
            case KEY_LEFT:
                if (regex_typing_x > 0) {// If we're not at the beginning
                    regex_typing_x--;// Go left
                }
                break;
            case KEY_RIGHT:
                if (regex_typing_x < input_text.length()) {// If we're not at the end
                    regex_typing_x++;// Go right
                }
                break;
            case ASCII_START:
                regex_typing_x = 0;// Go at the beginning of the line
                break;
            case KEY_END:
                regex_typing_x = static_cast<int>(input_text.size());// Go at the end of the line
                break;
            case KEY_BACKSPACE:
            case ASCII_BACKSPACE:
                if (regex_typing_x > 0) {// If there's something before the cursor to delete
                    input_text.erase(input_text.begin() + --regex_typing_x);// Delete it and update x
                    recalc_lines();// Then recalculate regex
                }
                break;
            case KEY_DC:
                if (regex_typing_x < input_text.length()) {// If there's something after the cursor to delete
                    input_text.erase(input_text.begin() + regex_typing_x);// Delete it
                    recalc_lines();// And recalculate regex
                }
                break;
            default:
                // Only write printable characters and stay within the screen bounds
                if (!isprint(c) || input_text.length() >= COLS) break;
                // Insert the char in the regex
                input_text.insert(input_text.begin() + regex_typing_x, static_cast<char>(c));
                // And update the x
                regex_typing_x++;
                recalc_lines();
        }
        mvprintw(LINES - 1, 0, input_text.data());
        clrtoeol();
    } else if (state == TypeState::TEXT) {
        Line *line = nullptr;
        switch (c) {
            case KEY_ENTER:
            case ASCII_ENTER:// Listed both for compatibility issues
                // Add new line with contents of the previous one (cutting from typing_x)
                lines.emplace(lines.begin() + (typing_y + 1), lines[typing_y].content.substr(typing_x));
                // Erase the old line from typing_x onwards
                lines[typing_y].content.erase(typing_x);
                typing_y++;// Update the y
                typing_x = 0;// And the x
                for (int y = typing_y - 1; y < lines.size(); y++) {// Reprint the changed lines
                    lines[y].print(y, 0);
                }
                break;
            case KEY_F(1):
                state = TypeState::REGEX;
                break;
            case KEY_DOWN:
                typing_y = typing_y >= lines.size() - 1 ? 0 : typing_y + 1;// Update y
                typing_x = min(static_cast<unsigned int>(lines[typing_y].content.length()), typing_x);// Update x
                break;
            case KEY_UP:
                typing_y = typing_y <= 0 ? static_cast<unsigned int>(lines.size() - 1) : typing_y - 1;// Update y
                typing_x = min(static_cast<unsigned int>(lines[typing_y].content.length()), typing_x);// Update x
                break;
            case KEY_LEFT:
                if (typing_x <= 0) {// If we're at the beginning of the line
                    // Select previous line
                    typing_y = typing_y <= 0 ? static_cast<unsigned int>(lines.size() - 1) : typing_y - 1;
                    typing_x = static_cast<int>(lines[typing_y].content.length());// And update x accordingly
                } else {// Else (we're in the middle/end of the line)
                    typing_x--;// Just move left by one
                }
                break;
            case KEY_RIGHT:
                if (typing_x >= lines[typing_y].content.length()) {// If we're at the end of the line
                    typing_y = typing_y >= lines.size() - 1 ? 0 : typing_y + 1;// Select the next line
                    typing_x = 0;// And update x accordingly
                } else {// Else (we're in the beginning/middle of the line)
                    typing_x++;// Just move right by one
                }
                break;
            case ASCII_START:// The little arrow over the END key
                typing_x = 0;// Go at the beginning of the line
                break;
            case KEY_END:
                typing_x = static_cast<int>(lines[typing_y].content.size());// Go at the end of the line
                break;
            case KEY_BACKSPACE:
            case ASCII_BACKSPACE:// Listed both for compatibility issues
                line = &lines[typing_y];
                if (typing_x > 0) {// Remove a character
                    line->content.erase(line->content.begin() + --typing_x);
                    line->recalc(input_regex);
                    line->print(typing_y, 0);
                } else if (lines.size() > 1) {// Remove a line
                    // Append the remaining contents to the previous one
                    auto prev_line_index = typing_y > 0 ? typing_y - 1 : static_cast<unsigned int>(lines.size()) - 1;
                    auto prev_line = &lines[prev_line_index];
                    // Prevent line overflow
                    if (prev_line->content.length() + line->content.length() > COLS) {
                        // Line overflow! don't copy it all!
                        prev_line->content.append(line->content, 0, COLS - prev_line->content.length());
                    } else {
                        // No line overflow, just copy it
                        prev_line->content.append(line->content);
                    }
                    prev_line->recalc(input_regex);// Recalc line matches

                    // Save the string length (we'll need it later)
                    auto prev_len = static_cast<unsigned int>(line->content.length());

                    lines.erase(lines.begin() + typing_y);// Remove the line from the list
                    auto old_typing_y = typing_y;// Save the y for later
                    // Calculate new y
                    // If we deleted the 0th line the previous one would be shifted too
                    typing_y = typing_y > 0 ? prev_line_index : prev_line_index - 1;
                    typing_x = static_cast<unsigned int>(lines[typing_y].content.length()) - prev_len;// And the new x

                    // Then clean the moved or modified lines
                    for (int y = min(old_typing_y, typing_y); y < lines.size(); y++) {
                        lines[y].print(y, 0);
                    }
                    move(lines.size(), 0);
                    clrtoeol();
                } // Else the user is pressing backspace in an empty screen, let him press
                break;
            case KEY_DC:// Delete character (a.k.a. canc)
                line = &lines[typing_y];
                if (typing_x < line->content.length()) {// Remove a character
                    line->content.erase(line->content.begin() + typing_x);
                    line->recalc(input_regex);
                    line->print(typing_y, 0);
                } else if (lines.size() > 1) {// Remove a line
                    // And append the contents to the next one
                    auto next_line_index = typing_y < lines.size() - 1 ? typing_y + 1 : 0;
                    auto next_line = &lines[next_line_index];
                    auto x_position = static_cast<unsigned int>(line->content.length());
                    // Prevent line overflow
                    if (x_position + next_line->content.length() > COLS) {
                        // Line overflow! don't copy it all!
                        line->content.append(next_line->content, 0, COLS - x_position);
                    } else {
                        // No line overflow, just copy it
                        line->content.append(next_line->content);
                    }
                    line->recalc(input_regex);// Recalc line matches

                    lines.erase(lines.begin() + next_line_index);// Remove the line from the list
                    auto old_typing_y = typing_y;// Save the y for later
                    // Calculate the new y
                    if (typing_y >= lines.size()) {// If we deleted the first line
                        typing_y--;// Then shift back the cursor
                    }
                    typing_x = x_position;

                    // Then clean the moved or modified lines
                    for (int y = typing_y < lines.size() - 1 ? typing_y : 0; y < lines.size(); y++) {
                        lines[y].print(y, 0);
                    }
                    // And clean the deleted line
                    move(lines.size(), 0);
                    clrtoeol();
                } // Else the user is pressing backspace in an empty screen, let him press
                break;
            default:
                if (!isprint(c)) break;// Only write printable characters

                line = &lines[typing_y];
                if (line->content.length() >= COLS) break;// Only write withing the screen bounds

                // Insert character in the line
                line->content.insert(line->content.begin() + typing_x++, static_cast<char>(c));
                line->recalc(input_regex);// Recalculate current line's regex
                line->print(typing_y, 0);// And reprint the line

        }
    }
    reprint();
}

void RegexCtl::reset_cursor_position() {
    switch (state) {
        case TypeState::REGEX:
            move(LINES - 1, regex_typing_x);
            break;
        case TypeState::TEXT:
            move(typing_y, typing_x);
            break;
    }
}

void RegexCtl::recalc_lines() {
    if (input_text.empty()) {
        error_message = "Regex matches infinetly";
        error_type = -1;
        return;
    }

    try {
        input_regex = input_text;// Build regex

        // And recalculate every line
        for (auto& line : lines) {
            line.recalc(input_regex);
        }

        // If we're here it means that nothing failed, so we can clear the error message
        error_message = nullptr;
    } catch (regex_error& e) {
        // Something's happened, save the error message and code
        error_message = e.what();
        error_type = e.code();
    }
}

void RegexCtl::reprint() {
    if (error_message != nullptr) {
        mvprintw(LINES - 2, 0, "Error %s (%i)", error_message, error_type);
        clrtoeol();
    } else {// If there's no error clear the error line
        move(LINES - 2, 0);
        clrtoeol();
    }

    if (error_message == nullptr) {// Only update highlights if there's no error
        for (int y = 0; y < lines.size(); y++) {
            lines[y].highlight(y, 0);
        }
    }

    mvprintw(LINES - 1, 0, input_text.data());
    clrtoeol();
    reset_cursor_position();// So the user sees where his cursor is
    refresh();
}

void RegexCtl::print_all_lines() {
    for (int y = 0; y < lines.size(); y++) {
        lines[y].print(y, 0);
    }
}

bool RegexCtl::check_common_key(int c) {
    switch (c) {
        case ASCII_ESC:
            // Quit
            running = false;
            break;
        case KEY_RESIZE:
            clear();// Clear the screen
            print_all_lines();// Reprint every line
            reprint();// Reprint matches and regex
            refresh();// Flush everything
        default:
            return false;
    }
    return true;
}
