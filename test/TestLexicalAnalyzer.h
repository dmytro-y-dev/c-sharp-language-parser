#ifndef TEST_LEXICAL_ANALYZER_H
#define TEST_LEXICAL_ANALYZER_H

#include <cxxtest/TestSuite.h>

#include <LexicalAnalyzer.h>
#include <FileSystemOperations.h>

class TestLexicalAnalyzer : public CxxTest::TestSuite
{
public:
    void testLexicalAnalysisParsingResults(void)
    {
        vector<Lexem> lexems;
        string fixtureTwoSmallClasses = LoadFileContentFromDiskToMemory("../test/fixtures/FixtureTwoSmallClasses.cs");
        
        TS_ASSERT(!fixtureTwoSmallClasses.empty());
        TS_ASSERT(DoLexicalAnalysis(fixtureTwoSmallClasses, lexems));
        
        // WriteLexemsToXLS(lexems, "lexems.xls");
        
        // TODO:
        // Compare values with FixtureTwoSmallClasses-ExpectedResult.xls
    }
};

#endif // TEST_LEXICAL_ANALYZER_H