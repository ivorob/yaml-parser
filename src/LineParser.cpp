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
        ComplexScalar,
        SequenceScalar,
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

    ParseStateHolder getScalarState() {
        return this->scalarState;
    }

    void setScalarState(ParseStateHolder state) {
        this->scalarState = state;
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
        this->scalar = rtrim(scalar);
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

    void generateSequenceEvent() {
        if (eventObserver != nullptr) {
            this->eventObserver->newSequenceItem(this->spaces);
        }
    }

    void makeEvents() {
        if (!scalar.empty() && this->eventObserver != nullptr) {
            this->eventObserver->newScalar(scalar);
        }

        init();
    }

    YAML::AbstractEventObserver *getObserver() {
        return this->eventObserver;
    }
private:
    void init() {
        setState(getState(State::Init));

        // init
        this->spaces = 0;
        this->scalar.clear();
    }

    std::string rtrim(const std::string& value) const {
        if (value.empty() || !std::isspace(value.back())) {
            return value;
        }

        std::string result = value;
        while (!result.empty() && std::isspace(result.back())) {
            result.pop_back();
        }

        return result;
    }
private:
    ParseStateHolder currentState;
    ParseStateHolder scalarState;
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
                case '-':
                    getContext()->setState(getContext()->getState(State::Sequence));
                    break;
                case '#':
                    context->setState(context->getState(State::Comments));
                    break;
                case ':':
                    input.ignore();
                    context->setState(context->getState(State::Map));
                    break;
                default:
                    context->setState(context->getScalarState());
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

        getContext()->setScalarState(getContext()->getState(State::Scalar));
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

class ParseComplexScalarState : public ParseState {
public:
    ParseComplexScalarState(ParseContextHolder context)
        : ParseState(context)
    {
    }

    bool parse(std::istream& input) override {
        if (input >> std::ws) {
            std::string scalar = readAll(input);
            if (!scalar.empty()) {
                getContext()->addScalar(scalar);
                getContext()->makeEvents();
            }
        } else {
            getContext()->makeEvents();
        }

        return true;
    }
private:
    std::string readAll(std::istream& input) {
        std::string result;

        char symbol = 0;
        while (input >> symbol) {
            if (symbol == '#' || symbol == '\r') {
                break;
            }

            result.push_back(symbol);
        }

        while (!result.empty() && std::isspace(result.back())) {
            result.pop_back();
        }

        input.setstate(std::ios_base::eofbit);
        return result;
    }
};

class ParseSequenceScalarState : public ParseState {
public:
    ParseSequenceScalarState(ParseContextHolder context)
        : ParseState(context)
    {
    }

    bool parse(std::istream& input) override {
        std::string scalar;
        auto startPosition = input.tellg();
        if (input >> std::ws) {
            int spaces = static_cast<int>(input.tellg() - startPosition);

            char symbol = 0;
            bool scalarContainsSpaces = false;
            bool hasSpaces = false;
            while (input >> symbol) {
                if (symbol == ':') {
                    if (scalarContainsSpaces) {
                        getContext()->setState(getContext()->getState(State::Error));
                        break;
                    }

                    getContext()->addScalar(scalar);
                    getContext()->setInitSpaces(getContext()->getInitSpaces() + spaces + 2);
                    getContext()->setState(getContext()->getState(State::Map));
                    break;
                } else if (!std::isspace(symbol)) {
                    scalar.push_back(symbol);

                    scalarContainsSpaces = hasSpaces;
                } else if (!scalar.empty()) {
                    hasSpaces = true;
                    scalar.push_back(symbol);
                }
            }
        }

        if (input.eof()) {
            getContext()->addScalar(scalar);
            getContext()->makeEvents();
        }

        return true;
    }
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

            getContext()->setScalarState(getContext()->getState(State::ComplexScalar));
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

class ParseSequenceState : public ParseState {
public:
    ParseSequenceState(ParseContextHolder context)
        : ParseState(context)
    {
    }

    bool parse(std::istream& input) override {
        char symbol = 0;
        if (input >> symbol && symbol == '-')
        {
            if (input >> symbol && !std::isspace(symbol)) {
                getContext()->setState(getContext()->getState(State::Error));
            } else {
                getContext()->generateSequenceEvent();
                getContext()->setState(getContext()->getState(State::SequenceScalar));
                return true;
            }
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
    parseContext->setState(initState);
    parseContext->setState(AbstractParseState::State::Init, initState);

    parseContext->setState(AbstractParseState::State::Spaces,
            std::make_shared<ParseSpacesState>(parseContext));
    parseContext->setState(AbstractParseState::State::Scalar,
            std::make_shared<ParseScalarState>(parseContext));
    parseContext->setState(AbstractParseState::State::ComplexScalar,
            std::make_shared<ParseComplexScalarState>(parseContext));
    parseContext->setState(AbstractParseState::State::SequenceScalar,
            std::make_shared<ParseSequenceScalarState>(parseContext));
    parseContext->setState(AbstractParseState::State::Map,
            std::make_shared<ParseMapState>(parseContext));
    parseContext->setState(AbstractParseState::State::Comments,
            std::make_shared<ParseCommentsState>(parseContext));
    parseContext->setState(AbstractParseState::State::Sequence,
            std::make_shared<ParseSequenceState>(parseContext));

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
