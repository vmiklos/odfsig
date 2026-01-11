#!/bin/bash -ex
# Copyright 2018 Miklos Vajna
#
# SPDX-License-Identifier: MIT

cmake_args="-DCMAKE_INSTALL_PREFIX:PATH=$PWD/instdir"
run_clang_tidy=""
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
            ;;
        --asan-ubsan)
	    export ASAN_OPTIONS=detect_stack_use_after_return=1
	    export CC="clang -fsanitize=address -fsanitize=undefined"
	    export CXX="clang++ -fsanitize=address -fsanitize=undefined"
	    export CCACHE_CPP2=YES
            cmake_args+=" -DODFSIG_INTERNAL_XMLSEC=ON"
            ;;
        --fuzz)
	    export ASAN_OPTIONS=detect_stack_use_after_return=1
	    export CC="$HOME/git/llvm/instdir/bin/clang -fsanitize=address -fsanitize=undefined -fsanitize=fuzzer-no-link"
	    export CXX="$HOME/git/llvm/instdir/bin/clang++ -fsanitize=address -fsanitize=undefined -fsanitize=fuzzer-no-link"
	    export CCACHE_CPP2=YES
            cmake_args+=" -DODFSIG_INTERNAL_XMLSEC=ON -DODFSIG_INTERNAL_LIBXML2=ON -DODFSIG_FUZZ=ON"
            ;;
        --tidy)
            export CC=clang
            export CXX=clang++
            run_clang_tidy=run-clang-tidy
            export CCACHE_CPP2=1
            ;;
        *)
            cmake_args+=" $arg"
    esac
done

rm -rf workdir instdir
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
    make -j$(getconf _NPROCESSORS_ONLN) || make
fi
make check
make install
cd ..
ln -s workdir/compile_commands.json .
if [ -n "$run_clang_tidy" ]; then
    # filter for tracked directories, i.e. implicitly filter out workdir
    directories="$(git ls-files|grep /|sed 's|/.*||'|sort -u|xargs echo|sed 's/ /|/g')"
    $run_clang_tidy -header-filter="^$PWD/(${directories})/.*"
fi

# vim:set shiftwidth=4 softtabstop=4 expandtab:
