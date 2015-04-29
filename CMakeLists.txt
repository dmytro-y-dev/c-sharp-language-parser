IF(BIICODE)
    include("cmake/Bii.cmake")
    
	return()
endif()

cmake_minimum_required(VERSION 3.1)

project(c-sharp-parser)

include("cmake/Build.cmake")
include("cmake/Test.cmake")
include("cmake/Install.cmake")
