CMAKE_MINIMUM_REQUIRED (VERSION 3.0)
PROJECT (yaml-parser-tests)

SET (SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET (SOURCES
        ${SRC_DIR}/tests.cpp
    )

ADD_EXECUTABLE (yaml-parser-tests ${SOURCES})
TARGET_LINK_LIBRARIES (yaml-parser-tests gtest)
