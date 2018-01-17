#ifndef REGEXCTL_LINE_H
#define REGEXCTL_LINE_H

#include <utility>
#include <vector>
#include <regex>


class Line {
private:
    // vector<pair<position, length>>
    std::vector<std::pair<int, int>> matches;

public:
    std::string content;

    explicit Line(std::string content);
    // In theory the copy-constructor should be deleted but then the emplace does not work

    void recalc(std::regex& reg);

    void print(int y, int x);

    void highlight(int y, int x);
};


#endif //REGEXCTL_LINE_H
