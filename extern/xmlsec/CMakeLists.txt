# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

if (NOT ODFSIG_INTERNAL_XMLSEC)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(XMLSEC REQUIRED xmlsec1-nss)
else ()
    include(ExternalProject)
    include(GNUInstallDirs)

    if (WIN32)
        set(CONFIGURE_COMMAND cd win32 &&
            cscript /e:javascript configure.js
            crypto=mscng
            xslt=no
            iconv=no
            static=no
            include=${CMAKE_CURRENT_BINARY_DIR}/../libxml2/extern-libxml2-project-prefix/src/extern-libxml2-project/include/
            lib=${CMAKE_CURRENT_BINARY_DIR}/../libxml2/extern-libxml2-project-prefix/src/extern-libxml2-project/win32/bin.msvc/)
        set(BUILD_COMMAND cd win32 &&
            nmake)
        set(INSTALL_COMMAND
            ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/$<CONFIG> &&
            ${CMAKE_COMMAND} -E copy win32/binaries/libxmlsec.dll win32/binaries/libxmlsec-mscng.dll ${CMAKE_BINARY_DIR}/bin/$<CONFIG>/
            )
        set(XMLSEC_INCLUDE_DIR
            ${CMAKE_CURRENT_BINARY_DIR}/extern-xmlsec-project-prefix/src/extern-xmlsec-project/include)
        set(XMLSEC_LIBRARY
            ${CMAKE_CURRENT_BINARY_DIR}/extern-xmlsec-project-prefix/src/extern-xmlsec-project/win32/binaries/libxmlsec.lib)
        set(XMLSEC_CRYPTO_LIBRARY
            ${CMAKE_CURRENT_BINARY_DIR}/extern-xmlsec-project-prefix/src/extern-xmlsec-project/win32/binaries/libxmlsec-mscng.lib)
    else ()
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(NSS REQUIRED nss)
        set (PATCH_COMMAND patch -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/nss-fix-memory-leak-in-GetCertName-220.patch)
        set(CONFIGURE_COMMAND ./configure
            --prefix=${CMAKE_CURRENT_BINARY_DIR}/install
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
        set(INSTALL_COMMAND $(MAKE) install)
        set(XMLSEC_INCLUDE_DIR
            ${CMAKE_CURRENT_BINARY_DIR}/install/include/xmlsec1)
        set(XMLSEC_LIBRARY
            ${CMAKE_CURRENT_BINARY_DIR}/install/${CMAKE_INSTALL_LIBDIR}/libxmlsec1.a)
        set(XMLSEC_CRYPTO_LIBRARY
            ${CMAKE_CURRENT_BINARY_DIR}/install/${CMAKE_INSTALL_LIBDIR}/libxmlsec1-nss.a)
    endif()
    if (NOT ODFSIG_INTERNAL_LIBXML2)
        pkg_check_modules(LIBXML REQUIRED libxml-2.0)
    else ()
        set(LIBXML_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/../libxml2/extern-libxml2-project-prefix/src/extern-libxml2-project/include/)
        set(LIBXML_LIBRARIES libxml2)
    endif ()

    ExternalProject_Add(extern-xmlsec-project
        URL http://www.aleksey.com/xmlsec/download/xmlsec1-1.2.26.tar.gz
        URL_HASH SHA256=8d8276c9c720ca42a3b0023df8b7ae41a2d6c5f9aa8d20ed1672d84cc8982d50
        PATCH_COMMAND ${PATCH_COMMAND}
        DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/extern/tarballs
        BUILD_IN_SOURCE ON
        CONFIGURE_COMMAND ${CONFIGURE_COMMAND}
        BUILD_COMMAND ${BUILD_COMMAND}
        INSTALL_COMMAND ${INSTALL_COMMAND}
        DEPENDS extern-libxml2-project
        )
    add_library(xmlsec1 STATIC IMPORTED)
    set_property(TARGET xmlsec1 PROPERTY IMPORTED_LOCATION ${XMLSEC_LIBRARY})
    add_dependencies(xmlsec1 extern-xmlsec-project)
    add_library(xmlsec1-nss STATIC IMPORTED)
    set_property(TARGET xmlsec1-nss PROPERTY IMPORTED_LOCATION ${XMLSEC_CRYPTO_LIBRARY})
    add_dependencies(xmlsec1-nss extern-xmlsec-project)

    set(XMLSEC_INCLUDE_DIRS
        ${NSS_INCLUDE_DIRS}
        ${LIBXML_INCLUDE_DIRS}
        ${XMLSEC_INCLUDE_DIR}
        )
    # nss after static libs to please --as-needed linkers.
    set(XMLSEC_LIBRARIES
        xmlsec1-nss
        xmlsec1
        ${NSS_LIBRARIES}
        ${LIBXML_LIBRARIES}
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
