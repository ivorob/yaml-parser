#pragma once

#include <istream>

#include "LineParser.h"

namespace YAML {

class Parser {
public:
    Parser() = default;
    Parser(AbstractEventObserver *eventObserver);

    bool parse(std::istream& input);
private:
    LineParser lineParser;
};

}
