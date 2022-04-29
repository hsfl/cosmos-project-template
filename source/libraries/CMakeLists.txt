file(GLOB directs "./*")

foreach(direct ${directs})
    get_filename_component(bext ${direct} EXT)

    if (${bext} MATCHES "")
    else()
        get_filename_component(bdirect ${direct} NAME_WE)
        add_subdirectory(${PROJECT_SOURCE_DIR}/libraries/${bdirect})
    endif()

endforeach()
