#pragma once

#include <istream>

namespace YAML {

class AbstractEventObserver;

class Parser {
public:
    Parser() = default;
    Parser(AbstractEventObserver *eventObserver);

    bool parse(std::istream& input);
private:
    bool parseLine(std::istream& input);

    int skipSpaces(std::istream& input);
private:
    AbstractEventObserver *eventObserver = nullptr;
};

}
