CMAKE_MINIMUM_REQUIRED (VERSION 3.0)

IF (BUILD_TESTING)
    SET (LOCAL_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})

    SET (BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    SET (gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    ADD_SUBDIRECTORY (googletest)

    SET (BUILD_SHARED_LIBS ${LOCAL_BUILD_SHARED_LIBS} CACHE BOOL "" FORCE)
ENDIF (BUILD_TESTING)
