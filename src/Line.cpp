#include "Line.h"
#include <ncurses.h>

using namespace std;

Line::Line(string content) : content(move(content)) {
}

void Line::recalc(regex& reg) {
    matches.clear();// Remove any previous results

    smatch match;
    // To get every match result we need to loop the string and remove the
    // previous match, so the offset keeps track of the offset from the string start
    string str = content;
    int offset = 0;

    while (regex_search(str, match, reg)) {
        matches.emplace_back(pair(match.position(0) + offset, match.length(0)));

        offset += match.position(0) + match.length(0);// Update offset
        str = match.suffix();// And prepare the rest of the string for testing
    }
}

void Line::print(int y, int x) {
    mvprintw(y, x, content.data());// Print the content
    clrtoeol();// And clear the rest of the line
}

void Line::highlight(int y, int x) {
    mvchgat(y, x, -1, A_NORMAL, 0, NULL);// Clear previous highlightings
    for (auto match : matches) {// And for every match
        mvchgat(y, x + match.first, match.second, A_REVERSE, 0, NULL);// Highlight it
    }
}
