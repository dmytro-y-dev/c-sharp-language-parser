#-----------------------------------------------
#  Unit-testing instructions
#-----------------------------------------------

find_package(CxxTest)

if (CXXTEST_FOUND)
    include_directories(${CXXTEST_INCLUDE_DIR})
    
    enable_testing()
    
    add_library(LexicalAnalyzerLib STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/LexicalAnalyzer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/FileSystemOperations.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/LexicalRule.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/LexicalDirectiveIf.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Lexem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/LexicalRulesBase.cpp
    )
    
    find_package(ExcelFormat REQUIRED)
    include_directories(${EXCELFORMAT_INCLUDE_DIRS})
    target_link_libraries(LexicalAnalyzerLib ${EXCELFORMAT_LIBRARIES})
    
    CXXTEST_ADD_TEST(TestLexicalAnalyzer TestLexicalAnalyzer.cpp ${CMAKE_CURRENT_SOURCE_DIR}/test/TestLexicalAnalyzer.h)
    target_link_libraries(TestLexicalAnalyzer LexicalAnalyzerLib)
else()
    message(SEND_ERROR "You must have CxxTest on your system to test source code" )
endif()
