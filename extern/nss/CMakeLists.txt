# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set(NSS_VERSION 3.40)
set(NSS_SHA256SUM 8381b57add736985a3846cc34d3cae9b98c46ca504ba65b86ea8178f78c8cb11)

if (ODFSIG_INTERNAL_NSS)
    include(ExternalProject)
    include(GNUInstallDirs)

    if (WIN32)
    elseif (APPLE)
        set(ODFSIG_MACOS_SDK MACOS_SDK_DIR=${CMAKE_OSX_SYSROOT})
        set(ODFSIG_MACOS_NSPR_SUBDIR out/)
    else ()
	# Drop compiler arguments (e.g. sanitizers) on Linux, as NSS build
	# would break with them.
        set(ODFSIG_LINUX_CC CC=${CMAKE_C_COMPILER})
        set(ODFSIG_LINUX_CXX CXX=${CMAKE_CXX_COMPILER})

        set(ODFSIG_LINUX_INSTALL
            COMMAND install -v -m755 dist/out/lib/libfreeblpriv3${CMAKE_SHARED_LIBRARY_SUFFIX} ${CMAKE_BINARY_DIR}/bin/
            COMMAND install -v -m755 dist/out/lib/libsqlite3${CMAKE_SHARED_LIBRARY_SUFFIX} ${CMAKE_BINARY_DIR}/bin/
        )
    endif ()

    # NSS_DISABLE_GTESTS=1 gives us a 2m13s speedup.
    ExternalProject_Add(extern-nss-project
        URL https://ftp.mozilla.org/pub/security/nss/releases/NSS_3_40_RTM/src/nss-${NSS_VERSION}-with-nspr-4.20.tar.gz
        URL_HASH SHA256=${NSS_SHA256SUM}
        DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/extern/tarballs
        BUILD_IN_SOURCE ON
        CONFIGURE_COMMAND
            echo NOP
        BUILD_COMMAND
            $(MAKE) -j1 -C nss nss_build_all BUILD_OPT=1 USE_64=1 OBJDIR_NAME=out NSS_DISABLE_GTESTS=1 ${ODFSIG_LINUX_CC} ${ODFSIG_LINUX_CXX} ${ODFSIG_MACOS_SDK}
        INSTALL_COMMAND
            COMMAND mkdir -p ${CMAKE_BINARY_DIR}/bin/${ODFSIG_MACOS_NSPR_SUBDIR}
            COMMAND install -v -m755 dist/out/lib/libfreebl3${CMAKE_SHARED_LIBRARY_SUFFIX} ${CMAKE_BINARY_DIR}/bin/
            COMMAND install -v -m755 dist/out/lib/libnspr4${CMAKE_SHARED_LIBRARY_SUFFIX} ${CMAKE_BINARY_DIR}/bin/${ODFSIG_MACOS_NSPR_SUBDIR}
            COMMAND install -v -m755 dist/out/lib/libnss3${CMAKE_SHARED_LIBRARY_SUFFIX} ${CMAKE_BINARY_DIR}/bin/
            COMMAND install -v -m755 dist/out/lib/libnssckbi${CMAKE_SHARED_LIBRARY_SUFFIX} ${CMAKE_BINARY_DIR}/bin/
            COMMAND install -v -m755 dist/out/lib/libnssdbm3${CMAKE_SHARED_LIBRARY_SUFFIX} ${CMAKE_BINARY_DIR}/bin/
            COMMAND install -v -m755 dist/out/lib/libnssutil3${CMAKE_SHARED_LIBRARY_SUFFIX} ${CMAKE_BINARY_DIR}/bin/
            COMMAND install -v -m755 dist/out/lib/libplc4${CMAKE_SHARED_LIBRARY_SUFFIX} ${CMAKE_BINARY_DIR}/bin/${ODFSIG_MACOS_NSPR_SUBDIR}
            COMMAND install -v -m755 dist/out/lib/libplds4${CMAKE_SHARED_LIBRARY_SUFFIX} ${CMAKE_BINARY_DIR}/bin/${ODFSIG_MACOS_NSPR_SUBDIR}
            COMMAND install -v -m755 dist/out/lib/libsmime3${CMAKE_SHARED_LIBRARY_SUFFIX} ${CMAKE_BINARY_DIR}/bin/
            COMMAND install -v -m755 dist/out/lib/libsoftokn3${CMAKE_SHARED_LIBRARY_SUFFIX} ${CMAKE_BINARY_DIR}/bin/
            COMMAND install -v -m755 dist/out/lib/libssl3${CMAKE_SHARED_LIBRARY_SUFFIX} ${CMAKE_BINARY_DIR}/bin/
            ${ODFSIG_LINUX_INSTALL}
        )

    # We link to a subset of the above libs, that subset is represented with
    # imported libraries as well.
    add_library(smime3 STATIC IMPORTED)
    set_property(TARGET smime3 PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/bin/libsmime3${CMAKE_SHARED_LIBRARY_SUFFIX})
    add_dependencies(smime3 extern-nss-project)

    add_library(nss3 STATIC IMPORTED)
    set_property(TARGET nss3 PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/bin/libnss3${CMAKE_SHARED_LIBRARY_SUFFIX})
    add_dependencies(nss3 extern-nss-project)

    add_library(nssutil3 STATIC IMPORTED)
    set_property(TARGET nssutil3 PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/bin/libnssutil3${CMAKE_SHARED_LIBRARY_SUFFIX})
    add_dependencies(nssutil3 extern-nss-project)

    add_library(plc4 STATIC IMPORTED)
    set_property(TARGET plc4 PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/bin/${ODFSIG_MACOS_NSPR_SUBDIR}libplc4${CMAKE_SHARED_LIBRARY_SUFFIX})
    add_dependencies(plc4 extern-nss-project)

    add_library(plds4 STATIC IMPORTED)
    set_property(TARGET plds4 PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/bin/${ODFSIG_MACOS_NSPR_SUBDIR}libplds4${CMAKE_SHARED_LIBRARY_SUFFIX})
    add_dependencies(plds4 extern-nss-project)

    add_library(nspr4 STATIC IMPORTED)
    set_property(TARGET nspr4 PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/bin/${ODFSIG_MACOS_NSPR_SUBDIR}libnspr4${CMAKE_SHARED_LIBRARY_SUFFIX})
    add_dependencies(nspr4 extern-nss-project)

    set(NSS_INCLUDE_DIRS
        ${CMAKE_CURRENT_BINARY_DIR}/../nss/extern-nss-project-prefix/src/extern-nss-project/dist/out/include
        ${CMAKE_CURRENT_BINARY_DIR}/../nss/extern-nss-project-prefix/src/extern-nss-project/dist/public/nss
        )
    set(NSS_LIBRARIES
        smime3
        nss3
        nssutil3
        plc4
        plds4
        nspr4
        )
endif ()

add_library(nss INTERFACE)
target_include_directories(nss INTERFACE ${NSS_INCLUDE_DIRS})
target_link_libraries(nss INTERFACE ${NSS_LIBRARIES})

# vim:set shiftwidth=4 softtabstop=4 expandtab:
