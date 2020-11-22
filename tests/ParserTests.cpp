#include <gtest/gtest.h>
#include <sstream>

#include "Parser.h"
#include "FakeEventObserver.h"

TEST(YamlParser, collectionTest)
{
    std::stringstream input("hr: 65");

    YAML::Parser parser;
    ASSERT_TRUE(parser.parse(input));
}

TEST(YamlParser, collectionEventTest)
{
    std::stringstream input("hr: 65");
    Fake::EventObserver observer;

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
    Fake::EventObserver observer;

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
                            "avg : 0.278  \r\n"
                            "rbi\t: 147    ");
    Fake::EventObserver observer;

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
    Fake::EventObserver observer;

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
    std::stringstream input("    hr: 65");
    Fake::EventObserver observer;

    YAML::Parser parser(&observer);
    ASSERT_TRUE(parser.parse(input));

    ASSERT_EQ(1, observer.events.size());
    ASSERT_EQ("65", observer.events["hr"].getValue());
    ASSERT_EQ(4, observer.events["hr"].getSpaces());
}

TEST(YamlParser, simpleSequencesEventTest)
{
    Fake::EventObserver observer;
    YAML::Parser parser(&observer);

    std::stringstream input("- Mark McGwire\r\n"
                            "- Sammy Sosa\r\n"
                            "- Key Griffey");
    ASSERT_TRUE(parser.parse(input));
    ASSERT_EQ(3, observer.sequences.size());

    ASSERT_EQ("Mark McGwire", observer.sequences[0].getValue());
    ASSERT_EQ("Sammy Sosa", observer.sequences[1].getValue());
    ASSERT_EQ("Key Griffey", observer.sequences[2].getValue());
}

TEST(YamlParser, parseSequenceOfMappingTest)
{
    Fake::EventObserver observer;
    YAML::Parser parser(&observer);

    std::stringstream input("-\n"
                            "   name: Mark McGwire\n"
                            "   hr: 65\n"
                            "   avg: 0.278\n"
                            "-\n"
                            "   name: Sammy Sosa\n"
                            "   hr: 63\n"
                            "   avg: 0.288");
    ASSERT_TRUE(parser.parse(input));
    ASSERT_EQ(2, observer.sequences.size());
    ASSERT_EQ(3, observer.events.size());
}

TEST(YamlParser, parseCompactSequenceOfMappingTest)
{
    Fake::EventObserver observer;
    YAML::Parser parser(&observer);

    std::stringstream input("# Products purchased\r\n"
                            "- item : Super Hoop\r\n"
                            "  quantity: 1\r\n"
                            "- item : Basketball\r\n"
                            "  quantity: 4\r\n"
                            "- item : Big Shoes\r\n"
                            "  quantity: 1");
    ASSERT_TRUE(parser.parse(input));
    ASSERT_EQ(3, observer.sequences.size());
    ASSERT_EQ(2, observer.events.size());

    ASSERT_EQ("Big Shoes", observer.events["item"].getValue());
    ASSERT_EQ(2, observer.events["item"].getSpaces());

    ASSERT_EQ("1", observer.events["quantity"].getValue());
    ASSERT_EQ(2, observer.events["quantity"].getSpaces());
}
