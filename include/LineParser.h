#pragma once

#include <istream>
#include <memory>

namespace YAML {

class AbstractEventObserver;
class AbstractParseState;

class LineParser {
public:
    LineParser();
    LineParser(AbstractEventObserver *eventObserver);

    bool parse(std::istream& input);
private:
    void initStateMachine(AbstractEventObserver *eventObserver = nullptr);
    int skipSpaces(std::istream& input);
private:
    std::shared_ptr<AbstractParseState> stateMachine;
};

}
