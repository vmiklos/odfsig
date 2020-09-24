# Minimal rules to link against NSS from the system + link a self-built pdfium statically.

PDFIUM_CFLAGS := -I${HOME}/git/pdfium/pdfium/public -I${HOME}/git/pdfium/pdfium/ -I/usr/include/freetype2
P := ${HOME}/git/pdfium/pdfium/workdir/debug
LIBCXX = $(wildcard ${P}/obj/buildtools/third_party/libc++/libc++/*.o) $(wildcard ${P}/obj/buildtools/third_party/libc++abi/libc++abi/*.o)
PDFIUM_LIBS := -Wl,--start-group \
	${P}/obj/libpdfium.a \
	${LIBCXX} \
	-Wl,--end-group  -ldl -lrt -lfreetype -pthread

NSS_CFLAGS := $(shell pkg-config --cflags nss)
NSS_LIBS := $(shell pkg-config --libs nss)

pdfiumsig: pdfiumsig.o Makefile
	clang++ -std=c++14 -g -o pdfiumsig pdfiumsig.o ${PDFIUM_LIBS} ${NSS_LIBS}

pdfiumsig.o: pdfiumsig.cpp Makefile
	clang++ -std=c++14 -g -c -o pdfiumsig.o ${PDFIUM_CFLAGS} ${NSS_CFLAGS} pdfiumsig.cpp
