#include <list>
#include <sstream>

#include "Parser.h"
#include "AbstractEventObserver.h"

YAML::Parser::Parser(AbstractEventObserver *eventObserver)
    : eventObserver(eventObserver)
{
}

bool
YAML::Parser::parse(std::istream& input)
{
    std::string line;
    while (std::getline(input, line)) {
        std::stringstream localInput(line);
        if (!parseLine(localInput)) {
            break;
        }
    }

    return !input.bad() && input.eof();
}

bool
YAML::Parser::parseLine(std::istream& input)
{
    std::string scalar, name;
    std::list<std::string> scalars;

    enum class State {
        Init,
        Scalar,
        Spaces,
        Map,
    } state = State::Init;
    //TODO: count spaces
    char symbol = 0;
    while (input >> symbol)
    {
        if (symbol == ' ' || symbol == '\t') {
            if (state == State::Scalar && !scalar.empty())
            {
                scalars.push_back(scalar);
                scalar.clear();
            }

            state = State::Spaces;
        } else if (symbol == ':') {
            if (state == State::Scalar && !scalar.empty()) {
                scalars.push_back(scalar);
                scalar.clear();
            } else if (!scalars.empty()) {
                //TODO: special exception type
                throw std::runtime_error("No name for mapping");
            }

            state = State::Map;

            name = scalars.back();
            scalars.pop_back();
        } else {
            state = State::Scalar;
            scalar.push_back(symbol);
        }
    }

    if (state == State::Scalar && !scalar.empty()) {
        scalars.push_back(scalar);
    }

    if (!name.empty() && eventObserver != nullptr && !scalars.empty()) {
        eventObserver->newMapItem(name, scalars.front(), 0);
    }

    return !input.bad() && input.eof();
}
