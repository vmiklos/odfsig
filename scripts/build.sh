#!/bin/bash -ex
# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_args="-DCMAKE_INSTALL_PREFIX:PATH=$PWD/instdir"
run_clang_tidy=""
run_valgrind=""
run_iwyu=""

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
            cmake_args+=" -DODFSIG_IWYU=ON"
            run_iwyu="y"

            if [ -z "$(type -p include-what-you-use)" ]; then
                CWD=$PWD
                TARNAME="include-what-you-use-0.11.src.tar.gz"
                TARPATH="$CWD/extern/tarballs/$TARNAME"
                if [ ! -e $TARPATH ]; then
                    mkdir -p extern/tarballs
                    wget -O $TARPATH https://include-what-you-use.org/downloads/$TARNAME
                fi
                rm -rf iwyu
                mkdir -p iwyu
                cd iwyu
                tar xvf $TARPATH
                cd include-what-you-use
                mkdir workdir
                cd workdir
                cmake \
                    -DCMAKE_INSTALL_PREFIX=$CWD/iwyu/install \
                    -DCMAKE_BUILD_TYPE=Release \
                    -DCMAKE_PREFIX_PATH=/usr/lib/llvm-7 \
                    -DCMAKE_C_COMPILER="clang-7" \
                    -DCMAKE_CXX_COMPILER="clang++-7" \
                    ..
                make -j$(getconf _NPROCESSORS_ONLN)
                make install
                cd $CWD/iwyu/install
                ln -s /usr/lib .
                export PATH=$PATH:$CWD/iwyu/install/bin
                cd $CWD
            fi
            ;;
        --asan-ubsan)
	    export ASAN_OPTIONS=handle_ioctl=1:detect_leaks=1:allow_user_segv_handler=1:use_sigaltstack=0:detect_deadlocks=1:intercept_tls_get_addr=1:check_initialization_order=1:detect_stack_use_after_return=1:strict_init_order=1:detect_invalid_pointer_pairs=1
	    export LSAN_OPTIONS=print_suppressions=0:report_objects=1
	    export UBSAN_OPTIONS=print_stacktrace=1
	    export CC="clang -fno-sanitize-recover=undefined,integer -fsanitize=address -fsanitize=undefined -fsanitize=local-bounds"
	    export CXX="clang++ -fno-sanitize-recover=undefined,integer -fsanitize-recover=float-divide-by-zero -fsanitize=address -fsanitize=undefined -fsanitize=local-bounds"
	    export CCACHE_CPP2=YES
            cmake_args+=" -DODFSIG_INTERNAL_XMLSEC=ON"
            ;;
        --fuzz)
	    export ASAN_OPTIONS=handle_ioctl=1:detect_leaks=1:allow_user_segv_handler=1:use_sigaltstack=0:detect_deadlocks=1:intercept_tls_get_addr=1:check_initialization_order=1:detect_stack_use_after_return=1:strict_init_order=1:detect_invalid_pointer_pairs=1
	    export LSAN_OPTIONS=print_suppressions=0:report_objects=1
	    export UBSAN_OPTIONS=print_stacktrace=1
	    export CC="$HOME/git/llvm/instdir/bin/clang -fno-sanitize-recover=undefined,integer -fsanitize=address -fsanitize=undefined -fsanitize=local-bounds -fsanitize=fuzzer-no-link"
	    export CXX="$HOME/git/llvm/instdir/bin/clang++ -fno-sanitize-recover=undefined,integer -fsanitize-recover=float-divide-by-zero -fsanitize=address -fsanitize=undefined -fsanitize=local-bounds -fsanitize=fuzzer-no-link"
	    export CCACHE_CPP2=YES
            cmake_args+=" -DODFSIG_INTERNAL_XMLSEC=ON -DODFSIG_INTERNAL_LIBXML2=ON -DODFSIG_FUZZ=ON"
            ;;
        --tidy)
            if [ "$CC" = "gcc-7" ]; then
                export CC=clang-7
                export CXX=clang++-7
                run_clang_tidy=run-clang-tidy-7
            else
                export CC=clang
                export CXX=clang++
                run_clang_tidy=run-clang-tidy
            fi
            export CCACHE_CPP2=1
            ;;
        --valgrind)
            cmake_args+=" -DODFSIG_INTERNAL_XMLSEC=ON"
            run_valgrind="y"
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
if [ -n "$run_iwyu" ]; then
    make -j$(getconf _NPROCESSORS_ONLN) 2>&1 | tee log
    # see
    # <https://stackoverflow.com/questions/15367674/bash-one-liner-to-exit-with-the-opposite-status-of-a-grep-command/31127157#31127157>,
    # simple '!' doesn't work with set -e.
    ! egrep 'should add|should remove' log || false
else
    make -j$(getconf _NPROCESSORS_ONLN)
fi
if [ -n "$run_valgrind" ]; then
    cd ..
    valgrind --leak-check=full --error-exitcode=1 workdir/bin/odfsigtest
    cd -
else
    make check
fi
make install
cd ..
ln -s workdir/compile_commands.json .
if [ -n "$run_clang_tidy" ]; then
    # filter for tracked directories, i.e. implicitly filter out workdir
    directories="$(git ls-files|grep /|sed 's|/.*||'|sort -u|xargs echo|sed 's/ /|/g')"
    $run_clang_tidy -header-filter="^$PWD/(${directories})/.*"
fi

# vim:set shiftwidth=4 softtabstop=4 expandtab:
