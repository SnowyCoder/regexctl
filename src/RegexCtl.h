#ifndef REGEXCTL_REGEXCTL_H
#define REGEXCTL_REGEXCTL_H


#include "Line.h"

class RegexCtl {
public:
    enum class TypeState {
        REGEX, TEXT
    };

    void init();

    void type_char(int c);

    void reset_cursor_position();

    void recalc_lines();

    void reprint();

    bool isRunning() {
            return running;
    }

private:
    std::vector<Line> lines;
    std::string input_text = "([A-Z])\\w+";
    std::regex input_regex;
    TypeState state = TypeState::REGEX;
    unsigned int typing_y = 0, typing_x = 0;
    unsigned int regex_typing_x = static_cast<int>(input_text.length());

    const char* error_message = nullptr;
    int error_type = -1;
    bool running = false;

    void print_all_lines();

    bool check_common_key(int c);
};


#endif //REGEXCTL_REGEXCTL_H
