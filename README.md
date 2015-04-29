C# Language Parser
==================

*Author: metamaker*

This is `C# language parser`. It is planned to improve it by implementation of semantics analysis.

**Lexical analysis:** Tokens reading is implemented by use of regular expressions. Directive "#if" is implemented using grammar (which is quite slow so I will
change such a way of things in future).

**Syntax analysis:** Application uses CYK algorithm to build parse tree. For more information see <http://web.cs.ucdavis.edu/~rogaway/classes/120/winter12/CYK.pdf>.

Project goes under GPL v3 license. Use it for educational purpose freely.

How to build
------------

You can use CMake (<http://www.cmake.org/>) to build this project. The steps are next:

1. Open command line and change current directory with `$ cd` command to the directory where you unpacked the project's source code.
2. Run `$ cmake -H. -Bbuild -G "Visual Studio 12"` to create Makefile for Visual Studio 2013 (or use other generator if you use other IDE). Don't forget to specify path to dependency libraries and headers with -D parameter.
3. Open `build/C-Sharp-Parser.sln` and build solution in Visual Studio.
4. Optionally, you can install this utility by building project which name is `INSTALL` in Visual Studio. Remember that you must have enough rights on your system to let VS copy files to installation folder.

Another option is to use bii (<http://www.biicode.com>). Building commands are next:

1. `$ cd` to the project's folder.
2. Run `$ bii init -L`.
3. Run `$ bii find`.
4. Run `$ bii cpp:configure`. You can specify your CMake generator with `-G` flag like `$ bii cpp:configure -G "Visual Studio 12"`.
5. Run `$ bii cpp:build` to build the project.

Dependencies
------------

Dependencies are automatically integrated into project when you use bii as build tool. If you prefer CMake you must compile dependencies and specify paths to them. The libraries are next:

- ExcelFormat: <http://www.biicode.com/metamaker/ExcelFormat>
- wxWidgets: <https://www.biicode.com/fenix/wxwidgets>
- cxxtest: <https://www.biicode.com/DevOmem/cxxtest>

License agreement
-----------------

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3 as published
by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>

Release history
---------------

29.04.2015 - Version 0.2
- Biicode support

28.04.2015 - Version 0.1
- First release
