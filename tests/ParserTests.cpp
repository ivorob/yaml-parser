#include <gtest/gtest.h>
#include <sstream>

#include "Parser.h"
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

        int getSpaces() const {
            return this->spaces;
        }
    private:
        std::string value;
        int spaces;
    };
public:
    void newMapItem(const std::string& name, const std::string& value, int spaces) override {
        this->events[name] = Value(value, spaces);
    }

    void newSequenceItem(const std::string& value, int spaces) override {
        static int i = 0;
        auto arrayName = "array" + std::to_string(++i);
        events[arrayName] = Value(value, spaces);
    }

    std::map<std::string, Value> events;
};

}

TEST(YamlParser, collectionTest)
{
    std::stringstream input("hr: 65");

    YAML::Parser parser;
    ASSERT_TRUE(parser.parse(input));
}

TEST(YamlParser, collectionEventTest)
{
    std::stringstream input("hr: 65");
    EventObserver observer;

    YAML::Parser parser(&observer);
    ASSERT_TRUE(parser.parse(input));

    ASSERT_EQ(1, observer.events.size());
    ASSERT_EQ("65", observer.events["hr"].getValue());
    ASSERT_EQ(0, observer.events["hr"].getSpaces());
}

TEST(YamlParser, fewCollectionEventsTest)
{
    std::stringstream input("hr: 65\r\n"
                            "avg: 0.278\r\n"
                            "rbi: 147");
    EventObserver observer;

    YAML::Parser parser(&observer);
    ASSERT_TRUE(parser.parse(input));

    ASSERT_EQ(3, observer.events.size());

    ASSERT_EQ("65", observer.events["hr"].getValue());
    ASSERT_EQ(0, observer.events["hr"].getSpaces());

    ASSERT_EQ("0.278", observer.events["avg"].getValue());
    ASSERT_EQ(0, observer.events["avg"].getSpaces());

    ASSERT_EQ("147", observer.events["rbi"].getValue());
    ASSERT_EQ(0, observer.events["rbi"].getSpaces());
}

TEST(YamlParser, fewCollectionEventsWithSpacesTest)
{
    std::stringstream input("hr: 65 \r\n"
                            "avg\t: 0.278  \r\n"
                            "rbi: 147    ");
    EventObserver observer;

    YAML::Parser parser(&observer);
    ASSERT_TRUE(parser.parse(input));

    ASSERT_EQ(3, observer.events.size());

    ASSERT_EQ("65", observer.events["hr"].getValue());
    ASSERT_EQ(0, observer.events["hr"].getSpaces());

    ASSERT_EQ("0.278", observer.events["avg"].getValue());
    ASSERT_EQ(0, observer.events["avg"].getSpaces());

    ASSERT_EQ("147", observer.events["rbi"].getValue());
    ASSERT_EQ(0, observer.events["rbi"].getSpaces());
}

TEST(YamlParser, fewCollectionEventsWithCommentsTest)
{
    std::stringstream input("hr: 65 # Home runs\n"
                            "avg: 0.278 # Batting average\n"
                            "rbi: 147 # Runs Batted In");
    EventObserver observer;

    YAML::Parser parser(&observer);
    ASSERT_TRUE(parser.parse(input));

    ASSERT_EQ(3, observer.events.size());

    ASSERT_EQ("65", observer.events["hr"].getValue());
    ASSERT_EQ(0, observer.events["hr"].getSpaces());

    ASSERT_EQ("0.278", observer.events["avg"].getValue());
    ASSERT_EQ(0, observer.events["avg"].getSpaces());

    ASSERT_EQ("147", observer.events["rbi"].getValue());
    ASSERT_EQ(0, observer.events["rbi"].getSpaces());
}

TEST(YamlParser, collectionEventWithSpacesAtBeginTest)
{
    std::stringstream input("    hr : 65");
    EventObserver observer;

    YAML::Parser parser(&observer);
    ASSERT_TRUE(parser.parse(input));

    ASSERT_EQ(1, observer.events.size());
    ASSERT_EQ("65", observer.events["hr"].getValue());
    ASSERT_EQ(4, observer.events["hr"].getSpaces());
}

TEST(YamlParser, simpleSequencesEventTest)
{
    EventObserver observer;
    YAML::Parser parser(&observer);

    std::stringstream input("- Mark McGwire\r\n"
                            "- Sammy Sosa\r\n"
                            "- Key Griffey");
    ASSERT_TRUE(parser.parse(input));
    ASSERT_EQ(3, observer.events.size());
}
