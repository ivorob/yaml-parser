#include <sstream>

#include "Parser.h"
#include "AbstractEventObserver.h"

YAML::Parser::Parser(AbstractEventObserver *eventObserver)
    : lineParser(eventObserver)
{
}

bool
YAML::Parser::parse(std::istream& input)
{
    std::string line;
    while (std::getline(input, line)) {
        std::stringstream localInput(line);
        localInput.unsetf(std::ios_base::skipws);
        if (!lineParser.parse(localInput)) {
            break;
        }
    }

    return !input.bad() && input.eof();
}
