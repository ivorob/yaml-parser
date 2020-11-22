CMAKE_MINIMUM_REQUIRED (VERSION 3.0)
PROJECT (yaml-parser-tests)

SET (SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET (MAIN_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src)
SET (MAIN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include)
SET (SOURCES
        ${SRC_DIR}/tests.cpp
        ${SRC_DIR}/ParserTests.cpp
        ${SRC_DIR}/LineParserTest.cpp
        ${SRC_DIR}/FakeEventObserver.cpp
        ${MAIN_SRC_DIR}/Parser.cpp
        ${MAIN_SRC_DIR}/LineParser.cpp
    )

SET (HEADERS
        ${SRC_DIR}/FakeEventObserver.h
        ${MAIN_INCLUDE_DIR}/Parser.h
        ${MAIN_INCLUDE_DIR}/LineParser.h
    )

ADD_EXECUTABLE (yaml-parser-tests ${SOURCES})
TARGET_INCLUDE_DIRECTORIES (yaml-parser-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
TARGET_LINK_LIBRARIES (yaml-parser-tests gtest)
