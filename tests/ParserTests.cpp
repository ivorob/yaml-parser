#include <gtest/gtest.h>
#include <sstream>

#include "Parser.h"
#include "AbstractEventObserver.h"

namespace {

class EventObserver : public YAML::AbstractEventObserver {
public:
    void newMapItem(const std::string& name, const std::string& value, int spaces) override {
        this->events[name] = value;
    }

    std::map<std::string, std::string> events;
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
    ASSERT_EQ("65", observer.events["hr"]);
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
    ASSERT_EQ("65", observer.events["hr"]);
    ASSERT_EQ("0.278", observer.events["avg"]);
    ASSERT_EQ("147", observer.events["rbi"]);
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
    ASSERT_EQ("65", observer.events["hr"]);
    ASSERT_EQ("0.278", observer.events["avg"]);
    ASSERT_EQ("147", observer.events["rbi"]);
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
    ASSERT_EQ("65", observer.events["hr"]);
    ASSERT_EQ("0.278", observer.events["avg"]);
    ASSERT_EQ("147", observer.events["rbi"]);
}
