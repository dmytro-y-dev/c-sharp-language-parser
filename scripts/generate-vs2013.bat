cd ..
cmake -H. -Bbuild -G "Visual Studio 12" -DBUILD_WITH_WXWIDGETS_GUI=0 -DWX_ROOT_DIR=E:\Projects\Libs\Trunk\C++\wxWidgets\ -DEXCELFORMAT_ROOT=E:\Projects\Workroom\c-sharp-parser\vendor\ExcelFormat\bin -DCXXTEST_INCLUDE_DIR=E:\Projects\Libs\Trunk\C++\CxxTest\ -DCXXTEST_PYTHON_TESTGEN_EXECUTABLE=E:\Projects\Libs\Trunk\C++\CxxTest\bin\cxxtestgen\
cd scripts