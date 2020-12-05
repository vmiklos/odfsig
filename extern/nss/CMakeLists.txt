# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set(NSS_VERSION 3.59)
set(NSS_SHA256SUM 2e2c09c17b1c9f43a2f0a5d83a30a712bff3016d2b7cf5a3dd904847292607ae)

if (NOT ODFSIG_INTERNAL_NSS)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(NSS REQUIRED nss)
else ()
    include(ExternalProject)
    include(GNUInstallDirs)
    find_package(Threads REQUIRED)

    if (WIN32)
    elseif (APPLE)
        set(START_GROUP "")
        set(END_GROUP "")
    else ()
	# Drop compiler arguments (e.g. sanitizers) on Linux, as NSS build
	# would break with them.
        set(ODFSIG_LINUX_CC CC=${CMAKE_C_COMPILER})
        set(ODFSIG_LINUX_CXX CXX=${CMAKE_CXX_COMPILER})

        set(START_GROUP "-Wl,--start-group")
        set(END_GROUP "-Wl,--end-group")
    endif ()
    # Dynamic nss could be avoided with --static.
    set(ODFSIG_NSS_NSPR_INCLUDE dist/Release/include/nspr)

    ExternalProject_Add(extern-nss-project
        URL https://ftp.mozilla.org/pub/security/nss/releases/NSS_3_59_RTM/src/nss-${NSS_VERSION}-with-nspr-4.29.tar.gz
        URL_HASH SHA256=${NSS_SHA256SUM}
        DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/extern/tarballs
        BUILD_IN_SOURCE ON
        CONFIGURE_COMMAND
            echo NOP
        BUILD_COMMAND
	    ${ODFSIG_LINUX_CC} ${ODFSIG_LINUX_CXX} nss/build.sh --opt --static --disable-tests -Duse_system_sqlite=0
        INSTALL_COMMAND
            echo NOP
        )

    ExternalProject_Get_Property(extern-nss-project SOURCE_DIR)

    add_library(nss_static STATIC IMPORTED)
    set_property(TARGET nss_static PROPERTY IMPORTED_LOCATION ${SOURCE_DIR}/dist/Release/lib/libnss_static.a)
    add_dependencies(nss_static extern-nss-project)

    add_library(certdb STATIC IMPORTED)
    set_property(TARGET certdb PROPERTY IMPORTED_LOCATION ${SOURCE_DIR}/dist/Release/lib/libcertdb.a)
    add_dependencies(certdb extern-nss-project)

    add_library(nspr4 STATIC IMPORTED)
    set_property(TARGET nspr4 PROPERTY IMPORTED_LOCATION ${SOURCE_DIR}/nspr/Release/dist/lib/libnspr4.a)
    add_dependencies(nspr4 extern-nss-project)

    add_library(pkcs12 STATIC IMPORTED)
    set_property(TARGET pkcs12 PROPERTY IMPORTED_LOCATION ${SOURCE_DIR}/dist/Release/lib/libpkcs12.a)
    add_dependencies(pkcs12 extern-nss-project)

    add_library(pk11wrap_static STATIC IMPORTED)
    set_property(TARGET pk11wrap_static PROPERTY IMPORTED_LOCATION ${SOURCE_DIR}/dist/Release/lib/libpk11wrap_static.a)
    add_dependencies(pk11wrap_static extern-nss-project)

    add_library(cryptohi STATIC IMPORTED)
    set_property(TARGET cryptohi PROPERTY IMPORTED_LOCATION ${SOURCE_DIR}/dist/Release/lib/libcryptohi.a)
    add_dependencies(cryptohi extern-nss-project)

    add_library(certhi STATIC IMPORTED)
    set_property(TARGET certhi PROPERTY IMPORTED_LOCATION ${SOURCE_DIR}/dist/Release/lib/libcerthi.a)
    add_dependencies(certhi extern-nss-project)

    add_library(nssutil STATIC IMPORTED)
    set_property(TARGET nssutil PROPERTY IMPORTED_LOCATION ${SOURCE_DIR}/dist/Release/lib/libnssutil.a)
    add_dependencies(nssutil extern-nss-project)

    add_library(nssb STATIC IMPORTED)
    set_property(TARGET nssb PROPERTY IMPORTED_LOCATION ${SOURCE_DIR}/dist/Release/lib/libnssb.a)
    add_dependencies(nssb extern-nss-project)

    add_library(nsspki STATIC IMPORTED)
    set_property(TARGET nsspki PROPERTY IMPORTED_LOCATION ${SOURCE_DIR}/dist/Release/lib/libnsspki.a)
    add_dependencies(nsspki extern-nss-project)

    add_library(plc4 STATIC IMPORTED)
    set_property(TARGET plc4 PROPERTY IMPORTED_LOCATION ${SOURCE_DIR}/nspr/Release/dist/lib/libplc4.a)
    add_dependencies(plc4 extern-nss-project)

    add_library(plds4 STATIC IMPORTED)
    set_property(TARGET plds4 PROPERTY IMPORTED_LOCATION ${SOURCE_DIR}/nspr/Release/dist/lib/libplds4.a)
    add_dependencies(plds4 extern-nss-project)

    add_library(nssdev STATIC IMPORTED)
    set_property(TARGET nssdev PROPERTY IMPORTED_LOCATION ${SOURCE_DIR}/dist/Release/lib/libnssdev.a)
    add_dependencies(nssdev extern-nss-project)

    add_library(pkcs7 STATIC IMPORTED)
    set_property(TARGET pkcs7 PROPERTY IMPORTED_LOCATION ${SOURCE_DIR}/dist/Release/lib/libpkcs7.a)
    add_dependencies(pkcs7 extern-nss-project)

    add_library(softokn_static STATIC IMPORTED)
    set_property(TARGET softokn_static PROPERTY IMPORTED_LOCATION ${SOURCE_DIR}/dist/Release/lib/libsoftokn_static.a)
    add_dependencies(softokn_static extern-nss-project)

    add_library(freebl STATIC IMPORTED)
    set_property(TARGET freebl PROPERTY IMPORTED_LOCATION ${SOURCE_DIR}/dist/Release/lib/libfreebl_static.a)
    add_dependencies(freebl extern-nss-project)

    add_library(sqlite STATIC IMPORTED)
    set_property(TARGET sqlite PROPERTY IMPORTED_LOCATION ${SOURCE_DIR}/dist/Release/lib/libsqlite.a)
    add_dependencies(sqlite extern-nss-project)

    add_library(hw-acc-crypto-avx2 STATIC IMPORTED)
    set_property(TARGET hw-acc-crypto-avx2 PROPERTY IMPORTED_LOCATION ${SOURCE_DIR}/dist/Release/lib/libhw-acc-crypto-avx2.a)
    add_dependencies(hw-acc-crypto-avx2 extern-nss-project)

    add_library(hw-acc-crypto-avx STATIC IMPORTED)
    set_property(TARGET hw-acc-crypto-avx PROPERTY IMPORTED_LOCATION ${SOURCE_DIR}/dist/Release/lib/libhw-acc-crypto-avx.a)
    add_dependencies(hw-acc-crypto-avx extern-nss-project)

    add_library(gcm-aes-x86_c_lib STATIC IMPORTED)
    set_property(TARGET gcm-aes-x86_c_lib PROPERTY IMPORTED_LOCATION ${SOURCE_DIR}/dist/Release/lib/libgcm-aes-x86_c_lib.a)
    add_dependencies(gcm-aes-x86_c_lib extern-nss-project)

    set(NSS_INCLUDE_DIRS
        ${SOURCE_DIR}/${ODFSIG_NSS_NSPR_INCLUDE}
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
