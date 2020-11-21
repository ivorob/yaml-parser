#include <gtest/gtest.h>
#include <map>
#include <sstream>

#include "LineParser.h"
#include "AbstractEventObserver.h"

namespace {

class EventObserver : public YAML::AbstractEventObserver {
public:
    class Value {
    public:
        Value()
            : spaces()
        {
        }

        Value(const std::string& value, int spaces)
            : value(value),
              spaces(spaces)
        {
        }

        const std::string& getValue() const {
            return this->value;
        }

        void setValue(const std::string& value) {
            this->value = value;
        }

        int getSpaces() const {
            return this->spaces;
        }
    private:
        std::string value;
        int spaces;
    };
public:
    void newMapItem(const std::string& name, int spaces) override {
        this->lastAddedMapItem = name;
        this->events[name] = Value("", spaces);
    }

    void newScalar(const std::string& scalar) override {
        if (this->events.count(this->lastAddedMapItem) != 0) {
            this->events[this->lastAddedMapItem].setValue(scalar);
        }
    }

    void newSequenceItem(const std::string& scalar, int spaces) override {
        this->sequences.emplace_back(scalar, spaces);
    }

    std::vector<Value> sequences;
    std::map<std::string, Value> events;
private:
    std::string lastAddedMapItem;
};

}

TEST(YamlLineParser, simpleCollectionParserTest)
{
    std::stringstream input("hr: 65");
    input >> std::noskipws;

    YAML::LineParser lineParser;
    ASSERT_TRUE(lineParser.parse(input));
}

TEST(YamlLineParser, simpleSequenceParserTest)
{
    std::stringstream input("- test");
    input >> std::noskipws;

    YAML::LineParser lineParser;
    ASSERT_TRUE(lineParser.parse(input));
}

TEST(YamlLineParser, simpleSequenceParseEventTest)
{
    std::stringstream input("- test");
    input >> std::noskipws;

    EventObserver eventObserver;
    YAML::LineParser lineParser(&eventObserver);
    ASSERT_TRUE(lineParser.parse(input));

    ASSERT_EQ(1, eventObserver.sequences.size());
    ASSERT_EQ("test", eventObserver.sequences.at(0).getValue());
    ASSERT_EQ(0, eventObserver.sequences.at(0).getSpaces());
}

TEST(YamlLineParser, simpleSequenceWithSpacesAtStartParseEventTest)
{
    std::stringstream input("    - test event");
    input >> std::noskipws;

    EventObserver eventObserver;
    YAML::LineParser lineParser(&eventObserver);
    ASSERT_TRUE(lineParser.parse(input));

    ASSERT_EQ(1, eventObserver.sequences.size());
    ASSERT_EQ("test event", eventObserver.sequences.at(0).getValue());
    ASSERT_EQ(4, eventObserver.sequences.at(0).getSpaces());
}

TEST(YamlLineParser, parseMultipleColonsInLineTest)
{
    std::stringstream input("Time: 2001-11-23 15:01:42 -5  ");
    input >> std::noskipws;

    EventObserver eventObserver;
    YAML::LineParser lineParser(&eventObserver);
    ASSERT_TRUE(lineParser.parse(input));

    ASSERT_EQ(1, eventObserver.events.size());
    ASSERT_EQ("2001-11-23 15:01:42 -5", eventObserver.events["Time"].getValue());
    ASSERT_EQ(0, eventObserver.events["Time"].getSpaces());
}

TEST(YamlLineParser, parseCollectionWithSpaceBeforeColonTest)
{
    std::stringstream input("avg : 0.278 ");
    input >> std::noskipws;

    EventObserver eventObserver;
    YAML::LineParser lineParser(&eventObserver);
    ASSERT_TRUE(lineParser.parse(input));

    ASSERT_EQ(1, eventObserver.events.size());
    ASSERT_EQ("0.278", eventObserver.events["avg"].getValue());
    ASSERT_EQ(0, eventObserver.events["avg"].getSpaces());
}
