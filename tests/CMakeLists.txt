CMAKE_MINIMUM_REQUIRED (VERSION 3.0)
PROJECT (yaml-parser-tests)

SET (SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET (MAIN_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src)
SET (SOURCES
        ${SRC_DIR}/tests.cpp
        ${SRC_DIR}/ParserTests.cpp
        ${SRC_DIR}/LineParserTest.cpp
        ${MAIN_SRC_DIR}/Parser.cpp
        ${MAIN_SRC_DIR}/LineParser.cpp
    )

ADD_EXECUTABLE (yaml-parser-tests ${SOURCES})
TARGET_INCLUDE_DIRECTORIES (yaml-parser-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
TARGET_LINK_LIBRARIES (yaml-parser-tests gtest)
