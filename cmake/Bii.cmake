IF(APPLE)
    SET(CMAKE_EXE_LINKER_FLAGS "-framework CoreFoundation")
ENDIF()

ADD_BII_TARGETS()

IF(WIN32 AND MSVC)
    SET_TARGET_PROPERTIES(
        ${BII_source_gui_wxWidgets_Application_TARGET} PROPERTIES
            COMPILE_FLAGS "/D _CRT_SECURE_NO_WARNINGS"
            LINK_FLAGS "/SUBSYSTEM:WINDOWS"
    )
ENDIF()