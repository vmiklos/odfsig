# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set(XMLSEC_VERSION 1.2.31)
set(XMLSEC_SHA256SUM 9b10bc52cc31e4f76162e3975e50db26b71ab49c571d810b311ca626be5a0b26)

if (NOT ODFSIG_INTERNAL_XMLSEC)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(XMLSEC REQUIRED xmlsec1-nss)
else ()
    include(ExternalProject)
    include(GNUInstallDirs)

    if (WIN32)
        ExternalProject_Get_Property(extern-libxml2-project SOURCE_DIR)
        set(CONFIGURE_COMMAND ${CMAKE_COMMAND} -E chdir win32
            cscript /e:javascript configure.js
            crypto=mscng
            xslt=no
            iconv=no
            static=no
            include=${SOURCE_DIR}/include/
            lib=${SOURCE_DIR}/win32/bin.msvc/)
        set(BUILD_COMMAND ${CMAKE_COMMAND} -E chdir win32
            nmake)
        set(INSTALL_COMMAND
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/$<CONFIG>
            COMMAND ${CMAKE_COMMAND} -E copy win32/binaries/libxmlsec.dll win32/binaries/libxmlsec-mscng.dll ${CMAKE_BINARY_DIR}/bin/$<CONFIG>/
            )
    else ()
        if (ODFSIG_INTERNAL_NSS)
            ExternalProject_Get_Property(extern-nss-project SOURCE_DIR)
            if (APPLE)
                set(ODFSIG_XMLSEC_NSS_OBJDIR out)
                set(ODFSIG_XMLSEC_NSS_NSPR_CFLAGS -I${SOURCE_DIR}/dist/${ODFSIG_XMLSEC_NSS_OBJDIR}/include)
            else ()
                set(ODFSIG_XMLSEC_NSS_OBJDIR Release)
                set(ODFSIG_XMLSEC_NSS_NSPR_CFLAGS -I${SOURCE_DIR}/dist/${ODFSIG_XMLSEC_NSS_OBJDIR}/include/nspr)
            endif ()
            set(ODFSIG_INTERNAL_XMLSEC_NSS
                NSPR_CFLAGS=${ODFSIG_XMLSEC_NSS_NSPR_CFLAGS}
                NSPR_LIBS=-L${CMAKE_BINARY_DIR}/bin\ -lnspr4
                NSS_CFLAGS=-I${SOURCE_DIR}/dist/public/nss
                NSS_LIBS=-L${CMAKE_BINARY_DIR}/bin\ -lsmime3\ -lnss3\ -lnssutil3
            )
        endif ()
        set(CONFIGURE_COMMAND ./configure
            ${ODFSIG_INTERNAL_XMLSEC_NSS}
            --prefix=<INSTALL_DIR>
            --with-pic
            --disable-shared
            --disable-crypto-dl
            --without-libxslt
            --without-gnutls
            --without-gcrypt
            --disable-apps
            --disable-docs
            --without-openssl)
        set(BUILD_COMMAND $(MAKE))
        set(INSTALL_COMMAND COMMAND $(MAKE) install)
    endif()

    if (ODFSIG_INTERNAL_LIBXML2)
        set(ODFSIG_LIBXML2_PROJECT extern-libxml2-project)
    endif ()
    if (ODFSIG_INTERNAL_NSS)
        set(ODFSIG_NSS_PROJECT extern-nss-project)
        set(NSS_LIBRARIES nss)
    endif ()
    ExternalProject_Add(extern-xmlsec-project
        URL http://www.aleksey.com/xmlsec/download/xmlsec1-${XMLSEC_VERSION}.tar.gz
        URL_HASH SHA256=${XMLSEC_SHA256SUM}
        DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/extern/tarballs
        BUILD_IN_SOURCE ON
        CONFIGURE_COMMAND ${CONFIGURE_COMMAND}
        BUILD_COMMAND ${BUILD_COMMAND}
        INSTALL_COMMAND ${INSTALL_COMMAND}
        DEPENDS ${ODFSIG_LIBXML2_PROJECT} ${ODFSIG_NSS_PROJECT}
        )
    ExternalProject_Get_Property(extern-xmlsec-project INSTALL_DIR)

    if (WIN32)
        ExternalProject_Get_Property(extern-xmlsec-project SOURCE_DIR)
        set(XMLSEC_INCLUDE_DIRS ${SOURCE_DIR}/include)
        set(XMLSEC_LIBRARY ${SOURCE_DIR}/win32/binaries/libxmlsec.lib)
        set(XMLSEC_CRYPTO_LIBRARY ${SOURCE_DIR}/win32/binaries/libxmlsec-mscng.lib)

        install(PROGRAMS
            ${CMAKE_BINARY_DIR}/bin/$<CONFIG>/libxmlsec.dll
            ${CMAKE_BINARY_DIR}/bin/$<CONFIG>/libxmlsec-mscng.dll
            DESTINATION ${CMAKE_INSTALL_BINDIR})
    else ()
        set(XMLSEC_INCLUDE_DIRS ${INSTALL_DIR}/include/xmlsec1)
        set(XMLSEC_LIBRARY ${INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}/libxmlsec1.a)
        set(XMLSEC_CRYPTO_LIBRARY ${INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}/libxmlsec1-nss.a)
    endif ()

    add_library(xmlsec1 STATIC IMPORTED)
    set_property(TARGET xmlsec1 PROPERTY IMPORTED_LOCATION ${XMLSEC_LIBRARY})
    add_dependencies(xmlsec1 extern-xmlsec-project)
    add_library(xmlsec1-crypto STATIC IMPORTED)
    set_property(TARGET xmlsec1-crypto PROPERTY IMPORTED_LOCATION ${XMLSEC_CRYPTO_LIBRARY})
    add_dependencies(xmlsec1-crypto extern-xmlsec-project)

    # nss after static libs to please --as-needed linkers.
    set(XMLSEC_LIBRARIES
        xmlsec1-crypto
        xmlsec1
        libxml2
        ${NSS_LIBRARIES}
        )
    set(XMLSEC_CFLAGS_OTHER
        -DXMLSEC_NO_SIZE_T
        -DXMLSEC_NO_XSLT
        )
endif ()

add_library(libxmlsec INTERFACE)
target_include_directories(libxmlsec INTERFACE ${XMLSEC_INCLUDE_DIRS})
target_link_libraries(libxmlsec INTERFACE ${XMLSEC_LIBRARIES})
target_compile_options(libxmlsec INTERFACE ${XMLSEC_CFLAGS_OTHER})

# vim:set shiftwidth=4 softtabstop=4 expandtab:
