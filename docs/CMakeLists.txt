# Copyright 2018 Miklos Vajna
#
# SPDX-License-Identifier: MIT

if (PANDOC)
    include(GNUInstallDirs)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/odfsig.1
        COMMAND ${PANDOC} --standalone --to man -o ${CMAKE_CURRENT_BINARY_DIR}/odfsig.1 ${CMAKE_CURRENT_SOURCE_DIR}/odfsig.md
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/odfsig.md
        COMMENT "Building manpage"
        VERBATIM)

    add_custom_target(man ALL DEPENDS odfsig.1)

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/odfsig.1
        DESTINATION ${CMAKE_INSTALL_MANDIR}/man1
        )
endif ()

add_custom_target(run-guide
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/guide
    COMMAND mdbook serve --hostname 127.0.0.1
    )

# vim:set shiftwidth=4 softtabstop=4 expandtab:
