#include <list>
#include <map>
#include <string>
#include <cctype>
#include <iostream>

#include "LineParser.h"
#include "AbstractEventObserver.h"

namespace YAML {

class AbstractParseState {
public:
    virtual ~AbstractParseState() = default;

    virtual bool parse(std::istream& input) = 0;
public:
    enum class State {
        Init,
        Scalar,
        Spaces,
        Map,
        Sequence,
        Comments,
        Error,
    };
};

}

namespace {

class ParseContext : public YAML::AbstractParseState {
public:
    using ParseStateHolder = std::shared_ptr<YAML::AbstractParseState>;
    using State = AbstractParseState::State;
public:
    ParseContext(YAML::AbstractEventObserver *eventObserver)
        : eventObserver(eventObserver)
    {
    }

    void setState(ParseStateHolder newState) {
        this->currentState = newState;
    }

    void setState(State state, ParseStateHolder newState) {
        this->states[state] = newState;
    }

    ParseStateHolder getState(State state) {
        return this->states[state];
    }

    bool parse(std::istream& input) override {
        return this->currentState && this->currentState->parse(input);
    }

    void setInitSpaces(int spaces) {
        this->spaces = spaces;
    }

    int getInitSpaces() const {
        return this->spaces;
    }

    void addScalar(const std::string& scalar) {
        this->scalar = scalar;
    }

    void generateMapEvent() {
        if (!scalar.empty()) {
            const auto& name = this->scalar;
            if (this->eventObserver != nullptr) {
                this->eventObserver->newMapItem(name, this->spaces);
            }

            scalar.clear();
        } else {
            setState(getState(State::Error));
        }
    }

    void makeEvents() {
        if (!scalar.empty() && this->eventObserver != nullptr) {
            this->eventObserver->newScalar(scalar);
        }

        setState(getState(State::Init));

        // init
        this->spaces = 0;
        this->scalar.clear();
    }

    YAML::AbstractEventObserver *getObserver() {
        return this->eventObserver;
    }
private:
    ParseStateHolder currentState;
    std::map<State, ParseStateHolder> states;
    std::string scalar;
    int spaces = 0;
    YAML::AbstractEventObserver *eventObserver = nullptr;
};

using ParseContextHolder = std::shared_ptr<ParseContext>;

class ParseState : public YAML::AbstractParseState {
public:
    using State = AbstractParseState::State;
public:
    ParseState(ParseContextHolder context)
        : context(context)
    {
    }

    bool parse(std::istream& input) override {
        char symbol = input.peek();
        if (symbol != EOF) {
            switch (symbol) {
                //case '-':
                //    getContext()->setState(getContext()->getState(State::Sequence));
                //    break;
                case '#':
                    context->setState(context->getState(State::Comments));
                    break;
                default:
                    context->setState(context->getState(State::Scalar));
                    break;
            }
        } else {
            context->makeEvents();
        }

        return true;
    }

    ParseContextHolder getContext() {
        return this->context;
    }
private:
    ParseContextHolder context;
};

class ParseInitState : public ParseState {
public:
    ParseInitState(ParseContextHolder context)
        : ParseState(context)
    {
    }

    bool parse(std::istream& input) override {
        auto startPosition = input.tellg();
        input >> std::ws;
        auto endPosition = input.tellg();
        getContext()->setInitSpaces(static_cast<int>(endPosition - startPosition));
        return ParseState::parse(input);
    }
};

class ParseSpacesState : public ParseState {
public:
    ParseSpacesState(ParseContextHolder context)
        : ParseState(context)
    {
    }

    bool parse(std::istream& input) override {
        input >> std::ws;
        return ParseState::parse(input);
    }
};

class ParseScalarState : public ParseState {
public:
    ParseScalarState(ParseContextHolder context)
        : ParseState(context)
    {
    }

    bool parse(std::istream& input) override {
        char symbol = 0;
        if (input >> symbol) {
            if (std::isspace(symbol)) {
                getContext()->addScalar(scalar);

                scalar.clear();
                getContext()->setState(getContext()->getState(State::Spaces));
            } else if (symbol == ':') {
                getContext()->addScalar(scalar);

                scalar.clear();
                getContext()->setState(getContext()->getState(State::Map));
            } else {
                scalar.push_back(symbol);
            }
        } else {
            getContext()->addScalar(scalar);

            scalar.clear();
            getContext()->makeEvents();
        }

        return true;
    }
private:
    std::string scalar;
};

class ParseMapState : public ParseState {
public:
    ParseMapState(ParseContextHolder context)
        : ParseState(context)
    {
    }

    bool parse(std::istream& input) override {
        char symbol = 0;
        if (!(input >> symbol) || std::isspace(symbol))
        {
            getContext()->generateMapEvent();

            getContext()->setState(getContext()->getState(State::Spaces));
            return true;
        }

        return false;
    }
};

class ParseCommentsState : public ParseState {
public:
    ParseCommentsState(ParseContextHolder context)
        : ParseState(context)
    {
    }

    bool parse(std::istream& input) override {
        char symbol = 0;
        if (input >> symbol && symbol == '#')
        {
            getContext()->makeEvents();

            input.setstate(std::ios_base::eofbit);
            return true;
        } else {
            getContext()->setState(getContext()->getState(State::Error));
        }

        return false;
    }
};

}

YAML::LineParser::LineParser()
{
    initStateMachine();
}

YAML::LineParser::LineParser(AbstractEventObserver *eventObserver)
{
    initStateMachine(eventObserver);
}

void
YAML::LineParser::initStateMachine(AbstractEventObserver *eventObserver)
{
    auto parseContext = std::make_shared<ParseContext>(eventObserver);

    auto initState = std::make_shared<ParseInitState>(parseContext);
    auto spacesState = std::make_shared<ParseSpacesState>(parseContext);
    auto scalarState = std::make_shared<ParseScalarState>(parseContext);
    auto mapState = std::make_shared<ParseMapState>(parseContext);
    auto commentsState = std::make_shared<ParseCommentsState>(parseContext);

    parseContext->setState(initState);
    parseContext->setState(AbstractParseState::State::Init, initState);
    parseContext->setState(AbstractParseState::State::Spaces, spacesState);
    parseContext->setState(AbstractParseState::State::Scalar, scalarState);
    parseContext->setState(AbstractParseState::State::Map, mapState);
    parseContext->setState(AbstractParseState::State::Comments, commentsState);

    stateMachine = parseContext;
}

bool
YAML::LineParser::parse(std::istream& input)
{
    if (stateMachine) {
        bool result = false;
        do {
            result = stateMachine->parse(input);
        } while (result && !input.eof());

        return result;
    }

    return false;
}

int
YAML::LineParser::skipSpaces(std::istream& input)
{
    auto startPosition = input.tellg();
    input >> std::ws;
    auto endPosition = input.tellg();
    return static_cast<int>(endPosition - startPosition);
}
