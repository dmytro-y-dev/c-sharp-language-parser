#-----------------------------------------------
#  Source code compilation instructions
#-----------------------------------------------

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

include_directories(source)

FILE(GLOB PROJECT_COMMON_SOURCE_FILES source/*.cpp)

if (BUILD_WITH_WXWIDGETS_GUI)
    add_executable(c-sharp-parser WIN32 ${PROJECT_COMMON_SOURCE_FILES} source/gui/wxWidgets_Application.cpp)
    
    find_package(wxWidgets REQUIRED core base)
    include(${wxWidgets_USE_FILE})
    target_link_libraries(c-sharp-parser ${wxWidgets_LIBRARIES})
else()
    add_executable(c-sharp-parser ${PROJECT_COMMON_SOURCE_FILES} source/console/Console.cpp)
endif()

find_package(ExcelFormat REQUIRED)
include_directories(${EXCELFORMAT_INCLUDE_DIRS})
target_link_libraries(c-sharp-parser ${EXCELFORMAT_LIBRARIES})
