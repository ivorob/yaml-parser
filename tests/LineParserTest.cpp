#include <gtest/gtest.h>
#include <sstream>

#include "LineParser.h"

TEST(YamlLineParser, simpleCollectionParserTest)
{
    std::stringstream input("hr: 65");
    YAML::LineParser lineParser;
    ASSERT_TRUE(lineParser.parse(input));
}
