#!/bin/bash -ex
# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_args="-DCMAKE_INSTALL_PREFIX:PATH=$PWD/instdir"
run_clang_tidy=""

for arg in "$@"
do
    case "$arg" in
        --debug)
            cmake_args+=" -DCMAKE_BUILD_TYPE=Debug"
            ;;
        --werror)
            cmake_args+=" -DODFSIG_ENABLE_WERROR=ON"
            ;;
        --internal-libs)
            cmake_args+=" -DODFSIG_INTERNAL_LIBS=ON"
            ;;
        --clang)
            export CC=clang
            export CXX=clang++
            export CCACHE_CPP2=1
            ;;
        --iwyu)
            cmake_args+=" -DCMAKE_CXX_INCLUDE_WHAT_YOU_USE=include-what-you-use"
            ;;
        --sanitizers)
	    export ASAN_OPTIONS=handle_ioctl=1:detect_leaks=1:allow_user_segv_handler=1:use_sigaltstack=0:detect_deadlocks=1:intercept_tls_get_addr=1:check_initialization_order=1:detect_stack_use_after_return=1:strict_init_order=1:detect_invalid_pointer_pairs=1
	    export LSAN_OPTIONS=print_suppressions=0:report_objects=1
	    export UBSAN_OPTIONS=print_stacktrace=1
	    export CC="clang -fno-sanitize-recover=undefined,integer -fsanitize=address -fsanitize=undefined -fsanitize=local-bounds"
	    export CXX="clang++ -fno-sanitize-recover=undefined,integer -fsanitize-recover=float-divide-by-zero -fsanitize=address -fsanitize=undefined -fsanitize=local-bounds"
	    export CCACHE_CPP2=YES
            cmake_args+=" -DODFSIG_INTERNAL_XMLSEC=ON"
            ;;
        --tidy)
            if [ "$CC" != "gcc-7" ]; then
                export CC=clang
                export CXX=clang++
                run_clang_tidy=run-clang-tidy
            else
                export CC=clang-7
                export CXX=clang++-7
                run_clang_tidy=run-clang-tidy-7
            fi
            export CCACHE_CPP2=1
            ;;
        *)
            cmake_args+=" $arg"
    esac
done

rm -rf workdir
rm -f compile_commands.json
mkdir workdir
cd workdir
cmake $cmake_args ..
make -j$(getconf _NPROCESSORS_ONLN)
make check
make install
cd ..
if [ -n "$run_clang_tidy" ]; then
    ln -s workdir/compile_commands.json .
    # filter for tracked directories, i.e. implicitly filter out workdir
    directories="$(git ls-files|grep /|sed 's|/.*||'|sort -u|xargs echo|sed 's/ /|/g')"
    $run_clang_tidy -header-filter="^$PWD/(${directories})/.*"
fi

# vim:set shiftwidth=4 softtabstop=4 expandtab:
