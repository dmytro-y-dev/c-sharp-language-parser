# c-sharp-language-parser

This is `C# language parser`. It is planned to improve it by implementation of semantics analysis.

**Lexical analysis:** Tokens reading is implemented by use of regular expressions. Directive "#if" is implemented using grammar (which is quite slow so I will
change such a way of things later).

**Syntax analysis:** CYK algorithm is used to build parse tree. For more information see <http://web.cs.ucdavis.edu/~rogaway/classes/120/winter12/CYK.pdf>.

Project goes under GPL v3 license. Use it for educational purpose for free.

The analyzer is made as homework during NTUU "KPI"'s "Programming Automatization Systems" course.