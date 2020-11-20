#include <gtest/gtest.h>
#include <sstream>

#include "LineParser.h"
#include "AbstractEventObserver.h"

namespace {

class EventObserver : public YAML::AbstractEventObserver {
public:
    class Value {
    public:
        Value(const std::string& value, int spaces)
            : value(value),
              spaces(spaces)
        {
        }

        const std::string& getValue() const {
            return this->value;
        }

        int getSpaces() const {
            return this->spaces;
        }
    private:
        std::string value;
        int spaces;
    };
public:
    void newMapItem(const std::string&, int) override {
    }

    void newScalar(const std::string&) override {
    }

    void newSequenceItem(const std::string& scalar, int spaces) override {
        this->sequences.emplace_back(scalar, spaces);
    }

    std::vector<Value> sequences;
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
