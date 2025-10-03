# Copyright 2018 Miklos Vajna
#
# SPDX-License-Identifier: MIT

set(NSS_VERSION 3.117)
set(NSS_SHA256SUM cda19ca82e3f4839b0cb72fc633545a83b3f73dd3385a24a176860c461773a70)

if (NOT ODFSIG_INTERNAL_NSS)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(NSS REQUIRED nss)
else ()
    include(ExternalProject)
    include(GNUInstallDirs)
    find_package(Threads REQUIRED)

    if (WIN32)
    else ()
        find_program(PYTHON3_COMMAND NAMES python3)
        if(NOT PYTHON3_COMMAND)
            message(FATAL_ERROR "ODFSIG_INTERNAL_NSS is ON but python3 is not found!")
        endif()
        find_program(GYP_COMMAND NAMES gyp)
        if(NOT GYP_COMMAND)
            message(FATAL_ERROR "ODFSIG_INTERNAL_NSS is ON but gyp is not found!")
        endif()
	# Drop compiler arguments (e.g. sanitizers) on Linux, as NSS build
	# would break with them.
        set(ODFSIG_LINUX_CC CC=${CMAKE_C_COMPILER})
        set(ODFSIG_LINUX_CXX CXX=${CMAKE_CXX_COMPILER})

        set(START_GROUP "-Wl,--start-group")
        set(END_GROUP "-Wl,--end-group")
    endif ()

    string(REPLACE "." "_" NSS_URL_VERSION ${NSS_VERSION})
    ExternalProject_Add(extern-nss-project
        URL https://ftp.mozilla.org/pub/security/nss/releases/NSS_${NSS_URL_VERSION}_RTM/src/nss-${NSS_VERSION}-with-nspr-4.37.tar.gz
        URL_HASH SHA256=${NSS_SHA256SUM}
        DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/extern/tarballs
        BUILD_IN_SOURCE ON
        CONFIGURE_COMMAND ""
        BUILD_COMMAND
	    ${ODFSIG_LINUX_CC} ${ODFSIG_LINUX_CXX} nss/build.sh -v --opt --static --disable-tests -Duse_system_sqlite=0 --python=python3
        INSTALL_COMMAND ""
        )

    ExternalProject_Get_Property(extern-nss-project SOURCE_DIR)

    macro(add_nss_library LIBNAME LIBPATH)
        add_library(${LIBNAME} STATIC IMPORTED)
        set_property(TARGET ${LIBNAME} PROPERTY IMPORTED_LOCATION ${SOURCE_DIR}/${LIBPATH})
        add_dependencies(${LIBNAME} extern-nss-project)
    endmacro()

    add_nss_library(nss_static dist/Release/lib/libnss_static.a)
    add_nss_library(certdb /dist/Release/lib/libcertdb.a)
    add_nss_library(nspr4 /nspr/Release/dist/lib/libnspr4.a)
    add_nss_library(pkcs12 /dist/Release/lib/libpkcs12.a)
    add_nss_library(pk11wrap_static /dist/Release/lib/libpk11wrap_static.a)
    add_nss_library(cryptohi /dist/Release/lib/libcryptohi.a)
    add_nss_library(certhi /dist/Release/lib/libcerthi.a)
    add_nss_library(nssutil /dist/Release/lib/libnssutil.a)
    add_nss_library(nssb /dist/Release/lib/libnssb.a)
    add_nss_library(nsspki /dist/Release/lib/libnsspki.a)
    add_nss_library(plc4 /nspr/Release/dist/lib/libplc4.a)
    add_nss_library(plds4 /nspr/Release/dist/lib/libplds4.a)
    add_nss_library(nssdev /dist/Release/lib/libnssdev.a)
    add_nss_library(pkcs7 /dist/Release/lib/libpkcs7.a)
    add_nss_library(softokn_static /dist/Release/lib/libsoftokn_static.a)
    add_nss_library(freebl /dist/Release/lib/libfreebl_static.a)
    add_nss_library(sqlite /dist/Release/lib/libsqlite.a)
    add_nss_library(hw-acc-crypto-avx2 /dist/Release/lib/libhw-acc-crypto-avx2.a)
    add_nss_library(hw-acc-crypto-avx /dist/Release/lib/libhw-acc-crypto-avx.a)
    add_nss_library(gcm-aes-x86_c_lib /dist/Release/lib/libgcm-aes-x86_c_lib.a)
    add_nss_library(sha-x86_c_lib /dist/Release/lib/libsha-x86_c_lib.a)

    set(NSS_INCLUDE_DIRS
        ${SOURCE_DIR}/dist/Release/include/nspr
        ${SOURCE_DIR}/dist/public/nss
        )
    set(NSS_LIBRARIES
        ${START_GROUP}
        certdb
        certhi
        cryptohi
        freebl
        gcm-aes-x86_c_lib
        hw-acc-crypto-avx
        hw-acc-crypto-avx2
        nspr4
        nss_static
        nssb
        nssdev
        nsspki
        nssutil
        pk11wrap_static
        pkcs12
        pkcs7
        plc4
        plds4
        sha-x86_c_lib
        softokn_static
        sqlite
        ${END_GROUP}
        ${CMAKE_THREAD_LIBS_INIT}
        ${CMAKE_DL_LIBS}
        )
endif ()

add_library(nss INTERFACE)
target_include_directories(nss INTERFACE ${NSS_INCLUDE_DIRS})
target_link_libraries(nss INTERFACE ${NSS_LIBRARIES})

# vim:set shiftwidth=4 softtabstop=4 expandtab:
